
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
from plg_synth import gain_version as version

class Agent(agent.Agent):

    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='gain',ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.output = bundles.Splitter(self.domain)
        self.gain = piw.linear_gain(self.output.cookie(),self.domain)
        self.input = bundles.VectorInput(self.gain.cookie(), self.domain, signals=(1,2,3,4,5,6,7,8,9,10))

        self[1]=audio.AudioOutput(self.output,2,1,names='outputs')
        self[2]=audio.AudioInput(self.input,2,1,names='inputs')
        self[4]=audio.AudioChannels(self[1],self[2])

        self[3]=atom.Atom(domain=domain.BoundedFloat(0,1), names="volume input", policy=self.input.merge_policy(1,policy.IsoStreamPolicy(1,0,0)))

class Upgrader(upgrade.Upgrader):
    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        root.get_node(1).erase()
        tools.substitute_connection(paths.makeid_list(address,1),paths.makeid_list(address,1,1))

        old_2 = root.get_node(2)
        root.get_node(2).erase()
        root.ensure_node(2,1).copy(old_2)
        root.ensure_node(2,1).rename(names='audio input')
        root.ensure_node(2,1,255,7).set_data(piw.makelong(1,0))
        return True

agent.main(Agent,Upgrader)

