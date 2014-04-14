
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
from . import midi_converter_version as version
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
        settings = self.__agent.midi_converter.get_settings()
        self.__agent.set_midi_channel(settings.get_midi_channel())
        self.__agent.set_min_channel(settings.get_minimum_midi_channel())
        self.__agent.set_max_channel(settings.get_maximum_midi_channel())
        self.__agent.set_min_decimation(settings.get_minimum_decimation())
        self.__agent.set_midi_notes(settings.get_send_notes())
        self.__agent.set_midi_pitchbend(settings.get_send_pitchbend())
        self.__agent.set_midi_hires_velocity(settings.get_send_hires_velocity())
        self.__agent.set_pitchbend_up(settings.get_pitchbend_semitones_up())
        self.__agent.set_pitchbend_down(settings.get_pitchbend_semitones_down())

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
        self.__midi_from_belcanto = midilib_native.midi_from_belcanto(self.__output.cookie(), self.__domain)
        self.midi_converter = midilib_native.midi_converter(self.__observer, self.__domain, self.__midi_from_belcanto, self.__get_title())
 
        self.parameter_list = inputparameter.List(self.midi_converter,self.midi_converter.clock_domain(),self.verb_container())

        # Inputs for generating keyboard driven MIDI signals
        # MIDI controllers are merged down with signals from keys (driven by pressure)
        self.__kinpt = bundles.VectorInput(self.__midi_from_belcanto.cookie(),self.__domain,signals=(1,2))
        self[2] = atom.Atom(names='inputs')
        self[2][1] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.vector_policy(1,False),names='pressure input')
        self[2][2] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.merge_nodefault_policy(2,False),names='frequency input')

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

        # other global settings inputs
        self[13] = atom.Atom(domain=domain.BoundedInt(0,100),init=0,names="minimum decimation",policy=atom.default_policy(self.set_min_decimation))
        self[14] = atom.Atom(domain=domain.Bool(),init=True,names="notes enable",policy=atom.default_policy(self.set_midi_notes))
        self[15] = atom.Atom(domain=domain.Bool(),init=True,names="pitch bend enable",policy=atom.default_policy(self.set_midi_pitchbend))
        self[16] = atom.Atom(domain=domain.Bool(),init=False,names="high resolution velocity enable",policy=atom.default_policy(self.set_midi_hires_velocity))
        self[17] = atom.Atom(domain=domain.BoundedInt(0,48),init=1,names="pitch bend range upper",policy=atom.default_policy(self.set_pitchbend_up))
        self[18] = atom.Atom(domain=domain.BoundedInt(0,48),init=1,names="pitch bend range lower",policy=atom.default_policy(self.set_pitchbend_down))

        # parameter mapping
        self[12] = self.parameter_list

        # control change
        self.add_verb2(1,'set([],~a,role(None,[matches([midi,program])]),role(to,[numeric]))',create_action=self.__set_program_change)
        self.add_verb2(2,'set([],~a,role(None,[matches([midi,bank])]),role(to,[numeric]))',create_action=self.__set_bank_change)
        self.add_verb2(3,'set([],~a,role(None,[mass([midi,controller])]),role(to,[numeric]))',create_action=self.__set_midi_control)
        self.add_verb2(4,'start([],~a,role(None,[matches([midi,clock])]))',create_action=self.__start_midi_clock)
        self.add_verb2(5,'stop([],~a,role(None,[matches([midi,clock])]))',create_action=self.__stop_midi_clock)

        self.set_midi_channel(0)

    def close_server(self):
        self.midi_converter.close();
        agent.Agent.close_server(self)

    def property_change(self,key,value,delegate):
        if key in ['name','ordinal']:
            self.__set_title()

    def set_enclosure(self,enclosure):
        agent.Agent.set_enclosure(self,enclosure)
        self.__set_title()

    def __set_title(self):
        self.midi_converter.set_title(self.__get_title())

    def __get_title(self):
        t = self.get_description().title()
        e = self.get_enclosure()
        if e:
            t = '%s (%s)' % (t,e.title())
        return t

    def set_midi_channel(self,c):
        self[3].set_value(c)
        self.midi_converter.set_midi_channel(c)
        return True

    def set_min_channel(self,c):
        self[7].set_value(c)
        self.midi_converter.set_min_midi_channel(c)
        return True

    def set_max_channel(self,c):
        self[8].set_value(c)
        self.midi_converter.set_max_midi_channel(c)
        return True

    def set_min_decimation(self,v):
        self[13].set_value(v)
        self.midi_converter.set_minimum_decimation(v)
        return True

    def set_midi_notes(self,v):
        self[14].set_value(v)
        self.midi_converter.set_midi_notes(v)
        return True

    def set_midi_pitchbend(self,v):
        self[15].set_value(v)
        self.midi_converter.set_midi_pitchbend(v)
        return True

    def set_midi_hires_velocity(self,v):
        self[16].set_value(v)
        self.midi_converter.set_midi_hires_velocity(v)
        return True

    def set_pitchbend_up(self,v):
        self[17].set_value(v)
        self.midi_converter.set_pitchbend_up(v)
        return True

    def set_pitchbend_down(self,v):
        self[18].set_value(v)
        self.midi_converter.set_pitchbend_down(v)
        return True

    def __set_samples(self,x):
        self.__midi_from_belcanto.set_velocity_samples(x)
        return True

    def __set_curve(self,x):
        self.__midi_from_belcanto.set_velocity_curve(x)
        return True

    def __set_scale(self,x):
        self.__midi_from_belcanto.set_velocity_scale(x)
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

    def __start_midi_clock(self,ctx,subj,dummy):
        return piw.trigger(self.__midi_from_belcanto.change_clock(),piw.makelong_nb(1,0)),None

    def __stop_midi_clock(self,ctx,subj,dummy):
        return piw.trigger(self.__midi_from_belcanto.change_clock(),piw.makelong_nb(0,0)),None

    def __agent_state_loaded(self,delegate,mapping):
        self.midi_converter.set_mapping(mapping)
        return

class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_3_to_1_0_4(self,tools,address):
        pass

    def phase2_1_0_4(self,tools,address):
        print 'upgrading midi converter',address
        root = tools.get_root(address)
        key_input = root.get_node(2,3)
        print 'disconnecting key input',key_input.id()
        conn = key_input.get_master()
        if not conn: return
        for c in conn:
            print 'connection',c
            upstream_addr,upstream_path = paths.breakid_list(c)
            upstream_root = tools.get_root(upstream_addr)
            if not upstream_root: continue
            upstream = upstream_root.get_node(*upstream_path)
            upstream_slaves = logic.parse_clauselist(upstream.get_meta_string('slave'))
            print 'old upstream slaves',upstream_slaves
            upstream_slaves.remove(key_input.id())
            print 'new upstream slaves',upstream_slaves
            upstream.set_meta_string('slave', logic.render_termlist(upstream_slaves))

    def upgrade_1_0_4_to_1_0_5(self,tools,address):
        print 'upgrading midi converter',address
        root = tools.get_root(address)
        state = root.get_node(255,1)
        if state.get_data().is_string():
            mapping = state.get_data().as_string()
            if mapping != '[]':
                term = logic.parse_term(mapping)
                for t in term.args:
                    if "s" == t.pred:
                        decimation = 0
                        notes = True
                        pitchbend = True
                        hiresvel = True 
                        pbup = 1.0
                        pbdown = 1.0
                        if t.arity >= 3:
                            decimation = float(t.args[0])
                            notes = bool(t.args[1])
                            pitchbend = bool(t.args[2])
                        if t.arity >= 4:
                            hiresvel = bool(t.args[3]) 
                        if t.arity >= 6:
                            pbup = float(t.args[4])
                            pbdown = float(t.args[5])
                        root.ensure_node(13,254).set_data(piw.makefloat_bounded(100,0,0,decimation,0))
                        root.ensure_node(14,254).set_data(piw.makebool(notes,0))
                        root.ensure_node(15,254).set_data(piw.makebool(pitchbend,0))
                        root.ensure_node(16,254).set_data(piw.makebool(hiresvel,0))
                        root.ensure_node(17,254).set_data(piw.makefloat_bounded(48,0,0,pbup,0))
                        root.ensure_node(18,254).set_data(piw.makefloat_bounded(48,0,0,pbdown,0))
                        break

agent.main(Agent,Upgrader,gui=True)
