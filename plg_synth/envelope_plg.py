
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
from pi import agent,atom,domain,bundles,upgrade,policy
from pi.logic.shortcuts import T
from plg_synth import envelope_version as version

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='envelope',ordinal=ordinal)

        self[2] = atom.Atom()
        self[2][2] = bundles.Output(2,False,names='pressure output')
        self[2][100] = bundles.Output(1,True,names='volume output')

        self.domain = piw.clockdomain_ctl()
        self.output = bundles.Splitter(self.domain,*self[2].values())
        self.adsr = synth_native.adsr(self.output.cookie(),self.domain)
        self.vel = piw.velocitydetect(self.adsr.cookie(), 2, 1)
        self.input = bundles.VectorInput(self.vel.cookie(), self.domain,signals=(1,2,8,9,10,11),threshold=5)

        time=(T('inc',0.01),T('biginc',0.2),T('control','updown'))
        self[1] = atom.Atom()

        self[1][1]=atom.Atom(domain=domain.BoundedFloat(0,1), names='activation input',policy=self.input.merge_policy(1,False))
        self[1][2]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, policy=self.input.vector_policy(2,False), names='pressure input')
        self[1][8]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0.01,names="attack input",policy=self.input.merge_policy(8,False))
        self[1][9]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0.05,names="decay input",policy=self.input.merge_policy(9,False))
        self[1][10]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, policy=self.input.merge_policy(10,policy.IsoStreamPolicy(1,0,0)), names='sustain input',fuzzy='++++pressure')
        self[1][11]=atom.Atom(domain=domain.BoundedFloat(0,60,hints=time),init=0.25,names="release input",policy=self.input.merge_policy(11,False))

        self[3] = atom.Atom(domain=domain.BoundedInt(1,1000),init=4,names="velocity sample",policy=atom.default_policy(self.__set_samples))
        self[4] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names="velocity curve",policy=atom.default_policy(self.__set_curve))
        self[5] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names="velocity scale",policy=atom.default_policy(self.__set_scale))

    def __set_samples(self,x):
        self.vel.set_samples(x)
        return True

    def __set_curve(self,x):
        self.vel.set_curve(x)
        return True

    def __set_scale(self,x):
        self.vel.set_scale(x)
        return True


agent.main(Agent)
