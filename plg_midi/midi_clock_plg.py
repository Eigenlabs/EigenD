
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
# MIDI clock plugin
#
# Agent to convert Belcanto metronome to MIDI clock
# ------------------------------------------------------------------------------------------------------------------

from pi import agent,atom,logic,node,utils,bundles,audio,domain,const,resource,guid,upgrade,policy
from pi.logic.shortcuts import T
from pibelcanto import lexicon
import piw,urllib,sys,os,operator,glob,shutil
from plg_midi import midi_clock_version as version
import midi_native


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        
        agent.Agent.__init__(self,names='midi clock',signature=version,container=100,ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        # the MIDI stream output
        self[1]=bundles.Output(1,False,names="midi output")
        self.output = bundles.Splitter(self.domain, self[1])

        self.midi_clock = midi_native.midi_clock(self.output.cookie(), self.domain)

        # the inputs from the metronome
        self.input = bundles.ScalarInput(self.midi_clock.cookie(), self.domain, signals=(1,2,3))

        self[2] = atom.Atom()
        self[2][1] = atom.Atom(domain=domain.Aniso(),policy=self.input.policy(1,False),names='running input')
        self[2][2] = atom.Atom(domain=domain.Aniso(),policy=self.input.policy(2,False),names='song beat input')
        self[2][3] = atom.Atom(domain=domain.BoundedFloat(0,100000),init=120,policy=self.input.policy(3,False),names="tempo input")

        h = (T('inc',0.5),T('biginc',10),T('control','updown'))
        self[3] = atom.Atom(domain=domain.BoundedFloat(-500,500,hints=h),init=0,policy=atom.default_policy(self.__set_delay),names="delay")

    def __set_delay(self,d):
        self.midi_clock.set_delay(d)
        return True

agent.main(Agent)







