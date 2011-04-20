
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

from pi import agent,atom,domain,utils,paths,bundles,action,logic,node,upgrade,resource,files,policy,errors,const
from plg_macosx import audio_version as version

import piw
import picross
import audio_native
import urllib
import os

def escape(s):
    return urllib.quote(s)

def unescape(s):
    return urllib.unquote(s)

class Port(atom.Atom):
    def __init__(self,agent):
        self.agent=agent
        atom.Atom.__init__(self,domain=domain.String(),names='audio port',protocols='virtual browse',policy=atom.default_policy(self.__change_device),init='')
        self.__mapping = [] # (uid,name)
        self.__timestamp = piw.tsd_time()
        self.__selected=None
        self.__update()

    def __change_device(self,uid):
        self.agent.change_device(uid)
        return False

    def rpc_displayname(self,arg):
        return 'audio ports'

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.__selected=str(selected)
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.agent.change_device(str(selected))
        return logic.render_term(('',''))

    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def set_mapping(self,m):
        self.__mapping = m
        print 'mapping:',m
        self.__update()

    def set_current(self,uid):
        print 'current set to',uid
        self.set_value(uid)
        self.__update()

    def get_current(self):
        return self.get_value()

    def __ideal(self,uid):
        return '[ideal([~server,port],%s)]' % logic.render_term(escape(uid))

    def rpc_fideal(self,arg):
        (path,uid) = logic.parse_clause(arg)
        return 'ideal([~server,port],%s)' % logic.render_term(uid)

    def resolve_name(self,name):
        print 'port resolve_name',name
        if name == 'current':
            return self.__ideal(self.get_value())
        if name =='selection':
            name=self.__selected
            print 'name=selected',self.__selected
            return self.__ideal(self.__selected)

        try:
            o=int(name)
        except:
            return '[]'

        if o>0 and o<len(self.__mapping)+1:
            return self.__ideal(self.__mapping[o-1][0])

        return '[]'

    def rpc_current(self,arg):
        if not self.get_value():
            return '[]'
        return '[[%s,[]]]' % logic.render_term(escape(self.get_value()))

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'port:__resolve resolving virtual',arg,(a,o)
        if a == ('current',) and o is None and self.get_value:
            print 'current is',self.get_value()
            return self.__ideal(self.get_value())

        if a or not o:
            return '[]'

        o=int(o)

        if o>0 and o<len(self.__mapping)+1:
            return self.__ideal(self.__mapping[o-1][0])

        return '[]'

    def rpc_enumerate(self,a):
        print 'enumerating',a
        return logic.render_term((len(self.__mapping),0))

    def rpc_cinfo(self,a):
        print 'cinfo',a
        return '[]'

    def rpc_finfo(self,a):
        (dlist,cnum) = logic.parse_clause(a)
        map = tuple([(escape(uid),dsc,None) for (uid,dsc) in self.__mapping[cnum:]])
        print 'finfo',a,map
        return logic.render_term(map)


class AudioDelegate(audio_native.audioctl):
    def __init__(self,agent,dom):
        audio_native.audioctl.__init__(self,dom,'coreaudio')
        self.agent = agent

    def device_changed(self,uid,sr,bs):
        self.agent.device_changed(uid,sr,bs)

    def device_list_changed(self):
        self.agent.device_list_changed()


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='audio', icon='plg_macosx/audio.png',container=9,ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.clone = piw.clone(True)
        self.audio = AudioDelegate(self,self.domain)
        self.recorder = piw.wavrecorder(self.domain)
        self.clone.set_output(1,self.audio.cookie())
        self.clone.set_output(2,self.recorder.cookie())
        self.input = bundles.ScalarInput(self.clone.cookie(), self.domain,signals=(1,2),threshold=0)
        self.__loading = False

        self[1]=atom.Atom(domain=domain.BoundedFloat(-1,1),names='left input', policy=self.input.nodefault_policy(1,True))
        self[2]=atom.Atom(domain=domain.BoundedFloat(-1,1),names='right input', policy=self.input.nodefault_policy(2,True))
        self[3]=atom.Atom(domain=domain.EnumOrNull(44100,48000,96000), names='sample rate', protocols='bind set', policy=atom.default_policy(self.__change_sample_rate),container=(None,'sample rate',self.verb_container()))
        self[3].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',callback=self.__set_sample_rate)
        self[3].add_verb2(2,'set([un],~a,role(None,[instance(~self)]))',callback=self.__unset_sample_rate)
        self[7] = Port(self)
        self[8]=atom.Atom(domain=domain.BoundedIntOrNull(0,3072,0), names='buffer size', protocols='bind set', policy=atom.default_policy(self.__change_buffer_size),container=(None,'buffer size',self.verb_container()))
        self[8].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',callback=self.__set_buffer_size)
        self[8].add_verb2(2,'set([un],~a,role(None,[instance(~self)]))',callback=self.__unset_buffer_size)

        self.add_verb2(1,'mute([],None)', self.__mute)
        self.add_verb2(2,'mute([un],None)', self.__unmute)
        self.add_verb2(3,'choose([],None,role(None,[ideal([~server,port]),singular]))',self.__choose)

        self.enum()

        self.__filename = resource.new_resource_file('audio','audio.wav')
        self.recorder.setfile(self.__filename)

        self[10]=atom.Atom(domain=domain.Bool(),init=False,transient=True,policy=policy.FastPolicy(self.recorder.record(),policy.TriggerStreamPolicy()), names='recorder running input')

    def load_state(self,state,delegate,phase):
        self.__loading = True
        agent.Agent.load_state(self,state,delegate,phase)
        self.__loading = False
        self.open_device()

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.advertise('<audio>')
        self.audio.enable_callbacks(True)

    def close_server(self):
        agent.Agent.close_server(self)
        self.audio.enable_callbacks(False)
        self.audio.close_device()
        self.audio.show_gui(False)

    def on_quit(self):
        self.close_server()

    def __get_recording(self,arg):
        return files.get_ideal(self.id(),'audio',files.FileSystemFile(self.__filename,'audio'),0)
    
    def rpc_get_test_data(self,arg):
        print 'getting test data',self.__filename
        f = files.get_ideal(self.id(),'audio',files.FileSystemFile(self.__filename,'audio'),0) if os.path.exists(self.__filename) else '[]'
        print 'got recording',f
        d = self.audio.get_dropout_count()
        print 'got dropouts',d
        return logic.render_term((f,d))

    def rpc_reset_test_data(self,arg):
        print 'reset test data'
        try:
            os.unlink(self.__filename)
        except:
            pass
        self.audio.reset_dropout_count()

    def resolve_file_cookie(self,cookie):
        print 'resolve_file_cookie',cookie
        if cookie=='audio':
            return files.FileSystemFile(self.__filename,'audio')
        return agent.Agent.resolve_file_cookie(self,cookie)

    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)

        if type=='port':
            return self[7].resolve_name(' '.join(arg))

        return action.marshal(())

    def __choose(self,subj,arg):
        (type,thing) = action.crack_ideal(action.arg_objects(arg)[0])
        self.change_device(thing)
        return action.nosync_return()

    def change_device(self,uid):
        self[7].set_current(uid)
        if not self.__loading:
            self[3].set_value(None)
            self[8].set_value(None)
            self.open_device()

    def open_device(self):
        if not self.__loading and self.running():
            actual_uid = self[7].get_current() or ""
            actual_sr = self[3].get_value() or 0
            actual_bs = self[8].get_value() or 0
            print 'opening audio port',actual_uid,actual_sr,actual_bs
            self.audio.open_device(actual_uid,actual_sr,actual_bs,True)
 
    def enum(self):
        num=self.audio.num_devices()
        map=[]

        for i in range(0,num):
            name=self.audio.device_name(i)
            uid=self.audio.device_uid(i)
            map.append((uid,name))

        self[7].set_mapping(map)

    def __mute(self,subj):
        self.audio.mute()

    def __unmute(self,subj):
        self.audio.unmute()

    @utils.nothrow
    def device_list_changed(self):
        print 'device list changed'
        self.enum()

    @utils.nothrow
    def device_changed(self,uid,sr,bs):
        print 'device change notification from audio',uid,sr,bs
        self[3].set_value(sr or None)
        self[8].set_value(bs or None)
        self[7].set_current(uid or '')

    def __change_buffer_size(self,bs):
        print 'change buffer size',bs
        self[8].set_value(int(bs) if bs else None)
        self.open_device()
        return False
    
    def __unset_buffer_size(self,*args):
        print 'un setting buffer size rate',args
        self[8].set_value(None)
        self.open_device()
        return action.nosync_return()

    def __set_buffer_size(self,subj,_,bs):
        print 'set_buffer_size',subj,bs
        bs = action.abstract_string(bs)
        self[8].set_value(int(bs) if bs else None)
        self.open_device()
        return action.nosync_return()

    def __change_sample_rate(self,sr):
        print 'change sample rate',sr
        self[3].set_value(int(sr) if sr else None)
        self.open_device()
        return False
    
    def __unset_sample_rate(self,*args):
        print 'un setting sample rate',args
        self[3].set_value(None)
        self.open_device()
        return action.nosync_return()

    def __set_sample_rate(self,subj,_,sr):
        print 'set_sample_rate',subj,sr
        sr = action.abstract_string(sr)
        self[3].set_value(int(sr))
        self.open_device()
        return action.nosync_return()


class Upgrader(upgrade.Upgrader):
    def upgrade_4_0_to_5_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(3,255,const.meta_verb)
        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(9).erase_children()
        return True


agent.main(Agent,Upgrader,gui=True)
