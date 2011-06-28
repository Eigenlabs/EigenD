
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
from pi import agent,atom,domain,errors,action,bundles,async,utils,resource,logic,node,upgrade,const,paths,collection
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

class Voice(atom.Atom):
    def __init__(self,agent,voice):
        self.agent = agent
        self.voice = voice

        self.__timestamp = piw.tsd_time()

        atom.Atom.__init__(self,domain=domain.String(),names='voice',ordinal=voice,protocols='browse',policy=atom.default_policy(self.set_loop))

        self.looper = loop_native.player(agent.aggregator.get_output(voice),agent.domain,utils.statusify(self.__loop_status))
        agent.clock_cloner.set_output(voice,self.looper.cookie())

        self.loop_off = self.looper.player(0)
        self.loop_on = self.looper.player(1)
        self.loop_toggle = self.looper.player(3)
        self.loop_once = self.looper.player(2)

        self.agent.lights.set_status(self.voice,const.status_inactive)

        self[1] = atom.Atom(domain=domain.BoundedFloat(0.0,100.0,0.0,hints=(T('inc',1),T('biginc',10),T('control','updown')),verbinc=10.0),init=100.0,policy=atom.default_policy(self.set_volume),names='volume')
        self[2] = atom.Atom(domain=domain.BoundedFloat(0.0,100.0,0.0,hints=(T('inc',1),T('biginc',10),T('control','updown')),verbinc=10.0),init=10.0,policy=atom.default_policy(self.set_chop),names='chop')
        self[3] = atom.Atom(domain=domain.Bool(),init=True,names='enable',policy=atom.default_policy(self.set_playing),protocols='set',container=(None,'voice%d'%voice,agent.verb_container()))

        self[3].add_verb2(1,'set([],~a,role(None,[instance(~self)]))', create_action=self.__enable_set, status_action=self.__enable_status)
        self[3].add_verb2(2,'set([un],~a,role(None,[instance(~self)]))', create_action=self.__enable_unset, status_action=self.__enable_status)
        self[3].add_verb2(3,'set([toggle],~a,role(None,[instance(~self)]))', create_action=self.__enable_toggle, status_action=self.__enable_status)

        self.looper.set_chop(10.0)
        self.looper.set_volume(1.0)

        piw.changelist_connect(agent.loop_on,self.loop_on)
        piw.changelist_connect(agent.loop_off,self.loop_off)
        piw.changelist_connect(agent.loop_toggle,self.loop_toggle)

        self.first()

    def __enable_set(self,*args):
        return (self.loop_on,None)

    def __enable_unset(self,*args):
        return (self.loop_off,None)

    def __enable_toggle(self,*args):
        return (self.loop_toggle,None)

    def __enable_status(self,*args):
        return 'dsc(~(a)"#6","%s")' % self.voice

    def __loop_status(self,v):
        print 'loop',self.voice,'status changed to',v

        self[3].set_value(v)

        if v:
            self.agent.lights.set_status(self.voice,const.status_active)
        else:
            self.agent.lights.set_status(self.voice,const.status_inactive)

        self.update()

    def get_dinfo(self):
        l=[]
        l.append(('voice %d volume' % self.voice,'%.1f' % self[1].get_value()))
        l.append(('file',self.get_value()))
        l.append(('playing','yes' if self[3].get_value() else 'no'))
        return l

    def set_playing(self,p):
        c = self.looper.player(1 if p else 0)
        fc = utils.fastchange(c)
        fc(piw.makefloat_bounded(1,0,0,1,0))
        return False

    def set_volume(self,volume):
        self.looper.set_volume(volume/100.0)
        self[1].set_value(volume)
        self.update()
        return False

    def set_chop(self,c):
        self.looper.set_chop(c)
        self[2].set_value(c)
        self.update()
        return False

    def finfo(self):
        c = self.get_current_id()
        if c is None:
            return self.voice,"idle",None
        (n,d) = self.agent.loopdb.describe(c)
        return self.voice,d,n

    def disconnect(self):
        piw.changelist_disconnect(self.agent.loop_on,self.loop_on)
        piw.changelist_disconnect(self.agent.loop_off,self.loop_off)
        piw.changelist_disconnect(self.agent.loop_toggle,self.loop_toggle)
        self.agent.aggregator.clear_output(self.voice)
        self.agent.clock_cloner.clear_output(self.voice)
        self.agent.lights.set_status(self.voice,const.status_off)

    def get_current_id(self):
        filename = self.get_value()

        if not filename:
            return None

        return self.agent.loopdb.idforfile(filename)

    def set_current_id(self,lid):
        if lid is not None:
            self.set_loop(self.agent.loopdb.file(lid))
        else:
            self.set_loop(None)

    def set_loop(self,filename):
        fileid = None

        if filename:
            fileid = self.agent.loopdb.idforfile(filename)

        print 'set loop',filename,fileid

        if fileid is None:
            self.looper.unload()
            self.set_value('')
        else:
            self.set_value(filename)
            self.looper.load(filename)

        self.update()


    def first(self):
        self.set_current_id(0)

    def next(self):
        if self.agent.loopdb.size() > 0:
            c = self.get_current_id()
            if c is not None:
                self.set_current_id((c+1)%self.agent.loopdb.size())
            else:
                self.set_current_id(0)

    def update(self):
        self.agent.updater.trigger(self.agent.update,self.__update)

    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __ideals(self,*indices):
        return '[%s]' % ','.join([self.__ideal(c) for c in indices])

    def __ideal(self,filename):
        return 'ideal([~server,loop],"%s")' % index

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        return self.__ideal(cookie)

    def rpc_current(self,arg):
        c = self.get_value()
        if c is None:
            return '[]'
        return '[["%s",[]]]' % c

    def rpc_setselected(self,arg):
        pass
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.set_loop(selected)
        return logic.render_term(('',''))

    def rpc_displayname(self,arg):
        return 'drummer voice '+ str(self.voice)

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
        return render_list(r,idx,lambda i,t: logic.render_term((str(t[0]),str(t[1]),str(t[2]))))

    def rpc_dinfo(self,a):
        current=self.get_current_id()

        l=[]
        l.append(('dinfo_id',self.get_description()))

        if current is not None:
            basename = self.agent.loopdb.basename(current)
            l.append(('Name',os.path.splitext(basename)[0]))
            v2=str(self.agent.loopdb.file(current))
            l.append(('File',v2))
            tg = ', '.join(self.agent.loopdb.tags(current))
            l.append(('Tags',tg))

        return logic.render_term(T('keyval',tuple(l) ))
 
class VoiceList(collection.Collection):
    def __init__(self,agent):
        self.__agent = agent
        self.__timestamp = piw.tsd_time()

        collection.Collection.__init__(self,names='voice list',protocols='browse',creator=self.__create_voice,wrecker=self.__wreck_voice,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self.update()

    def rpc_displayname(self,arg):
        return 'voices'

    def rpc_setselected(self,arg):
        pass
    
    def rpc_activated(self,arg):
        return logic.render_term(('',''))

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
        return '[]'

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

    def create_voice(self,ordinal=None):
        o = ordinal or self.find_hole()
        self.__agent.update_lights(o)
        e = Voice(self.__agent,o)
        self[o] = e
        self.__agent.update()
        return e

    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        e=self.create_voice(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,key,inst,ordinal):
        inst.disconnect()
        yield async.Coroutine.success()

    def __create_voice(self, index):
        self.__agent.update_lights(index)
        v = Voice(self.__agent,index)
        return v

    def __wreck_voice(self, index, node):
        node.disconnect()

    def get_voice(self,voice):
        return self.get(voice)

    def del_voice(self,index):
        v = self[index]
        del self[index]
        v.disconnect()

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

        vc = '[or([partof(~(s)"#5")],[numeric])]'

        self.add_verb2(5,'play([],None)',create_action=self.__play,clock=True)
        self.add_verb2(6,'play([un],None)',create_action=self.__unplay,clock=True)
        self.add_verb2(9,'select([],None,role(None,%s))' % vc,self.__select)
        self.add_verb2(10,'play([],None,role(None,%s))' % vc,create_action=self.__play_voice,clock=True,status_action=self.__status)
        self.add_verb2(11,'play([un],None,role(None,%s))' % vc,create_action=self.__unplay_voice,clock=True,status_action=self.__status)
        self.add_verb2(15,'first([],None,role(None,%s))' % vc,self.__firstv)
        self.add_verb2(16,'next([],None,role(None,%s))' % vc,self.__nextv)
        self.add_verb2(17,'play([once],None,role(None,%s))' % vc,create_action=self.__once,clock=True,status_action=self.__status)
        self.add_verb2(21,'scan([],~self)',self.__rescan)
        self.add_verb2(22,'play([toggle],~self,role(None,%s))' % vc,create_action=self.__toggle_voice,clock=True,status_action=self.__status)
        self.add_verb2(23,'play([toggle],None)',create_action=self.__toggle,clock=True)
        self.add_verb2(24,'create([un],None,role(None,%s))' % vc, self.__uncreate)
        self.add_verb2(25,'create([],None,role(None,[numeric]))', self.__create1)
        self.add_verb2(26,'create([],None,role(None,[matches([voice])]))', self.__create2)
        self.add_verb2(27,'create([],None,role(None,[mass([voice])]))', self.__create3)
        
        self.loop_on = piw.changelist()
        self.loop_off = piw.changelist()
        self.loop_toggle = piw.changelist()

        self[6]=bundles.Output(1,False,names='status output')
        self.light_output=bundles.Splitter(self.domain,self[6])
        self.lights=piw.lightsource(piw.change_nb(),0,self.light_output.cookie())
        self[5] = VoiceList(self)
        self[5].populate([1])


    def crack_voice(self,voice):
        if action.is_concrete(voice[0]):
            o = action.concrete_object(voice)
            (a,p) = paths.breakid_list(o)
            return p[1]
        return int(action.abstract_string(voice))

    def update_lights(self,voice):
        cl = self.lights.get_size()
        self.lights.set_size(max(cl,voice))

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))
        self[5].update()

    def __create3(self,subj,voice):
        id=int(action.mass_quantity(voice))
        voice = self[5].get_voice(id)

        if voice is not None:
            thing='voice %s' %str(id)
            return async.success(errors.already_exists(thing,'create'))
        
        self[5].create_voice(id)

    def __create2(self,subj,voice):
        self[5].create_voice()

    def __create1(self,subj,voice):
        id = int(action.abstract_string(voice))
        voice = self[5].get_voice(id)

        if voice is not None:
            thing='voice %s' %str(id)
            return async.success(errors.already_exists(thing,'create'))
        
        self[5].create_voice(id)

    def __select(self,subj,voice):
        pass 

    def __uncreate(self,subj,voice):
        id = self.crack_voice(voice)
        voice = self[5].get_voice(int(id))

        if voice is None:       
            thing='voice %s' %str(id)
            return async.success(errors.invalid_thing(thing,'first'))

        self[5].del_voice(voice.voice)

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


    def __rescan(self,subj):
        self.loopdb.rescan()
        self.update()
        return action.nosync_return()

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
    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        voices = tools.get_root(address).get_node(5)
        voices.erase_child(255)
        for v in voices.iter(exclude=(254,255)):
            v.erase_child(255)
            v.set_data(piw.makestring('',0))

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

