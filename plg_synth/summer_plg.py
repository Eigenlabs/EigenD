
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

from pi import agent,atom,domain,utils,bundles,upgrade,paths,audio
from plg_synth import summer_version as version

import piw
import synth_native

class Agent(agent.Agent):
    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='summer',ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()


        self.output = bundles.Splitter(self.domain)
        self.mixer = piw.stereosummer(self.domain,self.output.cookie(),1)
        self.input = bundles.VectorInput(self.mixer.cookie(), self.domain, signals=(1,2,3,4,5,6,7,8,9))

        self[2] = audio.AudioOutput(self.output,1,1,names='outputs')
        self[1] = audio.AudioInput(self.input,1,1,names='inputs')
        self[4] = audio.AudioChannels(self[1],self[2],self.mixer)

agent.main(Agent)

