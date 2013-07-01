

import piw
from pi import agent, domain, bundles,atom
from . import osc_input_version as version

from .oscpad_plg_native import osc_input

OUT_LIGHT=1

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='osc pad', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(OUT_LIGHT, False, names='light output', protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1][1])
        self.osc = osc_input(self.domain, self.output.cookie(),"localhost", "9000","9001")


        self.key_input = bundles.VectorInput(self.osc.cookie(), self.domain, signals=(1,2,3,4))
        self[2] = atom.Atom(names="inputs")
        self[2][1] = atom.Atom(domain=domain.Aniso(), policy=self.key_input.vector_policy(1,False), names='key input')
        self[2][2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.key_input.vector_policy(2,False), names='pressure input')
        self[2][3] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(3,False), names='roll input')
        self[2][4] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(4,False), names='yaw input')


#
# Define Agent as this agents top level class
#
agent.main(Agent)
