
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
import synth_native
from plg_synth import synth_filter_version as version
from pi.logic.shortcuts import T
from pi import agent,atom,domain,upgrade,action,utils,bundles

class Agent(agent.Agent):

    def __init__(self,address, ordinal):
        self.domain = piw.clockdomain_ctl()
        agent.Agent.__init__(self,signature=version,names='synth filter',container=(10,'agent',atom.VerbContainer(clock_domain=self.domain)),ordinal=ordinal)

        self[1]=bundles.Output(1,True,names="low pass output")
        self[2]=bundles.Output(2,True,names="high pass output")
        self[3]=bundles.Output(3,True,names="band pass output")

        self.output = bundles.Splitter(self.domain, self[1],self[2],self[3])
        self.filter = synth_native.synthfilter(self.output.cookie(),self.domain)
        self.bend_filter = piw.function2(True,3,4,1,piw.makefloat_bounded(96000,0,0,440,0),piw.makefloat_bounded(1,-1,0,0,0),self.filter.cookie())
        self.bender = piw.fast_pitchbender()
        self.bend_filter.set_functor(self.bender.bend_functor())
        self.input = bundles.VectorInput(self.bend_filter.cookie(), self.domain, signals=(5,2,3,4),threshold=5)
        self.input.add_upstream(self.verb_container().clock)

        self[4]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="audio input", policy=self.input.vector_policy(5,True))
        self[5]=atom.Atom(domain=domain.BoundedFloat(0,1,rest=0.5,hints=(T('control','updown'),T('inc',0.01),T('biginc',0.05))), names="resonance input",policy=self.input.merge_policy(2,False))
        self[6]=atom.Atom(domain=domain.BoundedFloat(0,96000), init=5000, names="cutoff frequency input",policy=self.input.merge_policy(3,False))
        self[7]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="bend input",policy=self.input.merge_policy(4,False))

        self[8]=atom.Atom(domain=domain.BoundedFloat(0,72),init=24,policy=atom.default_policy(self.__rchange),names='frequency range',protocols='bind set',container=(None,'range',self.verb_container()))
        self[8].add_verb2(1,'set([],None,role(None,[instance(~self)]),role(to,[]))',create_action=self.__rcreate,clock=True)

        self[9]=atom.Atom(domain=domain.BoundedFloat(-144,144,rest=0),init=24,policy=atom.default_policy(self.__ochange),names='frequency offset',protocols='bind set',container=(None,'offset',self.verb_container()))
        self[9].add_verb2(2,'set([],None,role(None,[instance(~self)]),role(to,[]))',create_action=self.__ocreate,clock=True)

        self.__rchange(24)
        self.__ochange(24)

    def __rcreate(self,ctx,subj,dummy,arg):
        val = float(action.abstract_string(arg))
        print 'rcreate',ctx,subj,arg,val
        if val<0 or val>72:
            return async.failure('range inappropriate')
        b = piw.change_nb2(self.bender.set_range(),piw.slowchange_polled(utils.changify(self[8].get_policy().set_data),5000))
        return piw.trigger(b,piw.makefloat_nb(val,0)),None

    def __ocreate(self,ctx,subj,dummy,arg):
        val = float(action.abstract_string(arg))
        print 'ocreate',ctx,subj,arg,val
        if val<-72 or val>72:
            return async.failure('offset inappropriate')
        b = piw.change_nb2(self.bender.set_offset(),piw.slowchange_polled(utils.changify(self[9].get_policy().set_data),5000))
        return piw.trigger(b,piw.makefloat_nb(val,0)),None

    def __rchange(self,v):
        fc = piw.fastchange(self.bender.set_range())
        fc(piw.makefloat(v,0))
        return True

    def __ochange(self,v):
        fc = piw.fastchange(self.bender.set_offset())
        fc(piw.makefloat(v,0))
        return True

class Upgrader(upgrade.Upgrader):
    pass

agent.main(Agent,Upgrader)
