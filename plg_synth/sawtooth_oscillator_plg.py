
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
from pi import const,agent,atom,domain,bundles,upgrade,policy
from . import sawtooth_oscillator_version as version,synth_native

class Agent(agent.Agent):

    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='sawtooth oscillator',ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()

        self[1] = bundles.Output(1,True, names='audio output')

        self.output = bundles.Splitter(self.domain, self[1])
        self.osc = synth_native.sawtooth(self.output.cookie(),self.domain)
        self.input = bundles.VectorInput(self.osc.cookie(), self.domain, signals=(1,2,4))

        self[2]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0, names='volume input',policy=self.input.local_policy(1,policy.IsoStreamPolicy(1,0,0)))
        self[3]=atom.Atom(domain=domain.BoundedFloat(0,96000,rest=440), names='frequency input',policy=self.input.merge_policy(2,False))
        self[4]=atom.Atom(init=0.0, domain=domain.BoundedFloat(-1200,1200), names='detune input',policy=self.input.merge_policy(4,False))


agent.main(Agent)
