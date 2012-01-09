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
from . import orb_version as version
from .primitive_plg_native import orb

#
# The Orb agent.
#
# Refer to the latch_plg.py agent for documentation, nothing new is covered here.
#
class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='orb', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self[2] = atom.Atom(names="outputs")
        self[2][1] = bundles.Output(1, False, names='angle output')
        self[2][2] = bundles.Output(2, False, names='radius output')

        self.output = bundles.Splitter(self.domain,*self[2].values())
        self.native = orb(self.domain, self.output.cookie())

        self.key_input = bundles.VectorInput(self.native.cookie(), self.domain, signals=(1,2))

        self[1] = atom.Atom(names="inputs")

        self[1][1] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(1,False), names='roll input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(2,False), names='yaw input')

agent.main(Agent)
