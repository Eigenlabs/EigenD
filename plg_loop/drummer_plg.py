
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
import loop_native
from pi import agent,atom,domain,errors,action,bundles,async,utils,resource,logic,node,upgrade,const,paths
from plg_loop import drummer_version as version
from plg_loop import loopdb

from pi.logic.shortcuts import T
import sys
import os
response_size = 1200

class Updater:
    def __init__(self):
        self.__thing = piw.thing()
        piw.tsd_thing(self.__thing)
        self.__queue = set()
        self.__thing.set_slow_timer_handler(utils.notify(self.__timer))
        self.__thing.timer_slow(500)

    def trigger(self,*f):
        self.__queue.update(f)

    def __timer(self):
        q = self.__queue
        self.__queue = set()
        for f in q:
            try: f()
            except: pass

def render_list(list,offset,renderer):
    txt='['

    for n,l in enumerate(list[offset:]):
        ltxt = '' if not n else ','
        ltxt = ltxt + renderer(n+offset,l)
        if len(txt+ltxt) > response_size-1:
            return txt+']'
        txt=txt+ltxt

    return txt+']'

class LoopBrowser(atom.Atom):
    def __init__(self,agent):
        self.agent = agent

        atom.Atom.__init__(self,names='loop',protocols='virtual browse')
        
        self.__timestamp = piw.tsd_time()
        self.__selected=None
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __ideals(self,*indices):
        return '[%s]' % ','.join([self.__ideal(c) for c in indices])

    def __ideal(self,index):
        return 'ideal([~server,loop],%d)' % index

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        index = self.find_cookie(cookie)
        if index is None:
            return async.failure('invalid cookie')
        return self.__ideal(index)

    def resolve_name(self,name):
        if name == 'current':
            c = self.agent.get_current()
            if c is None:
                return self.__ideals()
            return self.__ideals(c)

        if name== 'selection':
            c=self.__selected
            if c is None:
                return self.__ideals()
            return self.__ideals(int(c))    

        c =self.agent.loopdb.id(name)
        if c is not None:
            return self.__ideals(c)

        try:
            id=int(name)
            if id>=0 and id<self.agent.loopdb.size():
                return self.__ideals(id)
        except:
            pass

        return self.__ideals()

    def rpc_current(self,arg):
        c = self.agent.get_current()
        if c is None:
            return '[]'
        return '[[%d,[]]]' % c

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.__selected=selected
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.agent.activate_loop(int(selected))
        return logic.render_term(('',''))

    def rpc_displayname(self,arg):
        x =self.agent[5].current_voice()
        if x:
            x='drummer voice '+ str(x.voice) + ' loops'
            
        else:
            x='drummer loops'
        return x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)

        if a == ('current',) and o is None:
            c = self.agent.get_current()
            if c is None:
                return self.__ideals()
            return self.__ideals(c)

        if not a and o is not None:
            try:
                id=int(o)
                if id>=0 and id<self.agent.loopdb.size():
                    return self.__ideals(id)
            except:
                pass

        return self.__ideals()

    def rpc_enumerate(self, a):
        p = logic.parse_clause(a)
        return logic.render_term(self.agent.loopdb.enumerate(p))

    def rpc_cinfo(self,a):
        (path,idx) = logic.parse_clause(a)
        r = self.agent.loopdb.cinfo(path)
        return render_list(r,idx,lambda i,t: logic.render_term((str(t[0]))))

    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)
        r = self.agent.loopdb.finfo(path)
        return render_list(r,idx,lambda i,t: logic.render_term((t[0],str(t[1]),str(t[2]))))

    def rpc_dinfo(self,a):
#        path = logic.parse_clause(a)
        current=self.agent.get_current()
        l=[]
        if current is not None:
            l.append(('dinfo_id','current_loop'+str(current)))
            basename = self.agent.loopdb.basename(current)
            l.append(('Name',os.path.splitext(basename)[0]))
            v2=str(self.agent.loopdb.file(current))
            l.append(('File',v2))
            tg = ', '.join(self.agent.loopdb.tags(current))
            l.append(('Tags',tg))
            return logic.render_term(T('keyval',tuple(l) ))
        else:
            n=self.get_description()
            l.append(('dinfo_id',n))

            return logic.render_term(T('keyval',tuple(l) ))
 
    def find_cookie(self,cookie):
        try:
            id=int(cookie)
            if id>=0 and id<self.agent.loopdb.size():
                return id
        except:
            pass

    def rename(self,thing,name):
        self.agent.loopdb.rename(thing,name)
        self.update()

class Voice(atom.Atom):
    def __init__(self,agent,voice):
        self.agent = agent
        self.voice = voice
        self.__file = None
        self.__fileid = None
        atom.Atom.__init__(self,names='voice',ordinal=voice)

        self.__private = node.Server(value=piw.makenull(0),change=self.__change)
        self.set_private(self.__private)

        self.looper = loop_native.player(agent.aggregator.get_output(voice),agent.domain)
        agent.clock_cloner.set_output(voice,self.looper.cookie())

        self.loop_off = piw.change2(piw.fastchange(self.looper.player(0)),piw.slowchange(utils.changify(self.__off)))
        self.loop_on = piw.change2(piw.fastchange(self.looper.player(1)),piw.slowchange(utils.changify(self.__on)))
        self.loop_toggle = piw.change2(piw.fastchange(self.looper.player(3)),piw.slowchange(utils.changify(self.__toggle)))
        self.loop_once = self.looper.player(2)
        self.__playing = True
        self.agent.lights.set_status(self.voice,const.status_active)

        self[1] = atom.Atom(domain=domain.BoundedFloat(0.0,100.0,0.0,hints=(T('inc',1),T('biginc',10),T('control','updown')),verbinc=10.0),init=100.0,policy=atom.default_policy(self.set_volume),names='volume')
        self[2] = atom.Atom(domain=domain.BoundedFloat(0.0,100.0,0.0,hints=(T('inc',1),T('biginc',10),T('control','updown')),verbinc=10.0),init=10.0,policy=atom.default_policy(self.set_chop),names='chop')

        self.looper.set_chop(10.0)
        self.looper.set_volume(1.0)

        piw.changelist_connect(agent.loop_on,self.loop_on)
        piw.changelist_connect(agent.loop_off,self.loop_off)
        piw.changelist_connect(agent.loop_toggle,self.loop_toggle)

    def get_dinfo(self):
        l=[]
        l.append(('voice %d volume' % self.voice,'%.1f' % self[1].get_value()))
        l.append(('file',self.__file))
        l.append(('playing','yes' if self.__playing else 'no'))
        return l

    def __change(self,id):
        if not id.is_string():
            self.set_current(None)
            return

        t = logic.parse_clause(id.as_string())
        
        self.__set_current(t[0])
        self.__set_playing(t[1])
        self.agent.updater.trigger(self.__update)

    def __toggle(self,d):
        if d.as_norm() != 0:
            if self.__playing:
                self.__playing=False
                self.agent.lights.set_status(self.voice,const.status_inactive)
            else:
                self.__playing=True
                self.agent.lights.set_status(self.voice,const.status_active)
            self.__update()
            self.agent.updater.trigger(self.__update,self.agent.update)

    def __off(self,d):
        if d.as_norm() != 0:
            self.__playing = False
            self.agent.lights.set_status(self.voice,const.status_inactive)
            self.agent.updater.trigger(self.__update,self.agent.update)

    def __on(self,d):
        if d.as_norm() != 0:
            self.__playing = True
            self.agent.lights.set_status(self.voice,const.status_active)
            self.agent.updater.trigger(self.__update,self.agent.update)

    def doToggle(self):
        # used when activated by browser
        if self.__playing:
            self.__set_playing(False)
        else:
            self.__set_playing(True)

    def __set_playing(self,p):
        self.__playing = p
        if p:
            self.agent.lights.set_status(self.voice,const.status_active)
        else:
            self.agent.lights.set_status(self.voice,const.status_inactive)
        c = self.looper.player(1 if p else 0)
        fc = piw.fastchange(c)
        fc(piw.makefloat_bounded(1,0,0,1,0))

    def set_volume(self,volume):
        self.looper.set_volume(volume/100.0)
        self[1].set_value(volume)
        self.agent.updater.trigger(self.agent.update)
        return False

    def set_chop(self,c):
        self.looper.set_chop(c)
        self[2].set_value(c)
        self.agent.updater.trigger(self.agent.update)
        return False

    def finfo(self):
        c = self.get_current()
        if c is None:
            return self.voice,"idle",None

        (n,d) = self.agent.loopdb.describe(c)
        return self.voice,d,n

    def disconnect(self):
        piw.changelist_disconnect(self.agent.loop_on,self.loop_on)
        piw.changelist_disconnect(self.agent.loop_off,self.loop_off)
        self.agent.aggregator.clear_output(self.voice)
        self.agent.clock_cloner.clear_output(self.voice)
        self.agent.lights.set_status(self.voice,const.status_off)

    def set_current(self,id,file=None):
        if file is None:
            file=self.agent.loopdb.basename(id)
        if self.__set_current(file,id):
            self.agent.updater.trigger(self.__update)

    def __update(self):
        if self.__file is None:
            self.__private.set_data(piw.makenull(0))
            return

        self.__private.set_data(piw.makestring(logic.render_term((self.__file,self.__playing)),0))

    def __set_current(self,file,id=None):
        if file is None:
            self.looper.unload()
            self.__file = None
            self.__fileid = None
            return True

        if id is None:
            (filepath,fileid)=self.agent.loopdb.fileforbasename(file)
        else:
            fileid=id
            filepath = self.agent.loopdb.file(id)

        if filepath is None:
            self.looper.unload()
        else:
            self.looper.load(filepath)
            self.__file = file
            self.__fileid = fileid
            self.agent.updater.trigger(self.__update,self.agent.update)

        return True

    def get_current(self):
        if self.__file:
            return self.__fileid
        return None

    def first(self):
        self.set_current(0)

    def next(self):
        if self.agent.loopdb.size() > 0:
            c = self.get_current()
            if c is not None:
                self.set_current((c+1)%self.agent.loopdb.size())
            else:
                self.set_current(0)

class VoiceList(atom.Atom):
    def __init__(self,agent):
        self.__agent = agent
        self.__curvoice = None
        self.__timestamp = piw.tsd_time()

        atom.Atom.__init__(self,names='voice',protocols='browse',creator=self.__create_voice,wrecker=self.__wreck_voice)
        self.__selected=None
        self.set_private(node.Server())

        self.update()

    def rpc_displayname(self,arg):
        return 'voices'

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.__selected=selected
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.__curvoice=self[int(selected)]
        self.__agent.update()
        
        # return hint to browser to change view and target
        vid=''
        v =self.__agent[3]
        vid=v.id()
        return logic.render_term(('browseview',vid))

    def toggle(self,o):
        if o in self:
            self[o].doToggle()
            return True
        return False

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __ideals(self,*indices):
        return '[%s]' % ','.join([self.__ideal(c) for c in indices])

    def __ideal(self,index):
        return 'cnc(~(server)"#5.%d")' % index

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        return self.__ideal(cookie)

    def rpc_current(self,arg):
        c = self.__agent.current_voice()
        if c is None:
            return '[]'
        return '[[%d,[]]]' % c.voice

    def rpc_enumerate(self,a):
        
        r=logic.render_term((len(self),0))
        return r

    def rpc_cinfo(self,a):
        return logic.render_term(())

    def rpc_finfo(self,a):
        (dlist,idx) = logic.parse_clause(a)
        map = tuple([v.finfo() for v in self.itervalues()])
        map = map[idx:]
        return logic.render_term(map)

    def __create_voice(self, index):
        self.__agent.update_lights(index)
        v = Voice(self.__agent,index)
        if self.__curvoice is None:
            self.__curvoice = v
            self.__agent.update()
        
        return v

    def __wreck_voice(self, index, node):
        if self.__curvoice is node:
            self.__curvoice = None
            self.__agent.update()
        node.disconnect()

    def current_voice(self):
        return self.__curvoice

    def get_voice(self,voice):
        return self.get(voice)

    def choose_voice(self,voice):
        if voice in self:
            self.__curvoice = self[voice]
            self.__agent.update()
            return True
        return False

    def new_voice(self):
        i = self.find_hole()
        self.__agent.update_lights(i)
        v = Voice(self.__agent,i)
        self[i] = v
        self.__curvoice = v
        self.__agent.update()
        return v

    def remove_voice(self,voice):
        if voice in self:
            node = self[voice]
            del self[voice]
            node.disconnect()
            if node is self.__curvoice:
                self.__curvoice = None
                self.__agent.update()
            return True
        return False

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        agent.Agent.__init__(self, signature=version,names='drummer',icon='plg_loop/bass_drum_64.png',container=(4,'drummer',atom.VerbContainer(clock_domain=self.domain)),protocols='browse',ordinal=ordinal)

        self.updater = Updater()
        self.loopdb = loopdb.LoopDatabase()
        self.__timestamp = 0

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,True,names='left audio output')
        self[1][2] = bundles.Output(2,True,names='right audio output')

        self.output = bundles.Splitter(self.domain,*self[1].values())
        self.summer = piw.stereosummer(self.domain,self.output.cookie(),2)
        self.aggregator = piw.aggregator(self.summer.cookie(),self.domain)

        self.clock_cloner = piw.sclone()
        self.input = bundles.ScalarInput(self.clock_cloner.cookie(), self.domain,signals=(1,2))
        self.input.add_upstream(self.verb_container().clock)

        self[2] = atom.Atom(names='inputs')
        self[2][1] = atom.Atom(domain=domain.BoundedFloat(0,100), policy=self.input.policy(1, False), names='song beat input')
        self[2][2] = atom.Atom(domain=domain.Bool(), init=False, policy=self.input.policy(2,False), names='running input')

        self[3] = LoopBrowser(self)

        vc = '[or([partof(~(s)"#5")],[numeric])]'

        self.add_verb2(1,'first([],None)',self.__first)
        self.add_verb2(2,'next([],None)',self.__next)
        self.add_verb2(3,'name([],None,role(None,[ideal([~server,loop]),singular]),role(to,[abstract]))',self.__name)
        self.add_verb2(4,'choose([],None,role(None,[ideal([~server,loop]),singular]))',self.__choose)
        self.add_verb2(5,'play([],None)',create_action=self.__play,clock=True)
        self.add_verb2(6,'play([un],None)',create_action=self.__unplay,clock=True)
        self.add_verb2(7,'add([],None,option(None,[ideal([~server,loop]),singular]))',self.__add)
        self.add_verb2(8,'remove([],None,option(None,%s))' % vc,self.__remove)
        self.add_verb2(9,'select([],None,role(None,%s))' % vc,self.__select)
        self.add_verb2(10,'play([],None,role(None,%s))' % vc,create_action=self.__play_voice,clock=True,status_action=self.__status)
        self.add_verb2(11,'play([un],None,role(None,%s))' % vc,create_action=self.__unplay_voice,clock=True,status_action=self.__status)
        self.add_verb2(15,'first([],None,role(None,%s))' % vc,self.__firstv)
        self.add_verb2(16,'next([],None,role(None,%s))' % vc,self.__nextv)
        self.add_verb2(17,'play([once],None,role(None,%s))' % vc,create_action=self.__once,clock=True,status_action=self.__status)
        self.add_verb2(21,'scan([],~self)',self.__rescan)
        self.add_verb2(22,'play([toggle],~self,role(None,%s))' % vc,create_action=self.__toggle_voice,clock=True,status_action=self.__status)
        self.add_verb2(23,'play([toggle],None)',create_action=self.__toggle,clock=True)
        
        self.loop_on = piw.changelist()
        self.loop_off = piw.changelist()
        self.loop_toggle = piw.changelist()

        self[6]=bundles.Output(1,False,names='status output')
        self.light_output=bundles.Splitter(self.domain,self[6])
        self.lights=piw.lightsource(piw.change_nb(),0,self.light_output.cookie())
        self[5] = VoiceList(self)
        self[5].populate([1])
        self.__first(None)


    def crack_voice(self,voice):
        if action.is_concrete(voice[0]):
            o = action.concrete_object(voice)
            (a,p) = paths.breakid_list(o)
            return p[1]
        return int(action.abstract_string(voice))

    def update_lights(self,voice):
        cl = self.lights.get_size()
        self.lights.set_size(max(cl,voice))

    def activate_loop(self,selection):
        v=self.current_voice()
        if v:
            if not v.get_current()==selection:
                v.set_current(selection)

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))
        self[3].update()
        self[5].update()

    def current_voice(self):
        return self[5].current_voice()

    def get_current(self):
        v = self[5].current_voice()
        if not v:
            print 'drummer get_current: not v'
        return None if not v else v.get_current()

    def rpc_resolve_ideal(self,arg):
        (typ,name) = action.unmarshal(arg)
        if typ == 'loop': return self[3].resolve_name(' '.join(name))
        return '[]'

    def __firstv(self,subj,voice):
        id = self.crack_voice(voice)
        voice = self[5].get_voice(int(id))
        if voice is None:       
            thing='voice %s' %str(id)
            return async.success(errors.invalid_thing(thing,'first'))

        voice.first()

    def __nextv(self,subj,voice):
        id = self.crack_voice(voice)
        voice = self[5].get_voice(int(id))

        if voice is None:
            thing='voice %s' %str(id)
            return async.success(errors.invalid_thing(thing,'next'))

        voice.next()


    def __first(self,subj):
        v = self[5].current_voice()

        if not v:
            return async.success(errors.no_current_voice('','first'))

        v.first()
        return action.nosync_return()

    def __next(self,subj):
        v = self[5].current_voice()

        if not v:
            return async.success(errors.no_current_voice('','next'))

        v.next()
        return action.nosync_return()

    def __rescan(self,subj):
        self.loopdb.rescan()
        self.update()
        return action.nosync_return()

    def __choose(self,subj,arg):
        (type,id) = action.crack_ideal(action.arg_objects(arg)[0])

        if id >= self.loopdb.size():
            return async.success(errors.doesnt_exist('loop %s' % id,'choose'))

        file = self.loopdb.basename(id)
        voice = self[5].current_voice()

        if not voice:
            return async.success(errors.no_current_voice('','choose'))

        voice.set_current(id,file)
        return action.nosync_return()

    def __add(self,subj,arg):
        if arg is None:
            return action.nosync_return()

        (type,id) = action.crack_ideal(action.arg_objects(arg)[0])

        if id >= self.loopdb.size():
            return async.success(errors.doesnt_exist('loop %s' % str(id),'add'))

        file = self.loopdb.basename(id)
        voice = self[5].new_voice()
        voice.set_current(id,file)

        return action.nosync_return()

    def __select(self,subj,voice):
        id = self.crack_voice(voice)

        if self[5].choose_voice(id):
            return action.nosync_return()

        thing='voice %s' %str(id)
        return async.success(errors.invalid_thing(thing,'select'))

    def __status(self,subj,voice):
        id = self.crack_voice(voice)
        voice=self[5].get_voice(id)
        if voice is None:    
            return None
        return 'dsc(~(s)"#6","%s")' % id

    @utils.nothrow
    def __toggle_voice(self,ctx,subj,voice):
        id = self.crack_voice(voice)

        voice=self[5].get_voice(id)
        if voice is None:    
            thing='voice %s' %str(id)
            return None

        return voice.loop_toggle, None

    def __remove(self,subj,voice):
        if voice is None:
            voice = self[5].current_voice()
            if not voice:
                return async.success(errors.no_current_voice('','remove'))
            id = voice.voice
        else:
            id = self.crack_voice(voice)

        if self[5].remove_voice(id):
            return action.nosync_return()

        thing='voice %s' %str(id)
        return async.success(errors.invalid_thing(thing,'remove'))


    def __name(self,subj,thing,name):
        thing = action.crack_ideal(action.arg_objects(thing)[0])[1]
        name = action.abstract_string(name)
        self[3].rename(thing,name)
        return action.nosync_return()

    @utils.nothrow
    def __play_voice(self,ctx,subj,voice):
        id = self.crack_voice(voice)
        voice = self[5].get_voice(id)

        if voice is None:
            thing='voice %s' %str(id)
            return None


        return voice.loop_on,None

    @utils.nothrow
    def __once(self,ctx,subj,voice):
        id = self.crack_voice(voice)
        voice = self[5].get_voice(id)

        if voice is None:
            thing='voice %s' %str(id)
            return async.success(errors.invalid_thing(thing,'play'))


        return voice.loop_once,None

    @utils.nothrow
    def __unplay_voice(self,ctx,subj,voice):
        id = self.crack_voice(voice)
        voice = self[5].get_voice(id)

        if voice is None:
            thing='voice %s' %str(id)
            return None

        return voice.loop_off,None

    @utils.nothrow
    def __play(self,*a):
        return self.loop_on,None

    @utils.nothrow
    def __unplay(self,*a):
        return self.loop_off,None
    
    @utils.nothrow
    def __toggle(self,*a):
        return self.loop_toggle,None

    def rpc_enumerate(self,arg):
        return logic.render_term((0,0))

    def rpc_cinfo(self,arg):
        return '[]'

    def rpc_finfo(self,arg):
        return '[]'

    def get_dinfo(self):
        l=[]
        dsc=self.get_description()
        l.append(('dinfo_id',dsc))
        return l

    def rpc_dinfo(self,arg):
        l = self.get_dinfo()

        for c in self[5].values():
            l.extend(c.get_dinfo())

        v = logic.render_term(T('keyval',tuple(l)))
        return v
    
    def rpc_current(self,arg):
        #return '[]'
        uid=0
        return '[[%d,[]]]' % uid

    def rpc_setselected(self,arg):
        pass

    def rpc_activated(self,arg):
        return logic.render_term(('',''))

class Upgrader(upgrade.Upgrader):
    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        loop_node = root.get_node(3,254)
        loop = loop_node.get_data().as_long()
        loop_node.set_data(piw.makenull(0))
        root.ensure_node(5,255,6,1).set_data(piw.makestring(logic.render_term((loop,100.0)),0))
        root.ensure_node(5,255,const.meta_domain)
        root.ensure_node(5,255,const.meta_protocols)
        root.ensure_node(5,255,const.meta_names).set_data(piw.makestring('voice',0))
        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        root = tools.root(address)
        for v in xrange(1,254):
            n = root.get_node(5,255,6,v)
            if not n: continue
            if n.get_data().is_string():
                s = n.get_string()
                if not s: continue
                bits = logic.parse_clause(s)
                ns = logic.render_term((bits[0],bits[1],10.0))
                n.set_string(ns)
            else:
                n.erase()
        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        root = tools.root(address)
        for v in xrange(1,254):
            n = root.get_node(5,255,6,v)
            if not n: continue
            if n.get_data().is_string():
                s = n.get_string()
                bits = logic.parse_clause(s)
                ns = logic.render_term((bits[0],bits[1],bits[2],True))
                n.set_string(ns)
            else:
                n.erase()
        return True

    def upgrade_5_0_to_6_0(self,tools,address):
        db=loopdb.LoopDatabase()
        root = tools.root(address)
        for v in xrange(1,254):
            n=root.get_node(5,255,6,v)
            if not n: continue
            if n.get_data().is_string():
                s=n.get_string()
                t = logic.parse_clause(s)
                if str(t[0]).isdigit():
                    f=db.basename(t[0])
                    ns=logic.render_term((f,t[1],t[2],t[3]))
                    n.set_string(ns)
            else:
                n.erase()
        return True
    
    def upgrade_6_0_to_7_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(4).erase_children()
        return True

    def __addvoiceatom(self,root,voice,file,vol,chop,playing):
        voice_node = root.ensure_node(5,voice)
        voice_node.ensure_node(255,7).set_data(piw.makelong(voice,0))
        voice_node.ensure_node(255,8).set_data(piw.makestring('voice',0))
        voice_node.ensure_node(255,6).set_data(piw.makestring(logic.render_term((file,playing)),0))
        voice_node.ensure_node(255,1)
        voice_node.ensure_node(255,3)
        volume_node = voice_node.ensure_node(1)
        volume_node.ensure_node(255,8).set_data(piw.makestring('volume',0))
        volume_node.ensure_node(255,1)
        volume_node.ensure_node(255,3)
        volume_node.ensure_node(254).set_data(piw.makefloat(vol,0))
        chop_node = voice_node.ensure_node(2)
        chop_node.ensure_node(255,8).set_data(piw.makestring('chop',0))
        chop_node.ensure_node(255,1)
        chop_node.ensure_node(255,3)
        chop_node.ensure_node(254).set_data(piw.makefloat(chop,0))

    def __upgradeupdownverb(self,id,vschema,sub,newnum):
        vschema2 = list(vschema)
        vctxargs = list(vschema2[3])
        s3 = int(vctxargs[1][0].args[1])
        vctxargs[1] = (T('cnc','%s#5.%d.%d'%(id,s3,sub)),)
        vschema2[3] = tuple(vctxargs)
        vschema2[1] = newnum
        vschema = tuple(vschema2)
        return vschema

    def __upgradevoiceverb(self,id,vschema):
        vschema2 = list(vschema)
        vctxargs = list(vschema2[3])
        if vctxargs[1]:
            s3 = int(vctxargs[1][0].args[1])
            vctxargs[1] = (T('cnc','%s#5.%d'%(id,s3)),)
            vschema2[3] = tuple(vctxargs)
            vschema = tuple(vschema2)
        return vschema

    def __upgradeverb(self,id,vschema):
        vn = vschema[1]
        if vn==8: return self.__upgradevoiceverb(id,vschema)
        if vn==9: return self.__upgradevoiceverb(id,vschema)
        if vn==10: return self.__upgradevoiceverb(id,vschema)
        if vn==11: return self.__upgradevoiceverb(id,vschema)
        if vn==15: return self.__upgradevoiceverb(id,vschema)
        if vn==16: return self.__upgradevoiceverb(id,vschema)
        if vn==17: return self.__upgradevoiceverb(id,vschema)
        if vn==22: return self.__upgradevoiceverb(id,vschema)
        if vn==13: return self.__upgradeupdownverb(id,vschema,1,203)
        if vn==14: return self.__upgradeupdownverb(id,vschema,1,204)
        if vn==19: return self.__upgradeupdownverb(id,vschema,2,203)
        if vn==20: return self.__upgradeupdownverb(id,vschema,2,204)
        if vn==12: return self.__upgradeupdownverb(id,vschema,1,200)
        if vn==18: return self.__upgradeupdownverb(id,vschema,2,200)
        return vschema

    def upgrade_7_0_to_8_0(self,tools,address):
        root = tools.root(address)

        for v in xrange(1,254):
            n=root.get_node(5,255,6,v)
            if not n: continue
            if n.get_data().is_string():
                s=n.get_string()
                (file,vol,chop,playing) = logic.parse_clause(s)
                self.__addvoiceatom(root,v,file,vol,chop,playing)

        root.get_node(5,255,6).erase()
        root.ensure_node(5,255,6)
        root.ensure_node(255,6)

        for v in xrange(1,254):
            n=root.get_node(4,v,255,6)
            if not n: continue
            if not n.get_data().is_string(): continue
            v1 = logic.parse_clause(n.get_data().as_string())
            v2 = self.__upgradeverb(address,v1)
            n.set_data(piw.makestring(logic.render_term(v2),0))

        return True
    
    def upgrade_8_0_to_9_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(255,3)
        icon='ideal([~self,file],[True,[bass_drum_64.png,png,7752,e53b6b260d6f84e81e1b750b6be7cd0a],"pkg_res:plg_loop/bass_drum_64.png"])'
        root.ensure_node(255,16).set_data(piw.makestring(icon,0))
        return True

agent.main(Agent,Upgrader)

