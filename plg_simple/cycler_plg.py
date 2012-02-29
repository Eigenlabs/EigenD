
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
from pi import const,agent,atom,bundles,domain,upgrade,paths,utils,node
from . import cycler_version as version

from pi.logic.shortcuts import T

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='cycler',container=10,ordinal=ordinal)

        self.set_private(node.Server())
        self.get_private()[1]=node.Server(change=self.__cycle_set,value=piw.makebool(True,0))
        self.get_private()[2]=node.Server(change=self.__invert_set,value=piw.makebool(False,0))

        self[2] = atom.Atom(names='outputs')
        self[2][2] = bundles.Output(2,False,names='pressure output')
        self[2][3] = bundles.Output(3,False,names='roll output')
        self[2][4] = bundles.Output(4,False,names='yaw output')
        self[2][5] = bundles.Output(5,False,names='scale note output')
        self[2][6] = bundles.Output(6,False,names='frequency output')
        self[2][7] = bundles.Output(16,False,names='damper output')

        self[3] = atom.Atom(names="cycling",domain=domain.Bool(),init=True,policy=atom.default_policy(self.__setcycling))

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.main_output = bundles.Splitter(self.domain,self[2][2],self[2][3],self[2][4],self[2][5],self[2][6])
        self.damp_output = bundles.Splitter(self.domain,self[2][7])
        self.cycler = piw.cycler(self.domain,32,self.main_output.cookie(),self.damp_output.cookie(),False)
        self.input = bundles.VectorInput(self.cycler.main_cookie(),self.domain,signals=(2,3,4,5,6,16,17),threshold=5)
        self.feedback = bundles.VectorInput(self.cycler.feedback_cookie(),self.domain,signals=(1,),threshold=5)

        self.cycler.set_cycle(True)
        self.cycler.set_maxdamp(1.0)
        self.cycler.set_invert(True)
        self.cycler.set_curve(1.0)

        self[1] = atom.Atom(names='inputs')
        self[1][2]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(2,False),names='pressure input')
        self[1][3]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(3,False),names='roll input')
        self[1][4]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(4,False),names='yaw input')
        self[1][5]=atom.Atom(domain=domain.BoundedFloat(0,1000),policy=self.input.vector_policy(5,False),names='scale note input')
        self[1][6]=atom.Atom(domain=domain.BoundedFloat(0,96000),policy=self.input.vector_policy(6,False),names='frequency input')
        self[1][7]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.feedback.vector_policy(1,False,clocked=False),names='feedback input')
        self[1][8]=atom.Atom(domain=domain.BoundedFloat(0,1),init=0.0,policy=self.input.linger_policy(16,False),names='damper pedal input',container=(None,'damper',self.verb_container()))
        self[1][10]=atom.Atom(domain=domain.BoundedFloat(0,1,hints=(T('stageinc',0.01),T('inc',0.01),T('biginc',0.1),T('control','updown'))),init=1.0,policy=atom.default_policy(self.__setmaxdamp),names='damper maximum input')
        self[1][12]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.latch_policy(17,False),names='hold pedal input')
        self[1][13]=atom.Atom(domain=domain.BoundedFloat(0.1,10,hints=(T('stageinc',0.1),T('inc',0.1),T('biginc',1),T('control','updown'))),init=1,names="damper curve",policy=atom.default_policy(self.__setdcurve))

        self[4] = atom.Atom(names="inverted damper",domain=domain.Bool(),init=True,policy=atom.default_policy(self.__setinverted))

    def __setmaxdamp(self,v):
        self.cycler.set_maxdamp(v)
        return True

    def __setinverted(self,v):
        self.cycler.set_invert(v)
        return True

    def __setcycling(self,v):
        self.cycler.set_cycle(v)
        return True

    def __invert_set(self,d):
        if d.is_bool():
            self.get_private()[2].set_data(d)
            self.cycler.set_invert(d.as_bool())

    def __cycle_set(self,d):
        if d.is_bool():
            self.get_private()[1].set_data(d)
            self.cycler.set_cycle(d.as_bool())

    def __setdcurve(self,x):
        self.cycler.set_curve(x)
        return True


agent.main(Agent)
