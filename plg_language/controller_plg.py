
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

from pi import agent,atom,action,domain,bundles,async,container,paths,const,node,policy,logic,utils,proxy,collection
from pi.logic.shortcuts import *
import piw
from . import controller_version as version,language_native

class Trigger:
    def __init__(self,t,index,controller):
        self.__trigger = piw.fasttrigger(3)
        self.__trigger.attach_to(controller.controller,index)

    def set_key(self,d):
        self.__trigger.set_key(d)

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

    def set_key(self,d):
        self.__toggle.set_key(d)

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

    def set_key(self,d):
        self.__updown.set_key(d)

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

    def set_key(self,d):
        self.__selector.set_key(d)

    def detach(self):
        self.__selector.detach()
        self.__controller.clone.clear_output(self.__index+1)
        fc = piw.fastchange(self.__gate)
        fc(piw.makefloat_bounded(1,0,0,1,0))

    def fastdata(self):
        return self.__selector.fastdata()

    def reset(self,v):
        pass

class DataProxy(node.Client):
    def __init__(self,connector):
        self.__connector = connector
        node.Client.__init__(self)

    def client_opened(self):
        node.Client.client_opened(self)
        self.__connector.monitor_data(self.get_data())

    def close_client(self):
        node.Client.close_client(self)

    def client_data(self,v):
        self.__connector.monitor_data(v)

class Monitor(proxy.AtomProxy):

    monitor = set(['latency','domain'])

    def __init__(self,connector,address):
        proxy.AtomProxy.__init__(self)
        self.address = address
        self.__connector = connector
        self.__dataproxy = None
        self.__mainanchor = piw.canchor()
        self.__mainanchor.set_client(self)
        self.__mainanchor.set_address_str(address)

    def disconnect(self):
        self.__dataproxy = None
        self.set_data_clone(self.__dataproxy)
        self.__mainanchor.set_address_str('')

    def node_ready(self):
        self.__dataproxy = DataProxy(self.__connector)
        self.__connector.monitor_connected(self)
        self.set_data_clone(self.__dataproxy)

    def node_removed(self):
        self.__connector.monitor_disconnected()
        self.set_data_clone(None)
        self.__dataproxy = None

    def node_changed(self,parts):
        if 'domain' in parts:
            self.node_removed()
            self.node_ready()
            return

class ConnectorOutput(atom.Atom):

    def __init__(self,connector):
        self.__connector = connector
        self.__monitor = None

        atom.Atom.__init__(self,domain=domain.Aniso(),names='output',policy=policy.FastReadOnlyPolicy(),protocols="connect-static output nostage")

    def property_change(self,key,value,delegate):
        if key != 'slave':
            return

        cur_slaves = self.get_property_termlist('slave')

        if self.__monitor:
            if not self.__monitor.address in cur_slaves:
                self.__monitor.disconnect()
                self.__monitor = None

        if not self.__monitor:
            if cur_slaves:
                self.__monitor = Monitor(self.__connector,cur_slaves[0])
            

class Connector(atom.Atom):

    controls = dict(updown=UpDown,selector=Selector,trigger=Trigger,toggle=Toggle)

    def __init__(self,controller,index,tag):
        atom.Atom.__init__(self,names="connector",ordinal=index,protocols='remove')

        self.controller = controller
        self.index = index
        self.control = None
        self.monitor = None

        self[1] = atom.Atom(domain=domain.BoundedInt(-32767,32767),names='key row',init=0,policy=atom.default_policy(self.__change_key_row),protocols="input explicit")
        self[2] = atom.Atom(domain=domain.BoundedInt(-32767,32767),names='key column',init=0,policy=atom.default_policy(self.__change_key_column),protocols="input explicit")
        self[4] = ConnectorOutput(self)

    def monitor_connected(self,proxy):
        self.monitor_disconnected()
        factory = self.controls.get(proxy.domain().control)

        if factory:
            self.control = factory(proxy,self.index,self.controller) 
            self[4].get_policy().data_node().set_source(self.control.fastdata())
            self.__update_event_key()

    def monitor_disconnected(self):
        if self.control:
            self[4].get_policy().data_node().clear_source()
        self.disconnect()

    def __change_key_row(self,val):
        self[1].set_value(val)
        self.__update_event_key()
        return False

    def __change_key_column(self,val):
        self[2].set_value(val)
        self.__update_event_key()
        return False

    def __update_event_key(self):
        if self.control:
            key = utils.maketuple((piw.makelong(self[1].get_value(),0),piw.makelong(self[2].get_value(),0)), 0)
            self.control.set_key(key) 

    def set_controller_clock(self,clock):
        self.get_policy().set_clock(clock)

    def set_controller_latency(self,latency):
        self.set_latency(latency)

    def monitor_data(self,v):
        if self.control:
            self.control.reset(v)

    def disconnect(self):
        if self.control:
            self.control.detach()
            self.control = None

class ConnectorList(collection.Collection):

    def __init__(self,agent):
        self.__agent = agent
        self.__timestamp = piw.tsd_time()

        collection.Collection.__init__(self,names='connector',creator=self.__create_connector,wrecker=self.__wreck_connector,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __create_connector(self,index):
        return Connector(self.__agent, index, '')
    
    def __wreck_connector(self,index,connector):
        connector.disconnect()

    def create_connector(self,ordinal=None):
        o = ordinal or self.find_hole()
        o = int(o)
        e = Connector(self.__agent, o, '')
        self[o] = e
        self.__agent.update()
        return e 

    def get_connector(self,index):
        return self.get(index)

    def del_connector(self,index):
        v = self[index]
        del self[index]
        v.disconnect()
    
    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        e = self.create_connector(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,key,inst,ordinal):
        inst.disconnect()
        yield async.Coroutine.success()

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

class Agent(agent.Agent):

    def __init__(self,address,ordinal):
        agent.Agent.__init__(self,names='controller',signature=version,ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[4] = ConnectorList(self)

        self[3] = bundles.Output(1,False,names='light output',protocols='revconnect')
        self.lights = bundles.Splitter(self.domain,self[3])
        self.lightconvertor = piw.lightconvertor(False,self.lights.cookie())
        self.controller = Controller0(self,self.lightconvertor.cookie(),utils.pack_str(1,2,3,4,5,6))
        self.clone = piw.clone(True)
        self.clone.set_output(1,self.controller.event_cookie())
        self.input = bundles.VectorInput(self.clone.cookie(),self.domain,signals=(1,2,3,4,5,6))

        self[1] = atom.Atom(names='inputs')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(3,False),names='pressure input')
        self[1][3] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(4,False),names='roll input')
        self[1][4] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(5,False),names='yaw input')
        self[1][5] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.vector_policy(6,False),names='strip position input')
        self[1][6] = atom.Atom(domain=domain.Aniso(),policy=self.input.vector_policy(1,False), names='key input')
        self[1][7] = atom.Atom(domain=domain.Aniso(),policy=self.input.merge_nodefault_policy(2,False),names='controller input')

        self.add_verb2(1,'create([],None,role(None,[mass([connector])]))', self.__create_connector)
        self.add_verb2(2,'create([un],None,role(None,[mass([connector])]))', self.__uncreate_connector)

    def __create_connector(self,subj,mass):
        id = int(action.mass_quantity(mass))
        e = self[4].create_connector(id)
        if not isinstance(e,Connector):
            return e

    def __uncreate_connector(self,subj,mass):
        id = int(action.mass_quantity(mass))
        connector = self[4].get_connector(id)

        if connector is None:       
            thing='connector %s' %str(id)
            return async.success(errors.invalid_thing(thing,'un create'))

        self[4].del_connector(id)
        
    def clock_changed(self,clock):
        for i in self[4].itervalues():
            i.set_controller_clock(clock)

    def latency_changed(self,latency):
        for i in self[4].itervalues():
            i.set_controller_latency(latency)

    def close_server(self):
        for i in self[4].itervalues():
            i.disconnect()
        agent.Agent.close_server(self)

agent.main(Agent)
