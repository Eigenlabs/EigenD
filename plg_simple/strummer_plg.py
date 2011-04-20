
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

from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node
from plg_simple import strummer_version as version
import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        agent.Agent.__init__(self, signature=version, names='strummer',protocols='', ordinal=ordinal)
        
        self[1] = atom.Atom()
        self[1][1] = bundles.Output(1,False,names='pressure output', protocols='')

        self.output = bundles.Splitter(self.domain,*self[1].values())
        self.strummer = piw.strummer(self.output.cookie(),self.domain)
        self.ctl_input = bundles.VectorInput(self.strummer.ctl_cookie(),self.domain,signals=(1,))
        self.data_input = bundles.VectorInput(self.strummer.data_cookie(),self.domain,signals=(1,))

        self[4]=atom.Atom()
        self[4][1]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.ctl_input.vector_policy(1,False),names='breath input')
        self[4][2]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.data_input.vector_policy(1,False),names='pressure input')

        self[2]=atom.Atom()
        self[2][1] = atom.Atom(domain=domain.Bool(),init=False,names='enable',policy=atom.default_policy(self.strummer.enable))
        self[2][2] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.01,names='on threshold',policy=atom.default_policy(self.strummer.set_on_threshold))
        self[2][3] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.08,names='off threshold',policy=atom.default_policy(self.strummer.set_off_threshold))
        self[2][4] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.0,names='k mix',policy=atom.default_policy(self.strummer.set_key_mix))
        self[2][5] = atom.Atom(domain=domain.BoundedInt(0,2000),init=400,names='trigger window',policy=atom.default_policy(self.strummer.set_trigger_window))

        self.strummer.enable(False)
        self.strummer.set_on_threshold(0.01)
        self.strummer.set_off_threshold(0.08)
        self.strummer.set_key_mix(0.0)
        self.strummer.set_trigger_window(400)

class Upgrader(upgrade.Upgrader):
    pass

agent.main(Agent,Upgrader)
