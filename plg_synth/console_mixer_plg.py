
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

# -------------------------------------------------------------------------------------------------------------------------------------------
# console_mixer_plg.py
#
# Agent of a console style mixer with input channels and effect send/return channels.
# -------------------------------------------------------------------------------------------------------------------------------------------

import sys
import math
import piw
import picross
from pi import agent,atom,bundles,domain,async,action,upgrade,policy,node,container,utils,logic,const,errors,collection
from pi.logic.shortcuts import T
from plg_synth import console_mixer_version as version
import synth_native

num_inputs = 24
db_range = 70

def volume_function(f):
    if f<=0.01: return 0.0
    fn = f/100.0
    db = db_range*(1.0-fn)
    sc = pow(10.0,-db/20.0)
    return sc


pan_laws = {
    'equal power sine': lambda f: math.sin(((f+1.0)/2.0)*math.pi/2.0),
    'equal power square root': lambda f: math.sqrt((f+1.0)/2.0),
    'flattened linear': lambda f: (f+1.0) if f<0.0 else 1.0,
    'flattened sine': lambda f: math.sin((((f+1.0)*2.0)/2.0)*math.pi/2.0) if f<0.0 else 1.0,
    'flattened square root': lambda f: math.sqrt(f+1.0) if f<0.0 else 1.0,
    'linear': lambda f: (f+1.0)/2.0,
}

default_pan = 'linear'

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

# -------------------------------------------------------------------------------------------------------------------------------------------
# Effect send controls atom
# -------------------------------------------------------------------------------------------------------------------------------------------

class FxSendControls(atom.Atom):
    
    def __init__(self, chan_agent, cookie, name, ordinal_str, fx_chan_num, is_fx_chan):
        self.chan_agent = chan_agent
        self.fx_chan_num = fx_chan_num
        self.__is_fx_chan = is_fx_chan

        if ordinal_str!='':
            ordinal = int(ordinal_str)
        else:
            ordinal = None

        self.label = '%s %s' % (name,ordinal_str)
        atom.Atom.__init__(self, names=name, ordinal=ordinal)

        self.set_property_string('cname','effect')
        self.set_property_long('cordinal',fx_chan_num)

        # ------- effect send levels and enables ------- 
        # enable
        self[1] = atom.Atom(domain=domain.Bool(), init=False, names='enable', policy=atom.default_policy(self.__set_fx_send_enable))
        # send
        self.send_input = bundles.ScalarInput(cookie,chan_agent.main_agent.clk,signals=(1,))
        self[2] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('inc',1),T('biginc',10),T('control','updown'))), init=120, names='send', policy=self.send_input.notify_policy(1,policy.LopassStreamPolicy(1000,0.97),notify=self.chan_agent.main_agent.changes_pending), protocols='bind input')
        self[3] = atom.Atom(domain=domain.Bool(), init=False, names='prefader', policy=atom.default_policy(self.__set_fx_send_prefader))

    def __set_fx_send_enable(self, value):
        self.chan_agent.main_agent.mixer.set_fx_send_enable(value, self.chan_agent.get_chan_num()-1, self.fx_chan_num-1, self.__is_fx_chan)
        self.chan_agent.main_agent.changes_pending()
        
    def __set_fx_send_prefader(self, value):
        self.chan_agent.main_agent.mixer.set_fx_send_prefader(value, self.chan_agent.get_chan_num()-1, self.fx_chan_num-1, self.__is_fx_chan)
        self.chan_agent.main_agent.changes_pending()

        

# -------------------------------------------------------------------------------------------------------------------------------------------
# Effects send channel 
# -------------------------------------------------------------------------------------------------------------------------------------------

class FxChannel(atom.Atom):

    def __init__(self,main_agent,fx_chan_num):
        atom.Atom.__init__(self,names='effect',protocols='remove',ordinal=fx_chan_num)
        self.main_agent = main_agent
        self.fx_chan_num = fx_chan_num
           
        # audio send output
        self[3] = atom.Atom(names='outputs')
        self[3][1] = bundles.Output(1, True, names='left audio output')
        self[3][2] = bundles.Output(2, True, names='right audio output')
        self.send_output = bundles.Splitter(main_agent.clk, self[3][1], self[3][2])

        output_cookie = self.send_output.cookie()
        # create fx_chan_num-1 to start channel implementation index at 0
        input_cookie = main_agent.mixer.create_fx_channel(fx_chan_num-1, output_cookie)

        self.aggregator = piw.aggregator(input_cookie,main_agent.clk)

        self.__label = ''
        self.set_private(node.server(change=self.__setlabel,value=piw.makestring(self.__label,0)))

        self.control_input = bundles.ScalarInput(self.aggregator.get_output(1),main_agent.clk,signals=(1,2))        
        self[4] = atom.Atom(names='controls')
        self[4][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('inc',1),T('biginc',10),T('control','updown'))), init=100, names='volume', policy=self.control_input.notify_policy(1,policy.LopassStreamPolicy(1000,0.97),notify=main_agent.changes_pending), protocols='bind input')
        self[4][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('inc',0.02),T('biginc',0.2),T('control','updown'))), init=0, names='pan', policy=self.control_input.notify_policy(2,policy.LopassStreamPolicy(1000,0.97),notify=main_agent.changes_pending), protocols='bind input')
                
        # audio return input
        self.return_input = bundles.VectorInput(self.aggregator.get_output(2),main_agent.clk,signals=(1,2))
        self[1] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='left audio input',policy=self.return_input.vector_policy(1,True), protocols='obm')
        self[2] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='right audio input',policy=self.return_input.vector_policy(2,True), protocols='obm')

        # fx send controls
        self[5] = atom.Atom(names='effect send')

        self.set_id_data(None, None, str(fx_chan_num))

    def __setlabel(self,label):
        if label.is_string():
            self.set_label(label.as_string())

    def set_label(self,label):
        self.get_private().set_data(piw.makestring(label,0))
        self.__label = label
        self.main_agent.changes_pending()

    def inuse(self):
        return self[1].is_connected() or self[2].is_connected()

    def get_cinfo(self):
        if not self.__label:
            l = '%s %s' % (self.name,self.ordinal_str)
        else:
            l = self.__label
        return l+' fx'

    def __listener(self,veto,key,value):
        if not veto:
            if key == 'name':
                self.name = value.as_string()
                self.main_agent.changes_pending()

    def get_dinfo_pan(self):
        v = '%d' % self[4][2].get_value()
        return [(self.get_cinfo()+' pan',v)]

    def get_dinfo(self):
        v = '%d' % self[4][1].get_value()
        return [(self.get_cinfo()+' volume',v)]

    def get_dinfo_status(self):
        label = self.get_cinfo()
        l=[]
        l.append(('dinfo_id',self.get_cinfo()))
        l.append(('channel id','%d' % self.fx_chan_num))
        l.append(('channel volume','%d' % self[4][1].get_value()))
        l.append(('channel pan','%d' % self[4][2].get_value()))
        for fx in self[5].values():
            label = self.main_agent.fxchannels[fx.fx_chan_num].get_cinfo()
            l.append((label+' enable','yes' if fx[1].get_value() else 'no'))
            l.append((label+' send','%d' % fx[2].get_value()))

        return logic.render_term(T('keyval',tuple(l)))

    def disconnect(self):
        # remove send controls from input channels
        for k in self.main_agent.channels.iterkeys():
            self.main_agent.channels[k].remove_fx_send_ctrls(self.fx_chan_num)
        # remove send controls from fx channels
        for k in self.main_agent.fxchannels.iterkeys():
            if k!=self.fx_chan_num:
                self.main_agent.fxchannels[k].remove_fx_send_ctrls(self.fx_chan_num)

        self.main_agent.mixer.remove_fx_channel(self.fx_chan_num-1)

        self.main_agent.changes_pending()
        
    def add_fx_send_ctrls(self, key, name, ordinal, index):
        # add new atom with signals to the effects channel
        self[5][index] = FxSendControls(self, self.aggregator.get_output(2+index), name, ordinal, index, True)

    def remove_fx_send_ctrls(self, index):
        self.aggregator.clear_output(2+index)
        del self[5][index]

    def get_chan_num(self):
        return self.fx_chan_num

    def set_id_data(self, key, name, ordinal_str):
        if not name:
            name = ''

        if not key:
            if len(name) > 0:
                key = name+' '+ordinal_str
            else:
                key = ordinal_str

        self.key = key
        self.name = name
        self.ordinal_str = ordinal_str

        if len(name) > 0:
            self.set_names('effect '+name)
        else:
            self.set_names('effect')

        if ordinal_str!='':
            ordinal = int(ordinal_str)
            self.set_ordinal(ordinal)

    def get_id_data(self):
        return (self.key, self.name, self.ordinal_str, self.fx_chan_num)

# -------------------------------------------------------------------------------------------------------------------------------------------
# Effects send channel list
# -------------------------------------------------------------------------------------------------------------------------------------------

class FxChannelList(collection.Collection):

    def __init__(self,agent):
        self.__agent = agent
        self.__timestamp = piw.tsd_time()

        collection.Collection.__init__(self,names='effect channels',creator=self.__create_fxchannel,wrecker=self.__wreck_fxchannel,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __create_fxchannel(self, index):
        return FxChannel(self.__agent,index)
    
    def __wreck_fxchannel(self, index, node):
        node.disconnect()

    def create_fxchannel(self,o=None):
        o = o or self.find_hole()
        e = FxChannel(self.__agent,o)
        self[o] = e

        self.__connect_fxchannel(e)

        return e

    def create_named_fxchannel(self,key):
        # get list of strings
        key_words = key.split()
        # name can be anything, but check for an integer in last element and handle it like an ordinal
        last = key_words[len(key_words)-1]
        if last.isdigit():
            # strip off ordinal
            name = ' '.join(key_words[:len(key_words)-1])
            ordinal = last
        else:
            name = key
            ordinal = ''

        o = self.find_hole()
        e = FxChannel(self.__agent,o)
        self[o] = e

        # set id data - the key(=name+ordinal), name and ordinal
        e.set_id_data(key, name, ordinal)

        self.__connect_fxchannel(e)

        return e

    def __connect_fxchannel(self,channel):
        (key, name, ordinal, index) = channel.get_id_data()

        # add new fx chan to all other input chans
        for k in self.__agent.channels.iterkeys():
            self.__agent.channels[k].add_fx_send_ctrls(key, name, ordinal, index)

        # add new fx chan to all other fx chans and vice versa
        for k in self.iterkeys():
            if k!=index:
                self[k].add_fx_send_ctrls(key, name, ordinal, index)
                # get data of other channels
                (key2, name2, ordinal2, index2) = self[k].get_id_data()
                # add to this channel
                self[index].add_fx_send_ctrls(key2, name2, ordinal2, index2)

        self.__agent.changes_pending()

    def del_fxchannel(self,index):
        v = self[index]
        del self[index]
        v.disconnect()
    
    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        e=self.create_fxchannel(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,key,inst,ordinal):
        inst.disconnect()
        yield async.Coroutine.success()

# -------------------------------------------------------------------------------------------------------------------------------------------
# Input channel agent
# -------------------------------------------------------------------------------------------------------------------------------------------

class Channel(atom.Atom):
    def __init__(self,main_agent,chan_num):
        atom.Atom.__init__(self,names='mixer channel',ordinal=chan_num)
        self.main_agent = main_agent
        self.chan_num = chan_num 

        self.aggregator = piw.aggregator(main_agent.mixer.create_channel(chan_num-1),main_agent.clk)

        self.control_input = bundles.ScalarInput(self.aggregator.get_output(1),main_agent.clk,signals=(1,2))        
        self.audio_input = bundles.VectorInput(self.aggregator.get_output(2),main_agent.clk,signals=(1,2))

        self.__label = ''
        self.set_private(node.server(change=self.__setlabel,value=piw.makestring(self.__label,0)))
        
        self[1] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='left audio input', policy=self.audio_input.vector_policy(1,True), protocols='obm')
        self[2] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='right audio input', policy=self.audio_input.vector_policy(2,True), protocols='obm')

        self[3] = atom.Atom(names='controls')
        self[3][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('inc',1),T('biginc',10),T('control','updown'))), init=100, names='volume', policy=self.control_input.notify_policy(1,policy.LopassStreamPolicy(1000,0.97),notify=main_agent.changes_pending), protocols='bind input')
        self[3][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('inc',0.02),T('biginc',0.2),T('control','updown'))), init=0, names='pan', policy=self.control_input.notify_policy(2,policy.LopassStreamPolicy(1000,0.97),notify=main_agent.changes_pending), protocols='bind input')

        # fx send controls
        self[4] = atom.Atom(names='effect send')

    def __setlabel(self,label):
        if label.is_string():
            self.set_label(label.as_string())

    def set_label(self,label):
        self.get_private().set_data(piw.makestring(label,0))
        self.__label = label
        self.main_agent.changes_pending()

    def inuse(self):
        return self[1].is_connected() or self[2].is_connected()

    def get_cinfo(self):
        if not self.__label:
            l = 'channel %d' % self.chan_num
        else:
            l = self.__label
        return l

    def get_dinfo_pan(self):
        v = '%.1f' % self[3][2].get_value()
        return [(self.get_cinfo()+' pan',v)]

    def get_dinfo(self):
        v = '%d' % self[3][1].get_value()
        return [(self.get_cinfo()+' volume',v)]

    def get_dinfo_status(self):
        label = self.get_cinfo()
        l=[]
        l.append(('dinfo_id',self.get_cinfo()))
        l.append(('channel id','%d' % self.chan_num))
        l.append(('channel volume','%d' % self[3][1].get_value()))
        l.append(('channel pan','%.1f' % self[3][2].get_value()))
        for fx in self[4].values():
            label = self.main_agent.fxchannels[fx.fx_chan_num].get_cinfo()
            l.append((label+' enable','yes' if fx[1].get_value() else 'no'))
            l.append((label+' send','%.f' % fx[2].get_value()))

        return logic.render_term(T('keyval',tuple(l)))

    def disconnect(self):
        self[1].clear_connections()
        self[2].clear_connections()

        self.main_agent.mixer.remove_channel(self.chan_num-1)

        self.main_agent.changes_pending()

    def add_fx_send_ctrls(self, key, name, ordinal, index):
        # add new atom with signals to the effects channel
        self[4][index] = FxSendControls(self, self.aggregator.get_output(2+index), name, ordinal, index, False)

    def remove_fx_send_ctrls(self, index):
        self.aggregator.clear_output(2+index)
        del self[4][index]

    def get_chan_num(self):
        return self.chan_num

# -------------------------------------------------------------------------------------------------------------------------------------------
# Channel list
# -------------------------------------------------------------------------------------------------------------------------------------------

class ChannelList(atom.Atom):

    def __init__(self,agent):
        self.__agent = agent
        self.__timestamp = piw.tsd_time()

        atom.Atom.__init__(self,names='channels')
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def create_channel(self,o=None):
        o = o or self.find_hole()
        e = Channel(self.__agent,o)
        self[o] = e

        return e

    def del_channel(self,index):
        v = self[index]
        del self[index]
        v.disconnect()


# -------------------------------------------------------------------------------------------------------------------------------------------
# Main agent
# -------------------------------------------------------------------------------------------------------------------------------------------

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        # self[3] is the verb container
        agent.Agent.__init__(self, signature=version, names='console mixer', protocols='inputlist has_subsys browse', icon='plg_synth/mixer.png', container=3,ordinal=ordinal)

        self.clk = piw.clockdomain_ctl()

        pan_function = pan_laws[default_pan]

        # make vol and pan tables
        self.vol = piw.make_f2f_table(0,120,1000,picross.make_f2f_functor(volume_function))
        self.pan = piw.make_f2f_table(-1,1,1000,picross.make_f2f_functor(pan_function))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1, True, names='left audio output')
        self[1][2] = bundles.Output(2, True, names='right audio output')
        self.output = bundles.Splitter(self.clk, self[1][1], self[1][2])

        self.mixer = piw.consolemixer(self.vol,self.pan,self.clk,self.output.cookie())

        sh=(T('choices',*pan_laws.keys()), T('control','selector'))
        self[10] = atom.Atom(names='pan curve',domain=domain.String(hints=sh),init='default',policy=atom.default_policy(self.__set_pan))

        self.master_controls_input = bundles.ScalarInput(self.mixer.master_controls_cookie(),self.clk,signals=(1,2))

        self[2] = atom.Atom(names='master')
        self[2][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('inc',1),T('biginc',10),T('control','updown'))),
                            init=100, names='master volume',
                            policy=self.master_controls_input.notify_policy(1,policy.LopassStreamPolicy(100,0.97),notify=self.changes_pending))
        self[2][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('inc',0.02),T('biginc',0.2),T('control','updown'))),
                            init=0, names='master pan',
                            policy=self.master_controls_input.policy(2,policy.LopassStreamPolicy(100,0.97)))

        self[3] = ChannelList(self)
        self[4] = FxChannelList(self)

        self.channels = self[3]
        self.fxchannels = self[4]

        # add channels numbered 1..24, which means effect channels cannot be called 1..24 ...
        for n in range(0,num_inputs):
            self.channels.create_channel(n+1)
            
        # verbs
        # verb to create a named effect channel
        self.add_verb2(1,'create([],None,role(None,[abstract,matches([effect])]), role(called,[abstract]))',self.__create_named_fx_chan)
        self.add_verb2(2,'create([un],None,role(None,[abstract,matches([effect])]), role(called,[abstract]))',self.__uncreate_named_fx_chan)

        self.add_verb2(3,'label([],None,role(None,[mass([channel])]),role(to,[abstract]))', self.__label)
        self.add_verb2(4,'label([un],None,role(None,[mass([channel])]))', self.__unlabel)
        self.add_verb2(5,'label([],None,role(None,[mass([effect])]),role(to,[abstract]))', self.__labelfx)
        self.add_verb2(6,'label([un],None,role(None,[mass([effect])]))', self.__unlabelfx)
        
        self.__timestamp = piw.tsd_time()
        self.__selected=None

        self.__pending = True
        self.__thing = piw.thing()
        piw.tsd_thing(self.__thing)
        self.__thing.set_slow_timer_handler(utils.notify(self.update))
        self.__thing.timer_slow(500)

    def __set_pan(self,v):
        if v not in pan_laws:
            return errors.doesnt_exist(v,'set')

        pan_function = pan_laws[v]
        self.pan = piw.make_f2f_table(-1,1,1000,picross.make_f2f_functor(pan_function))
        print 'set pan law',v,self.pan(-1),self.pan(0),self.pan(1)
        self.mixer.set_curves(self.vol,self.pan)

    def changes_pending(self):
        self.__pending = True

    def __labelfx(self,subj,channel,label):
        channel = int(action.mass_quantity(channel))
        label = action.abstract_string(label)
        if channel in self.fxchannels:
            self.fxchannels[channel].set_label(label)
        self.changes_pending()

    def __unlabelfx(self,subj,channel):
        channel = action.mass_quantity(channel)
        if channel in self.fxchannels:
            self.fxchannels[channel].set_label('')
        self.changes_pending()

    def __label(self,subj,channel,label):
        channel = int(action.mass_quantity(channel))-1
        label = action.abstract_string(label)
        if channel in self.channels:
            self.channels[channel].set_label(label)
        self.changes_pending()

    def __unlabel(self,subj,channel):
        channel = action.mass_quantity(channel)-1
        if channel in self.channels:
            self.channels[channel].set_label('')
        self.changes_pending()

    def update(self):
        if self.__pending:
            self.__pending = False
            self.__timestamp = self.__timestamp+1
            self.set_property_string('timestamp',str(self.__timestamp))
            self.channels.update()
            self.fxchannels.update()

    def __channels(self):
        channels = [c for c in self.channels.itervalues() if c.inuse()]
        channels.extend([c for c in self.fxchannels.itervalues() if c.inuse()])
        return channels

    def rpc_enumerate(self,arg):
        path=logic.parse_clause(arg)
        if len(path)==0:
            channels = self.__channels()
            return logic.render_term((0,1+len(channels)))
        return logic.render_term((0,0))

    def rpc_cinfo(self,arg):
        (path,idx) = logic.parse_clause(arg)

        if len(path):
            return '[]'

        channels = [c.get_cinfo() for c in self.__channels()]
        channels.append('Pan Settings')
        return render_list(channels,idx,lambda i,t: logic.render_term(t))

    def rpc_finfo(self,arg):
        return '[]'

    def rpc_dinfo(self,arg):
        path=logic.parse_clause(arg)

        if len(path)==0 or len(path)>1:
            return self.get_dinfo()

        l = path[0]

        if l == 'Pan Settings':
            return self.get_dinfo_pan()

        for c in self.__channels():
            if c.get_cinfo()==l:
                return c.get_dinfo_status()

        return self.get_dinfo()

    def get_dinfo_pan(self):
        l=[]
        dsc=self.get_description()
        l.append(('dinfo_id',dsc))
        k = 'master'
        v = '%.1f' % self[2][2].get_value()
        l.append((k+' pan',v))

        l2=[]

        for c in self.channels.values():
            if c.inuse():
                l2.extend(c.get_dinfo_pan())

        l3=[]

        for c in self.fxchannels.values():
            if c.inuse():
                l3.extend(c.get_dinfo_pan())

        l2.sort()
        l3.sort()

        return logic.render_term(T('keyval',tuple(l+l2+l3)))
    
    def get_dinfo(self):
        l=[]
        dsc=self.get_description()
        l.append(('dinfo_id',dsc))
        k = 'master'
        v = '%d' % self[2][1].get_value()
        l.append((k+' volume',v))

        l2=[]

        for c in self.channels.values():
            if c.inuse():
                l2.extend(c.get_dinfo())

        l3=[]

        for c in self.fxchannels.values():
            if c.inuse():
                l3.extend(c.get_dinfo())

        l2.sort()
        l3.sort()

        return logic.render_term(T('keyval',tuple(l+l2+l3)))
    
    def rpc_current(self,arg):
        uid=0
        return '[[%d,[]]]' % uid

    def rpc_setselected(self,arg):
        pass

    def rpc_activated(self,arg):
        return logic.render_term(('',''))


    def rpc_addinput(self,dummy):
        for (k,v) in self.iter_subsys_items():
            if not v.inuse():
                return async.success(action.marshal((v.id(),False)))

        return async.failure('no free mixer channels')

    def rpc_delinput(self,id):
        for (k,v) in self.iter_subsys_items():
            if id=='' or v.id()==id:
                v.disconnect()

    def rpc_lstinput(self,dummy):
        r=tuple([s.id() for (k,s) in self.iter_subsys_items() if s.inuse()])
        return async.success(action.marshal(r))


    def __create_named_fx_chan(self,subject,dummy,tags):
        key = action.abstract_string(tags)

        for v in self.fxchannels.itervalues():
            if key == v.get_id_data()[0]:
                return async.failure('Console Mixer: effect channel %s already exists' % key)

        new_fx_chan = self.fxchannels.create_named_fxchannel(key)

        return action.concrete_return(new_fx_chan.id())

    def __uncreate_named_fx_chan(self,subject,dummy,tags):
        key = action.abstract_string(tags)

        for k,v in self.fxchannels.iteritems():
            if key == v.get_id_data()[0]:
                self.fxchannels.del_fxchannel(k)
                break
        

class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        channels = tools.get_subsystems(address)
        for i in channels:
            tools.delete_agent(i)

        tools.invalidate_connections()

    def upgrade_1_0_to_2_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(255,6)
        return True
        
        

agent.main(Agent,Upgrader)

# -------------------------------------------------------------------------------------------------------------------------------------------
