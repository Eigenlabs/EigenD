import piw
from pi import action,agent,atom,bundles,domain,errors,logic
from . import midi_pgm_chooser_version as version,midi_native

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

        self[3] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__bank),names='bank')
        self[4] = atom.Atom(domain=domain.BoundedInt(0,127),init=0,policy=atom.default_policy(self.__current),names='current')
        
        self.add_verb2(1,'reset([],None)',callback=self.__reset)

    def __reset(self,*arg):
        self.chooser.reset()

    def __bank(self,value):
        self[3].set_value(value)
        self.chooser.bank_mode(self[3].get_value())
        return True

    def __current(self,value):
        self[4].set_value(value)
        self.chooser.current(self[4].get_value())
        return True

agent.main(Agent)

