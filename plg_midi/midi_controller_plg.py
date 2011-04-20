
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

# ------------------------------------------------------------------------------------------------------------------
# MIDI controller plugin
#
# ------------------------------------------------------------------------------------------------------------------

from pi import agent,upgrade
from plg_midi import midi_controller_version as version
import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,names='midi controller',signature=version,container=100,ordinal=ordinal)

class Upgrader(upgrade.Upgrader):
    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)

        for i in range(1,10):
            root.ensure_node(11,i,255,1)
            root.ensure_node(11,i,255,3)
            root.ensure_node(11,i,254).set_data(piw.makefloat(10.0,0))

        root.ensure_node(11,1,255,8).set_string("volume rate")
        root.ensure_node(11,2,255,8).set_string("modwheel rate")
        root.ensure_node(11,3,255,8).set_string("pan rate")
        root.ensure_node(11,4,255,8).set_string("foot pedal rate")
        root.ensure_node(11,5,255,8).set_string("expression rate")
        root.ensure_node(11,6,255,8).set_string("channel pressure rate")
        root.ensure_node(11,7,255,8).set_string("poly pressure rate")
        root.ensure_node(11,8,255,8).set_string("pitch bend rate")
        root.ensure_node(11,9,255,8).set_string("sustain pedal rate")
        root.ensure_node(11,10,255,8).set_string("continuous rate")
        root.ensure_node(11,11,255,8).set_string("continuous rate")
        root.ensure_node(11,12,255,8).set_string("continuous rate")
        root.ensure_node(11,13,255,8).set_string("continuous rate")
        root.ensure_node(11,10,255,7).set_long(1)
        root.ensure_node(11,11,255,7).set_long(2)
        root.ensure_node(11,12,255,7).set_long(3)
        root.ensure_node(11,13,255,7).set_long(4)

        return True

agent.main(Agent,Upgrader)







