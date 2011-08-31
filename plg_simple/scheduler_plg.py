
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

from pi import agent,atom,action,domain,bundles,utils,logic,node,async,schedproxy,const,upgrade,policy
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

class Event(atom.Atom):
    def __init__(self,scheduler, index):
        atom.Atom.__init__(self,domain=domain.Aniso(),policy=policy.FastReadOnlyPolicy(),protocols='timeline')
        self.event = piw.event(scheduler.scheduler,False,utils.changify(self.__enable_changed))
        self.__private = node.Server()
        self.__private[1] = node.Server(value=piw.makestring('',0), change=self.__change_schema)
        self.__private[2] = node.Server(value=piw.makebool(False,0), change=self.__change_enabled)
        self.scheduler = scheduler
        self.set_private(self.__private)
        self.get_policy().set_source(self.event.fastdata())
        self.index = index

    def ordinal(self):
        return self.event.ordinal()

    def describe(self):
        o = self.ordinal()+1
        d = describe_schema(self.__private[1].get_data().as_string())
        return '%s' %  d

    def get_schema(self):
        try:
            return self.__private[1].get_data().as_string()
        except:
            return None

    def compare(self,qschema):
        schema = action.unmarshal(self.get_schema())
        print schema,qschema
        return schema.args[1]==qschema.args[1]

    def __change_schema(self,schema):
        print 'disabling event',id(self.event)
        self.event.detach()
        self.event.disable()
        if schema.is_string() and schema.as_string():
            s = action.unmarshal(schema.as_string())
            apply_schema(self.event,s)
            if self.__private[2].get_data().as_bool():
                self.event.enable()
                self.event.event_enable()
            self.event.attach(self.scheduler.controller)
            self.__private[1].set_data(schema)
            print 'enabling event',id(self.event),'for',s

    def setup(self,schema):
        print 'disabling event',id(self.event)
        self.event.detach()
        self.event.disable()
        self.__private[1].set_data(piw.makestring('',0))

        if schema.is_string() and schema.as_string():
            s = action.unmarshal(schema.as_string())
            apply_schema(self.event,s)
            self.event.enable()
            self.event.event_enable()
            self.event.attach(self.scheduler.controller)
            self.__private[1].set_data(schema)
            print 'enabling event',id(self.event),'for',s

    def cancel(self):
        print 'canceling event',id(self.event)
        self.event.detach()
        self.event.disable()

    def __enable_changed(self,d):
        if not d.is_bool():
            return
        print 'setting enabled state to',d
        self.__private[2].set_data(d)

    def __change_enabled(self,d):
        if not d.is_bool():
            return
        if d.as_bool():
            self.event.enable()
        else:
            self.event.disable()
        print 'setting enabled state to',d
        self.__private[2].set_data(d)


class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='scheduler',container=6,protocols='scheduler',ordinal=ordinal)

        c = dict(c=schedproxy.get_constraints())

        self.add_mode2(1,'mode([],role(at,%(c)s),option(until,%(c)s),option(every,%(c)s))'%c, lambda t,a,u,e: self.__mode(t,tsm(a),tsm(u),tsm(e)), lambda a,u,e: self.__query(tsm(a),tsm(u),tsm(e)), self.__mode_cancel)
        self.add_mode2(2,'mode([],role(until,%(c)s), option(every,%(c)s))'%c, lambda t,u,e: self.__mode(t,{},tsm(u),tsm(e)), lambda u,e: self.__query({},tsm(u),tsm(e)), self.__mode_cancel)
        self.add_mode2(3,'mode([],role(every,%(c)s))'%c, lambda t,e: self.__mode(t,{},{},tsm(e)), lambda e: self.__query({},{},tsm(e)), self.__mode_cancel)

        self.add_verb2(4,'cancel([],None,role(None,[ideal([~server,event])]))', self.__verb_cancel)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.scheduler = piw.scheduler(4)
        self.input = bundles.ScalarInput(self.scheduler.cookie(), self.domain,signals=(1,2,3,4,5))

        self[2] = atom.Atom()
        self[2][1] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(1,False), names='running input')
        self[2][2] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(2,False), names='song time input')
        self[2][3] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(3,False), names='bar beat input')
        self[2][4] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(4,False), names='song beat input')
        self[2][5] = atom.Atom(domain=domain.Aniso(), policy=self.input.policy(5,False), names='bar input')

        self[3] = atom.Null(creator = self.__create, wrecker = self.__wreck)

        self[4] = atom.Atom(names='controller')
        self[4][1] = bundles.Output(1,False, names='light output',protocols='revconnect')
        self.control_output = bundles.Splitter(self.domain,self[4][1])
        self.light_convertor = piw.lightconvertor(self.control_output.cookie())
        self.controller = piw.controller(self.light_convertor.cookie(),utils.pack_str(1))
        self.control_input = bundles.VectorInput(self.controller.cookie(), self.domain, signals=(1,))

        self[4][2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.control_input.local_policy(1,False),names='activation input',protocols='nostage')

        self[5] = EventBrowser(self.__eventlist)

    def __eventlist(self):
        return self[3].values()

    def rpc_delete_trigger(self,args):
        trigger = action.unmarshal(args)
        self.__mode_cancel(trigger)

    def rpc_create_trigger(self,schema):
        print 'event schema is:',schema
        i = self[3].find_hole()
        e = Event(self,i)
        e.setup(piw.makestring(schema,0))
        self[3][i] = e
        self[5].update()
        return async.success(e.id())

    def __mode(self, text, at, until, every):
        schema = schedproxy.make_schema(at,until,every,desc=text)
        print 'mode schema is:',schema

        i = self[3].find_hole()
        e = Event(self,i)
        e.setup(piw.makestring(schema,0))
        self[3][i] = e
        self[5].update()
        return logic.render_term((e.id(),('transient',)))

    def __query(self, at, until, every):
        schema = action.unmarshal(schedproxy.make_schema(at,until,every))
        r = [ e.id() for e in self[3].itervalues() if e.compare(schema) ]
        return r

    def __mode_cancel(self,id):
        for (i,e) in self[3].iteritems():
            if e.id()==id:
                print 'deleting event',id
                e.cancel()
                del self[3][i]
                self[5].update()
                return True
        return False

    def __verb_cancel(self,subject,phrase):
        rv=[]
        for o in action.arg_objects(phrase):
            type,thing = action.crack_ideal(o)
            event_id = thing[1]
            rv.append(action.cancel_return(self.id(),1,event_id))
        return rv

    def __wreck(self,i,e):
        e.cancel()
        self[5].update()

    def __create(self,i):
        self[5].update()
        return Event(self,i)

    def rpc_resolve_ideal(self,arg):
        (typ,name) = action.unmarshal(arg)

        if typ != 'event':
            return '[]'

        return self[5].resolve_name(' '.join(name))

class Upgrader(upgrade.Upgrader):
    def upgrade_5_0_to_6_0(self,tools,address):
        root = tools.root(address)
        events = root.get_node(3)
        for e in events.iter():
            schema = e.get_node(255,6).get_data()
            e.ensure_node(255,6).set_data(piw.makenull(0))
            e.ensure_node(255,6,1).set_data(schema)
            e.ensure_node(255,6,2).set_data(piw.makebool(False,0))
        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(6).erase_children()
        root.ensure_node(3).erase_children()
        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        root = tools.root(address)
        events = root.get_node(3)
        for e in events.iter():
            schema_node = e.get_node(255,6)
            old_schema=schema_node.get_string()
            old_schema_term=logic.parse_clause(old_schema)
            new_schema=logic.render_term(T('schema',None,old_schema_term))
            schema_node.set_string(new_schema)
        return True


agent.main(Agent,Upgrader)
