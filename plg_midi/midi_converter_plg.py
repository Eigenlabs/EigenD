
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

from pi import agent,atom,logic,node,utils,bundles,audio,domain,const,resource,guid,upgrade,policy,errors,action,inputparameter,paths
from pibelcanto import lexicon
import piw,urllib,sys,os,operator,glob,shutil,string
from plg_midi import midi_converter_version as version

import piw_native

class AgentStateNode(node.server):
    def __init__(self,**kw):
        node.server.__init__(self,change=lambda d: self.set_data(d),**kw)


class AgentState(node.server):
    def __init__(self,callback):
        node.server.__init__(self)
        self.__load_callback = callback
        self[1] = AgentStateNode()

    def load_state(self,state,delegate,phase):
        node.server.load_state(self,state,delegate,phase)
        mapping = self[1].get_data().as_string() if self[1].get_data().is_string() else '[]'
        self.__load_callback(delegate,mapping)


class MidiChannelDelegate(piw_native.midi_channel_delegate):
    def __init__(self,agent):
        piw_native.midi_channel_delegate.__init__(self)
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


class MappingObserver(piw_native.mapping_observer):
    def __init__(self,state,agent):
        piw_native.mapping_observer.__init__(self)
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
        self.__midi_from_belcanto = piw.midi_from_belcanto(self.__output.cookie(), self.__domain)
        self.__midi_converter = piw.midi_converter(self.__observer, self.__channel_delegate, self.__domain, self.__midi_from_belcanto, self.__get_title())
 
        self.parameter_list = inputparameter.List(self.__midi_converter,self.__midi_converter.clock_domain(),self.verb_container())

        # velocity detector, reads pressure input (signal 1) and generates a note on velocity (signal 4), passes thru all other signals in bundle
        self.__kvelo = piw.velocitydetect(self.__midi_from_belcanto.cookie(),1,4)

        # Inputs for generating keyboard driven MIDI signals
        # MIDI controllers are merged down with signals from keys (driven by pressure)
        self.__kinpt = bundles.VectorInput(self.__kvelo.cookie(),self.__domain,signals=(1,2,5))
        self[2] = atom.Atom()
        self[2][1] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.vector_policy(1,False),names='pressure input')
        self[2][2] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.merge_nodefault_policy(2,False),names='frequency input')
        self[2][3] = atom.Atom(domain=domain.Aniso(),policy=self.__kinpt.merge_nodefault_policy(5,False),names='key input')

         # input to set the MIDI channel
        self[3] = atom.Atom(domain=domain.BoundedInt(0,16),init=0,names="midi channel",policy=atom.default_policy(self.set_midi_channel))

        # inputs to control the velocity curve
        self[4] = atom.Atom()
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
        if c>=self[8].get_value():
            return False
        self[7].set_value(c)
        self.__midi_converter.set_min_midi_channel(c)
        return True

    def set_max_channel(self,c):
        if c<=self[7].get_value():
            return False
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


class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_2_to_1_0_3(self,tools,address):
        print 'upgrading midi converter',address
        root = tools.get_root(address)

        # remove midi program and bank atoms
        root.erase_child(5)
        root.erase_child(6)

        # ensure input atom names
        root.ensure_node(2,3).set_name('key input')

    def phase2_1_0_3(self,tools,address):
        root = tools.get_root(address)

        # hook up the key input
        root.mimic_connections((2,1),(2,3),'key output')

    def upgrade_1_0_1_to_1_0_2(self,tools,address):
        pass

    def phase2_1_0_2(self,tools,address):
        print 'upgrading converter',address
        root = tools.get_root(address)
        freq_input = root.ensure_node(2,2)
        scaler_conn = freq_input.get_master()
        if not scaler_conn: return
        scaler_addr = paths.id2server(scaler_conn[0])
        scaler_root = tools.get_root(scaler_addr)
        if not scaler_root: return
        scaler_type = scaler_root.get_meta_string('plugin')
        if scaler_type != 'scaler': return
        print 'scaler is',scaler_addr
        scaler_pressure_input = scaler_root.get_node(4,2)
        recorder_conn = scaler_pressure_input.get_master()
        if not recorder_conn: return
        recorder_addr = paths.id2server(recorder_conn[0])
        recorder_root = tools.get_root(recorder_addr)
        if not recorder_root: return
        recorder_type = recorder_root.get_meta_string('plugin')
        if recorder_type != 'recorder': return
        print 'recorder is',recorder_addr

        available_sources = {}
        available_sources['pressure'] = paths.makeid_list(scaler_addr,1,2)
        available_sources['roll'] = paths.makeid_list(scaler_addr,1,3)
        available_sources['yaw'] = paths.makeid_list(scaler_addr,1,4)
        available_sources['key position'] = paths.makeid_list(scaler_addr,1,2)

        for i in range(5,15):
            recorder_input = recorder_root.get_node(1,i)
            if recorder_input:
                input_name = recorder_input.get_name()
                input_conn = recorder_input.get_master()
                if not input_conn: continue
                upstream_addr,upstream_path = paths.breakid_list(input_conn[0])
                upstream_root = tools.get_root(upstream_addr)
                if not upstream_root: continue
                upstream = upstream_root.get_node(*upstream_path)
                if not upstream: continue
                upstream_name = upstream.get_name().split()
                if 'output' in upstream_name: upstream_name.remove('output')
                upstream_name = ' '.join(upstream_name)
                if upstream_name == 'controller': continue
                output_id = paths.makeid_list(recorder_addr,2,i)
                print 'recorder output',i,'at',output_id,'called',input_name,'connected to',upstream_name
                available_sources[upstream_name] = output_id

        for n,id in available_sources.items():
            print 'source',n,'at',id


        desired_sources = {
            'pressure': 1,
            'roll': 2,
            'yaw': 3,
            'breath': 4,
            'strip position 1': 5,
            'absolute strip 1': 6,
            'strip position 2': 7,
            'absolute strip 2': 8,
            'pedal 1': 9,
            'pedal 2': 10,
            'pedal 3': 11,
            'pedal 4': 12,
            'key position': 16,
        }

        active_sources = {}

        for k in range(1,17):
            parm_node = root.ensure_node(12,k)
            parm_node.set_meta('master',piw.data())
            parm_node.set_name('parameter %d' % k)
            
        for k,v in desired_sources.items():
            if k in available_sources:
                conn = logic.make_term('conn',None,None,available_sources[k],None)
                conn_txt = logic.render_term(conn)
                parm_node = root.ensure_node(12,v)
                parm_node.set_meta_string('master',conn_txt)
                parm_node.set_name(k)
                if v < 16: # dont map anything to key position
                    active_sources[available_sources[k]] = v
                print 'connected',parm_node.id(),'to',conn_txt,'called',k

        print 'checking for midi output',active_sources

        my_controller = None

        all_agents = tools.get_agents()
        for agent in all_agents:
            agent_root = tools.get_root(agent)
            if not agent_root: continue
            agent_type = agent_root.get_meta_string('plugin')
            if agent_type != 'midi_output': continue
            agent_midi_input = agent_root.get_node(2)
            if not agent_midi_input: continue
            agent_conn = [ paths.id2server(i) for i in agent_midi_input.get_master()]
            print agent,'is midi output',agent_conn,root.id()
            if root.id() not in agent_conn: continue
            print 'my midi output',agent
            for i in agent_conn:
                controller_root = tools.get_root(i)
                controller_type = controller_root.get_meta_string('plugin')
                if controller_type != 'midi_controller': continue
                my_controller = controller_root
                break
            if my_controller: break

        if not my_controller: return

        cc1 = self.get_midi_cc(my_controller,1)
        cc2 = self.get_midi_cc(my_controller,2)
        cc3 = self.get_midi_cc(my_controller,3)
        cc4 = self.get_midi_cc(my_controller,4)

        print 'midi controller',my_controller.id(),cc1,cc2,cc3,cc4

        global_map = {}

        self.check_midi_controller(my_controller,(2,1),7,active_sources,global_map) # volume
        self.check_midi_controller(my_controller,(2,2),1,active_sources,global_map) # modwheel
        self.check_midi_controller(my_controller,(2,3),10,active_sources,global_map) # pan
        self.check_midi_controller(my_controller,(2,4),4,active_sources,global_map) # foot pedal
        self.check_midi_controller(my_controller,(2,5),11,active_sources,global_map) # EXPRESSION
        self.check_midi_controller(my_controller,(2,6),130,active_sources,global_map) # channel pressure
        self.check_midi_controller(my_controller,(2,7),131,active_sources,global_map) # pitch bend
        self.check_midi_controller(my_controller,(2,8),64,active_sources,global_map) # sustain pedal
        self.check_midi_controller(my_controller,(2,9),cc1,active_sources,global_map)
        self.check_midi_controller(my_controller,(2,10),cc2,active_sources,global_map)
        self.check_midi_controller(my_controller,(2,11),cc3,active_sources,global_map)
        self.check_midi_controller(my_controller,(2,12),cc4,active_sources,global_map)

        local_map = {}

        self.check_midi_controller(root,(2,4),7,active_sources,local_map) # volume
        self.check_midi_controller(root,(2,5),1,active_sources,local_map) # modwheel
        self.check_midi_controller(root,(2,6),10,active_sources,local_map) # pan
        self.check_midi_controller(root,(2,7),4,active_sources,local_map) # foot pedal
        self.check_midi_controller(root,(2,8),11,active_sources,local_map) # expression
        self.check_midi_controller(root,(2,9),130,active_sources,local_map) # channel pressure
        self.check_midi_controller(root,(2,10),128,active_sources,local_map) # poly pressure
        self.check_midi_controller(root,(2,11),131,active_sources,local_map) # pitch bend
        self.check_midi_controller(root,(2,12),64,active_sources,local_map) # sustain pedal

        print 'per key mappings',local_map
        print 'global mappings',global_map

        map_node = root.ensure_node(255,1)
        map_data = map_node.get_data()
        map_list = []

        if map_data.is_string() and map_data.as_string() and map_data.as_string() != '[]':
            print 'map data',map_data.as_string()
            map_term = logic.parse_term(map_data.as_string())
            map_list = map_term.args

        bits_14 = set([0x07,0x01,0x0a,0x04,0x0b])

        for cc,p in local_map.items():
            ccl = (cc+32) if cc<32 else -1
            res = const.resolution_bits_7 if cc not in bits_14 else const.resolution_bits_14
            map_list=list(map_list)
            map_list.append(logic.make_term('c',p,cc,1.0,0,0,1.0,True,const.scope_pernote,ccl,ccl))

        for cc,p in global_map.items():
            ccl = (cc+32) if cc<32 else -1
            res = const.resolution_bits_7 if cc not in bits_14 else const.resolution_bits_14
            map_list.append(logic.make_term('c',p,cc,1.0,0,0,1.0,True,const.scope_global,ccl,ccl))

        map_term = logic.make_term('mapping',*map_list)
        map_node.set_data(piw.makestring(logic.render_term(map_term),0))

    def get_midi_cc(self,root,ccn):
        node = root.get_node(6,ccn,254)
        if not node: return None
        cc = node.get_data()
        if cc.is_float(): return int(cc.as_float())
        return None

    def check_midi_controller(self,root,path,cc,controls,global_map):
        input_node = root.get_node(*path)
        if not input_node:
            return

        input_conn = input_node.get_master()
        for i in input_conn:
            if i in controls:
                print root.id(),path,'input cc=',cc,'connected to',i,'->',controls[i]
                global_map[cc] = controls[i]
                return


    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        root = tools.get_root(address)

        root.ensure_node(11,1).set_name('parameter rate 1')
        root.ensure_node(11,2).set_name('parameter rate 2')
        root.ensure_node(11,3).set_name('parameter rate 3')
        root.ensure_node(11,4).set_name('parameter rate 4')
        root.ensure_node(11,5).set_name('parameter rate 5')
        root.ensure_node(11,6).set_name('parameter rate 6')
        root.ensure_node(11,7).set_name('parameter rate 7')
        root.ensure_node(11,8).set_name('parameter rate 8')
        root.ensure_node(11,9).set_name('parameter rate 9')


    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)

        root.ensure_node(9,255,1)
        root.ensure_node(9,255,8).set_string('auto channel pressure')
        root.ensure_node(9,255,3)
        root.ensure_node(9,254).set_data(piw.makebool(False,0))

        root.ensure_node(10,255,1)
        root.ensure_node(10,255,8).set_string('auto poly pressure')
        root.ensure_node(10,255,3)
        root.ensure_node(10,254).set_data(piw.makebool(False,0))

        return True

    def upgrade_1_0_to_2_0(self,tools,address):
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

        return True

agent.main(Agent,Upgrader,gui=True)
