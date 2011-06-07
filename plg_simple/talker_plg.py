
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

from pi import agent,atom,action,domain,bundles,utils,logic,node,async,schedproxy,const,upgrade,policy,paths,talker
from plg_simple import talker_version as version
import piw
import operator

class PhraseBrowser(atom.Atom):
    def __init__(self,eventlist,keylist):
        atom.Atom.__init__(self,names='phrase',protocols='virtual browse')

        self.__eventlist = eventlist
        self.__keylist = keylist
        self.__timestamp = piw.tsd_time()
        self.update()

    def rpc_displayname(self,arg):
        return 'phrases'
   
    def rpc_setselected(self,arg):
        pass
    
    def rpc_activated(self,arg):
        return logic.render_term(('',''))

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __ideal(self,e):
        return 'ideal([~server,event],[~server,%s])' % logic.render_term(e.id())

    def rpc_fideal(self,arg):
        (path,uid) = logic.parse_clause(arg)
        all = [e for k in self.__keylist() for e in self.__eventlist(k)]
        e = all[uid]
        return self.__ideal(e)

    def rpc_current(self,arg):
        return '[]'

    def resolve_name(self,name):
        try:
            o=int(name)
        except:
            return '[]'

        events=[ self.__ideal(e) for e in self.__eventlist(o)]
        return '[%s]' % ','.join(events)

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)

        if a:
            return '[]'

        events=[ self.__ideal(e) for k in self.__keylist() for e in self.__eventlist(k)]

        if o:
            o=int(o)
            if o>0 and o<=len(events):
                return '[%s]' % events[o-1]
            return '[]'

        return '[%s]' % ','.join(events)

    def rpc_enumerate(self,a):
        path=logic.parse_clause(a)
        if len(path)==0:
            k=reduce(operator.add,[len(self.__eventlist(k)) for k in self.__keylist()],0)
            c=0
        else:
            k=0
            c=0
        return logic.render_term((k,c))

    def rpc_cinfo(self,a):
        return '[]'

    def rpc_finfo(self,a):
        (dlist,cnum) = logic.parse_clause(a)
        all = list(enumerate([e for k in self.__keylist() for e in self.__eventlist(k)]))
        map = tuple([(i,e.describe(),None) for (i,e) in all[cnum:] ])
        return logic.render_term(map)

class Collection(atom.Atom):
    def __init__(self,protocols=None,inst_creator=None,inst_wrecker=None,*args,**kwds):
        p = protocols+' create' if protocols else 'create hidden-connection'
        self.__creator = inst_creator or self.instance_create
        self.__wrecker = inst_wrecker or self.instance_wreck
        atom.Atom.__init__(self,protocols=p,*args,**kwds)

    def instance_wreck(self,k,v,o):
        return async.success()

    def instance_create(self,o):
        return async.failure('not implemented')

    def listinstances(self):
        return [ self[i].get_property_long('ordinal',0) for i in self ]

    def rpc_listinstances(self,arg):
        i = self.listinstances()
        return logic.render_termlist(i)

    @async.coroutine('internal error')
    def rpc_createinstance(self,arg):
        name = int(arg)
        outputs = self.listinstances()

        if name in outputs:
            yield async.Coroutine.failure('output in use')

        oresult = self.__creator(name)
        yield oresult

        if not oresult.status():
            yield async.Coroutine.failure(*oresult.args(),**oresult.kwds())

        output = oresult.args()[0]
        yield async.Coroutine.success(output.id())

    @async.coroutine('internal error')
    def rpc_delinstance(self,arg):
        name = int(arg)

        for k,v in self.items():
            o = v.get_property_long('ordinal',0)
            if o == name:
                oid = v.id()
                oresult = self.__wrecker(k,v,o)
                yield oresult
                if k in self: del self[k]
                yield async.Coroutine.success(oid)

        yield async.Coroutine.failure('output not in use')


class Event(talker.Talker):
    def __init__(self,key,fast,index):
        self.__key = key
        self.__index = index
        cookie = self.__key.key_aggregator.get_output(self.__index)

        talker.Talker.__init__(self,self.__key.agent.finder,fast,cookie,names='event',ordinal=index,connection_index=index)

    def detach_event(self):
        self.__key.key_aggregator.clear_output(self.__index)

    def property_change(self,key,value):
        if key=='help' and value and value.is_string():
            self.__key.agent.update()

    def describe(self):
        return self.get_property_string('help')



class Key(Collection):
    def __init__(self,agent,controller,index):
        Collection.__init__(self,creator=self.__create,wrecker=self.__wreck,ordinal=index,names='k',protocols='hidden-connection')
        self.__event = piw.fasttrigger(const.light_unknown)
        self.__event.attach_to(controller,index)
        self.__handler = piw.change2_nb(self.__event.trigger(),utils.changify(self.event_triggered))
        self.key_aggregator = piw.aggregator(agent.light_aggregator.get_output(index+1),agent.domain)
        self.agent = agent
        self.index = index
        self.set_private(node.Server(value=piw.makelong(3,0),change=self.__change_color))
        self.set_internal(250,atom.Atom(domain=domain.Trigger(),init=False,names='activate',policy=policy.TriggerPolicy(self.__handler),transient=True))
        self.agent.light_convertor.set_status_handler(self.index, piw.slowchange(utils.changify(self.set_status)))

    @async.coroutine('internal error')
    def instance_create(self,name):
        e = Event(self,self.__event.fastdata(),name)
        self[name] = e
        e.attached()

    @async.coroutine('internal error')
    def instance_wreck(self,k,e,name):
        print 'killing event',k
        del self[k]
        e.detach_event()
        r = e.clear_phrase()
        yield r
        print 'killed event',k

    def event_triggered(self,v):
        self.get_internal(250).get_policy().set_status(piw.makelong(0,0))
        self.set_status(piw.makelong(self.agent.light_convertor.get_status(self.index),piw.tsd_time()))

    def set_status(self,v):
        self.get_internal(250).get_policy().set_status(v)

    def isinternal(self,k):
        if k == 250: return True
        return atom.Atom.isinternal(self,k)

    def __change_color(self,d):
        if d.is_long():
            self.set_color(d.as_long())

    def set_color(self,c):
        self.agent.light_convertor.set_default_color(self.index,c)
        self.get_private().set_data(piw.makelong(c,0))

    def __create(self,i):
        self.agent.update()
        return Event(self,self.__event.fastdata(),i)

    def __wreck(self,k,v):
        v.detach_event()
        self.agent.update()

    def detach_key(self):
        self.agent.light_convertor.remove_status_handler(self.index)
        self.agent.light_aggregator.clear_output(self.index+1)

    @async.coroutine('internal error')
    def create_event(self,text,called=None):
        if called:
            if called in self:
                yield async.Coroutine.failure('phrase exists')
            index = called
        else:
            index = self.find_hole()

        print 'create event on key',self.id()

        e = Event(self,self.__event.fastdata(),index)
        self[index] = e
        e.attached()
        r = e.set_phrase(text)
        yield r

    @async.coroutine('internal error')
    def cancel_event(self,called=None):
        for c,e in self.items():
            if not called or called==c:
                del self[c]
                e.detach_event()
                r = e.clear_phrase()
                yield r

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,signature=version,names='talker',container=5,ordinal=ordinal)

        self.add_verb2(2,'do([],None,role(None,[abstract]),role(when,[singular,numeric]),option(called,[singular,numeric]))', self.__do_verb)
        self.add_verb2(8,'cancel([],None,role(None,[singular,numeric]),option(called,[singular,numeric]))', self.__cancel_verb)
        self.add_verb2(5,'colour([],None,role(None,[singular,numeric]),role(to,[singular,numeric]))', self.__color_verb)
        self.add_verb2(6,'colour([],None,role(None,[singular,numeric]),role(to,[singular,numeric]),role(from,[singular,numeric]))', self.__all_color_verb)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.__size = 0
        self.finder = talker.TalkerFinder()

        self[1] = bundles.Output(1,False, names='light output',protocols='revconnect')

        self.light_output = bundles.Splitter(self.domain,self[1])
        self.light_convertor = piw.lightconvertor(self.light_output.cookie())
        self.light_aggregator = piw.aggregator(self.light_convertor.cookie(),self.domain)
        self.controller = piw.controller(self.light_aggregator.get_output(1),utils.pack_str(1))

        self.activation_input = bundles.VectorInput(self.controller.cookie(), self.domain,signals=(1,))

        self[2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.activation_input.local_policy(1,False),names='activation input')
        self[3] = Collection(creator=self.__create,wrecker=self.__wreck,names='k',inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self[4] = PhraseBrowser(self.__eventlist,self.__keylist)

    def __eventlist(self,k):
        el=[]
        if k in self[3]:
            for e in self[3][k].itervalues():
                el.append(e)
        return el

    def __keylist(self):
        kl=[]
        for k in self[3].iterkeys():
            kl.append(k)
        return kl

    def __update_lights(self,k):
        if k > self.__size:
            self.__size = k

    def update(self):
        self[4].update()
            
    def __create(self,k):
        self.__update_lights(k)
        return Key(self,self.controller,k)

    def __wreck(self,k,v):
        v.detach_key()

    @async.coroutine('internal error')
    def __create_inst(self,k):
        self.__update_lights(k)
        self[3][k] = Key(self,self.controller,k)

    @async.coroutine('internal error')
    def __wreck_inst(self,k,e,name):
        del self[3][k]
        r = e.cancel_event()
        yield r

    def __all_color_verb(self,subject,k,c,f):
        k = int(action.abstract_string(k))
        c = int(action.abstract_string(c))
        f = int(action.abstract_string(f))

        for (i,t) in self[3].items():
            t.set_color(c if (k==i) else f)

    def __color_verb(self,subject,k,c):
        k = int(action.abstract_string(k))
        c = int(action.abstract_string(c))
        if k in self[3]:
            self[3][k].set_color(c)

    @async.coroutine('internal error')
    def __do_verb(self,subject,t,k,c):
        t = action.abstract_string(t)
        k = int(action.abstract_string(k))
        c = int(action.abstract_string(c)) if c else None
        
        if k not in self[3]:
            self.__update_lights(k)
            self[3][k] = Key(self,self.controller,k)

        if c and c in self[3][k]:
                yield async.Coroutine.success(action.error_return('name in use','','do'))

        r = self[3][k].create_event(t,c)
        yield r
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __cancel_verb(self,subject,k,c):
        k = int(action.abstract_string(k))
        c = int(action.abstract_string(c)) if c else None

        rv=[]
        if k not in self[3]:
            yield async.Coroutine.success()

        if c and c not in self[3][k]:
            yield async.Coroutine.success()

        r = self[3][k].cancel_event(c)
        yield r
        yield async.Coroutine.success()


class Upgrader(upgrade.Upgrader):
    
    def move_connections(self,tools,connections,old_id,new_id):
        actions = []

        old_server,old_path = paths.breakid_list(old_id)
        old_path = tuple(old_path)
        old_path_len = len(old_path)
        old_connections = connections.get(old_server,{})
        new_server,new_path = paths.breakid_list(new_id)
        new_path = tuple(new_path)

        for src_path,tgt_id_set in old_connections.items():
            if src_path[:old_path_len] == old_path:
                old_src_id = paths.makeid_list(old_server,*src_path)
                new_src_id = paths.makeid_list(new_server,*(new_path+src_path[old_path_len:]))
                for tgt_id in tgt_id_set:
                    tgt_server,tgt_path = paths.breakid_list(tgt_id)
                    tgt_node = tools.get_root(tgt_server).get_node(*tgt_path)
                    if tgt_node:
                        tgt_node.move_connection(old_src_id,new_src_id)
                        actions.append("deferred_action('%s',None)" % tgt_id)

        return ','.join(actions)

    def upgrade_1_0_1_to_1_0_2(self,tools,address):
        connections = tools.get_connections()
        keys = tools.get_root(address).get_node(3)
        keys.set_name('k')
        for k in keys.iter(exclude=(254,255)):
            for e in k.iter(exclude=(250,254,255)):
                em = e.get_data()
                em = piw.dictset(em,'interpreter',piw.makestring('<interpreter>',0))
                et = em.as_dict_lookup('help')
                e.ensure_node(254).set_data(et)
                t = e.ensure_node(2)
                t.set_name('trigger output')
                actions = self.move_connections(tools,connections,e.id(),t.id())
                em = piw.dictset(em,'actions',piw.makestring(actions,0))
                e.set_data(em)
                print 'upgrading',e.id(),et,actions

    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        keys = tools.get_root(address).get_node(3)
        for k in keys.iter(exclude=(254,255)):
            ko = k.path[-1]
            km = k.get_data()
            km = piw.dictset(km,'name',piw.makestring('key',0))
            km = piw.dictset(km,'ordinal',piw.makelong(ko,0))
            k.set_data(km)

            for e in k.iter(exclude=(254,255)):
                eo = e.path[-1]
                em = e.get_data()
                et = e.ensure_node(255).get_data()
                em = piw.dictset(em,'name',piw.makestring('action',0))
                em = piw.dictset(em,'ordinal',piw.makelong(ko,0))
                em = piw.dictset(em,'help',et if et.is_string() else piw.makestring('',0))
                e.set_data(em)
                e.erase_child(255)

    def __get_conn(self,tools,id,schema):
        if not schema:
            return []
        s = logic.parse_clause(schema)
        a = paths.id2server(id)
        t = tools.agent_type(a)
        v = s[1]

        if t=='drummer' and v==10: return [(paths.makeid_list(a,6),s[3][1][0].args[1])]
        if t=='drummer' and v==11: return [(paths.makeid_list(a,6),s[3][1][0].args[1])]
        if t=='drummer' and v==17: return [(paths.makeid_list(a,6),s[3][1][0].args[1])]
        if t=='drummer' and v==22: return [(paths.makeid_list(a,6),s[3][1][0].args[1])]
        if t=='metronome' and v==1: return [(paths.makeid_list(a,14),1)]
        if t=='metronome' and v==2: return [(paths.makeid_list(a,14),1)]
        if t=='metronome' and v==3: return [(paths.makeid_list(a,14),1)]
        if t=='clicker' and v==1: return [(paths.makeid_list(a,4),1)]
        if t=='clicker' and v==2: return [(paths.makeid_list(a,4),1)]
        if t=='clicker' and v==3: return [(paths.makeid_list(a,4),1)]

        return []

    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)
        for k in root.ensure_node(3).iter():
            for t in k.iter():
                status = []
                for e in tools.find_slaves(t.id()):
                    try:
                        (es,ep) = paths.breakid_list(e)
                        er = tools.root(es).get_node(*ep).get_node(255,6).get_data().as_string()
                        status.extend(self.__get_conn(tools,e,er))
                    except:
                        pass
                conn = ','.join([ 'conn(%d,%d,"%s","%s")' % (i,k.path[-1],s,p) for (i,(s,p)) in enumerate(status) ])
                ts = t.ensure_node(1)
                ts.setmeta(1)
                ts.setmeta(8,'status input')
                ts.setmeta(2,conn)

        return True

    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        say = set()
        echosay = set()
        nodes = set()
        for k in root.ensure_node(3).iter():
            for t in k.iter():
                try:
                    words = t.get_node(255,6).get_data().as_string().split()
                    if words[-1]=='say':
                        echosay.add(' '.join(words))
                        say.add(' '.join(words[:-3]))
                        nodes.add(t)
                except:
                    pass

        say_seen = set()
        echosay_seen = set()
        for t in nodes:
            for e in tools.find_slaves(t.id()):
                try:
                    (es,ep) = paths.breakid_list(e)
                    en = tools.root(es).get_node(*ep)
                    er = en.get_node(255,6).get_data().as_string()
                    if not er: continue
                    c = logic.parse_clause(er)

                    # say
                    if c[1]==29:
                        phrase = ' '.join(c[3][1][0].args[0])
                        if phrase not in say:
                            print 'dangling say:',phrase
                            en.remove()
                        if phrase in say_seen:
                            print 'dup say:',phrase
                            en.remove()
                        say_seen.add(phrase)

                    # echo say
                    if c[1]==30:
                        words = c[3][1][0].args[0]
                        if words[-1]=='say':
                            phrase = ' '.join(words)
                            if phrase not in echosay:
                                print 'dangling echo say:',phrase
                                en.remove()
                            if phrase in echosay_seen:
                                print 'dup echo say:',phrase
                                en.remove()
                            echosay_seen.add(phrase)
                except:
                    utils.log_trace()

        return True

    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)
        #root.ensure_node(5).erase_children()
        root.ensure_node(3).erase_children()
        return True

agent.main(Agent,Upgrader)
