#
# Copyright 2011 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

import piw
from pi import agent, domain, bundles, atom
from plg_primitive import latch_version as version

from primitive_plg_native import latch

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='latch', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self[2] = atom.Atom()
        self[2][1] = bundles.Output(1, False, names='activation output')
        self[2][2] = bundles.Output(2, False, names='pressure output')
        self[2][3] = bundles.Output(3, False, names='roll output')
        self[2][4] = bundles.Output(4, False, names='yaw output')

        self.output = bundles.Splitter(self.domain,*self[2].values())
        self.latch = latch(self.domain, self.output.cookie())

        self.key_input = bundles.VectorInput(self.latch.cookie(), self.domain, signals=(1,2,3,4))

        self[1] = atom.Atom(names="inputs")

        self[1][1] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.key_input.vector_policy(1,False), names='activation input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.key_input.vector_policy(2,False), names='pressure input')
        self[1][3] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(3,False), names='roll input')
        self[1][4] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(4,False), names='yaw input')

        self[3] = atom.Atom(domain=domain.BoundedFloat(0,1), init=0.5, policy=atom.default_policy(self.__minimum), names='minimum')
        self[4] = atom.Atom(domain=domain.BoundedInt(2,4), init=2, policy=atom.default_policy(self.__controller), names='controller')

    def __minimum(self,m):
        self.latch.set_minimum(m)
        return True

    def __controller(self,c):
        self.latch.set_controller(c)
        return True

agent.main(Agent)
