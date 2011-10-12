
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

from pi import agent,atom,logic,node,utils,bundles,audio,domain,const,upgrade,errors,action,async,resource,inputparameter,paths
from pibelcanto import lexicon
from plg_host import audio_unit_version as version
import piw
import urllib
import sys
import os
import operator
import glob
import shutil
import zlib
import host_native
import piw_native
import midilib_native
import string

response_size = 1200
def render_list(list,offset,renderer):
    txt='['

    for n,l in enumerate(list[offset:]):
        ltxt = '' if not n else ','
        ltxt = ltxt + renderer(n+offset,l)
        if len(txt+ltxt) > response_size-1:
            return txt+']'
        txt=txt+ltxt

    return txt+']'


def plugin_id(plg):
    return os.path.basename(plg.id())

def plugin_id_escaped(plg):
    return urllib.quote(plugin_id(plg))

class PluginList:

    __plugins_cache = os.path.join(resource.user_resource_dir('Plugins',version=''),'plugins_cache')

    def __init__(self):
        self.plugins_by_manufacturer = dict()
        self.plugins_by_id = dict()
        self.listeners = []
        self.__list = host_native.plugin_list(self.__plugins_cache,utils.notify(self.__setup))
        self.__setup()

    def check_description(self,desc):
        if desc.id() in self.plugins_by_id:
            return desc

        index = self.__list.find_plugin(desc)

        if index < 0:
            return  None

        return self.__list.get_plugin(index)

    def __setup(self):
        self.plugins_by_manufacturer.clear()
        self.plugins_by_id.clear()
        num_plugins = self.__list.num_plugins()
        for i in range(num_plugins):
            p = self.__list.get_plugin(i)
            self.plugins_by_manufacturer.setdefault(p.manufacturer(),[]).append(p)
            self.plugins_by_id[plugin_id(p)] = p
        for l in self.listeners:
            l()


global_plugin_list = None

class PluginBrowser(atom.Atom):

    def __init__(self,agent):
        global global_plugin_list
        if global_plugin_list is None:
            global_plugin_list = PluginList()
        self.__plugin_list = global_plugin_list

        atom.Atom.__init__(self,names='plugin',protocols='virtual browse')

        self.__agent = agent
        self.__timestamp = piw.tsd_time()
        self.__update()

    def rpc_current(self,args):
        return '[]'

    def rpc_resolve(self,args):
        return '[]'

    def rpc_setselected(self,args):
        return None

    def server_opened(self):
        atom.Atom.server_opened(self)
        self.__plugin_list.listeners.append(self.__update)

    def server_closed(self):
        atom.Atom.server_closed(self)
        self.__plugin_list.listeners.remove(self.__update)

    def get_description(self,id):
        return self.__plugin_list.plugins_by_id.get(id)

    def check_description(self,desc):
        return self.__plugin_list.check_description(desc)

    def refresh_plugin_list(self,*arg):
        pass

    def rpc_displayname(self,arg):
        return self.__agent.get_description()
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        if len(path)==1:
            self.__agent.set_plugin(selected)
        return logic.render_term(('',''))

    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_fideal(self,arg):
        (path,id) = logic.parse_clause(arg)
        return 'ideal([~server,plugin],%s)' % logic.render_term(id)

    def rpc_enumerate(self,a):
        path=logic.parse_clause(a)
        if len(path)==0:
            r = logic.render_term((0,len(self.__plugin_list.plugins_by_manufacturer)))
            return r

        if len(path)==1:
            p = self.__plugin_list.plugins_by_manufacturer.get(path[0])
            if p:
                r = logic.render_term((len(p),0))
                return r

        r = logic.render_term((0,0))
        return r

    def rpc_cinfo(self,a):
        (path,idx) = logic.parse_clause(a)
        if len(path)!=0: return '[]'
        l = self.__plugin_list.plugins_by_manufacturer.keys()
        l.sort()
        return render_list(l,idx,lambda i,t: logic.render_term((t)))

    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)
        if len(path) == 1:
            p = self.__plugin_list.plugins_by_manufacturer.get(path[0])
            if p:
                return render_list(p,idx,lambda i,t: logic.render_term((plugin_id_escaped(t),t.name(),t.description())))
        return logic.render_term(())


preset_chunk_size = 8000

def chunker(s):
    z = zlib.compress(s)
    zl = len(z)
    chunk = 1
    index = 0
    remain = zl
    while remain>0:
        l = min(preset_chunk_size,remain)
        yield chunk,piw.makeblob2(z[index:index+l],0)
        chunk += 1
        index += l
        remain -= l

class PluginStateNode(node.server):
    def __init__(self,**kw):
        node.server.__init__(self,change=lambda d: self.set_data(d),**kw)


class PluginStateBlob(node.server):
    def __init__(self):
        node.server.__init__(self,creator=self.__create,extension=255)

    def __create(self,k):
        return PluginStateNode()

    def store(self,state):
        self.clear()
        if state.is_blob():
            blob = state.as_blob2()
            for i,c in chunker(blob):
                self[i] = self.__create(i)
                self[i].set_data(c)

    def get_blob(self):
        z = ''.join([ n.get_data().as_blob2() for n in self.itervalues() ])
        return piw.makeblob2(zlib.decompress(z) if z else '',0)


class PluginState(node.server):
    def __init__(self,callback):
        node.server.__init__(self)
        self.__load_callback = callback
        self[1] = PluginStateNode()
        self[2] = PluginStateBlob()
        self[3] = PluginStateNode()
        self[4] = PluginStateNode()
        self[5] = PluginStateNode()
        self[6] = PluginStateBlob()

    def load_state(self,state,delegate,phase):
        node.server.load_state(self,state,delegate,phase)
        show = self[1].get_data().as_bool() if self[1].get_data().is_bool() else False
        state = self[2].get_blob()
        desc = urllib.unquote(self[3].get_data().as_string()) if self[3].get_data().is_string() else ''
        bypassed = self[4].get_data().as_bool() if self[4].get_data().is_bool() else False
        mapping = self[5].get_data().as_string() if self[5].get_data().is_string() else '[]'
        bounds = self[6].get_blob()
        self.__load_callback(delegate,state,desc,show,bypassed,mapping,bounds)

    def save_state(self,state,bounds):
        self[2].store(state)
        self[6].store(bounds)


class MidiChannelDelegate(midilib_native.midi_channel_delegate):
    def __init__(self,agent):
        midilib_native.midi_channel_delegate.__init__(self)
        self.__agent = agent

    def set_midi_channel(self, channel):
        self.__agent[7].set_value(channel)
        self.__agent.set_midi_channel(channel)

    def set_min_channel(self, channel):
        self.__agent[11].set_value(channel)
        self.__agent.set_min_channel(channel)

    def set_max_channel(self, channel):
        self.__agent[12].set_value(channel)
        self.__agent.set_max_channel(channel)

    def get_midi_channel(self):
        return int(self.__agent[7].get_value())

    def get_min_channel(self):
        return int(self.__agent[11].get_value())

    def get_max_channel(self):
        return int(self.__agent[12].get_value())


class PluginObserver(host_native.plugin_observer):
    def __init__(self,state,agent):
        host_native.plugin_observer.__init__(self)
        self.__state = state
        self.__agent = agent

    def showing_changed(self,show):
        self.__state[1].set_data(piw.makebool(show,0))
        self.__agent.set_light(2,show)

    def description_changed(self,desc):
        self.__state[3].set_data(piw.makestring(urllib.quote(desc),0))

    def bypassed_changed(self,bypassed):
        self.__state[4].set_data(piw.makebool(bypassed,0))

    def mapping_changed(self,mapping):
        self.__state[5].set_data(piw.makestring(mapping,0))

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
        agent.Agent.__init__(self,signature=version,names='audio unit',container=100,ordinal=ordinal)

        self.__state = PluginState(self.__plugin_state_loaded)
        self.set_private(self.__state)

        self.__browser = PluginBrowser(self)
        self.__domain = piw.clockdomain_ctl()
        self.__audio_output = audio.AudioOutput(bundles.Splitter(self.__domain),1,2)
        self.__audio_output_channels = audio.AudioChannels(self.__audio_output)
        self.__midi_output = bundles.Splitter(self.__domain)
        self.__observer = PluginObserver(self.__state,self)
        self.__channel_delegate = MidiChannelDelegate(self)
        self.__host = host_native.plugin_instance(
            self.__observer, self.__channel_delegate,
            self.__domain,self.__audio_output.cookie(),self.__midi_output.cookie(),
            utils.statusify(self.__window_state_changed))
        self.parameter_list = inputparameter.List(self.__host,self.__host.clock_domain(),self.verb_container())
        self.__audio_input = audio.AudioInput(bundles.ScalarInput(self.__host.audio_input(),self.__domain,signals=range(1,65)),1,2)
        self.__audio_input_channels = audio.AudioChannels(self.__audio_input)
        self.__velocity_detector = piw.velocitydetect(self.__host.midi_from_belcanto(),1,4)
        self.__key_input = bundles.VectorInput(self.__velocity_detector.cookie(),self.__domain,signals=(1,2,5))
        self.__midi_input = bundles.ScalarInput(self.__host.midi_aggregator(),self.__domain,signals=(1,))
        self.__metronome_input = bundles.ScalarInput(self.__host.metronome_input(),self.__domain,signals=(1,2,3,4))
        self.__host.set_bypassed(True)

        # plugin browser
        self[1] = self.__browser

        # audio output
        self[2] = atom.Atom(names='audio outputs')
        self[2][1] = self.__audio_output
        self[2][2] = self.__audio_output_channels

        # audio input
        self[3] = atom.Atom(names='audio inputs')
        self[3][1] = self.__audio_input
        self[3][2] = self.__audio_input_channels

        # parameter mapping
        self[4] = self.parameter_list

        # metronome input
        self[5] = atom.Atom(names='metronome inputs')
        self[5][1] = atom.Atom(domain=domain.Aniso(),policy=self.__metronome_input.nodefault_policy(1,False),names='song beat input')
        self[5][2] = atom.Atom(domain=domain.Aniso(),policy=self.__metronome_input.nodefault_policy(2,False),names='running input')
        self[5][3] = atom.Atom(domain=domain.Aniso(),policy=self.__metronome_input.nodefault_policy(3,False),names='tempo input')
        self[5][4] = atom.Atom(domain=domain.Aniso(),policy=self.__metronome_input.nodefault_policy(4,False),names='bar beat input')

        # kbd/controller inputs
        self[6] = atom.Atom(names='controller inputs')
        self[6][1] = atom.Atom(domain=domain.Aniso(),policy=self.__key_input.vector_policy(1,False),names='pressure input')
        self[6][2] = atom.Atom(domain=domain.Aniso(),policy=self.__key_input.merge_nodefault_policy(2,False),names='frequency input')
        self[6][3] = atom.Atom(domain=domain.Aniso(),policy=self.__key_input.merge_nodefault_policy(5,False),names='key input')

        # midi channel 
        self[7] =  atom.Atom(domain=domain.BoundedInt(0,16),init=0,names='midi channel',policy=atom.default_policy(self.set_midi_channel))

        # velocity curve control
        self[8] = atom.Atom(names='velocity curve controls')
        self[8][1] = atom.Atom(domain=domain.BoundedInt(1,1000),init=4,names='velocity sample',policy=atom.default_policy(self.__set_velocity_samples))
        self[8][2] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names='velocity curve',policy=atom.default_policy(self.__set_velocity_curve))
        self[8][3] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names='velocity scale',policy=atom.default_policy(self.__set_velocity_scale))

        self[10] = atom.Atom(domain=domain.BoundedFloatOrNull(0,100000),init=None,names='tail time',policy=atom.default_policy(self.__set_tail_time))
        self[11] = atom.Atom(domain=domain.BoundedInt(1,16),init=1,names='minimum channel',policy=atom.default_policy(self.set_min_channel))
        self[12] = atom.Atom(domain=domain.BoundedInt(1,16),init=16,names='maximum channel',policy=atom.default_policy(self.set_max_channel))

        # status output to drive the talker lights
        self[13] = bundles.Output(1,False,names='status output')
        self.light_output = bundles.Splitter(self.__domain,self[13])
        self.lights = piw.lightsource(piw.change_nb(),0,self.light_output.cookie())
        self.lights.set_size(2)
        self.set_light(1,True)
        self.set_light(2,False)

        # midi I/O
        # midi input
        self[14] = atom.Atom(domain=domain.Aniso(),policy=self.__midi_input.nodefault_policy(1,False),names='midi input')
        # midi output
        self[15] = self.__midi_output.add_output(bundles.Output(1,False,names='midi output'))

        self.add_verb2(1,'show([],None)',callback=self.__show)
        self.add_verb2(2,'show([un],None)',callback=self.__unshow)
        self.add_verb2(3,'close([],None)',callback=self.__close)
        self.add_verb2(4,'bypass([toggle],None)',callback=self.__tog_bypass,status_action=self.__status_bypass)
        self.add_verb2(5,'show([toggle],None)',callback=self.__tog_show,status_action=self.__status_show)
        self.add_verb2(6,'open([],None,role(None,[abstract]))',callback=self.__open)
        self.add_verb2(7,'scan([],None)',callback=self.__browser.refresh_plugin_list)
        self.add_verb2(8,'bypass([],None)',callback=self.__bypass)
        self.add_verb2(9,'bypass([un],None)',callback=self.__unbypass)

        # control change
        self.add_verb2(10,'set([],~a,role(None,[matches([midi,program])]),role(to,[numeric]))',create_action=self.__set_program_change)
        self.add_verb2(11,'set([],~a,role(None,[matches([midi,bank])]),role(to,[numeric]))',create_action=self.__set_bank_change)
        self.add_verb2(12,'set([],~a,role(None,[mass([midi,controller])]),role(to,[numeric]))',create_action=self.__set_midi_control)

        self.set_ordinal(ordinal)

    def close_server(self):
        self.__host.close();
        agent.Agent.close_server(self)

    def set_midi_channel(self,c):
        self[7].set_value(c)
        self.__host.set_midi_channel(c)
        return True

    def set_min_channel(self,c):
        self[11].set_value(c)
        self.__host.set_min_midi_channel(c)
        return True

    def set_max_channel(self,c):
        self[12].set_value(c)
        self.__host.set_max_midi_channel(c)
        return True

    def __set_program_change(self,ctx,subj,dummy,val):
        to = action.abstract_wordlist(val)[0]
        to_val = int(to)
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to, 'set')
        return piw.trigger(self.__host.change_program(),piw.makelong_nb(to_val,0)),None

    def __set_bank_change(self,ctx,subj,dummy,val):
        to = action.abstract_wordlist(val)[0]
        to_val = int(to)
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to, 'set')
        return piw.trigger(self.__host.change_bank(),piw.makelong_nb(to_val,0)),None

    def __set_midi_control(self,ctx,subj,ctl,val):
        c = action.mass_quantity(ctl)
        to = action.abstract_wordlist(val)[0]
        c_val = int(c)
        to_val = int(to)
        if c_val < 0 or c_val > 127:
            return errors.invalid_thing(c, 'set')
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to, 'set')
        return piw.trigger(self.__host.change_cc(),utils.makedict_nb({'ctl':piw.makelong_nb(c_val,0),'val':piw.makelong_nb(to_val,0)},0)),None

    def __set_velocity_samples(self,samples):
        self.__velocity_detector.set_samples(samples)
        return True

    def __set_velocity_curve(self,curve):
        self.__velocity_detector.set_curve(curve)
        return True

    def __set_velocity_scale(self,scale):
        self.__velocity_detector.set_scale(scale)
        return True

    def __open(self,arg,id):
        id = action.abstract_string(id)
        if self.set_plugin(id):
            return action.nosync_return()
        return async.success(errors.doesnt_exist('plugin "%s"'%id,'open'))

    def __close(self,*arg):
        self.__host.close()
        return action.nosync_return()

    def __show(self,*arg):
        self.__host.set_showing(True)
        self.set_light(2,True)
        return action.nosync_return()

    def __unshow(self,*arg):
        self.__host.set_showing(False)
        self.set_light(2,False)
        return action.nosync_return()

    def __tog_show(self,*arg):
        self.__host.set_showing(not self.__host.is_showing())
        self.set_light(2,self.__host.is_showing())
        return action.nosync_return()

    def __status_show(self,*arg):
        return 'dsc(~(s)"#13","2")'

    def __status_bypass(self,*arg):
        return 'dsc(~(s)"#13","1")'

    def __bypass(self,*arg):
        self.__host.set_bypassed(True)
        self.set_light(1,True)
        return action.nosync_return()

    def __unbypass(self,*arg):
        self.__host.set_bypassed(False)
        self.set_light(1,False)
        return action.nosync_return()

    def __tog_bypass(self,*arg):
        self.__host.set_bypassed(not self.__host.is_bypassed())
        self.set_light(1,self.__host.is_bypassed())
        return action.nosync_return()

    def __window_state_changed(self,visible):
        self.__host.set_showing(visible)

    def set_plugin(self,id):
        desc = self.__browser.get_description(id)
        print 'set_plugin',id,desc
        if desc is not None:
            if self.__host.open(desc):
                self.__audio_input_channels.set_channels(self.__host.input_channel_count())
                self.__audio_output_channels.set_channels(self.__host.output_channel_count())
                self.__unbypass()
                self.__show()
                self.__set_title()
                return True
        return False

    def __plugin_state_loaded(self,delegate,state,desc_xml,show,bypassed,mapping,bounds):
        if desc_xml:
            desc = host_native.create_plugin_description(desc_xml)

            if desc:
                desc_checked = self.__browser.check_description(desc)

                if desc_checked and self.__host.open(desc_checked):
                    self.__host.set_state(state)
                    self.__host.set_showing(show)
                    self.__host.set_mapping(mapping)

                    if bounds:
                        self.__host.set_bounds(bounds)

                    self.__set_title()

                    if bypassed:
                        self.__bypass()
                    else:
                        self.__unbypass();

                    return

                delegate.add_error("Can't load " + desc.description());
            else:
                delegate.add_error("Can't load Audio Plugin");

        self.__host.close()

    def __set_title(self):
        if self.__host is None: return
        o = self.get_property_long('ordinal',0)
        desc = self.__host.get_description()
        t = '%s %s %d' % (desc.name(),desc.format(),o)
        self.__host.set_title(t)

    def property_change(self,key,value):
        if key == 'ordinal':
            self.__set_title()

    def agent_preload(self):
        self.agent_presave(None)

    def agent_presave(self,tag):
        state = self.__host.get_state()
        bounds = self.__host.get_bounds()
        self.__state.save_state(state,bounds)

    def set_light(self,ord,active):
        if active:
            self.lights.set_status(ord,const.status_active)
        else:
            self.lights.set_status(ord,const.status_inactive)

    def __set_tail_time(self,tt):
        if tt is None:
            self.__host.disable_idling()
        else:
            self.__host.set_idle_time(tt)
        return True

    def on_quit(self):
        self.__host.close()

class Upgrader(upgrade.Upgrader):

    def upgrade_1_0_2_to_1_0_3(self,tools,address):
        print 'upgrading plugin host',address
        root = tools.get_root(address)

        # ensure input atom names
        root.ensure_node(6,3).set_name('key input')

    def phase2_1_0_3(self,tools,address):
        root = tools.get_root(address)

        # hook up the key input
        root.mimic_connections((6,1),(6,3),'key output')

    def phase2_1_0_2(self,tools,address):
        return self.phase2_1_0_1(tools,address)

    def upgrade_1_0_1_to_1_0_2(self,tools,address):
        pass

    def phase2_1_0_2(self,tools,address):
        return self.phase2_1_0_1(tools,address)

    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        pass

    def phase2_1_0_1(self,tools,address):
        print 'upgrading plugin host',address
        root = tools.get_root(address)
        freq_input = root.ensure_node(6,2)
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

        canonical_names = {}
        available_sources = {}
        blocked_sources = []

        available_sources['pressure'] = paths.makeid_list(scaler_addr,1,2)
        available_sources['roll'] = paths.makeid_list(scaler_addr,1,3)
        available_sources['yaw'] = paths.makeid_list(scaler_addr,1,4)

        for (k,v) in available_sources.items():
            canonical_names[v] = k

        for i in range(5,15):
            recorder_input = recorder_root.get_node(1,i)
            if recorder_input:
                input_name = recorder_input.get_name()
                input_conn = recorder_input.get_master()
                output_id = paths.makeid_list(recorder_addr,2,i)
                if not input_conn:
                    blocked_sources.append(output_id)
                    print 'unavailable input',output_id
                    continue
                upstream_addr,upstream_path = paths.breakid_list(input_conn[0])
                upstream_root = tools.get_root(upstream_addr)
                if not upstream_root: continue
                upstream = upstream_root.get_node(*upstream_path)
                if not upstream: continue
                upstream_name = upstream.get_name().split()
                if 'output' in upstream_name: upstream_name.remove('output')
                upstream_name = ' '.join(upstream_name)
                if upstream_name == 'controller': continue
                print 'recorder output',i,'at',output_id,'called',input_name,'connected to',upstream_name
                available_sources[upstream_name] = output_id
                canonical_names[output_id] = upstream_name

        for n,id in available_sources.items():
            print 'source',n,'at',id

        current_parms = {}
        unused_parms = range(1,16)
        existing_names = []

        for k in range(1,16):
            parm_node = root.get_node(4,k)
            parm_name = parm_node.get_name()
            if not parm_node: continue
            parm_conn = parm_node.get_master()
            if not parm_conn: continue
            parm_master = parm_conn[0]

            if parm_master in canonical_names:
                parm_name = canonical_names[parm_master]
            else:
                split_name = parm_name.split()
                kstr = str(k)
                if split_name and split_name[-1]==kstr:
                    parm_name = ' '.join(split_name[:-1])

            if parm_master not in current_parms:
                current_parms[parm_master] = (k,parm_name)
                existing_names.append(parm_name)
                print 'existing source',k,'called',parm_name,'connected to',parm_master
                unused_parms.remove(k)

        for pname,pmaster in available_sources.items():
            if pmaster not in current_parms and pname not in existing_names:
                if unused_parms:
                    k = unused_parms[0]
                    unused_parms = unused_parms[1:]
                    current_parms[pmaster] = (k,pname)
                    print 'adding available source',k,pname,'from',pmaster


        print 'parameter map:'
        for pmaster,(k,pname) in current_parms.items():
            print 'parameter',k,'called',pname,'connected to',pmaster

        for k in range(1,17):
            parm_node = root.ensure_node(4,k)
            parm_node.set_meta('master',piw.data())
            parm_node.set_name('parameter %d' % k)
            
        for pmaster,(k,pname) in current_parms.items():
            if pmaster in blocked_sources:
                continue
            conn = logic.make_term('conn',None,None,pmaster,None)
            conn_txt = logic.render_term(conn)
            parm_node = root.ensure_node(4,k)
            parm_node.set_meta_string('master',conn_txt)
            parm_node.set_name(pname)
            print 'connected',parm_node.id(),'to',conn_txt,'called',pname,'parameter',k

        conn = logic.make_term('conn',None,None,paths.makeid_list(scaler_addr,1,2),None)
        conn_txt = logic.render_term(conn)
        parm_node = root.ensure_node(4,16)
        parm_node.set_meta_string('master',conn_txt)
        parm_node.set_name('key position')

        local_map = {}

        self.check_midi_controller(root,(6,4),7,current_parms,local_map) # volume
        self.check_midi_controller(root,(6,5),1,current_parms,local_map) # modwheel
        self.check_midi_controller(root,(6,6),10,current_parms,local_map) # pan
        self.check_midi_controller(root,(6,7),4,current_parms,local_map) # foot pedal
        self.check_midi_controller(root,(6,8),11,current_parms,local_map) # expression
        self.check_midi_controller(root,(6,9),130,current_parms,local_map) # channel pressure
        self.check_midi_controller(root,(6,10),128,current_parms,local_map) # poly pressure
        self.check_midi_controller(root,(6,11),131,current_parms,local_map) # pitch bend
        self.check_midi_controller(root,(6,12),64,current_parms,local_map) # sustain pedal

        print 'local map',local_map

        map_node = root.ensure_node(255,5)
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
            map_list.append(logic.make_term('c',p,cc,1.0,0,0,1.0,True,const.scope_pernote,res,ccl))

        map_term = logic.make_term('mapping',*map_list)
        map_node.set_data(piw.makestring(logic.render_term(map_term),0))

    def check_midi_controller(self,root,path,cc,controls,midi_map):
        input_node = root.get_node(*path)
        if not input_node:
            return

        input_conn = input_node.get_master()
        for i in input_conn:
            if i in controls:
                print 'converter input',cc,'connected to',i,'->',controls[i]
                midi_map[cc] = controls[i][0]
                return

    def upgrade_6_0_to_7_0(self,tools,address):
        root = tools.root(address)
        old1 = root.get_node(2,253)
        old2 = root.get_node(3,253)
        if old1 is not None:
            new1 = root.ensure_node(2,2)
            new1.copy(old1)
        if old2 is not None:
            new2 = root.ensure_node(3,2)
            new2.copy(old2)

    def upgrade_5_0_to_6_0(self,tools,address):
        root = tools.root(address)

        root.ensure_node(2,253,254).set_long(2)
        root.ensure_node(2,253,255.1)
        root.ensure_node(2,253,255.3)
        root.ensure_node(2,253,255.4).set_string('channel')
        root.ensure_node(2,253,255.8).set_string('count')
        root.ensure_node(2,1,255,8).set_string('')

        root.ensure_node(3,253,254).set_long(2)
        root.ensure_node(3,253,255.1)
        root.ensure_node(3,253,255.3)
        root.ensure_node(3,253,255.4).set_string('channel')
        root.ensure_node(3,253,255.8).set_string('count')
        root.ensure_node(3,1,255,8).set_string('')

        pmap = []
        for p in range(1,17):
            pnode = root.ensure_node(4,p)
            for n in pnode.iter():
                pbits = n.get_string().split(',')
                pmap.append(logic.make_term('m',p,int(pbits[2]),1.0))

            pnode.ensure_node(255,17)

        # replace 0 tail time (meaning, plugin value) with a sensible default
        n = root.ensure_node(10,254)
        if n.get_data().is_float() and n.get_data().as_float()==0.0:
            n.set_data(piw.makefloat(30.0,0))

        root.ensure_node(14,255.1)
        root.ensure_node(14,255.3)
        root.ensure_node(14,255.8).set_string('midi input')

        root.ensure_node(15,255.1)
        root.ensure_node(15,255.3)
        root.ensure_node(15,255.8).set_string('midi output')

        id_bits = root.ensure_node(255,6).get_string().split('|')
        plug_id = id_bits[0]
        plug_name = id_bits[1] if len(id_bits)>1 else ''
        au_bits = plug_id[0:4],plug_id[4:8],plug_id[8:12]

        what = ''
        if au_bits[0]=='aumu': what='Synths'
        if au_bits[0]=='aumf' or au_bits[0]=='aufx': what='Effects'
        if what:
            desc = host_native.plugin_description('AudioUnit:%s/%s' % (what,','.join(au_bits)),plug_name)
            root.ensure_node(255,6,3).set_string(urllib.quote(desc.to_xml()))
            root.ensure_node(255,6,4).set_data(root.get_node(9,254).get_data())
            root.ensure_node(255,6,5).set_string(logic.render_term(logic.make_term('mapping',*tuple(pmap))))

        root.remove(99)

        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        olddir = resource.user_resource_dir('audiounit',version=tools.oldrversion(address))
        priv = tools.root(address).get_node(255,6)
        if priv.get_data().is_string():
            bits = priv.get_data().as_string().split('|')
            if len(bits)>1:
                path = os.path.join(olddir,bits[1])
                if os.path.isfile(path):
                    print 'upgrading AU preset',bits[1]
                    state = open(path).read()
                    for i,c in preset_chunker(state):
                        priv.ensure_node(2,i).set_data(c)
                else:
                    print 'AU preset file',bits[1],'not found'
                id = bits[0]
                name = bits[2] if len(bits)==3 else 'unknown'
                priv.set_string('%s|%s' % (id,name))
            else:
                print 'AU has no preset defined in ',bits
        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        olist = tools.root(address).get_node(2)
        if olist is not None:
            for o in olist.iter():
                ord = o.path[-1]
                o.setmetalong(const.meta_ordinal,ord)
                o.setmeta(const.meta_names,'output bus')

        ilist = tools.root(address).get_node(3)
        if ilist is not None:
            for i in ilist.iter():
                ord = i.path[-1]
                i.setmetalong(const.meta_ordinal,ord)
                i.setmeta(const.meta_names,'input bus')

        return True

    def upgrade_2_0_to_3_0(self,tools,address):
        plist = tools.root(address).get_node(4)
        if plist is None:
            return True

        index = 1
        for p in plist.iter():
            priv = p.get_node(255,6)
            if priv is None:
                continue
            p.ensure_node(1).set_data(priv.get_data())
            p.setmetalong(const.meta_ordinal,index)
            p.setmeta(const.meta_names,'parameter')
            if index!=p.path[-1]:
                plist.ensure_node(index).copy(p)
                p.erase()
            index += 1

        return True
 
    def upgrade_1_0_to_2_0(self,tools,address):
        tools.root(address).ensure_node(99,255,const.meta_names).set_string('zzz')
        return True

    def upgrade_0_0_to_1_0(self,tools,address):
        tools.root(address).ensure_node(99,255,const.meta_names).set_string('plugin')
        return True

agent.main(Agent,Upgrader,gui=True)

