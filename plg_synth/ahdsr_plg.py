
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
from pi import agent,atom,domain,bundles,upgrade,policy,paths
from pi.logic.shortcuts import T
from . import ahdsr_version as version,synth_native

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='ahdsr',ordinal=ordinal)

        self[2] = atom.Atom(names='outputs')
        self[2][1] = bundles.Output(1,True,names='volume output')
        self[2][2] = bundles.Output(2,False,names='activation output')
        self[2][3] = bundles.Output(8,False,names='pressure output')

        self.domain = piw.clockdomain_ctl()
        self.output = bundles.Splitter(self.domain,self[2][1],self[2][2],self[2][3])
        self.adsr = synth_native.adsr2(self.output.cookie(),self.domain)
        self.vdetect = piw.velocitydetect(self.adsr.cookie(),8,1)

        vel=(T('stageinc',0.1),T('inc',0.1),T('biginc',1),T('control','updown'))
        self[3] = atom.Atom(domain=domain.BoundedInt(1,1000),init=4,names="velocity sample",policy=atom.default_policy(self.__set_samples))
        self[4] = atom.Atom(domain=domain.BoundedFloat(0.1,10,hints=vel),init=4,names="velocity curve",policy=atom.default_policy(self.__set_curve))
        self[5] = atom.Atom(domain=domain.BoundedFloat(0.1,10,hints=vel),init=4,names="velocity scale",policy=atom.default_policy(self.__set_scale))
        self.input = bundles.VectorInput(self.vdetect.cookie(), self.domain,signals=(2,3,4,5,6,7,8,9,10,11,12,13,14),threshold=5)

        time=(T('stageinc',0.01),T('inc',0.01),T('biginc',0.2),T('control','updown'))
        self[1] = atom.Atom(names='inputs')
        self[1][1]=atom.Atom(domain=domain.BoundedFloat(0,1), init=1.0, names="activation input", policy=self.input.merge_policy(13,False), protocols="nostage")
        self[1][2]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0, names="delay input", policy=self.input.merge_policy(2,False))
        self[1][3]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0.01, names="attack input", policy=self.input.merge_policy(3,False))
        self[1][4]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0, names="hold input", policy=self.input.merge_policy(4,False))
        self[1][5]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0.05, names="decay input", policy=self.input.merge_policy(5,False))
        self[1][6]=atom.Atom(domain=domain.BoundedFloat(0,1), init=1.0, names="sustain input", policy=self.input.merge_policy(6,False))
        self[1][7]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0.5, names="release input", policy=self.input.merge_policy(7,False))
        self[1][8]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, names="pressure input", ordinal=1, policy=self.input.vector_policy(8,False))
        self[1][9]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, names="pressure input", ordinal=2, policy=self.input.vector_policy(9,policy.IsoStreamPolicy(1,0,0)))
        self[1][10]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, names="velocity sensitivity input", policy=self.input.merge_policy(10,False))
        self[1][11]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, names="aftertouch input", policy=self.input.merge_policy(11,False))
        self[1][12]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, names="pedal input", policy=self.input.latch_policy(12,False))
        self[1][13]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0.5, names="damper input", policy=self.input.linger_policy(14,False))

        self.__set_samples(4)
        self.__set_curve(4)
        self.__set_scale(4)

    def __set_samples(self,x):
        self.vdetect.set_samples(x)
        return True

    def __set_curve(self,x):
        self.vdetect.set_curve(x)
        return True

    def __set_scale(self,x):
        self.vdetect.set_scale(x)
        return True

agent.main(Agent)
