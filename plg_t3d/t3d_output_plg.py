import piw
from pi import agent, domain, bundles,atom
from . import t3d_output_version as version

from .t3d_plg_native import t3d_output

# same as in cpp

IN_KEY=1
IN_PRESSURE=2
IN_ROLL=3
IN_YAW=4
IN_FREQ=5

IN_CONTROL=1


# t3d_output output agent - T3D is touch protocol for 3d keys over OSC
class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='t3d output', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self.t3d_output = t3d_output(self.domain, "localhost", 3123);
        self.output = self.t3d_output.create_output("", True, 0)
        self.input = bundles.VectorInput(self.output, self.domain, signals=(IN_KEY,IN_PRESSURE,IN_ROLL,IN_YAW,IN_FREQ))

		# related inputs for a key
        self[1] = atom.Atom(names="inputs")
        self[1][1] = atom.Atom(domain=domain.Aniso(), policy=self.input.vector_policy(IN_KEY,False), names='key input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.input.vector_policy(IN_PRESSURE,False), names='pressure input')
        self[1][3] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.input.vector_policy(IN_ROLL,False), names='roll input')
        self[1][4] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.input.vector_policy(IN_YAW,False), names='yaw input')

        self[1][5] = atom.Atom(names='frequency input', domain=domain.BoundedFloat(1,96000), policy=self.input.vector_policy(IN_FREQ,False))


		# breath output
        self.breath_output = self.t3d_output.create_output("breath",False,1)
        self.breath_input = bundles.VectorInput(self.breath_output, self.domain,signals=(1,))
        self[1][6] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.breath_input.vector_policy(1,False), names='breath input')

        # outputs for strips
        self.absstrip1_output = self.t3d_output.create_output("strip1",False,4)
        self.absstrip1_input = bundles.VectorInput(self.absstrip1_output, self.domain,signals=(1,))
        self[1][7] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.absstrip1_input.vector_policy(1,False), names='absolute strip input', ordinal=1)
        self.absstrip2_output = self.t3d_output.create_output("strip2",False,5)
        self.absstrip2_input = bundles.VectorInput(self.absstrip2_output, self.domain,signals=(1,))
        self[1][8] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.absstrip2_input.vector_policy(1,False), names='absolute strip input', ordinal=2)
        self.strippos1_output = self.t3d_output.create_output("d_strip1",False,2)
        self.strippos1_input = bundles.VectorInput(self.strippos1_output, self.domain,signals=(1,))
        self[1][9] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.strippos1_input.vector_policy(1,False), names='strip position input', ordinal=1)
        self.strippos2_output = self.t3d_output.create_output("d_strip2",False,3)
        self.strippos2_input = bundles.VectorInput(self.strippos2_output, self.domain,signals=(1,))
        self[1][10] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.strippos2_input.vector_policy(1,False), names='strip position input', ordinal=2)
        
        self[2] = atom.Atom(names="host")
        self[2][1] = atom.Atom(domain=domain.String(), init='localhost', names='name', policy=atom.default_policy(self.__set_host));
        self[2][2] = atom.Atom(domain=domain.BoundedInt(1,9999), init=3123, names='port', policy=atom.default_policy(self.__set_port), )

        self[3] = atom.Atom(domain=domain.BoundedInt(1,1000), init=250, policy=atom.default_policy(self.__set_data_freq), names='data frequency')

        self[5] = atom.Atom(domain=domain.BoundedInt(1,16), init=16, policy=atom.default_policy(self.__set_max_voice_count), names='max voice count')
        self[6] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_kyma_mode),names='kyma')
   
        self.ctlr_fb = piw.functor_backend(1,True)
        self.ctlr_fb.set_functor(piw.pathnull(0),self.t3d_output.control())
        self.ctlr_input = bundles.ScalarInput(self.ctlr_fb.cookie(),self.domain,signals=(IN_CONTROL,))
        self[7] = atom.Atom(domain=domain.Aniso(),policy=self.ctlr_input.policy(IN_CONTROL,False),names='controller input')
 
        self[8] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_continuous_mode),names='continuous')
  
        print "connect ", self[2][1].get_value(), " port ", self[2][2].get_value()
        self.t3d_output.connect(self[2][1].get_value(), self[2][2].get_value())
        print "connect b ", self[2][1].get_value(), " port ", self[2][2].get_value()
  
    def close_server(self):
        agent.Agent.close_server(self)
    	self.t3d_output.stop()

    def __set_data_freq(self,value):
        self[3].set_value(value)
        self.t3d_output.set_data_freq(value)
        return True

    def __set_max_voice_count(self,value):
        self[5].set_value(value)
        self.t3d_output.set_max_voice_count(value)
        return True

    def __set_kyma_mode(self,value):
        self[6].set_value(value)
        self.t3d_output.set_kyma_mode(value)
        return True

    def __set_continuous_mode(self,value):
        self[8].set_value(value)
        self.t3d_output.set_continuous_mode(value)
        return True


    def __set_host(self,value):
        self[2][1].set_value(value)
        self.t3d_output.connect(self[2][1].get_value(), self[2][2].get_value())
        return True

    def __set_port(self,value):
        self[2][2].set_value(value)
        self.t3d_output.connect(self[2][1].get_value(), self[2][2].get_value())
        return True

#
# Define Agent as this agents top level class
#
agent.main(Agent)
