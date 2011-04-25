
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

from pi import agent,atom,action,domain,bundles,utils,logic,node,async,schedproxy,const,upgrade,policy,paths
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

class Event(atom.Atom):
    def __init__(self,key,fast,text,index):
        self.__key = key
        self.__index = index

        self.__fastdata = piw.fastdata(0)
        piw.tsd_fastdata(self.__fastdata)
        self.__fastdata.set_upstream(fast)

        atom.Atom.__init__(self,domain=domain.Aniso(),policy=policy.FastReadOnlyPolicy(),names='action',ordinal=index)

        self.get_policy().set_source(self.__fastdata)
        self.status_input = bundles.VectorInput(self.__key.key_aggregator.get_output(self.__index), self.__key.agent.domain,signals=(1,))
        self.set_property_string('help',text)

        self[1] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.status_input.vector_policy(1,False,clocked=False),names='status input',protocols='nostage')

    def cancel(self):
        self.clear_connections()
        self.__key.key_aggregator.clear_output(self.__index)

    def property_change(self,key,value):
        if key=='help' and value and value.is_string():
            self.__key.agent.update()

    def describe(self):
        return self.get_property_string('help')

    def attach(self,*args):
        c = []
        for i,a in enumerate(args):
            (id,ch) = a.args
            c.append(logic.make_term('conn',i,self.__key.index,id,ch))

        self[1].set_connections(logic.render_termlist(c))

class Key(atom.Atom):
    def __init__(self,agent,controller,index):
        atom.Atom.__init__(self,creator=self.__create,wrecker=self.__wreck,ordinal=index,names='key')
        self.__event = piw.fasttrigger(const.light_unknown)
        self.__event.attach_to(controller,index)
        self.__handler = piw.change2_nb(self.__event.trigger(),utils.changify(self.event_triggered))
        self.key_aggregator = piw.aggregator(agent.light_aggregator.get_output(index+1),agent.domain)
        self.agent = agent
        self.index = index
        self.set_private(node.Server(value=piw.makelong(3,0),change=self.__change_color))
        self.set_internal(250,atom.Atom(domain=domain.Trigger(),init=False,names='activate',policy=policy.TriggerPolicy(self.__handler),transient=True))

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
        self.agent.light_convertor.set_status_handler(self.index, utils.changify(self.set_status))
        self.agent.update()
        return Event(self,self.__event.fastdata(),'',i)

    def __wreck(self,k,v):
        self.agent.light_convertor.remove_status_handler(self.index)
        v.cancel()
        self.agent.update()

    def eventcalled(self,text,called):
        if called in self:
            return None
        e = Event(self,self.__event.fastdata(),text,called)
        self[called] = e
        return e.id()

    def event(self,text):
        i = self.find_hole()
        e = Event(self,self.__event.fastdata(),text,i)
        self[i] = e
        return e.id()

    def detach(self,id=None,del_similar=False):
        did = False
        for n,e in self.items():
            if id is None or e.id()==id:
                e.cancel()
                del self[n]
                did = True

        if len(self)==0:
            self.__event.detach()
            self.agent.light_aggregator.clear_output(self.index+1)

        return did

    def attach(self,id,*args):
        did = False
        for n,e in self.items():
            if e.id()==id:
                did = True
                e.attach(*args)
                break

        return did

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,signature=version,names='talker',container=5,ordinal=ordinal)

        self.add_mode2(4,'mode([],role(when,[singular,numeric]),option(using,[instance(~server)]))', self.__mode, self.__query, self.__cancel_mode, self.__attach_mode)
        self.add_mode2(7,'mode([],role(when,[singular,numeric]),role(called,[singular,numeric]),option(using,[instance(~server)]))', self.__cmode, self.__cquery, self.__cancel_mode, self.__attach_mode)
        self.add_verb2(1,'cancel([],None,role(None,[ideal([~server,event])]))', self.__cancel_verb)
        self.add_verb2(8,'cancel([],None,role(None,[singular,numeric]),role(called,[singular,numeric]))', self.__cancel_verb_called)
        self.add_verb2(5,'colour([],None,role(None,[singular,numeric]),role(to,[singular,numeric]))', self.__color_verb)
        self.add_verb2(6,'colour([],None,role(None,[singular,numeric]),role(to,[singular,numeric]),role(from,[singular,numeric]))', self.__all_color_verb)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.__size = 0

        self[1] = bundles.Output(1,False, names='light output',protocols='revconnect')
        self.light_output = bundles.Splitter(self.domain,self[1])
        self.light_convertor = piw.lightconvertor(self.light_output.cookie())
        self.light_aggregator = piw.aggregator(self.light_convertor.cookie(),self.domain)
        self.controller = piw.controller(self.light_aggregator.get_output(1),utils.pack_str(1))

        self.activation_input = bundles.VectorInput(self.controller.cookie(), self.domain,signals=(1,))

        self[2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.activation_input.local_policy(1,False),names='activation input')
        self[3] = atom.Atom(creator=self.__create,wrecker=self.__wreck)
        self[4] = PhraseBrowser(self.__eventbykey,self.__keylist)

    def rpc_resolve_ideal(self,arg):
        (typ,name) = action.unmarshal(arg)
        if typ != 'event': return '[]'
        name = ' '.join(name)
        return self[4].resolve_name(name)

    def __eventbykey(self,k):
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
        v.detach()

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

    def __cquery(self,k,c,u):
        k = int(action.abstract_string(k))
        c = int(action.abstract_string(c))

        if k in self[3]:
            if c in self[3][k]:
                return [ self[3][k][c].id() ]
        else:
            return []

    def __cmode(self,text,k,c,u):
        k = int(action.abstract_string(k))
        c = int(action.abstract_string(c))

        if k not in self[3]:
            self.__update_lights(k)
            self[3][k] = Key(self,self.controller,k)

        id = self[3][k].eventcalled(text,c)

        if id is None:
            return async.failure('mode in use')

        self[4].update()
        return logic.render_term((id,('echo',)))

    def __query(self,k,u):
        k = int(action.abstract_string(k))

        if k in self[3]:
             return [ v.id() for v in self[3][k].itervalues() ]
        else:
             return []

    def __mode(self,text,k,u):
        k = int(action.abstract_string(k))

        if k not in self[3]:
            self.__update_lights(k)
            self[3][k] = Key(self,self.controller,k)

        id = self[3][k].event(text)
        self[4].update()
        return logic.render_term((id,('echo',)))

    def __cancel_mode(self,id):
        for k,v in self[3].iteritems():
            if v.detach(id,True):
                self[4].update()
                if len(v)==0:
                    del self[3][k]
                return
   
    def __cancel_verb_called(self,subject,key,called):
        key = int(action.abstract_string(key))
        called = int(action.abstract_string(called))

        rv=[]
        if key in self[3]:
            if called in self[3][key]:
                rv.append(action.cancel_return(self.id(),4,self[3][key][called].id()))
                
        return rv

    def __cancel_verb(self,subject,phrase):
        rv=[]
        for o in action.arg_objects(phrase):
            type,thing = action.crack_ideal(o)
            event_id = thing[1]
            rv.append(action.cancel_return(self.id(),4,event_id))
        return rv

    def __attach_mode(self,id,*args):
        for k,v in self[3].iteritems():
            if v.attach(id,*args):
                return

class Upgrader(upgrade.Upgrader):
    
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
