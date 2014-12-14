import piw
from pi import agent, domain, bundles, atom, action, utils
from pi.logic.shortcuts import T
from . import t3d_device_version as version

from .t3d_plg_native import t3d_device

OUT_KEY=1
OUT_STRIP_1=2
OUT_STRIP_2=3
OUT_BREATH=4
OUT_PEDAL_1=5
OUT_PEDAL_2=6
OUT_PEDAL_3=7
OUT_PEDAL_4=8

IN_LIGHT=1


# t3d device agent - agent to connect devices which broadcast t3d protocol over OSC
class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='t3d device', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,False,names='key output')
        self[1][2] = bundles.Output(2,False,names='pressure output')
        self[1][3] = bundles.Output(3,False,names='roll output')
        self[1][4] = bundles.Output(4,False,names='yaw output')
        self[1][5] = bundles.Output(5,False,names='frequency')

        self[1][6] = bundles.Output(1,False,names='breath output')

        self[1][7] = bundles.Output(1,False,names='strip position output',ordinal=1)
        self[1][8] = bundles.Output(2,False,names='absolute strip output',ordinal=1)
        self[1][9] = bundles.Output(1,False,names='strip position output',ordinal=2)
        self[1][10] = bundles.Output(2,False,names='absolute strip output',ordinal=2)

        self[1][11] = bundles.Output(1,False,names='pedal output',ordinal=1)
        self[1][12] = bundles.Output(1,False,names='pedal output',ordinal=2)
        self[1][13] = bundles.Output(1,False,names='pedal output',ordinal=3)
        self[1][14] = bundles.Output(1,False,names='pedal output',ordinal=4)
 
		# key outputs
        self.koutput = bundles.Splitter(self.domain,self[1][1],self[1][2],self[1][3],self[1][4],self[1][5])
        self.kpoly = piw.polyctl(10,self.koutput.cookie(),False,6)
		# breath
        self.boutput = bundles.Splitter(self.domain,self[1][6])
		# strips
        self.s1output = bundles.Splitter(self.domain,self[1][7],self[1][8])
        self.s2output = bundles.Splitter(self.domain,self[1][9],self[1][10])
		# pedals
        self.poutput1 = bundles.Splitter(self.domain,self[1][11])
        self.poutput2 = bundles.Splitter(self.domain,self[1][12])
        self.poutput3 = bundles.Splitter(self.domain,self[1][13])
        self.poutput4 = bundles.Splitter(self.domain,self[1][14])

		# now we need to set of outputs
        self.output=piw.sclone()
        self.output.set_filtered_output(OUT_KEY,self.kpoly.cookie(),piw.first_filter(OUT_KEY))
        self.output.set_filtered_output(OUT_STRIP_1,self.s1output.cookie(),piw.first_filter(OUT_STRIP_1))
        self.output.set_filtered_output(OUT_STRIP_2,self.s2output.cookie(),piw.first_filter(OUT_STRIP_2))
        self.output.set_filtered_output(OUT_BREATH,self.boutput.cookie(),piw.first_filter(OUT_BREATH)) 
        self.output.set_filtered_output(OUT_PEDAL_1,self.poutput1.cookie(),piw.first_filter(OUT_PEDAL_1))
        self.output.set_filtered_output(OUT_PEDAL_2,self.poutput2.cookie(),piw.first_filter(OUT_PEDAL_2))
        self.output.set_filtered_output(OUT_PEDAL_3,self.poutput3.cookie(),piw.first_filter(OUT_PEDAL_4))
        self.output.set_filtered_output(OUT_PEDAL_4,self.poutput4.cookie(),piw.first_filter(OUT_PEDAL_4))

        
        self.device = t3d_device(self.domain, self.output.cookie())

        self[1][15] = atom.Atom(names='controller output',domain=domain.Aniso(),init=self.controllerinit())
                
        self[3] = atom.Atom(domain=domain.BoundedInt(1,30),init=30,policy=atom.default_policy(self.__row_size),names='row size')
        self[4] = atom.Atom(domain=domain.BoundedInt(1,5),init=5,policy=atom.default_policy(self.__col_size),names='col size')
        self[5] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__whole_roll),names='whole roll')
        self[6] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__whole_yaw),names='whole yaw')
        self[7] = atom.Atom(domain=domain.BoundedInt(0,9999),init=3123,policy=atom.default_policy(self.__server_port),names='server port')

        self.input = bundles.VectorInput(self.device.cookie(), self.domain, signals=(IN_LIGHT,))
        self[8] = atom.Atom(domain=domain.BoundedInt(0,9999),init=0,policy=atom.default_policy(self.__light_port),names='light port')
        self[9] = atom.Atom(domain=domain.Aniso(), protocols='revconnect', policy=self.input.vector_policy(IN_LIGHT,False,clocked=False), names="light input")
        self[10] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__continuous_key),names='continuous key')
        self[11] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__touch_mode),names='touch mode')
         
        print "connect ", self[7], "," , self[8]
        self.device.connect(self[7].get_value(),self[8].get_value())


    def close_server(self):
        agent.Agent.close_server(self)
    	self.device.stop()
        
    def controllerinit(self):
    	scale=piw.makestring('[0,1,2,3,4,5,6,7,8,9,10,11,12]',0)
    	octave=piw.makefloat_bounded(9,-1,0,-1,0)
        dict=utils.makedict({'columnlen':self.device.get_columnlen(),'columnoffset':self.device.get_columnoffset(),'courselen':self.device.get_courselen(),'courseoffset':self.device.get_courseoffset(),'octave':octave,'scale':scale},0)
        return dict

    def __row_size(self,value):
        self[3].set_value(value)
        self.device.row_size(value)
        self[1][15].set_value(self.controllerinit());
        return True

    def __col_size(self,value):
        self[4].set_value(value)
        self.device.col_size(value)
        self[1][15].set_value(self.controllerinit());
        return True

    def __whole_roll(self,value):
        self[5].set_value(value)
        self.device.whole_roll(value)
        return True

    def __whole_yaw(self,value):
        self[6].set_value(value)
        self.device.whole_yaw(value)
        return True

    def __server_port(self,value):
        self[7].set_value(value)
        self.device.connect(self[7].get_value(),self[8].get_value())
        return True

    def __light_port(self,value):
        self[8].set_value(value)
        self.device.connect(self[7].get_value(),self[8].get_value())
        return True

    def __continuous_key(self,value):
    	if value is True and self[11].get_value() is False:
    		self[11].set_value(true)
    		
        self[10].set_value(value)
        self.device.continuous_key(value)
        return True

    def __touch_mode(self,value):
        self[11].set_value(value)
        self.device.touch_mode(value)
        return True

#
# Define Agent as this agents top level class
#
agent.main(Agent)
