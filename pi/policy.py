
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
from pi import const,domain,proxy,node,utils,logic,paths,container

class FilterStreamPolicy:
    def __init__(self,f):
        self.__f = f

    def create_converter(self,iso):
        return piw.filtering_converter(self.__f)

class TriggerStreamPolicy:
    def create_converter(self,iso):
        return piw.triggering_converter()

class ImpulseStreamPolicy:
    def create_converter(self,iso):
        return piw.impulse_converter()

class LopassStreamPolicy:
    def __init__(self,f,c):
        self.__f = f
        self.__c = c

    def create_converter(self,iso):
        return piw.lopass_converter(self.__f,self.__c)

class AnisoStreamPolicy:
    def create_converter(self,iso):
        return piw.null_converter()

class IsoStreamPolicy:
    def __init__(self,ubound,lbound,rest):
        self.__ubound=ubound
        self.__lbound=lbound
        self.__rest=rest

    def create_converter(self,iso):
        if iso:
            return piw.resampling_converter()
        else:
            return piw.interpolating_converter(self.__ubound,self.__lbound,self.__rest)

class ThrottleStreamPolicy:
    def __init__(self,interval):
        self.__interval=interval

    def create_converter(self,iso):
        return piw.throttling_converter(self.__interval)

def DefaultStreamPolicy(iso):
    if iso is True: return IsoStreamPolicy(1,-1,0)
    if iso is False: return AnisoStreamPolicy()
    return iso

class NullPolicyImpl:
    protocols = ''

    def __init__(self,atom,data_domain,init,transient):
        self.__datanode = node.Server(transient=transient)

    def data_node(self):
        return self.__datanode

    def get_data(self):
        raise RuntimeError("unimplemented in Null Policy")

    def set_data(self,d):
        raise RuntimeError("unimplemented in Null Policy")

    def change_value(self,v,t=0,p=True):
        raise RuntimeError("unimplemented in Null Policy")

    def set_value(self,v,t=0):
        raise RuntimeError("unimplemented in Null Policy")

    def get_value(self):
        raise RuntimeError("unimplemented in Null Policy")

    def close(self):
        pass

class ReadOnlyPolicyImpl:
    protocols = 'output'

    def __init__(self,atom,data_domain,init,transient):
        self.__datanode = node.Server(transient=transient)
        atom.set_property_string('domain',str(data_domain))
        self.__value = init
        self.__data_domain = data_domain
        self.set_value(self.__value)

    def data_node(self):
        return self.__datanode

    def get_data(self):
        return self.data_node().get_data()

    def set_data(self,d):
        self.data_node().set_data(d)

    def change_value(self,v,t=0,p=False):
        d=self.__data_domain.value2data(v,t)
        self.data_node().set_data(d)
        self.__value=v

    def set_value(self,v,t=0):
        d=self.__data_domain.value2data(v,t)
        self.data_node().set_data(d)
        self.__value=v

    def get_value(self):
        return self.__value

    def close(self):
        pass

class FastReadOnlyPolicyImpl:
    protocols = 'output'

    def __init__(self,atom,data_domain,init,transient):
        self.__data_domain = data_domain
        self.__datanode = node.Server(transient = transient)
        atom.set_property_string('domain',str(data_domain))

    def data_node(self):
        return self.__datanode

    def set_source(self,src):
        self.data_node().set_source(src)

    def set_clock(self,src):
        self.data_node().set_clock(src)

    def close(self):
        pass

    def get_data(self):
        raise RuntimeError("unimplemented in FastReadOnlyPolicy")

    def set_data(self,d):
        raise RuntimeError("unimplemented in FastReadOnlyPolicy")

    def change_value(self,v,t=0,p=True):
        raise RuntimeError("unimplemented in FastReadOnlyPolicy")

    def set_value(self,v,t=0):
        raise RuntimeError("unimplemented in FastReadOnlyPolicy")

    def change_value(self,v,t=0,p=True):
        raise RuntimeError("unimplemented in FastReadOnlyPolicy")

    def get_value(self):
        raise RuntimeError("unimplemented in FastReadOnlyPolicy")


class PlumberConnector(piw.connector):
    def __init__(self,correlator,factory,config):
        self.__factory = factory
        piw.connector.__init__(self,correlator,config.iid,config.signal,config.priority,config.type,config.filter,config.clocked)

    def create_converter(self):
        return self.__factory()

class Plumber(proxy.AtomProxy):

    input_merge = 0
    input_latch = 1
    input_input = 2
    input_linger = 3

    monitor = set(['latency','domain'])

    def __init__(self,policy,config):
        proxy.AtomProxy.__init__(self)
        self.__policy = policy
        self.__config = config
        self.__connector = None
        self.__correlator = None 
        self.__stream_policy = None
        self.__mainanchor = piw.canchor()
        self.__mainanchor.set_client(self)
        self.__mainanchor.set_address_str(config.address)

    def clocked(self):
        return self.__config.clocked

    def connect_static(self):
        return self.__config.hint == 'ctl'

    def prepare(self,correlator,stream_policy):
        self.__correlator = correlator
        self.__stream_policy = stream_policy

    def disconnect(self):
        self.set_data_clone(None)
        self.__mainanchor.set_address_str('')
        self.__connector = None

    def node_ready(self):
        self.__policy.prepare_plumber(self)

        iso = self.domain().iso()
        pol = self.__stream_policy

        def __factory():
            return pol.create_converter(iso)

        self.__connector = PlumberConnector(self.__correlator,__factory,self.__config)
        self.__correlator.set_latency(self.__config.signal,self.__config.iid,self.latency())
        self.set_data_clone(self.__connector)

        if self.__config.callback:
            self.__config.callback(self)

    def set_clocked(self,c):
        self.__clock = c
        if self.__connector:
            self.__connector.set_clocked(c)

    def node_removed(self):
        self.set_data_clone(None)
        self.__connector = None
        self.__correlator.remove_latency(self.__config.signal,self.__config.iid)

        if self.__config.callback:
            self.__config.callback(None)

    def node_changed(self,parts):
        if 'domain' in parts:
            self.node_removed()
            self.node_ready()
            return

        if 'latency' in parts:
            self.__correlator.set_latency(self.__config.signal,self.__config.iid,self.latency())

class PlumberConfig:

    __slots__ = ('address','filter','iid','hint','clocked','signal','priority','type','callback')

    def __init__(self,address,filter,iid,hint=None,clocked=False,signal=1,priority=-1,type=Plumber.input_input,callback=None):
        self.address=address
        self.filter=filter
        self.iid=iid
        self.hint=hint
        self.clocked=clocked
        self.signal=signal
        self.priority=priority
        self.type=type
        self.callback=None

class PlumberSlot:
    def __init__(self,iid,src,plumber):
        self.src = src
        self.plumber = plumber
        self.iid = iid

    def set_clocked(self,clock):
        self.plumber.set_clocked(clock)

    def __cmp__(self,other):
        if isinstance(other,PlumberSlot):
            return cmp(self.src,other.src)
        return -1

    def __hash__(self):
        return hash(self.src)


class ConnectablePolicyImpl:
    protocols = 'input'

    def __init__(self,atom,data_domain,init,clocked,data_node,auto_slot):
        self.__closed = False
        self.__datanode = data_node
        self.__value = init
        self.__data_domain = data_domain
        self.__clock = clocked
        self.__connection_iids = set()
        self.__auto_slot = auto_slot
        atom.set_property_string('domain',str(data_domain))
        self.__connections = container.PersistentMetaData(atom,'master',asserted=self.__add_connection, retracted=self.__del_connection)
        self.set_value(self.__value)

    def data_node(self):
        return self.__datanode

    def get_data(self):
        return self.data_node().get_data()

    def get_domain(self):
        return self.__data_domain

    def set_data(self,d):
        self.data_node().set_data(d)

    def change_value(self,v,t=0,p=False):
        d=self.__data_domain.value2data(v,t)
        self.data_node().set_data(d)
        self.__value=v

    def set_value(self,v,t=0):
        d=self.__data_domain.value2data(v,t)
        self.data_node().set_data(d)
        self.__value=v

    def get_value(self):
        return self.__value

    def closed(self):
        return self.__closed

    def destroy_plumber(self,term,plumber):
        pass

    def create_plumber(self,init,config):
        return None

    def prepare_plumber(self,plumber):
        pass

    def make_filter(self,stream,slot):
        if logic.is_pred_arity(stream,'conn',5,5):
            if stream.args[4] == 'ctl':
                using = slot
            else:
                if stream.args[0] is not None:
                    using = int(stream.args[0])
                else:
                    using = 0

                if self.__auto_slot:
                    using = using | (slot<<8)

            if stream.args[1] is not None:
                tgt = int(stream.args[1])
            else:
                tgt = 0

            id=stream.args[2]
            path=stream.args[3]

            if path is not None:
                return (id,piw.signal_dsc_filter(using,tgt,path))
            else:
                return (id,piw.signal_cnc_filter(using,tgt))

        print 'cop out of',stream
        return ('',piw.null_filter())

    def close(self):
        self.__closed = True
        self.__connections.clear()

    def set_clocked(self,c):
        self.__clock = c
        self.__connections.visit(lambda v,s: s.set_clocked(c))

    def __add_connection(self,src):
        first = (len(self.__connection_iids)==0)
        iid = (max(self.__connection_iids)+1 if self.__connection_iids else 1)

        (a,f) = self.make_filter(src,iid)

        if not paths.valid_id(a):
            return None

        self.__connection_iids.add(iid)

        hint = None
        if logic.is_pred_arity(src,'conn',5,5):
            hint = src.args[4]

        return PlumberSlot(iid,src,self.create_plumber(first,PlumberConfig(a,f,iid,hint,self.__clock)))

    def __del_connection(self,src,slot):
        self.__connection_iids.discard(slot.iid)
        slot.plumber.disconnect()
        last = (len(self.__connection_iids)==0)
        self.destroy_plumber(last,slot.plumber)


class FunctorController:
    def __init__(self,cdomain,policy=None,functor=None):
        self.__policy = policy or ThrottleStreamPolicy(10)
        f = functor or piw.slowchange(utils.changify(self.__dump))
        self.__backend = piw.functor_backend(1,False)
        self.__backend.set_gfunctor(utils.make_change_nb(f))
        self.__correlator = piw.correlator(cdomain,chr(1),piw.null_filter(),self.__backend.cookie(),0,0)

    def __dump(self,d):
        print 'default control',d

    def prepare(self,plumber):
        plumber.prepare(self.__correlator,self.__policy)

class FastPolicyImpl(ConnectablePolicyImpl):
    def __init__(self,atom,data_domain,init,transient,handler,stream_policy=None,clock=True):
        self.__stream_policy = stream_policy or ThrottleStreamPolicy(500)
        self.__handler = handler
        self.__slow_handler = utils.fastchange(self.__handler)
        self.__ctl_handler = piw.change2(self.__slow_handler,piw.slowchange(utils.changify(self.__control)))
        self.__clock_domain = piw.clockdomain_ctl()
        self.__clock_domain.set_source(piw.makestring('*', 0L))
        self.__clock = piw.clocksink()
        self.__clock_domain.sink(self.__clock,'FastPolicy')
        self.__upstream = None
        self.__backend = None
        self.__correlator = None
        self.__ctrl = None
        ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clock,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__slow_handler)
        self.data_node().set_data(self.get_domain().value2data(init))

    def __control(self,d):
        v = self.get_domain().data2value(d)
        ConnectablePolicyImpl.set_value(self,v)

    def set_value(self,v,t=0):
        d = self.get_domain().value2data(v,t)
        self.__slow_handler(d)

    def change_value(self,v,t=0,p=True):
        self.set_value(v,t)

    def get_clock(self):
        return self.__clock

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                self.__ctrl = FunctorController(self.__clock_domain,self.__stream_policy,self.__ctl_handler)
            self.__ctrl.prepare(plumber)
        else:
            if self.__backend is None:
                self.__backend = piw.functor_backend(1,True)
                self.__backend.set_functor(piw.pathnull(0),utils.make_change_nb(self.__handler))
                self.__correlator = piw.correlator(self.__clock_domain,chr(1),piw.root_filter(),self.__backend.cookie(),0,0)
            if plumber.clocked():
                self.__set_clock(self.__backend.get_clock())
            plumber.prepare(self.__correlator,self.__stream_policy)

    def create_plumber(self,init,config):
        return Plumber(self,config)

    def __set_clock(self,clock):
        if self.__upstream is not None:
            self.__clock.remove_upstream(self.__upstream)

        self.__upstream = clock

        if self.__upstream is not None:
            self.__clock.add_upstream(self.__upstream)

    def destroy_plumber(self,term,plumber):
        if term:
            self.__correlator = None
            self.__backend = None
            self.__ctrl = None


class TriggerFunctorController:
    def __init__(self,cdomain,policy=None,functor=None):
        self.__policy = policy or ThrottleStreamPolicy(10)
        f = functor or piw.slowchange(utils.changify(self.__dump))
        self.__backend = piw.functor_backend(1,True)
        self.__backend.send_duplicates(True)
        self.__backend.set_gfunctor(f)
        self.__correlator = piw.correlator(cdomain,chr(1),piw.null_filter(),self.__backend.cookie(),0,0)

    def __dump(self,d):
        print 'default control',d

    def prepare(self,plumber):
        plumber.prepare(self.__correlator,self.__policy)

class TriggerPolicyImpl(ConnectablePolicyImpl):
    def __init__(self,atom,data_domain,init,transient,handler,stream_policy=None,clock=True):
        self.__stream_policy = stream_policy or ThrottleStreamPolicy(500)
        self.__handler = handler
        self.__slow_handler = piw.fastchange(self.__handler)
        self.__clock_domain = piw.clockdomain_ctl()
        self.__clock_domain.set_source(piw.makestring('*', 0L))
        self.__clock = piw.clocksink()
        self.__clock_domain.sink(self.__clock,'TriggerPolicy')
        self.__upstream = None
        self.__backend = None
        self.__correlator = None
        self.__ctrl = None
        ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clock,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__slow_handler)
        self.data_node().set_data(self.get_domain().value2data(0))

    def set_value(self,v,t=0):
        pass

    def get_value(self):
        return 0

    def set_status(self,v):
        ConnectablePolicyImpl.set_value(self, v.as_long())

    def change_value(self,v,t=0,p=True):
        pass

    def get_clock(self):
        return self.__clock

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                self.__ctrl = TriggerFunctorController(self.__clock_domain,self.__stream_policy,self.__handler)
            self.__ctrl.prepare(plumber)
        else:
            if self.__correlator is None:
                self.__backend = piw.functor_backend(1,True)
                self.__backend.set_functor(piw.pathnull_lck(0),self.__handler)
                self.__backend.send_duplicates(True)
                self.__correlator = piw.correlator(self.__clock_domain,chr(1),piw.root_filter(),self.__backend.cookie(),0,0)
            if plumber.clocked():
                self.__set_clock(self.__backend.get_clock())
            plumber.prepare(self.__correlator,self.__stream_policy)

    def create_plumber(self,init,config):
        return Plumber(self,config)

    def __set_clock(self,clock):
        if self.__upstream is not None:
            self.__clock.remove_upstream(self.__upstream)

        self.__upstream = clock

        if self.__upstream is not None:
            self.__clock.add_upstream(self.__upstream)

    def destroy_plumber(self,term,plumber):
        if term:
            self.__correlator = None
            self.__backend = None
            self.__ctrl = None


class LoadPolicyNode(node.Server):
    def __init__(self,load_func,transient):
        node.Server.__init__(self,transient=transient)
        self.__load_func = load_func
        self.set_readwrite()

    def load_value(self,delegate,value):
        return self.__load_func(delegate,value)

class LoadPolicyDelegate:
    def retval(self): pass
    def set_residual(self,n,r): pass
    def set_deferred(self,n,r): pass
    def add_error(self,msg): pass

class LoadPolicyImpl(ConnectablePolicyImpl):
    def __init__(self,atom,data_domain,init,transient,handler,stream_policy=None,clock=False):
        self.__stream_policy = stream_policy or ThrottleStreamPolicy(500)
        self.__handler = utils.weaken(handler)
        self.__clock_domain = piw.clockdomain_ctl()
        self.__clock_domain.set_source(piw.makestring('*', 0L))
        self.__backend = None
        self.__correlator = None
        self.__ctrl = None
        ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clock,LoadPolicyNode(self.__handler,transient),False)
        self.data_node().set_data(self.get_domain().value2data(init))

    def __slow_handler(self,d):
        self.__handler(LoadPolicyDelegate(),d)

    def change_value(self,v,t=0,p=True):
        d = self.get_domain().value2data(v,t)
        self.__slow_handler(d)

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                self.__ctrl = FunctorController(self.__clock_domain,functor=piw.slowchange(utils.changify(self.__slow_handler)))
            self.__ctrl.prepare(plumber)
        else:
            if self.__correlator is None:
                self.__backend = piw.functor_backend(1,True)
                self.__backend.set_functor(piw.pathnull(0),utils.make_change_nb(piw.slowchange(utils.changify(self.__slow_handler))))
                self.__correlator = piw.correlator(self.__clock_domain,chr(1),piw.root_filter(),self.__backend.cookie(),0,0)
            plumber.prepare(self.__correlator,self.__stream_policy)

    def create_plumber(self,init,config):
        return Plumber(self,config)

    def destroy_plumber(self,term,plumber):
        if term:
            self.__correlator = None
            self.__backend = None
            self.__ctrl = None


class SlowPolicyImpl(ConnectablePolicyImpl):
    def __init__(self,atom,data_domain,init,transient,handler,stream_policy=None,clock=False,callback=None):
        self.__stream_policy = stream_policy or ThrottleStreamPolicy(500)
        self.__handler = utils.weaken(handler)
        self.__clock_domain = piw.clockdomain_ctl()
        self.__clock_domain.set_source(piw.makestring('*', 0L))
        self.__backend = None
        self.__correlator = None
        self.__callback = callback
        self.__ctrl = None
        ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clock,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__slow_handler)
        self.data_node().set_data(self.get_domain().value2data(init))

    def __slow_handler(self,d):
        v = self.get_domain().data2value(d)
        if self.__handler(v) is not False:
            self.set_value(v,d.time())

    def change_value(self,v,t=0,p=True):
        if self.__handler(v) is not False:
            self.set_value(v,t)

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                self.__ctrl = FunctorController(self.__clock_domain,functor=piw.slowchange(utils.changify(self.__slow_handler)))
            self.__ctrl.prepare(plumber)
        else:
            if self.__correlator is None:
                self.__backend = piw.functor_backend(1,True)
                self.__backend.set_functor(piw.pathnull(0),utils.make_change_nb(piw.slowchange(utils.changify(self.__slow_handler))))
                self.__correlator = piw.correlator(self.__clock_domain,chr(1),piw.root_filter(),self.__backend.cookie(),0,0)
            plumber.prepare(self.__correlator,self.__stream_policy)

    def create_plumber(self,init,config):
        config.callback=self.__callback
        return Plumber(self,config)

    def destroy_plumber(self,term,plumber):
        if term:
            self.__correlator = None
            self.__backend = None
            self.__ctrl = None


class PolicyFactory:
    def __init__(self,klass,*args,**kwds):
        self.__klass = klass
        self.__args = args
        self.__kwds = kwds

    def __call__(self,atom,data_domain,init,transient):
        return self.__klass(atom,data_domain,init,transient,*self.__args,**self.__kwds)

def LoadPolicy(*args,**kwds):
    return PolicyFactory(LoadPolicyImpl,*args,**kwds)

def SlowPolicy(*args,**kwds):
    return PolicyFactory(SlowPolicyImpl,*args,**kwds)

def FastPolicy(*args,**kwds):
    return PolicyFactory(FastPolicyImpl,*args,**kwds)

def TriggerPolicy(*args,**kwds):
    return PolicyFactory(TriggerPolicyImpl,*args,**kwds)

def FastReadOnlyPolicy(*args,**kwds):
    return PolicyFactory(FastReadOnlyPolicyImpl,*args,**kwds)

def ReadOnlyPolicy(*args,**kwds):
    return PolicyFactory(ReadOnlyPolicyImpl,*args,**kwds)

def NullPolicy(*args,**kwds):
    return PolicyFactory(NullPolicyImpl,*args,**kwds)
