
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
import picross
import synth_native

from pi import agent,atom,bundles,domain,paths,upgrade,policy,utils,audio
from plg_synth import panner_version as version

def pan_function(f):
    return (f+1.0)/2.0

class Agent(agent.Agent):

    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='panner',ordinal=ordinal)

        self.pan = piw.make_f2f_table(-1,1,1000,picross.make_f2f_functor(pan_function))

        self.domain = piw.clockdomain_ctl()
        self.output = bundles.Splitter(self.domain)
        self.panner = piw.panner(self.pan,self.output.cookie(),self.domain)
        self.input = bundles.VectorInput(self.panner.cookie(), self.domain, signals=(1,2,3))

        self[1]=audio.AudioOutput(self.output,1,2,names='outputs')
        self[2]=audio.AudioInput(self.input,2,2,names='inputs')

        self[3]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="pan input", policy=self.input.merge_policy(1,policy.IsoStreamPolicy(1,-1,0)))

class Upgrader(upgrade.Upgrader):
    pass

agent.main(Agent,Upgrader)

