
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
from pi import agent,atom,domain,upgrade,bundles
from plg_synth import shaper_version as version

class Agent(agent.Agent):
    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='shaper',ordinal=ordinal)

        self[2] = atom.Atom(names='outputs')
        self[2][1] = bundles.Output(1,False,names='activation output')
        self[2][2] = bundles.Output(2,False,names='pressure output')

        self.domain = piw.clockdomain_ctl()
        self.output = bundles.Splitter(self.domain,*self[2].values())

        Z = piw.makefloat_bounded(1,0,0,0,0)

        self.compress = piw.function1(False,2,2,Z,self.output.cookie())
        self.compress.set_functor(synth_native.compressor(0))

        self.sharpen = piw.function2(False,1,2,2,Z,Z,self.compress.cookie())
        self.sharpen.set_functor(synth_native.sharpener(0))

        self.input = bundles.VectorInput(self.sharpen.cookie(), self.domain,signals=(1,2))

        self[1] = atom.Atom(names='inputs')
        self[1][1]=atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.input.vector_policy(1,False), names='activation input')
        self[1][2]=atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.input.vector_policy(2,False), names='pressure input')

        self[1][3]=atom.Atom(domain=domain.BoundedFloat(0,1),names="compression",policy=atom.default_policy(self.__compression))
        self[1][4]=atom.Atom(domain=domain.BoundedFloat(0,1),names="sharpness",policy=atom.default_policy(self.__sharpness))

    def __sharpness(self,v):
        self.sharpen.set_functor(synth_native.sharpener(v))
        return True

    def __compression(self,v):
        self.compress.set_functor(synth_native.compressor(v))
        return True


agent.main(Agent)
