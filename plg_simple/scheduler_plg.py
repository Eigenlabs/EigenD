
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

from pi import agent,atom,action,domain,bundles,utils,logic,node,async,schedproxy,const,upgrade,policy,talker,collection
from plg_simple import scheduler_version as version
import piw
from pi.logic.shortcuts import *

def tsm(arg):
    if not arg:
        return {}
    return action.timespec_map(arg)

def apply_schema(event, schema):
    event.event_clear()

    for c in schema.args[1]:
        if logic.is_pred_arity(c,'u',2):
            event.upper_bound(int(c.args[0]), float(c.args[1]))

        if logic.is_pred_arity(c,'l',2):
            event.lower_bound(int(c.args[0]), float(c.args[1]))

        if logic.is_pred_arity(c,'m',3):
            event.modulo(int(c.args[0]), int(c.args[1]), float(c.args[2]))

        if logic.is_pred_arity(c,'z',4):
            event.zone(int(c.args[0]), int(c.args[1]), float(c.args[2]), float(c.args[3]))

def describe_schema(schema):
    d=[]
    schema = action.unmarshal(schema)

    if schema.args[0]:
        return schema.args[0]

    return schedproxy.describe_schema_list(schema.args[1])

class EventBrowser(atom.Atom):
    def __init__(self,eventlist):
        atom.Atom.__init__(self,names='event',protocols='virtual browse')

        self.__eventlist = eventlist
        self.__timestamp = piw.tsd_time()
        self.__selected=None
        self.update()

    def rpc_displayname(self,arg):
        return 'events'

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'EventBrowser:setselected',path,selected    
        self.__selected=selected
    
    def rpc_activated(self,arg):
        print 'EventBrowser:activated',arg    
        return logic.render_term(('',''))
    
    def rpc_current(self,arg):
        return '[]'

    def resolve_name(self,name):
        print 'EventBrowser:resolve_name',name
        if name=='selection':
            o=self.__selected
            print 'name=selection',o
        else:
            try:
                o=int(name)
                o=int(o)-1
                print 'o=',o
            except:
                return '[]'

        for e in self.__eventlist():
            print 'e.ordinal=',e.ordinal()
            if e.ordinal() == o:
                return '[%s]' % self.__ideal(e)

        return '[]'

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __ideal(self,e):
        return 'ideal([~server,event],[~server,%s])' % logic.render_term(e.id())

    def rpc_fideal(self,arg):
        (path,uid) = logic.parse_clause(arg)
        e = self.__eventlist()[uid]
        return self.__ideal(e)

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',arg,(a,o)

        if a or not o:
            return '[]'

        o=int(o)-1

        for e in self.__eventlist():
            if e.ordinal() == o:
                return '[%s]' % self.__ideal(e)

        return '[]'

    def rpc_enumerate(self,a):
        print 'enumerating',a
        return logic.render_term((len(self.__eventlist()),0))

    def rpc_cinfo(self,a):
        print 'cinfo',a
        return '[]'

    def rpc_finfo(self,a):
        print 'finfo',a
        (dlist,cnum) = logic.parse_clause(a)
        # XXX
        #map = tuple([(i+cnum,e.describe(),None) for (i,e) in enumerate(self.__eventlist()[cnum:])])
        map = tuple([(e.ordinal(),e.describe(),None) for (i,e) in enumerate(self.__eventlist()[cnum:])])
        return logic.render_term(map)

class Event(talker.Talker):
    def __init__(self,scheduler,index):
        self.scheduler = scheduler
        self.index = index
        self.event = piw.event(scheduler.scheduler,False,utils.changify(self.__enable_changed))

        self.key_mapper = piw.function1(True,2,2,piw.data(),scheduler.light_aggregator.get_output(index+1))
        self.key_mapper.set_functor(piw.d2d_const(utils.maketuple_longs((0,self.index),0)))

        talker.Talker.__init__(self,scheduler.finder,self.event.fastdata(),self.key_mapper.cookie(),names='event',ordinal=index,protocols='remove')

        self[3] = atom.Atom(domain=domain.String(), policy=atom.default_policy(self.__change_schema), names='schema')
        self[4] = atom.Atom(domain=domain.Bool(), init=False, policy=atom.default_policy(self.__change_enabled), names='enabled')

    def ordinal(self):
        return self.index

    def describe(self):
        o = self.ordinal()+1
        d = describe_schema(self.get_schema())
        return '%s' %  d

    def get_schema(self):
        try:
            return self[3].get_value()
        except:
            return None

    def compare(self,qschema):
        schema = action.unmarshal(self.get_schema())
        print schema,qschema
        return schema.args[1]==qschema.args[1]

    def __change_schema(self,schema):
        print 'disabling event',id(self.event),schema
        self.event.detach()
        self.event.disable()
        if schema:
            s = action.unmarshal(schema)
            apply_schema(self.event,s)
            if self[4].get_value():
                self.event.enable()
                self.event.event_enable()
            self.event.attach(self.scheduler.controller)
            self.event.set_key(utils.maketuple((piw.makelong(0,0),piw.makelong(self.index,0)), 0))
            self[3].set_value(schema)
            print 'enabling event',id(self.event),'for',s

    def setup(self,schema):
        print 'disabling event',id(self.event)
        self.event.detach()
        self.event.disable()
        self[3].set_value('')

        if schema.is_string() and schema.as_string():
            s = action.unmarshal(schema.as_string())
            apply_schema(self.event,s)
            self.event.enable()
            self.event.event_enable()
            self.event.attach(self.scheduler.controller)
            self.event.set_key(utils.maketuple((piw.makelong(0,0),piw.makelong(self.index,0)), 0))
            self[3].set_value(schema.as_string())
            print 'enabling event',id(self.event),'for',s

    def cancel(self):
        print 'canceling event',id(self.event)
        self.event.detach()
        self.event.disable()
        self.scheduler.light_aggregator.clear_output(self.index+1)
        return self.clear_phrase()

    def __enable_changed(self,d):
        if not d.is_bool():
            return
        print 'setting enabled state to',d
        self[4].set_value(d.as_bool())

    def __change_enabled(self,d):
        if d:
            self.event.enable()
        else:
            self.event.disable()
        print 'setting enabled state to',d
        self[4].set_value(d)


class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='scheduler',container=6,protocols='scheduler',ordinal=ordinal)

        c = dict(c=schedproxy.get_constraints())

        self.add_verb2(1,'do([],None,role(None,[abstract]),role(at,%(c)s),option(until,%(c)s),option(every,%(c)s),option(called,[singular,numeric]))'%c, lambda s,t,a,u,e,c: self.__do_verb(s,t,tsm(a),tsm(u),tsm(e),c))
        self.add_verb2(2,'do([],None,role(None,[abstract]),role(until,%(c)s),option(every,%(c)s),option(called,[singular,numeric]))'%c, lambda s,t,u,e,c: self.__do_verb(s,t,{},tsm(u),tsm(e),c))
        self.add_verb2(3,'do([],None,role(None,[abstract]),role(every,%(c)s),option(called,[singular,numeric]))'%c, lambda s,t,e,c: self.__do_verb(s,t,{},{},tsm(e),c))
        self.add_verb2(4,'cancel([],None,role(None,[singular,numeric]))', self.__verb_cancel)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.scheduler = piw.scheduler(4)
        self.input = bundles.ScalarInput(self.scheduler.cookie(), self.domain,signals=(1,2,3,4,5))
        self.finder = talker.TalkerFinder()

        self[2] = atom.Atom(names='inputs')
        self[2][1] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(1,False), names='running input')
        self[2][2] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(2,False), names='song time input')
        self[2][3] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(3,False), names='bar beat input')
        self[2][4] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(4,False), names='song beat input')
        self[2][5] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(5,False), names='bar input')

        self[3] = collection.Collection(creator=self.__create,wrecker=self.__wreck,names='event',inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst,protocols='hidden-connection')

        self[4] = atom.Atom(names='controller')
        self[4][1] = bundles.Output(1,False, names='light output',protocols='revconnect')
        self.light_output = bundles.Splitter(self.domain,self[4][1])
        self.light_convertor = piw.lightconvertor(self.light_output.cookie())
        self.light_aggregator = piw.aggregator(self.light_convertor.cookie(),self.domain)
        self.controller = piw.controller(self.light_aggregator.get_output(1),utils.pack_str(1,2))
        self.activation_input = bundles.VectorInput(self.controller.event_cookie(), self.domain,signals=(1,2))
        self[4][2] = atom.Atom(domain=domain.Aniso(),policy=self.activation_input.merge_nodefault_policy(2,False),names='controller input',protocols='nostage')
        self[4][3] = atom.Atom(domain=domain.Aniso(),policy=self.activation_input.local_policy(1,False), names='key input',protocols='nostage')

        self[5] = EventBrowser(self.__eventlist)

    def __eventlist(self):
        return self[3].values()

    def rpc_delete_trigger(self,args):
        trigger = action.unmarshal(args)
        for (i,e) in self[3].iteritems():
            if e.trigger_id()==id:
                print 'deleting event',id
                e.cancel()
                del self[3][i]
                self[5].update()

    def __create_event(self,schema,called=None):
        if called:
            if called in self[3]:
                return self[3][called]
            i = called
        else:
            i = self[3].find_hole()
        e = Event(self,i)
        e.setup(piw.makestring(schema,0))
        self[3][i] = e
        e.attached()
        self[5].update()
        return e

    def rpc_create_trigger(self,schema):
        print 'event schema is:',schema
        e = self.__create_event(schema)
        return async.success(e.trigger_id())

    @async.coroutine('internal error')
    def __do_verb(self,subject,text,at,until,every,called):
        text = action.abstract_string(text)
        schema = schedproxy.make_schema(at,until,every)
        called = int(action.abstract_string(called)) if called else None
        print 'mode schema is:',schema

        if called and called in self[3]:
            yield async.Coroutine.success(action.error_return('name in use','','do'))

        e = self.__create_event(schema,called)
        r = e.set_phrase(text)
        yield r 
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __verb_cancel(self,subject,called):
        called = int(action.abstract_string(called))

        if called not in self[3]:
            yield async.Coroutine.success()

        r = self[3][called].cancel()
        del self[3][called]
        self[5].update()

        yield r
        yield async.Coroutine.success()

    def __wreck(self,i,e):
        e.cancel()
        self[5].update()

    def __create(self,i):
        e = Event(self,i)
        self[5].update()
        return e

    @async.coroutine('internal error')
    def __create_inst(self,k):
        e = Event(self,k)
        self[3][k] = e
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,k,e,name):
        r = e.cancel()
        yield r
        yield async.Coroutine.success()

    def rpc_resolve_ideal(self,arg):
        (typ,name) = action.unmarshal(arg)

        if typ != 'event':
            return '[]'

        return self[5].resolve_name(' '.join(name))


agent.main(Agent)
