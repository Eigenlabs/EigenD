
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
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
from pi import const,agent,atom,bundles,domain,upgrade,paths
from plg_simple import channeliser_version as version

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='channeliser',ordinal=ordinal)

        self[2] = atom.Atom()
        self[2][1] = bundles.Output(1,False,names='activation output')
        self[2][2] = bundles.Output(2,False,names='pressure output')
        self[2][3] = bundles.Output(3,False,names='roll output')
        self[2][4] = bundles.Output(4,False,names='yaw output')
        self[2][5] = bundles.Output(5,False,names='scale note output')
        self[2][6] = bundles.Output(6,False,names='frequency output')

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.output = bundles.Splitter(self.domain,*self[2].values())
        self.poly = piw.polyctl(32,self.output.cookie(),False,0)
        self.input = bundles.VectorInput(self.poly.cookie(), self.domain,signals=(1,2,3,4,5,6))
        #self.input = bundles.VectorInput(self.output.cookie(), self.domain,signals=(1,2,3,4,5,6))

        self[1] = atom.Atom()
        self[1][1]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(1,False),names='activation input')
        self[1][2]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(2,False),names='pressure input')
        self[1][3]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(3,False),names='roll input')
        self[1][4]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(4,False),names='yaw input')
        self[1][5]=atom.Atom(domain=domain.BoundedFloat(0,1000),policy=self.input.vector_policy(5,False),names='scale note input')
        self[1][6]=atom.Atom(domain=domain.BoundedFloat(0,96000),policy=self.input.vector_policy(6,False),names='frequency input')


agent.main(Agent)
