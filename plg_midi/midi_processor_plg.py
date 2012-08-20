
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
from pi import action,agent,atom,bundles,domain,errors,logic
from . import midi_processor_version as version,midi_native

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,names='midi processor',signature=version,ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[2] = atom.Atom(names='outputs')
        self[2][1] = bundles.Output(1, False, names='wet midi output')
        self[2][2] = bundles.Output(2, False, names='dry midi output')

        self.output = bundles.Splitter(self.domain, self[2][1], self[2][2])
        self.midiprocessor = midi_native.midi_processor(self.output.cookie(), self.domain)
        self.input = bundles.VectorInput(self.midiprocessor.cookie(), self.domain, signals=(1,))

        self[1] = atom.Atom(domain=domain.Aniso(), names="midi input", policy=self.input.vector_policy(1,False))

        self[3] = atom.Atom(domain=domain.Bool(), init=False, names='closed', policy=atom.default_policy(self.__closed))

        self[4] = atom.Atom(names='filter')
        self[4][1] = atom.Atom(domain=domain.Bool(), init=True, names='note on enabled', policy=atom.default_policy(self.__noteon_enabled))
        self[4][2] = atom.Atom(domain=domain.Bool(), init=True, names='note off enabled', policy=atom.default_policy(self.__noteoff_enabled))
        self[4][3] = atom.Atom(domain=domain.Bool(), init=True, names='polyphonic pressure enabled', policy=atom.default_policy(self.__polypressure_enabled))
        self[4][4] = atom.Atom(domain=domain.Bool(), init=True, names='continuous controller enabled', policy=atom.default_policy(self.__cc_enabled))
        self[4][5] = atom.Atom(domain=domain.Bool(), init=True, names='program change enabled', policy=atom.default_policy(self.__programchange_enabled))
        self[4][6] = atom.Atom(domain=domain.Bool(), init=True, names='channel pressure enabled', policy=atom.default_policy(self.__channelpressure_enabled))
        self[4][7] = atom.Atom(domain=domain.Bool(), init=True, names='pitch bend enabled', policy=atom.default_policy(self.__pitchbend_enabled))
        self[4][8] = atom.Atom(domain=domain.Bool(), init=True, names='messages enabled', policy=atom.default_policy(self.__messages_enabled))

        self[5] = atom.Atom(names='mapping')
        self[5][1] = atom.Atom(domain=domain.String(), init='[]', names='channel mapping', policy=atom.default_policy(self.__set_channel_map))

        self.add_verb2(1,'clear([],none,role(none,[matches([channel,mapping])]))', callback=self.__clear_channel_map)
        self.add_verb2(2,'map([],none,role(none,[mass([channel])]),role(to,[numeric]))', callback=self.__map_channel)
        self.add_verb2(3,'map([un],none,role(none,[mass([channel])]))', callback=self.__unmap_channel)

    def __closed(self,v):
        self.midiprocessor.closed(v)
        self[3].set_value(v)

    def __noteon_enabled(self,v):
        self.midiprocessor.noteon_enabled(v)
        self[4][1].set_value(v)

    def __noteoff_enabled(self,v):
        self.midiprocessor.noteoff_enabled(v)
        self[4][2].set_value(v)

    def __polypressure_enabled(self,v):
        self.midiprocessor.polypressure_enabled(v)
        self[4][3].set_value(v)

    def __cc_enabled(self,v):
        self.midiprocessor.cc_enabled(v)
        self[4][4].set_value(v)

    def __programchange_enabled(self,v):
        self.midiprocessor.programchange_enabled(v)
        self[4][5].set_value(v)

    def __channelpressure_enabled(self,v):
        self.midiprocessor.channelpressure_enabled(v)
        self[4][6].set_value(v)

    def __pitchbend_enabled(self,v):
        self.midiprocessor.pitchbend_enabled(v)
        self[4][7].set_value(v)

    def __messages_enabled(self,v):
        self.midiprocessor.messages_enabled(v)
        self[4][8].set_value(v)

    def __current_channel_mapping(self):
        return logic.parse_clause(self[5][1].get_value())

    def __clear_channel_map(self,subject,v):
        self.__set_channel_mapping(())

    def __map_channel(self,subject,cfrom,cto):
        cfrom = int(action.mass_quantity(cfrom))
        cto = int(action.mass_quantity(cto))
        if cfrom<1 or cfrom>16:
            return errors.invalid_thing(str(cfrom), 'map')
        if cto<1 or cto>16:
            return errors.invalid_thing(str(cto), 'map')

        new_mapping = []
        new_mapping.append((cfrom,cto))
        for m in self.__current_channel_mapping():
            if m[0] != cfrom:
                new_mapping.append(m)

        self.__set_channel_mapping(new_mapping)

    def __unmap_channel(self,subject,cfrom):
        cfrom = int(action.mass_quantity(cfrom))
        if cfrom<1 or cfrom>16:
            return errors.invalid_thing(str(cfrom), 'map')

        new_mapping = []
        for m in self.__current_channel_mapping():
            if m[0] != cfrom:
                new_mapping.append(m)

        self.__set_channel_mapping(new_mapping)

    def __set_channel_map(self,v):
        mapping = logic.parse_clause(v)
        self.__set_channel_mapping(mapping)

    def __set_channel_mapping(self,v):
        mapping = list(v)
        mapping.sort()

        pruned_mapping = list()
        prev_key = None
        for entry in mapping:
            key = entry[0]
            value = entry[1]
            if key == value:
                continue
            if key == prev_key:
                continue
            if key>=1 and key<=16 and value>=1 and value<=16:
                pruned_mapping.append(entry)
            prev_key = key
        pruned_mapping.sort()

        self.midiprocessor.clear_channel_mapping()
        for (fr,to) in pruned_mapping:
            self.midiprocessor.set_channel_mapping(fr,to)
        self.midiprocessor.activate_channel_mapping()

        self[5][1].set_value(logic.render_term(pruned_mapping))

agent.main(Agent)

