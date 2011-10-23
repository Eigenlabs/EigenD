
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
from plg_simple import ranger_version as version

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='ranger',ordinal=ordinal)

        self[2] = atom.Atom(names='outputs')
        self[2][1] = bundles.Output(1,False,names='output')

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.output = bundles.Splitter(self.domain,*self[2].values())
        self.ranger = piw.ranger(self.domain,self.output.cookie())

        self.ctl_input = bundles.ScalarInput(self.ranger.ctl_cookie(), self.domain,signals=(1,2,3))
        self.data_input = bundles.VectorInput(self.ranger.data_cookie(), self.domain,signals=(1,))

        self[1] = atom.Atom(names='controls')
        self[1][1]=atom.Atom(domain=domain.BoundedFloat(-10000000,10000000),init=-1,policy=self.ctl_input.policy(1,False),names='minimum')
        self[1][2]=atom.Atom(domain=domain.BoundedFloat(-10000000,10000000),init=1,policy=self.ctl_input.policy(2,False),names='maximum')
        self[1][3]=atom.Atom(domain=domain.BoundedFloat(-10000000,10000000),init=0,policy=self.ctl_input.policy(3,False),names='rest')
        self[1][4]=atom.Atom(domain=domain.BoundedFloat(-1,1),init=0,policy=self.data_input.vector_policy(1,False),names='input')

        self[3]=atom.Atom(domain=domain.Bool(),policy=atom.default_policy(self.__setsticky),names='sticky')
        self[4]=atom.Atom(domain=domain.BoundedFloat(-10,10),policy=atom.default_policy(self.__setcurve),names='curve')
        self[5]=atom.Atom(domain=domain.Bool(),policy=atom.default_policy(self.__setmono),names='mono')
        self[6]=atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__setabsolute),names='absolute')

        self.ranger.set_absolute(True)
        self.add_verb2(1,'reset([],None)',self.__reset)

    def __setabsolute(self,s):
        self.ranger.set_absolute(s)
        return True

    def __setmono(self,s):
        self.ranger.set_mono(s)
        return True

    def __reset(self,subj):
        self.ranger.reset()

    def __setsticky(self,s):
        self.ranger.set_sticky(s)
        return True

    def __setcurve(self,c):
        self.ranger.set_curve(c)
        return True


agent.main(Agent)
