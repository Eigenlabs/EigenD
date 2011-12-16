
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
# MIDI converter plugin
#
# Agent to convert Belcanto note data into a MIDI stream
# ------------------------------------------------------------------------------------------------------------------

from pi import agent,atom,logic,node,utils,bundles,audio,domain,const,resource,guid,upgrade,policy,errors,action,inputparameter,paths,async
from pibelcanto import lexicon
import piw,urllib,sys,os,operator,glob,shutil,string
from plg_midi import midi_converter_version as version

import piw_native
import midilib_native

class AgentStateNode(node.server):
    def __init__(self,**kw):
        node.server.__init__(self,change=lambda d: self.set_data(d),**kw)


class AgentState(node.server):
    def __init__(self,callback):
        node.server.__init__(self)
        self.__load_callback = callback
        self[1] = AgentStateNode()

    @async.coroutine('internal error')
    def load_state(self,state,delegate,phase):
        yield node.server.load_state(self,state,delegate,phase)
        mapping = self[1].get_data().as_string() if self[1].get_data().is_string() else '[]'
        self.__load_callback(delegate,mapping)


class MidiChannelDelegate(midilib_native.midi_channel_delegate):
    def __init__(self,agent):
        midilib_native.midi_channel_delegate.__init__(self)
        self.__agent = agent

    def set_midi_channel(self, channel):
        self.__agent[3].set_value(channel)
        self.__agent.set_midi_channel(channel)

    def set_min_channel(self, channel):
        self.__agent[7].set_value(channel)
        self.__agent.set_min_channel(channel)

    def set_max_channel(self, channel):
        self.__agent[8].set_value(channel)
        self.__agent.set_max_channel(channel)

    def get_midi_channel(self):
        return int(self.__agent[3].get_value())

    def get_min_channel(self):
        return int(self.__agent[7].get_value())

    def get_max_channel(self):
        return int(self.__agent[8].get_value())


class MappingObserver(midilib_native.mapping_observer):
    def __init__(self,state,agent):
        midilib_native.mapping_observer.__init__(self)
        self.__state = state
        self.__agent = agent

    def mapping_changed(self,mapping):
        self.__state[1].set_data(piw.makestring(mapping,0))

    def parameter_changed(self,param):
        pass

    def settings_changed(self):
        pass

    def get_parameter_name(self,index):
        n = string.capwords(self.__agent.parameter_list[index].get_property_string('name'))
        if n!='Parameter':
            o = self.__agent.parameter_list[index].get_property_long('ordinal')
            if o:
                return str(index)+": "+n+" "+str(o)
            return str(index)+": "+n
        return str(index)


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        
        agent.Agent.__init__(self,names='midi converter',signature=version,container=100,ordinal=ordinal)

        self.__domain = piw.clockdomain_ctl()

        self.__state = AgentState(self.__agent_state_loaded)
        self.set_private(self.__state)

        # the MIDI stream output
        self[1]=bundles.Output(1,False,names="midi output")
        self.__output = bundles.Splitter(self.__domain, self[1])

        self.__observer = MappingObserver(self.__state,self)
        self.__channel_delegate = MidiChannelDelegate(self)
        self.__midi_from_belcanto = midilib_native.midi_from_belcanto(self.__output.cookie(), self.__domain)
        self.__midi_converter = midilib_native.midi_converter(self.__observer, self.__channel_delegate, self.__domain, self.__midi_from_belcanto, self.__get_title())
 
        self.parameter_list = inputparameter.List(self.__midi_converter,self.__midi_converter.clock_domain(),self.verb_container())

        # velocity detector, reads pressure input (signal 1) and generates a note on velocity (signal 4), passes thru all other signals in bundle
        self.__kvelo = piw.velocitydetect(self.__midi_from_belcanto.cookie(),1,4)

        # Inputs for generating keyboard driven MIDI signals
        # MIDI controllers are merged down with signals from keys (driven by pressure)
        self.__kinpt = bundles.VectorInput(self.__kvelo.cookie(),self.__domain,signals=(1,2,5))
        self[2] = atom.Atom(names='inputs')
        self[2][1] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.vector_policy(1,False),names='pressure input')
        self[2][2] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.merge_nodefault_policy(2,False),names='frequency input')
        self[2][3] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.merge_nodefault_policy(5,False),names='key input')

         # input to set the MIDI channel
        self[3] = atom.Atom(domain=domain.BoundedInt(0,16),init=0,names="midi channel",policy=atom.default_policy(self.set_midi_channel))

        # inputs to control the velocity curve
        self[4] = atom.Atom(names='velocity curve controls')
        self[4][1] = atom.Atom(domain=domain.BoundedInt(1,1000),init=4,names="velocity sample",policy=atom.default_policy(self.__set_samples))
        self[4][2] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names="velocity curve",policy=atom.default_policy(self.__set_curve))
        self[4][3] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names="velocity scale",policy=atom.default_policy(self.__set_scale))

        # inputs to set the minimum and maximum MIDI channel when in poly mode
        self[7] = atom.Atom(domain=domain.BoundedInt(1,16),init=1,names="minimum channel",policy=atom.default_policy(self.set_min_channel))
        self[8] = atom.Atom(domain=domain.BoundedInt(1,16),init=16,names="maximum channel",policy=atom.default_policy(self.set_max_channel))

        # parameter mapping
        self[12] = self.parameter_list

        # control change
        self.add_verb2(1,'set([],~a,role(None,[matches([midi,program])]),role(to,[numeric]))',create_action=self.__set_program_change)
        self.add_verb2(2,'set([],~a,role(None,[matches([midi,bank])]),role(to,[numeric]))',create_action=self.__set_bank_change)
        self.add_verb2(3,'set([],~a,role(None,[mass([midi,controller])]),role(to,[numeric]))',create_action=self.__set_midi_control)

        self.set_midi_channel(0)

    def close_server(self):
        self.__midi_converter.close();
        agent.Agent.close_server(self)

    def __get_title(self):
        n = string.capwords(self.get_property_string('name',0))
        o = self.get_property_long('ordinal',0)
        t = '%s %d' % (n,o)
        return t

    def set_midi_channel(self,c):
        self[3].set_value(c)
        self.__midi_converter.set_midi_channel(c)
        return True

    def set_min_channel(self,c):
        self[7].set_value(c)
        self.__midi_converter.set_min_midi_channel(c)
        return True

    def set_max_channel(self,c):
        self[8].set_value(c)
        self.__midi_converter.set_max_midi_channel(c)
        return True

    def __set_samples(self,x):
        self.__kvelo.set_samples(x)
        return True

    def __set_curve(self,x):
        self.__kvelo.set_curve(x)
        return True

    def __set_scale(self,x):
        self.__kvelo.set_scale(x)
        return True

    def __set_program_change(self,ctx,subj,dummy,val):
        to = action.abstract_wordlist(val)[0]
        to_val = int(to)
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to, 'set')
        return piw.trigger(self.__midi_from_belcanto.change_program(),piw.makelong_nb(to_val,0)),None

    def __set_bank_change(self,ctx,subj,dummy,val):
        to = action.abstract_wordlist(val)[0]
        to_val = int(to)
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to, 'set')
        return piw.trigger(self.__midi_from_belcanto.change_bank(),piw.makelong_nb(to_val,0)),None

    def __set_midi_control(self,ctx,subj,ctl,val):
        c = action.mass_quantity(ctl)
        to = action.abstract_wordlist(val)[0]
        c_val = int(c)
        to_val = int(to)
        if c_val < 0 or c_val > 127:
            return errors.invalid_thing(c, 'set')
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to, 'set')
        return piw.trigger(self.__midi_from_belcanto.change_cc(),utils.makedict_nb({'ctl':piw.makelong_nb(c_val,0),'val':piw.makelong_nb(to_val,0)},0)),None

    def __agent_state_loaded(self,delegate,mapping):
        self.__midi_converter.set_mapping(mapping)
        return


agent.main(Agent,gui=True)
