
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

"""
Bundle Basics.  Classes to input to, output from, construct and explode bundles
"""

from pi import atom, domain, const, utils, policy, logic, node

import piw
import sys

PolicyFactory = policy.PolicyFactory

class FastSender(piw.fastdata):
    def __init__(self,size=2):
        piw.fastdata.__init__(self,const.fastdata_sender)
        piw.tsd_fastdata(self)
        self.__queue = piw.tsd_dataqueue(size)
        piw.fastdata.send_slow(self,piw.pathnull(piw.tsd_time()),self.__queue)

    def trigger(self):
        self.__queue.trigger_slow()

    def send(self,d):
        if not d.is_null():
            self.__queue.write_slow(d)

    def sender(self):
        return self.__queue.sender_slow()

class Splitter(piw.splitter):
    def __init__(self,clockdomain,*outputs):
        piw.splitter.__init__(self)
        self.__outputs = []

        self.clockdomain = clockdomain
        self.clockdomain.add_listener(utils.notify(self.__listener))
        self.__listener()

        for o in outputs:
            self.add_output(o)

    def add_output(self,output):
        self.__outputs.append(output)
        src=self.clockdomain.get_source().as_string()
        sr=self.clockdomain.get_sample_rate()
        bs=self.clockdomain.get_buffer_size()
        output.set_iso_domain(src,sr,bs)
        output.data_node().set_clock(self.get_clock())
        output.set_latency(self.get_latency())
        self.add_signal(output.signal,output.data_node())
        return output

    def remove_output(self,output):
        if output in self.__outputs:
            self.remove_signal(output.signal)
            self.__outputs.remove(output)

    def set_clock(self,c):
        for o in self.__outputs: o.data_node().set_clock(c)

    def set_latency(self,l):
        for o in self.__outputs: o.set_latency(l)

    def __listener(self):
        src=self.clockdomain.get_source().as_string()
        sr=self.clockdomain.get_sample_rate()
        bs=self.clockdomain.get_buffer_size()
        for o in self.__outputs: o.set_iso_domain(src,sr,bs)

class Output(atom.Atom):
    def __init__(self, signal, iso, continuous=False, **kwds):
        assert 'domain' not in kwds
        self.signal = signal
        self.iso = iso

        if continuous:
            self.hints = (logic.make_term('continuous'),)
        else:
            self.hints = ()

        if iso:
            dom = domain.Null(hints=self.hints)
        else:
            dom = domain.Aniso(hints=self.hints)

        atom.Atom.__init__(self, policy=BundleOutputPolicy(), domain=dom, **kwds)

    def set_iso_domain(self,src,bs,sr):
        if self.iso:
            self.set_property_string('domain',str(domain.Iso(src,sr,bs,hints=self.hints)))

class BundleOutputPolicyImpl:
    protocols = 'output'

    def __init__(self,atom,data_domain,init,transient):
        self.__datanode = piw.splitter_node()
        atom.set_property_string('domain',str(data_domain))
 
    def data_node(self):
        return self.__datanode

    def get_data(self):
        raise RuntimeError("unimplemented in BundleOutputPolicy")

    def set_data(self,d):
        raise RuntimeError("unimplemented in BundleOutputPolicy")

    def change_value(self,v,t=0,p=True):
        raise RuntimeError("unimplemented in BundleOutputPolicy")

    def set_value(self,v,t=0):
        raise RuntimeError("unimplemented in BundleOutputPolicy")

    def get_value(self):
        raise RuntimeError("unimplemented in BundleOutputPolicy")

    def close(self):
        pass

def BundleOutputPolicy(*args,**kwds):
    return PolicyFactory(BundleOutputPolicyImpl,*args,**kwds)

class BundleInput:
    def __init__(self,cookie,clock_domain,filter,signals,threshold,poly):
        sigmap = ''.join([chr(c) for c in signals])
        self.clock_domain = clock_domain
        self.correlator = piw.correlator(clock_domain,sigmap,filter,cookie,threshold,poly)
        self.signals = signals

    def add_upstream(self,c):
        self.correlator.clocksink().add_upstream(c)

    def remove_upstream(self,c):
        self.correlator.clocksink().remove_upstream(c)

    def check_signal(self,signal):
        assert signal in self.signals

class VectorPolicyImpl(policy.ConnectablePolicyImpl):
    protocols = 'input nostage'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,clocked,callback,auto_slot):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__callback = callback
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clocked,node.Server(transient=transient),auto_slot)

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                self.__ctrl = policy.FunctorController(self.__input.clock_domain)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_input,-1)

    def create_plumber(self,config):
        config.callback=self.__callback
        return policy.Plumber(self,config)

    def destroy_plumber(self,plumber):
        if plumber.connect_static() and self.count_control_connections()==0:
            self.__ctrl = None

class LatchPolicyImpl(policy.ConnectablePolicyImpl):
    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,clocked,callback):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__callback = callback
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clocked,node.Server(transient=transient),False)

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v,d.time(),True)

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                f=piw.slowchange(utils.changify(self.__change))
                self.__ctrl = policy.FunctorController(self.__input.clock_domain,functor=f)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_latch,1)

    def create_plumber(self,config):
        assert config.clocked
        config.clocked=True
        config.callback=self.__callback
        return policy.Plumber(self,config)

    def destroy_plumber(self,plumber):
        if plumber.connect_static() and self.count_control_connections()==0:
            self.__ctrl = None

class LocalPolicyImpl(policy.ConnectablePolicyImpl):
    protocols = 'input output'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,clocked,callback):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__static_data = FastSender()
        self.__callback = callback
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clocked,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__change)
        self.__input.correlator.plumb_input(self.__signal,1,piw.pathnull(0),-1,False,self.__static_data,self.__stream_policy.create_converter(False),piw.null_filter())

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v)

    def change_value(self,v,t=0,p=True):
        self.set_value(v,t)

    def set_value(self,v,t=0):
        d = self.get_domain().value2data(v,t)
        self.__static_data.send(d)
        policy.ConnectablePolicyImpl.set_value(self,v,t)

    def close(self):
        policy.ConnectablePolicyImpl.close(self)
        self.data_node().clear_change_handler()
        self.__input.correlator.unplumb_input(self.__signal,1,piw.pathnull(0),-1)

    def destroy_plumber(self,plumber):
        if plumber.connect_static():
            if self.count_control_connections()==0:
                self.__ctrl = None
            return

        if self.count_data_connections()==0:
            if not self.closed():
                self.__input.correlator.plumb_input(self.__signal,1,piw.pathnull(0),-1,False,self.__static_data,self.__stream_policy.create_converter(False),piw.null_filter())

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                f=piw.slowchange(utils.changify(self.__change))
                self.__ctrl = policy.FunctorController(self.__input.clock_domain,functor=f)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_input,-1)

    def create_plumber(self,config):
        assert config.clocked
        if not config.connect_static() and self.count_data_connections()==0:
            self.__input.correlator.unplumb_input(self.__signal,1,piw.pathnull(0),-1)
        config.clocked=True
        config.callback=self.__callback
        return policy.Plumber(self,config)

class LingerPolicyImpl(policy.ConnectablePolicyImpl):
    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,clocked,callback):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__static_data = FastSender()
        self.__callback = callback
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clocked,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__change)
        self.__input.correlator.plumb_input(self.__signal,255,piw.pathnull(0),10,policy.Plumber.input_linger,self.__static_data,self.__stream_policy.create_converter(False),piw.null_filter())

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v)

    def change_value(self,v,t=0,p=True):
        self.set_value(v,t)

    def set_value(self,v,t=0):
        d = self.get_domain().value2data(v,t)
        self.__static_data.send(d)
        policy.ConnectablePolicyImpl.set_value(self,v,t)

    def close(self):
        policy.ConnectablePolicyImpl.close(self)
        self.data_node().clear_change_handler()
        self.__input.correlator.unplumb_input(self.__signal,255,piw.pathnull(0),10)

    def destroy_plumber(self,plumber):
        if plumber.connect_static() and self.count_control_connections()==0:
            self.__ctrl = None

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                f=piw.slowchange(utils.changify(self.__change))
                self.__ctrl = policy.FunctorController(self.__input.clock_domain,functor=f)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_linger,10)

    def create_plumber(self,config):
        assert config.clocked
        config.clocked=True
        config.callback=self.__callback
        return policy.Plumber(self,config)

class MergePolicyImpl(policy.ConnectablePolicyImpl):
    protocols = 'input output'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,clocked,callback):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__callback = callback
        self.__static_data = FastSender()
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clocked,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__change)
        self.__input.correlator.plumb_input(self.__signal,255,piw.pathnull(0),10,policy.Plumber.input_merge,self.__static_data,self.__stream_policy.create_converter(False),piw.null_filter())

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v)

    def change_value(self,v,t=0,p=True):
        self.set_value(v,t)

    def set_value(self,v,t=0):
        d = self.get_domain().value2data(v,t)
        self.__static_data.send(d)
        policy.ConnectablePolicyImpl.set_value(self,v,t)

    def close(self):
        self.__input.correlator.unplumb_input(self.__signal,255,piw.pathnull(0),10)
        self.data_node().clear_change_handler()
        policy.ConnectablePolicyImpl.close(self)

    def destroy_plumber(self,plumber):
        if plumber.connect_static() and self.count_control_connections()==0:
            self.__ctrl = None

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                f=piw.slowchange(utils.changify(self.__change))
                self.__ctrl = policy.FunctorController(self.__input.clock_domain,functor=f)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_merge,1)

    def create_plumber(self,config):
        assert config.clocked
        config.clocked=True
        config.callback=self.__callback
        return policy.Plumber(self,config)

class MergeNoDefaultPolicyImpl(policy.ConnectablePolicyImpl):
    protocols = 'input nostage'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,clocked,callback):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__callback = callback
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,clocked,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__change)

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v)

    def change_value(self,v,t=0,p=True):
        self.set_value(v,t)

    def set_value(self,v,t=0):
        d = self.get_domain().value2data(v,t)
        policy.ConnectablePolicyImpl.set_value(self,v,t)

    def close(self):
        self.__input.correlator.unplumb_input(self.__signal,255,piw.pathnull(0),10)
        self.data_node().clear_change_handler()
        policy.ConnectablePolicyImpl.close(self)

    def destroy_plumber(self,plumber):
        if plumber.connect_static() and self.count_control_connections()==0:
            self.__ctrl = None

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                f=piw.slowchange(utils.changify(self.__change))
                self.__ctrl = policy.FunctorController(self.__input.clock_domain,functor=f)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_merge,1)

    def create_plumber(self,config):
        assert config.clocked
        config.clocked=True
        config.callback=self.__callback
        return policy.Plumber(self,config)

class PlumbingController:
    def __init__(self,corr,sig,pol):
        self.__correlator = corr
        self.__sig = sig
        self.__pol = pol

    def prepare(self,plumber):
        plumber.prepare(self.__correlator,self.__pol)

class ScalarNoDefaultPolicyImpl(policy.ConnectablePolicyImpl):
    protocols = 'input nostage'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy):
        self.__stream_policy = stream_policy
        self.__input = input
        self.__signal = signal
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,True,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__change)

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v)

    def destroy_plumber(self,plumber):
        if plumber.connect_static() and self.count_control_connections()==0:
            self.__ctrl = None

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                self.__ctrl = PlumbingController(self.__input.correlator,self.__signal,self.__stream_policy)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_input,-1)

    def close(self):
        self.data_node().clear_change_handler()
        policy.ConnectablePolicyImpl.close(self)

    def create_plumber(self,config):
        assert config.clocked
        config.clocked=True
        return policy.Plumber(self,config)

class ScalarPolicyImpl(policy.ConnectablePolicyImpl):
    protocols = 'input output'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,merge=False):
        self.__stream_policy = stream_policy
        self.__merge = merge
        self.__input = input
        self.__signal = signal
        self.__static_data = FastSender()
        self.__ctrl = None
        policy.ConnectablePolicyImpl.__init__(self,atom,data_domain,init,True,node.Server(transient=transient),False)
        self.data_node().set_change_handler(self.__change)
        self.__plumb_static()

    def __change(self,d):
        v = self.get_domain().data2value(d)
        self.change_value(v)

    def change_value(self,v,t=0,p=True):
        self.set_value(v,t)

    def set_value(self,v,t=0):
        d = self.get_domain().value2data(v,t)
        self.__static_data.send(d)
        policy.ConnectablePolicyImpl.set_value(self,v,t)

    def __plumb_static(self):
        if self.__merge:
            self.__input.correlator.plumb_input(self.__signal,1,piw.pathnull(0),10,policy.Plumber.input_merge,self.__static_data,self.__stream_policy.create_converter(False),piw.null_filter())
        else:
            self.__input.correlator.plumb_input(self.__signal,1,piw.pathnull(0),-1,policy.Plumber.input_input,self.__static_data,self.__stream_policy.create_converter(False),piw.null_filter())

    def __unplumb_static(self):
        if self.__merge:
            self.__input.correlator.unplumb_input(self.__signal,1,piw.pathnull(0),10)
        else:
            self.__input.correlator.unplumb_input(self.__signal,1,piw.pathnull(0),-1)

    def close(self):
        self.__unplumb_static()
        self.data_node().clear_change_handler()
        policy.ConnectablePolicyImpl.close(self)

    def destroy_plumber(self,plumber):
        if plumber.connect_static():
            if self.count_control_connections()==0:
                self.__ctrl = None
            return

        if self.count_data_connections()==0:
            if not self.closed():
                self.__plumb_static()

    def prepare_plumber(self,plumber):
        if plumber.connect_static():
            if self.__ctrl is None:
                f=piw.slowchange(utils.changify(self.__change))
                self.__ctrl = policy.FunctorController(self.__input.clock_domain,functor=f)
            self.__ctrl.prepare(plumber)
        else:
            plumber.prepare(self.__input.correlator,self.__stream_policy,self.__signal,policy.Plumber.input_input,-1)


    def create_plumber(self,config):
        assert config.clocked
        if not config.connect_static() and self.count_data_connections()==0:
            self.__unplumb_static()
        config.clocked=True
        return policy.Plumber(self,config)

class NotifyScalarPolicyImpl(ScalarPolicyImpl):
    protocols = 'input output'

    def __init__(self,atom,data_domain,init,transient,input,signal,stream_policy,merge=False,notify=None):
        self.__notify = notify
        ScalarPolicyImpl.__init__(self,atom,data_domain,init,transient,input,signal,stream_policy,merge)

    def change_value(self,v,t=0,p=True):
        ScalarPolicyImpl.change_value(self,v,t=t,p=p)
        if self.__notify:
            self.__notify()

class VectorInput(BundleInput):
    def __init__(self,cookie,clock_domain,signals=(),filter=None,threshold=0,poly=0):
        BundleInput.__init__(self,cookie,clock_domain,filter or piw.null_filter(),signals,threshold,poly)

    def vector_policy(self, signal, stream_policy, clocked=True, callback=None, auto_slot=False):
        """
        Pure vector policy.  These inputs dont have a static value (domain is ignored)
        and input wires are created only when connected
        """
        self.check_signal(signal)
        return PolicyFactory(VectorPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),clocked,callback,auto_slot)
    def linger_policy(self, signal, stream_policy, clocked=True, callback=None):
        """
        These inputs are merged into wires created by other policies: they never cause
        the creation of wires.  No global default.  Merged events will cause the 
        channel to stay open
        """
        self.check_signal(signal)
        return PolicyFactory(LingerPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),clocked,callback)
    def latch_policy(self, signal, stream_policy, clocked=True, callback=None):
        """
        These inputs are merged into wires created by other policies: they never cause
        the creation of wires.  No global default.  Merged events will cause the 
        channel to stay open
        """
        self.check_signal(signal)
        return PolicyFactory(LatchPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),clocked,callback)
    def merge_policy(self, signal, stream_policy,clocked=True,callback=None):
        """
        These inputs are merged into wires created by other policies: they never cause
        the creation of wires.  The static value acts as a global default.  connected
        signals may act as defaults too.
        """
        self.check_signal(signal)
        return PolicyFactory(MergePolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),clocked,callback)
    def merge_nodefault_policy(self, signal, stream_policy,clocked=True,callback=None):
        """
        These inputs are merged into wires created by other policies: they never cause
        the creation of wires.  There is no default from the static value.
        """
        self.check_signal(signal)
        return PolicyFactory(MergeNoDefaultPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),clocked,callback)
    def local_policy(self, signal, stream_policy,clocked=True,callback=None):
        """
        When unconnected, a root stream is created carrying the static value.  When
        connected, the static value is disconnected and the input is used in a pure
        vector fashion.
        """
        self.check_signal(signal)
        return PolicyFactory(LocalPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),clocked,callback)

class ScalarInput(BundleInput):
    def __init__(self,cookie,clock_domain,signals,threshold=0):
        BundleInput.__init__(self,cookie,clock_domain,piw.root_filter(),signals,threshold,1)

    def nodefault_policy(self,signal,stream_policy):
        self.check_signal(signal)
        return PolicyFactory(ScalarNoDefaultPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy))

    def notify_policy(self, signal, stream_policy, notify = None):
        self.check_signal(signal)
        return PolicyFactory(NotifyScalarPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy), notify=notify)

    def policy(self, signal, stream_policy):
        self.check_signal(signal)
        return PolicyFactory(ScalarPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy))

    def merge_policy(self,signal,stream_policy):
        self.check_signal(signal)
        return PolicyFactory(ScalarPolicyImpl,self,signal,policy.DefaultStreamPolicy(stream_policy),merge=True)
