
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
from . import console_mixer_version as version,synth_native

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
    
    def __init__(self, channel, fx_chan_num, is_fx_chan):
        self.channel = channel
        self.fx_chan_num = fx_chan_num
        self.__is_fx_chan = is_fx_chan

        atom.Atom.__init__(self, names='effect', ordinal=fx_chan_num)

        cookie = channel.aggregator.get_output(2+fx_chan_num)

        # ------- effect send levels and enables ------- 
        # enable
        self[1] = atom.Atom(domain=domain.Bool(), init=False, names='enable', policy=atom.default_policy(self.__set_fx_send_enable))
        # send
        self.send_input = bundles.ScalarInput(cookie,channel.main_agent.clk,signals=(1,))
        self[2] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('stageinc',1),T('inc',1),T('biginc',10),T('control','updown'))), init=100, names='send', policy=self.send_input.notify_policy(1,policy.LopassStreamPolicy(1000,0.97),notify=self.channel.main_agent.changes_pending), protocols='bind input')
        self[3] = atom.Atom(domain=domain.Bool(), init=False, names='prefader', policy=atom.default_policy(self.__set_fx_send_prefader))

    def property_veto(self,key,value):
        if atom.Atom.property_veto(self,key,value):
            return True

        return key in ['name','ordinal']

    def update_name(self,name,ordinal):
        self.set_property_string('name',name,allow_veto=False)
        self.set_property_long('ordinal',ordinal,allow_veto=False)

    def disconnect(self):
        self.channel.aggregator.clear_output(2+self.fx_chan_num)

    def __set_fx_send_enable(self, value):
        self.channel.main_agent.mixer.set_fx_send_enable(value, self.channel.get_chan_num()-1, self.fx_chan_num-1, self.__is_fx_chan)
        self.channel.main_agent.changes_pending()
        
    def __set_fx_send_prefader(self, value):
        self.channel.main_agent.mixer.set_fx_send_prefader(value, self.channel.get_chan_num()-1, self.fx_chan_num-1, self.__is_fx_chan)
        self.channel.main_agent.changes_pending()
            
# -------------------------------------------------------------------------------------------------------------------------------------------
# Effect send controls list atom
#
# Since effects sends are created as a side effect of creating fx channels,
# they might not exist when the setup for a given channel is loaded.  This
# defers the loading of the send setup until after the rest of the agent is
# configured.
#
# The dynlist flag signals the setup manager that this node will change as
# a side effect, causing it to send the whole setup for this sub tree rather
# than just setup for existing nodes.
#
# -------------------------------------------------------------------------------------------------------------------------------------------

class FxSendControlsList(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,names='effect send',dynlist=True)

    def load_state(self,state,delegate,phase):
        if phase == 1:
            delegate.set_deferred(self,state)
            return async.success()

        return atom.Atom.load_state(self,state,delegate,phase-1)

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

        self.control_input = bundles.ScalarInput(self.aggregator.get_output(1),main_agent.clk,signals=(1,2))        
        self[4] = atom.Atom(names='controls')
        self[4][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('stageinc',1),T('inc',1),T('biginc',10),T('control','updown'))), init=100, names='volume', policy=self.control_input.notify_policy(1,False,notify=main_agent.changes_pending), protocols='bind input')
        self[4][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('stageinc',0.1),T('inc',0.02),T('biginc',0.2),T('control','updown'))), init=0, names='pan', policy=self.control_input.notify_policy(2,False,notify=main_agent.changes_pending), protocols='bind input')
                
        # audio return input
        self.return_input = bundles.VectorInput(self.aggregator.get_output(2),main_agent.clk,signals=(1,2))
        self[1] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='left audio input',policy=self.return_input.vector_policy(1,True), protocols='obm')
        self[2] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='right audio input',policy=self.return_input.vector_policy(2,True), protocols='obm')

        # fx send controls
        self[5] = FxSendControlsList()

    def inuse(self):
        return self[1].is_connected() or self[2].is_connected()

    def get_cinfo(self):
        l = self.get_description()
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

    def property_change(self,k,v,delegate):
        if k in [ 'name','ordinal' ]:
            print 'property change',self.fx_chan_num,k,v
            for k,v in self.main_agent.channels.iteritems():
                v.update_fx_send_controls(self.fx_chan_num)

            for k,v in self.main_agent.fxchannels.iteritems():
                if k!=self.fx_chan_num:
                    v.update_fx_send_controls(self.fx_chan_num)
            

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
        for k,v in self.main_agent.channels.iteritems():
            v.remove_fx_send_ctrls(self.fx_chan_num)

        for k,v in self.main_agent.fxchannels.iteritems():
            if k!=self.fx_chan_num:
                v.remove_fx_send_ctrls(self.fx_chan_num)

        self.main_agent.mixer.remove_fx_channel(self.fx_chan_num-1)

        self.main_agent.changes_pending()
        
    def add_fx_send_ctrls(self, index):
        self[5][index] = FxSendControls(self, index, True)
        self.update_fx_send_controls(index)

    def update_fx_send_controls(self, index):
        if index != self.fx_chan_num:
            c = self.main_agent.fxchannels[index]
            o = c.get_property_long('ordinal',None)
            n = c.get_property_string('name',None)
            print 'fx rename',index,n,o
            self[5][index].update_name(n,o)
            self.main_agent.changes_pending()

    def remove_fx_send_ctrls(self, index):
        self[5][index].disconnect()
        del self[5][index]

    def plumb_fx_send_ctrls(self):
        for k,v in self.main_agent.channels.iteritems():
            v.add_fx_send_ctrls(self.fx_chan_num)

        for k,v in self.main_agent.fxchannels.iteritems():
            if k!=self.fx_chan_num:
                v.add_fx_send_ctrls(self.fx_chan_num)
                self.add_fx_send_ctrls(k)

        self.main_agent.changes_pending()

    def get_chan_num(self):
        return self.fx_chan_num

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
        channel = FxChannel(self.__agent,index)
        self[index] = channel
        channel.plumb_fx_send_ctrls()
        return channel
    
    def __wreck_fxchannel(self, index, node):
        node.disconnect()

    def create_named_fxchannel(self,name,ordinal):
        o = self.find_hole()
        e = FxChannel(self.__agent,o)
        self[o] = e
        e.plumb_fx_send_ctrls()
        e.set_ordinal(ordinal)
        e.set_names(name)
        return e

    def del_fxchannel(self,index):
        v = self[index]
        del self[index]
        v.disconnect()
    
    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        o = self.find_hole()
        e = FxChannel(self.__agent,o)
        self[o] = e
        e.plumb_fx_send_ctrls()
        e.set_ordinal(int(ordinal))
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

        self[1] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='left audio input', policy=self.audio_input.vector_policy(1,True), protocols='obm')
        self[2] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='right audio input', policy=self.audio_input.vector_policy(2,True), protocols='obm')

        self[3] = atom.Atom(names='controls')
        self[3][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('stageinc',1),T('inc',1),T('biginc',10),T('control','updown'))), init=100, names='volume', policy=self.control_input.notify_policy(1,policy.LopassStreamPolicy(1000,0.97),notify=main_agent.changes_pending), protocols='bind input')
        self[3][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('stageinc',0.1),T('inc',0.02),T('biginc',0.2),T('control','updown'))), init=0, names='pan', policy=self.control_input.notify_policy(2,policy.LopassStreamPolicy(1000,0.97),notify=main_agent.changes_pending), protocols='bind input')

        # fx send controls
        self[4] = FxSendControlsList()

    def inuse(self):
        return self[1].is_connected() or self[2].is_connected()

    def get_cinfo(self):
        l = self.get_description()
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

    def add_fx_send_ctrls(self, index):
        # add new atom with signals to the effects channel
        self[4][index] = FxSendControls(self, index, False)

    def update_fx_send_controls(self, index):
        c = self.main_agent.fxchannels[index]
        o = c.get_property_long('ordinal',None)
        n = c.get_property_string('name',None)
        print 'rename',index,n,o
        self[4][index].update_name(n,o)
        self.main_agent.changes_pending()

    def remove_fx_send_ctrls(self, index):
        self[4][index].disconnect()
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
        agent.Agent.__init__(self, signature=version, names='console mixer', protocols='inputlist has_subsys oldbrowse', ordinal=ordinal)

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

        self[10] = atom.Atom(names='pan curve',domain=domain.StringEnum(*sorted(pan_laws.keys())),init='default',policy=atom.default_policy(self.__set_pan))

        self.master_controls_input = bundles.ScalarInput(self.mixer.master_controls_cookie(),self.clk,signals=(1,2))

        self[2] = atom.Atom(names='master')
        self[2][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('stageinc',1),T('inc',1),T('biginc',10),T('control','updown'))),
                            init=100, names='master volume',
                            policy=self.master_controls_input.notify_policy(1,policy.LopassStreamPolicy(100,0.97),notify=self.changes_pending))
        self[2][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('stageinc',0.1),T('inc',0.02),T('biginc',0.2),T('control','updown'))),
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
        self.add_verb2(1,'create([],None,role(None,[abstract,matches([effect])]), option(called,[abstract]))',self.__create_fx_chan)
        self.add_verb2(2,'create([un],None,role(None,[concrete,singular,partof(~(a)#"4")]))', self.__uncreate_fx_chan)

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


    def __create_fx_chan(self,subject,dummy,tags):
        name = 'effect'
        ordinal = 0

        if tags:
            name_words = action.abstract_string(tags).split()
            last = name_words[len(name_words)-1]

            if last.isdigit():
                ordinal = int(last)
                if name_words:
                    name = ' '.join(name_words[:len(name_words)-1])
            else:
                name = ' '.join(name_words)
        else:
            ordinal = self.fxchannels.freeinstance()
            

        if False:
            return async.failure('Console Mixer: effect channel %s already exists' % key)

        new_fx_chan = self.fxchannels.create_named_fxchannel(name,ordinal)
        return action.concrete_return(new_fx_chan.id())

    def __uncreate_fx_chan(self,subject,chan):
        a = action.concrete_object(chan)

        for k,v in self.fxchannels.iteritems():
            if v.id() == a:
                self.fxchannels.del_fxchannel(k)
                return
        
        return async.failure('Console Mixer: effect channel doesnt exist')


agent.main(Agent)

# -------------------------------------------------------------------------------------------------------------------------------------------
