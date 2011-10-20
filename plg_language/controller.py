
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

from pi import atom,action,domain,bundles,async,container,paths,const,node,policy,logic,utils,agent,proxy
from plg_language import interpreter, interpreter_version as version
from pi.logic.shortcuts import *

import piw
import language_native

rules_controller = """
    control_part(P,O) :- @assocwith_extended(P,O).
    control_part(P,O) :- @partof_extended(P,O).
    control_part(P,O) :- @assocwith(X,O),@bind(X,P).
    find_controls(T,OL) :- @is(OL,$alluniq([O,C],control_part(O,T),db_control(C,O))).
"""

class Trigger:
    def __init__(self,t,index,controller):
        self.__trigger = piw.fasttrigger(3)
        self.__trigger.attach_to(controller.controller,index)

    def detach(self):
        self.__trigger.detach()

    def fastdata(self):
        return self.__trigger.fastdata()

    def reset(self,v):
        pass

class Toggle:
    def __init__(self,t,index,controller):
        self.__toggle = language_native.toggle(t.get_data())
        self.__toggle.attach_to(controller.controller,index)

    def detach(self):
        self.__toggle.detach()

    def fastdata(self):
        return self.__toggle.fastdata()

    def reset(self,v):
        self.__toggle.reset(v)

class UpDown:
    def __init__(self,t,index,controller):
        self.__updown = language_native.updown(t.get_data(),t.domain().biginc,t.domain().inc)
        self.__updown.attach_to(controller.controller,index)

    def detach(self):
        self.__updown.detach()

    def fastdata(self):
        return self.__updown.fastdata()

    def reset(self,v):
        self.__updown.reset(v)

class Selector:
    def __init__(self,t,index,controller):
        self.__controller = controller
        self.__index = index
        self.__gate = self.__controller.clone.gate(self.__index+1)
        self.__selector = language_native.xselector(self.__gate,t.get_data())
        self.__controller.clone.set_output(self.__index+1,self.__selector.cookie())

        choices = t.domain().hint('choices')
        for (i,c) in enumerate(choices):
            self.__selector.set_choice(i+1,piw.makestring(c,0))

        self.__selector.attach_to(self.__controller.controller,self.__index)

    def detach(self):
        self.__selector.detach()
        self.__controller.clone.clear_output(self.__index+1)
        fc = piw.fastchange(self.__gate)
        fc(piw.makefloat_bounded(1,0,0,1,0))

    def fastdata(self):
        return self.__selector.fastdata()

    def reset(self,v):
        pass

class Monitor(proxy.AtomProxy):
    def __init__(self,ready,handler):
        proxy.AtomProxy.__init__(self)
        self.set_change_handler(utils.changify(handler))
        self.__ready = ready
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)

    def attach(self,id):
        self.__anchor.set_address_str(id)

    def detach(self):
        self.__anchor.set_address_str('')

    def node_ready(self):
        self.__ready()

class Connector(atom.Atom):

    controls = dict(updown=UpDown,selector=Selector,trigger=Trigger,toggle=Toggle)

    def __init__(self,controller,index,tag):
        atom.Atom.__init__(self,domain=domain.Aniso(),policy=policy.FastReadOnlyPolicy())
        self.set_private(node.Server(value=piw.makestring(tag,0),change=self.__change))

        self.set_latency(controller.controller.latency)
        self.get_policy().set_clock(controller.controller.clock)

        self.controller = controller
        self.index = index
        self.control = None
        self.opened = async.Deferred()
        self.monitor = Monitor(self.__ready,self.__reset)
        self.factory = None
        self.control = None
        self.target_domain = None

    def set_controller_clock(self,clock):
        self.get_policy().set_clock(clock)

    def set_controller_latency(self,latency):
        self.set_latency(latency)

    def server_opened(self):
        atom.Atom.server_opened(self)
        tag = self.get_private().get_data()
        if tag.is_string() and tag.as_string():
            self.__setup(tag.as_string())
            self.opened.succeeded()

    def __change(self,tag):
        if tag.is_string() and tag.as_string():
            if self.__setup(tag.as_string()):
                self.get_private().set_data(tag)

    def __ready(self):
        factory = self.controls.get(self.factory)
        if factory:
            self.control = factory(self.monitor,self.index,self.controller)
            self.get_policy().set_source(self.control.fastdata())

    def __reset(self,v):
        if self.control:
            self.control.reset(v)

    def __setup(self,tag):
        bits = tag.split(':')

        if len(bits)!=2 or bits[1] not in self.controls:
            return False

        self.monitor.attach(bits[0])
        self.factory = bits[1]
        return True

    def disconnect(self):
        self.monitor.detach()
        self.monitor = None
        if self.control:
            self.control.detach()
            self.control = None

class Controller0(piw.controller):
    def __init__(self,controller,cookie,sigmap):
        piw.controller.__init__(self,cookie,sigmap)
        self.clock = None
        self.latency = 0
        self.controller = controller

    def controller_clock(self,c):
        self.clock = c
        self.controller.clock_changed(c)

    def controller_latency(self,l):
        self.latency = l
        self.controller.latency_changed(l)

class Controller(agent.Agent):

    subsys_relation = ('create','by')

    def __init__(self,database,master_agent,ordinal):
        agent.Agent.__init__(self,names='controller',signature=version,subsystem='controller',container=2,protocols='is_subsys',ordinal=ordinal)
        self.database = database
        self.agent = master_agent
        self.domain = master_agent.domain

        self.add_verb2(1,'control([],None,role(None,[concrete,singular]))',callback=self.__control)
        self.add_verb2(2,'control([],None,role(None,[concrete,singular]),role(with,[numeric]))',callback=self.__controlwith)
        self.add_verb2(3,'control([un],None)',callback=self.__uncontrol)

        self[4] = atom.Atom(creator=self.__create,wrecker=self.__wreck)

        self[3] = bundles.Output(1,False,names='light output',protocols='revconnect')
        self.lights = bundles.Splitter(self.domain,self[3])
        self.lightconvertor = piw.lightconvertor(self.lights.cookie())
        self.controller = Controller0(self,self.lightconvertor.cookie(),utils.pack_str(1,2,3,4,5))
        self.clone = piw.clone(True)
        self.clone.set_output(1,self.controller.cookie())
        self.input = bundles.VectorInput(self.clone.cookie(),self.domain,signals=(1,2,3,4,5))

        self[1] = atom.Atom()
        self[1][1] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(1,False),names='activation input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(2,False),names='pressure input')
        self[1][3] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(3,False),names='roll input')
        self[1][4] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(4,False),names='yaw input')
        self[1][5] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(5,False),names='strip position input')


    def server_opened(self):
        agent.Agent.server_opened(self)
        self.set_frelation('create(cnc("%s"),role(by,[instance("%s")]))' % (self.id(),self.agent.id()))

    def disconnect(self):
        self.set_frelation('')

    @async.coroutine('internal error')
    def __control(self,s,t):
        t = action.concrete_object(t)
        r = self.database.search_any_key('OL',T('find_controls',t,V('OL')))

        for target,ctl in r:
            tag=':'.join((target,ctl))
            i = self[4].find_hole()
            c = Connector(self,i,tag) 
            p = self.database.find_item(target)
            self[4][i] = c
            yield c.opened
            cs = logic.render_term(logic.make_term('ctl',c.id(),None))
            yield interpreter.RpcAdapter(p.invoke_rpc('control',cs))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __controlwith(self,s,t,k):
        k = int(action.abstract_string(k))
        if self[4].has_key(k):
            yield async.Coroutine.failure('key already defined')

        t = action.concrete_object(t)
        p = self.database.find_item(t)
        ctl = p.domain().hint('control')
        if not ctl:
            yield async.Coroutine.failure('%s cannot be controlled' % t)

        c = Connector(self,k,':'.join((t,ctl[0])))
        self[4][k] = c

        yield c.opened
        cs = logic.render_term(logic.make_term('ctl',c.id(),None))
        yield interpreter.RpcAdapter(p.invoke_rpc('control',cs))
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __uncontrol(self,s):
        for k,v in self[4].items():
            tag = v.get_private().get_data().as_string()
            if tag:
                target,ctl = tag.split(':')
                p = self.database.find_item(target)
                cs = logic.render_term(logic.make_term('ctl',v.id(),None))
                yield interpreter.RpcAdapter(p.invoke_rpc('uncontrol',cs))

            v.disconnect()
            del self[4][k]

        yield async.Coroutine.success()

    def clock_changed(self,clock):
        for i in self[4].itervalues():
            i.set_controller_clock(clock)

    def latency_changed(self,latency):
        for i in self[4].itervalues():
            i.set_controller_latency(latency)

    def __create(self,index):
        connector = Connector(self,index,'')
        return connector

    def __wreck(self,index,connector):
        connector.disconnect()

    def close_server(self):
        for i in self[4].itervalues():
            i.disconnect()
        agent.Agent.close_server(self)

def upgrade_0_0_to_1_0(root):
    root[1][1].rename(adjectives='input',names='activation')
    root[1][2].rename(adjectives='input',names='pressure')
    root[1][3].rename(adjectives='input',names='roll')
    root[1][4].rename(adjectives='input',names='yaw')
    root[1][5].rename(adjectives='input',names='strip position')
    root[3].rename(adjectives='output',names='light')

def upgrade_1_0_to_2_0(root):
    root[1][1].rename(names='input')
    root[1][2].rename(names='input')
    root[1][3].rename(names='input')
    root[1][4].rename(names='input')
    root[1][5].rename(names='input')
    root[3].rename(names='output')

def upgrade_5_0_to_6_0(tools,address,ss):
    root = tools.root(address)
    root.ensure_node(2).erase_children()
    return True

def upgrade_3_0_to_4_0(tools,address,ss):
    root = tools.root(ss)
    ctl = root[4]

    for connector in ctl.iter():
        conn_id = paths.makeid_list(ss,*connector.path)
        target_id = connector[255][6].get_data().as_string().split(':')[0]
        target_addr,target_path = paths.breakid_list(target_id)
        old_conn = T('conn',None,None,conn_id,None)
        new_conn = logic.render_term(T('ctl',conn_id,None))
        print 'upgrade',conn_id,target_addr,target_path
        target_node = tools.root(target_addr).get_node(*target_path)
        target_conn = logic.parse_clauselist(target_node[255][2].get_data().as_string())
        if old_conn in target_conn:
            print 'removing',old_conn,'from',target_conn,type(target_conn)
            target_conn = list(target_conn)
            target_conn.remove(old_conn)
            target_conn = tuple(target_conn)
            target_node[255][2].set_data(piw.makestring(logic.render_termlist(target_conn),0))
            print 'target conn',target_conn

        target_node.ensure_node(255,const.meta_control).set_data(piw.makestring(new_conn,0))

    return True

def upgrade_7_0_to_8_0(tools,address,ss):
    print 'upgrading',ss
    root = tools.root(ss)
    for k in root.ensure_node(4).iter():
        bits = k.get_node(255,const.meta_private).get_data().as_string().split(':')
        addr,path = paths.breakid_list(bits[0])
        n = tools.root(addr).ensure_node(*path).ensure_node(255,const.meta_control)
        t = T('ctl',k.id(),None)
        meta = set(logic.parse_clauselist(n.get_data().as_string()))
        meta.add(t)
        meta = tuple(meta)
        n.set_data(piw.makestring(logic.render_termlist(meta),0))
        print bits[0],'new control meta',n.get_data()
    return True

