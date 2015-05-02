import piw
from pi import action,agent,atom,bundles,domain,errors,logic
from . import midi_program_chooser_version as version,midi_native

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='midi program chooser', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1, False, names='light output', protocols='revconnect')
        self[1][2] = bundles.Output(2, False, names='midi output')
        self.output = bundles.Splitter(self.domain, self[1][1], self[1][2])

        self.chooser = midi_native.midi_pgm_chooser(self.output.cookie(), self.domain)

        self.input = bundles.VectorInput(self.chooser.cookie(), self.domain, signals=(1,2,))

        self[2] = atom.Atom(names='inputs')
        self[2][1] = atom.Atom(domain=domain.Aniso(), policy=self.input.vector_policy(1,False), names='key input')
        self[2][2] = atom.Atom(domain=domain.Aniso(),policy=self.input.vector_policy(2,False),names='controller input')

        self[3] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__bankview),names='bank view')
        self[4] = atom.Atom(domain=domain.BoundedInt(0,127),init=0,policy=atom.default_policy(self.__program),names='program')
        self[5] = atom.Atom(domain=domain.BoundedInt(0,16),init=1,policy=atom.default_policy(self.__channel),names='midi channel')
        self[6] = atom.Atom(domain=domain.BoundedInt(0,127),init=0,policy=atom.default_policy(self.__window),names='window')
        self[7] = atom.Atom(domain=domain.BoundedInt(0,127),init=0,policy=atom.default_policy(self.__bank),names='bank')
        
        self.add_verb2(1,'reset([],None)',callback=self.__reset)
        self.add_verb2(2,'up([],None)',callback=self.__up)
        self.add_verb2(3,'down([],None)',callback=self.__down)

    def __reset(self,*arg):
        self.chooser.reset()

    def __up(self,*arg):
        self.chooser.up()

    def __down(self,*arg):
        self.chooser.down()

    def __bankview(self,value):
        self[3].set_value(value)
        self.chooser.bank_mode(self[3].get_value())
        return True

    def __program(self,value):
        self[4].set_value(value)
        self.chooser.program(self[4].get_value())
        return True

    def __channel(self,value):
        self[5].set_value(value)
        self.chooser.channel(self[5].get_value())
        return True

    def __window(self,value):
        self[6].set_value(value)
        self.chooser.window(self[6].get_value())
        return True

    def __bank(self,value):
        self[7].set_value(value)
        self.chooser.bank(self[7].get_value())
        return True


agent.main(Agent)

