
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
from pi.logic.shortcuts import T
from . import audio_unit_version as version, host_native

import piw
import urllib
import sys
import os
import operator
import glob
import shutil
import zlib
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

    __plugins_cache = os.path.join(resource.user_resource_dir(resource.plugins_dir,version=''),'plugins_cache')

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

def prepad_not_empty(a):
    if len(a):
        return ' '+a
    return a

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
        c = self.__agent.get_title()
        if c is None:
            return '[]'
        return '[["%s",[]]]' % c

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
                return render_list(p,idx,lambda i,t: logic.render_term((plugin_id_escaped(t),t.name()+' ('+t.format()+prepad_not_empty(t.category())+')',t.description())))
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
        self.__state_loaded = False

    @async.coroutine('internal error')
    def load_state(self,state,delegate,phase):
        yield node.server.load_state(self,state,delegate,phase)
        self.__state_loaded = True

    def apply_state(self):
        if self.__state_loaded:
            show = self[1].get_data().as_bool() if self[1].get_data().is_bool() else False
            state = self[2].get_blob()
            desc = urllib.unquote(self[3].get_data().as_string()) if self[3].get_data().is_string() else ''
            bypassed = self[4].get_data().as_bool() if self[4].get_data().is_bool() else False
            mapping = self[5].get_data().as_string() if self[5].get_data().is_string() else '[]'
            bounds = self[6].get_blob()
            self.__load_callback(state,desc,show,bypassed,mapping,bounds)

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
        self.__audio_output = audio.AudioOutput(bundles.Splitter(self.__domain),1,2,names='channels')
        self.__audio_output_channels = audio.AudioChannels(self.__audio_output)
        self.__midi_output = bundles.Splitter(self.__domain)
        self.__observer = PluginObserver(self.__state,self)
        self.__channel_delegate = MidiChannelDelegate(self)
        self.__host = host_native.plugin_instance(
            self.__observer, self.__channel_delegate,
            self.__domain,self.__audio_output.cookie(),self.__midi_output.cookie(),
            utils.statusify(self.__window_state_changed))
        self.parameter_list = inputparameter.List(self.__host,self.__host.clock_domain(),self.verb_container())
        self.__audio_input = audio.AudioInput(bundles.ScalarInput(self.__host.audio_input_cookie(),self.__domain,signals=range(1,65)),1,2,names='channels')
        self.__audio_input_channels = audio.AudioChannels(self.__audio_input)
        self.__key_input = bundles.VectorInput(self.__host.midi_from_belcanto_cookie(),self.__domain,signals=(1,2))
        self.__midi_input = bundles.ScalarInput(self.__host.midi_aggregator_cookie(),self.__domain,signals=(1,))
        self.__metronome_input = bundles.ScalarInput(self.__host.metronome_input_cookie(),self.__domain,signals=(1,2,3,4))
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
        self[6] = atom.Atom(names='musical inputs')
        self[6][1] = atom.Atom(domain=domain.Aniso(),policy=self.__key_input.vector_policy(1,False),names='pressure input')
        self[6][2] = atom.Atom(domain=domain.Aniso(),policy=self.__key_input.merge_nodefault_policy(2,False),names='frequency input')

        # midi channel 
        self[7] = atom.Atom(domain=domain.BoundedInt(0,16),init=0,names='midi channel',policy=atom.default_policy(self.set_midi_channel))

        # velocity curve control
        vel=(T('stageinc',0.1),T('inc',0.1),T('biginc',1),T('control','updown'))
        self[8] = atom.Atom(names='velocity curve controls')
        self[8][1] = atom.Atom(domain=domain.BoundedInt(1,1000),init=4,names='velocity sample',policy=atom.default_policy(self.__set_velocity_samples))
        self[8][2] = atom.Atom(domain=domain.BoundedFloat(0.1,10,hints=vel),init=4,names='velocity curve',policy=atom.default_policy(self.__set_velocity_curve))
        self[8][3] = atom.Atom(domain=domain.BoundedFloat(0.1,10,hints=vel),init=4,names='velocity scale',policy=atom.default_policy(self.__set_velocity_scale))

        self[9] = atom.Atom(domain=domain.Bool(),init=True,names='tail time enable',policy=atom.default_policy(self.__set_tail_time_enabled))
        self[10] = atom.Atom(domain=domain.BoundedFloatOrNull(0,100000),init=10,names='tail time',policy=atom.default_policy(self.__set_tail_time))
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

        # controller input/output to add labels
        self[16] = atom.Atom(domain=domain.Aniso(hints=(logic.make_term('continuous'),)), names='controller output', policy=atom.readonly_policy())

        self[17] = atom.Atom(domain=domain.String(), init='Plugin', names='label category', policy=atom.default_policy(self.__set_category))
        self[18] = atom.Atom(domain=domain.String(), init='', names='label', policy=atom.default_policy(self.__set_label))

        self.ctl_functor = piw.functor_backend(1, True)
        self.ctl_input = bundles.VectorInput(self.ctl_functor.cookie(), self.__domain, signals=(1,))
        self[19] = atom.Atom(domain=domain.Aniso(), policy=self.ctl_input.vector_policy(1,False), names='controller input')
        self.ctl_functor.set_functor(piw.pathnull(0), utils.make_change_nb(piw.slowchange(utils.changify(self.__controller_input))))

        self.__ctl = []

        # verbs
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
        self.__host.set_velocity_samples(samples)
        return True

    def __set_velocity_curve(self,curve):
        self.__host.set_velocity_curve(curve)
        return True

    def __set_velocity_scale(self,scale):
        self.__host.set_velocity_scale(scale)
        return True

    def __open(self,arg,id):
        id = action.abstract_string(id)
        if self.set_plugin(id):
            return action.nosync_return()
        return async.success(errors.doesnt_exist('plugin "%s"'%id,'open'))

    def __close(self,*arg):
        self.__host.close()
        self.__update_labels()
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
                self.__update_labels()
                return True
        return False

    def __plugin_state_loaded(self,state,desc_xml,show,bypassed,mapping,bounds):
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

        self.__host.close()

    def __set_title(self):
        if self.__host is None: return
        e = self.get_enclosure()
        d = self.get_description().title()
        desc = self.__host.get_description()
        if e:
            t = '%s %s (%s)' % (desc.name(),d,e.title())
        else:
            t = '%s %s' % (desc.name(),d)
        self.__host.set_title(t)

    def get_title(self):
        if self.__host is None:return
        desc = self.__host.get_description()
        t = '%s %s' % (desc.name(),desc.format())
        return t 

    def property_change(self,key,value,delegate):
        if key in ['name','ordinal']:
            self.__set_title()

    def set_enclosure(self,enclosure):
        agent.Agent.set_enclosure(self,enclosure)
        self.__set_title()

    def agent_preload(self,filename):
        self.__host.close()

    def agent_postload(self,filename):
        self.__state.apply_state()
        agent.Agent.agent_postload(self,filename)

    def agent_presave(self,filename):
        state = self.__host.get_state()
        bounds = self.__host.get_bounds()
        self.__state.save_state(state,bounds)

    def set_light(self,ord,active):
        if active:
            self.lights.set_status(ord,const.status_active)
        else:
            self.lights.set_status(ord,const.status_inactive)

    def __set_tail_time_enabled(self,v):
        self[9].set_value(v)
        self.__host.enable_idling(v)
        return True

    def __set_tail_time(self,tt):
        self[10].set_value(tt)
        self.__host.set_idle_time(tt or 0)
        return True

    def on_quit(self):
        self.__host.close()
 
    def __set_category(self,v):
        if v != self[17].get_value():
            self[17].set_value(v)
            self.__update_labels()
 
    def __set_label(self,v):
        if v != self[18].get_value():
            self[18].set_value(v)
            self.__update_labels()

    def __controller_input(self,c):
        self.__ctl = utils.dict_items(c);
        self.__update_labels()

    def __update_labels(self):
        # extract all control stream entries into a new list
        # extract the labels entry into a deticated list, if it exists
        new_ctl = []
        labels = []
        for e in self.__ctl:
            if e[0] == 'labels':
                labels = utils.tuple_items(e[1])
            else:
                new_ctl.append(e)

        # append the local category and label to the labels list
        category = self[17].get_value()
        label = self[18].get_value()
        if len(label) == 0 and self.__host is not None and self.__host.has_plugin():
            desc = self.__host.get_description()
            if desc is not None:
                label = desc.name()

        if category and len(category) > 0 and label and len(label) > 0:
            labels.append(utils.maketuple([piw.makestring(category,0), piw.makestring(label,0)],0))

        # reintegrate the labels list and set the new control stream value
        new_ctl.append(['labels',utils.maketuple(labels,0)])
        self[16].set_value(utils.makedict(new_ctl,0))

class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_3_to_1_0_4(self,tools,address):
        pass

    def phase2_1_0_4(self,tools,address):
        print 'upgrading host',address
        root = tools.get_root(address)
        key_input = root.get_node(6,3)
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

agent.main(Agent,Upgrader,gui=True)

