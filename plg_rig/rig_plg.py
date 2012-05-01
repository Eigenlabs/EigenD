
from pi import agent,atom,domain,utils,bundles,upgrade,paths,audio,async,collection,policy,proxy,node,container,logic,action
from pisession import workspace
import piw
from . import rig_version as version,rig_native
import os

def name_subst(name,find,repl):
    oname = []
    for w in name.split():
        if w == find: w = repl
        oname.append(w)
    return ' '.join(oname)

class DataProxy(node.Client):
    def __init__(self,handler):
        self.__handler = handler
        node.Client.__init__(self)

    def client_opened(self):
        node.Client.client_opened(self)
        self.__handler(self.get_data())

    def close_client(self):
        node.Client.close_client(self)

    def client_data(self,v):
        self.__handler(v)

class RigMonitor(proxy.AtomProxy):

    monitor = set(['latency','domain'])

    def __init__(self,input,address):
        proxy.AtomProxy.__init__(self)
        self.__input = input
        self.__connector = None
        self.__mainanchor = piw.canchor()
        self.__mainanchor.set_client(self)
        self.__mainanchor.set_address_str(address)

    def disconnect(self):
        self.__connector = None
        self.set_data_clone(self.__connector)
        self.__mainanchor.set_address_str('')

    def node_ready(self):
        self.__input.add_monitor(self)
        self.__connector = DataProxy(self.__input.handler)
        self.set_data_clone(self.__connector)

    def node_removed(self):
        self.__input.del_monitor(self)
        self.set_data_clone(None)
        self.__connector = None

    def node_changed(self,parts):
        if 'domain' in parts:
            self.node_removed()
            self.node_ready()
            return

class RigOutputPolicyImpl:
    protocols = 'output'

    def __init__(self,atom,data_domain,init,transient):
        self.__datanode = rig_native.output()
 
    def data_node(self):
        return self.__datanode

    def get_data(self):
        raise RuntimeError("unimplemented in RigOutputPolicy")

    def set_data(self,d):
        raise RuntimeError("unimplemented in RigOutputPolicy")

    def change_value(self,v,t=0,p=True):
        raise RuntimeError("unimplemented in RigOutputPolicy")

    def set_value(self,v,t=0):
        raise RuntimeError("unimplemented in RigOutputPolicy")

    def get_value(self):
        raise RuntimeError("unimplemented in RigOutputPolicy")

    def close(self):
        pass


def RigOutputPolicy(*args,**kwds):
    return policy.PolicyFactory(RigOutputPolicyImpl,*args,**kwds)

class RigOutput(atom.Atom):
    def __init__(self,master,ordinal,scope):
        atom.Atom.__init__(self,ordinal=ordinal,policy=RigOutputPolicy(),protocols='remove')
        self.set_connection_scope(scope)

        self.__inputs = {}
        self.__master = master
        self.__monitors = {}
        self.__scope = scope

        self.__clockdom = piw.clockdomain_ctl()
        self.__clockdom.set_source(piw.makestring('*',0))
        self.__clock = piw.clocksink()
        self.__clockdom.sink(self.__clock,"rig input")
        self.__output = self.get_policy().data_node()
        self.__output.set_clock(self.__clock)

        self.set_domain(domain.Aniso())

    def set_ordinal(self,value):
        self.__master.set_ordinal(value)

    def set_names(self,value):
        self.__master.set_names(value)

    def property_change(self,key,value,delegate):
        if key != 'slave':
            return

        cur_slaves = set(self.get_property_termlist('slave'))
        old_slaves = set(self.__monitors.keys())

        dead_slaves = old_slaves.difference(cur_slaves)
        new_slaves = cur_slaves.difference(old_slaves)

        for s in dead_slaves:
            m = self.__monitors[s]
            del self.__monitors[s]
            m.disconnect()

        for s in new_slaves:
            id=paths.to_absolute(s,self.__scope)
            m = RigMonitor(self.__master,id)
            self.__monitors[s] = m


    def property_veto(self,key,value):
        if atom.Atom.property_veto(self,key,value):
            return True

        return key in ['name','ordinal']

    def plumb_clocks(self):
        for v in self.__inputs.values():
            v.set_downstream_clock(self.__clock)

    def unplumb_clocks(self):
        for v in self.__inputs.values():
            v.clear_downstream_clock()

    def set_domain(self,dom):
        self.unplumb_clocks()
        self.set_property_string('domain',str(dom))

        if dom.iso():
            self.__clockdom.set_source(piw.makestring('',0))
        else:
            self.__clockdom.set_source(piw.makestring('*',0))

        self.plumb_clocks()

    def __setup(self):
        first_iso = None
        first_aniso = None

        for (k,v) in self.__inputs.items():
            if v.domain().iso():
                if first_iso is None:
                    first_iso = k
            else:
                if first_aniso is None:
                    first_aniso = k

        if first_iso is not None:
            self.set_domain(self.__inputs[first_iso].domain())
            return

        if first_aniso is not None:
            self.set_domain(self.__inputs[first_aniso].domain())
            return

        self.set_domain(domain.Aniso())

    def add_input(self,iid,inp):
        if iid in self.__inputs:
            self.__inputs[iid].clear_downstream_clock()
            del self.__inputs[iid]
        self.__inputs[iid] = inp
        self.__setup()

    def del_input(self,iid):
        if iid in self.__inputs:
            self.__inputs[iid].clear_downstream_clock()
            del self.__inputs[iid]
            self.__setup()

    def input(self):
        return self.__master


class RigInputPlumber(proxy.AtomProxy):

    monitor = set(['latency','domain'])

    def __init__(self,output,iid,address,filt,ctl):
        proxy.AtomProxy.__init__(self)
        self.__output = output
        self.__iid = iid
        self.__filt = filt
        self.__connector = None
        self.__ctl = ctl
        self.__mainanchor = piw.canchor()
        self.__mainanchor.set_client(self)
        self.__mainanchor.set_address_str(address)

    def disconnect(self):
        self.set_data_clone(None)
        self.__mainanchor.set_address_str('')
        self.__connector = None

    def clear_downstream_clock(self):
        if self.__connector:
            self.__connector.clear_downstream()

    def set_downstream_clock(self,clock):
        if self.__connector:
            self.__connector.set_downstream(clock)

    def node_ready(self):
        self.__output.add_input(self.__iid,self)
        self.__connector = rig_native.connector(self.__ctl,self.__output.get_policy().data_node(),self.__iid,self.__filt)
        self.set_data_clone(self.__connector)

    def node_removed(self):
        self.__output.del_input(self.__iid)
        self.set_data_clone(None)
        self.__connector = None

    def node_changed(self,parts):
        if 'domain' in parts:
            self.node_removed()
            self.node_ready()
            return

class RigInputPolicyImpl:
    protocols = 'input'

    def __init__(self,atom,data_domain,init,transient,scope,output):
        self.__closed = False
        self.__scope = scope
        self.__datanode = node.Server(transient=transient)
        self.__data_domain = data_domain
        self.__connection_iids = set()
        self.__output = output
        self.__clockdom = piw.clockdomain_ctl()
        self.__clockdom.set_source(piw.makestring('*',0))
        self.__ctrl = None
        atom.set_property_string('domain',str(data_domain))
        self.__connections = container.PersistentMetaData(atom,'master',asserted=self.__add_connection, retracted=self.__del_connection)

    def data_node(self):
        return self.__datanode

    def get_data(self):
        return self.data_node().get_data()

    def get_domain(self):
        return self.__data_domain

    def set_data(self,d):
        self.data_node.set_data(d)

    def change_value(self,v,t=0,p=False):
        self.set_data(v)

    def set_value(self,v,t=0):
        self.set_data(v)

    def get_value(self):
        return self.get_data()

    def closed(self):
        return self.__closed

    def make_filter(self,stream,slot):
        if logic.is_pred_arity(stream,'conn',5,5):
            if stream.args[0] is not None:
                using = int(stream.args[0])
            else:
                using = 0

            if stream.args[1] is not None:
                tgt = int(stream.args[1])
            else:
                tgt = 0

            id=paths.to_absolute(stream.args[2],self.__scope)
            path=stream.args[3]

            ctl=True if stream.args[4] == 'ctl' else False

            if path is not None:
                return (id,piw.signal_dsc_filter(using,tgt,path),ctl)
            else:
                return (id,piw.signal_cnc_filter(using,tgt),ctl)

        return ('',piw.null_filter(),False)

    def close(self):
        self.__closed = True
        self.__connections.clear()

    def get_controller_backend(self,config):
        return self.__ctrl.get_backend(config)

    def get_data_backend(self,config):
        return self.__ctrl.get_backend(config)

    def __add_connection(self,src,delegate):
        iid = (max(self.__connection_iids)+1 if self.__connection_iids else 1)

        (a,f,c) = self.make_filter(src,iid)

        if not paths.valid_id(a):
            return None

        self.__connection_iids.add(iid)

        plumber = RigInputPlumber(self.__output,iid,a,f,c)
        return policy.PlumberSlot(iid,src,None,plumber)

    def __del_connection(self,src,slot,destroy):
        self.__connection_iids.discard(slot.iid)
        slot.plumber.disconnect()


def RigInputPolicy(*args,**kwds):
    return policy.PolicyFactory(RigInputPolicyImpl,*args,**kwds)


class RigInput(atom.Atom):
    def __init__(self,scope,index,output_peer):
        self.__output_peer = output_peer
        self.__index = index
        self.__scope = scope
        self.__monitors = {}

        self.__output_peer[self.__index] = RigOutput(self,ordinal=index,scope=self.__output_peer.scope())
        policy=RigInputPolicy(self.__scope,self.__output_peer[self.__index])

        atom.Atom.__init__(self,ordinal=index,domain=domain.Aniso(),policy=policy,protocols='remove')
        self.set_connection_scope(scope)


    def destroy_input(self):
        del self.__output_peer[self.__index]

    def property_change(self,key,value,delegate):
        if key in ['name','ordinal']:
            self.__output_peer[self.__index].set_property(key,value,notify=False,allow_veto=False)

    def handler(self,v):
        self.data_node().set_data(v)

    def set_domain(self,dom):
        self.set_property_string('domain',str(dom))

    def __setup(self):
        first_aniso = None

        for (k,v) in self.__monitors.items():
            if not v.domain().iso():
                if first_aniso is None:
                    first_aniso = k

        if first_aniso is not None:
            self.set_domain(self.__monitors[first_aniso].domain())
            return

        self.set_domain(domain.Aniso())

    def notify_destroy(self):
        self.__output_peer[self.__index].notify_destroy()
        atom.Atom.notify_destroy(self)

    def add_monitor(self,inp):
        iid = id(inp)
        if iid in self.__monitors:
            self.__monitors[iid].clear_downstream()
            del self.__monitors[iid]
        self.__monitors[iid] = inp
        self.__setup()

    def del_monitor(self,inp):
        iid = id(inp)
        if iid in self.__monitors:
            self.__monitors[iid].clear_downstream()
            del self.__monitors[iid]
            self.__setup()

class InputList(collection.Collection):
    def __init__(self,scope):
        self.__output_peer = None
        self.__scope = scope
        collection.Collection.__init__(self,names='input')

    def set_output_peer(self,output_peer):
        self.__output_peer = output_peer
        self.__output_peer.set_peer(self)

    def scope(self):
        return self.__scope

    def create_input(self,name):
        names = name.split()

        if names:
            try:
                ordinal=int(names[-1])
                names = names[:-1]
            except:
                ordinal = None
        else:
            ordinal = self.freeinstance()
            names = ''

        k = self.find_hole()
        j = RigInput(self.__scope,k,self.__output_peer)
        j.set_names(' '.join(names))
        j.set_ordinal(ordinal)
        self[k] = j


    @async.coroutine('internal error')
    def instance_create(self,name):
        k = self.find_hole()
        j = RigInput(self.__scope,k,self.__output_peer)
        j.set_ordinal(name)
        self[k] = j
        yield async.Coroutine.success(j)

    @async.coroutine('internal error')
    def instance_wreck(self,k,e,name):
        del self[k]
        e.destroy_input()
        yield async.Coroutine.success()

    def dynamic_create(self,i):
        return RigInput(self.__scope,i,self.__output_peer)

    def dynamic_destroy(self,i,v):
        v.destroy_input()

    @async.coroutine('internal error')
    def rpc_delinstance(self,arg):
        iid = paths.to_relative(paths.to_absolute(arg,scope=self.scope()))
        r = collection.Collection.rpc_delinstance(self,iid)
        yield r
        yield async.Coroutine.completion(r.status(),*r.args(),**r.kwds())

        
class OutputList(atom.Atom):
    def __init__(self,scope=None):
        atom.Atom.__init__(self,names="output",protocols='create',dynlist=True)
        self.__peer = None
        self.__scope = scope

    def scope(self):
        return self.__scope

    def load_state(self,state,delegate,phase):
        if phase == 1:
            delegate.set_deferred(self,state)
            return async.success()

        return atom.Atom.load_state(self,state,delegate,phase-1)

    def set_peer(self,peer):
        self.__peer = peer

    def rpc_createinstance(self,arg):
        return self.__peer.rpc_createinstance(arg)

    def rpc_listinstances(self,arg):
        return self.__peer.rpc_listinstances(arg)

    def rpc_instancename(self,arg):
        return self.get_property_string('name');

    @async.coroutine('internal error')
    def rpc_delinstance(self,arg):
        iid = paths.to_relative(paths.to_absolute(arg,scope=self.scope()))
        pid = None
        for v in self.values():
            if iid == v.id():
                pid = v.input().id()
                break

        if pid:
            r =  self.__peer.rpc_delinstance(pid)
            yield r
            yield async.Coroutine.completion(r.status(),*r.args(),**r.kwds())
        else:
            yield async.Coroutine.failure('output not in use')

class InnerAgent(agent.Agent):
    def __init__(self,outer_agent):
        agent.Agent.__init__(self,signature=version,names='rig gateway',ordinal=1)

        self.__description = outer_agent.get_description(full=True)
        print 'inner agent',self.__description
        self.__registry = workspace.get_registry()
        self.__outer_agent = outer_agent
        self.__name = outer_agent.inner_name
        self.__workspace = workspace.Workspace(self.__name,self,self.__registry,enclosure=self.__description)

        self[1] = self.__workspace
        self[2] = OutputList(self.__name)
        self[3] = InputList(self.__name)

        self.add_verb2(1,'create([],None,role(None,[abstract,matches([input])]),option(called,[abstract]))',self.__create_input)
        self.add_verb2(2,'create([],None,role(None,[abstract,matches([output])]),option(called,[abstract]))',self.__create_output)

    def update_description(self):
        self.__description = self.__outer_agent.get_description(full=True)
        print 'inner agent description now',self.__description
        self.__workspace.set_enclosure(self.__description)

    def __create_input(self,subject,dummy,name):
        name = action.abstract_string(name)
        self[3].create_input(name)

    def __create_output(self,subject,dummy,name):
        name = action.abstract_string(name)
        self.__outer_agent[3].create_input(name)

    def save(self,filename):
        print 'saving rig',self.__name,'to',filename
        return self.__workspace.save_file(filename)

    def load(self,filename):
        return self.__workspace.load_file(filename,post_load=False)

    def post_load(self,filename):
        return self.__workspace.post_load(filename)

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.advertise('<%s:main>'%self.__name)

    def rpc_listmodules(self,arg):
        return self.__workspace.listmodules_rpc(arg)

    def rpc_addmodule(self,arg):
        return self.__workspace.addmodule_rpc(arg)

    def rpc_destroy(self,arg):
        a=logic.parse_clause(arg)
        self.__workspace.unload(a,True)

    def load_started(self,label):
        pass

    def load_status(self,msg,percent):
        pass

    def load_ended(self,errors=[]):
        pass

    def stop_gc(self):
        pass

    def start_gc(self):
        pass

    def run_foreground_sync(self,func,*args,**kwds):
        return func(*args,**kwds)

    def unload(self,destroy):
        self.__workspace.unload_all(destroy)
        self.__workspace.shutdown()


class OuterAgent(agent.Agent):
    def __init__(self,address,ordinal):
        agent.Agent.__init__(self,signature=version,names='rig',ordinal=ordinal)

        self.file_name = 'rig%d' % ordinal
        self.inner_name = '%s.%s' % (piw.tsd_scope(),self.file_name)
        self.__inner_agent = InnerAgent(self)
        self.__domain = piw.clockdomain_ctl()

        self.set_property_string('rig',self.file_name)

        self[2] = OutputList(None)
        self[3] = InputList(None)

        self[3].set_output_peer(self.__inner_agent[2])

        self.__inner_agent[3].set_output_peer(self[2])

        self.add_verb2(1,'create([],None,role(None,[abstract,matches([input])]),option(called,[abstract]))',self.__create_input)
        self.add_verb2(2,'create([],None,role(None,[abstract,matches([output])]),option(called,[abstract]))',self.__create_output)

    def rig_file(self,filename):
        return "%s#%s" % (filename,self.file_name)

    def property_veto(self,key,value):
        if agent.Agent.property_veto(self,key,value):
            return True
        return key == 'rig'

    def property_change(self,key,value,delegate):
        if key in ['name','ordinal']:
            self.__inner_agent.update_description()

    def unload(self,destroy):
        self.__inner_agent.unload(destroy)
        agent.Agent.unload(self,destroy)

    def __create_input(self,subject,dummy,name):
        name = action.abstract_string(name)
        self[3].create_input(name)

    def __create_output(self,subject,dummy,name):
        name = action.abstract_string(name)
        self.__inner_agent[3].create_input(name)

    @async.coroutine('internal error')
    def load_state(self,state,delegate,phase):
        yield agent.Agent.load_state(self,state,delegate,phase)
        rig_file = self.rig_file(delegate.path)
        print 'rig load state',phase,rig_file
        if os.path.exists(rig_file):
            print 'loading rig',self.inner_name,'from',rig_file
            r = self.__inner_agent.load(rig_file)
            yield r
            print 'rig load errors',r.args()[0]

    def agent_presave(self,filename):
        print 'starting presave',filename
        return self.__inner_agent.save(self.rig_file(filename))

    def agent_postload(self,filename):
        rig_file = self.rig_file(filename)
        agent.Agent.agent_postload(self,filename)
        return self.__inner_agent.post_load(rig_file)

    def agent_preload(self,filename):
        agent.Agent.agent_preload(self,filename)

    def server_opened(self):
        agent.Agent.server_opened(self)
        piw.tsd_server(paths.to_absolute('<eigend1>',self.inner_name),self.__inner_agent)

    def close_server(self):
        agent.Agent.close_server(self)
        self.__inner_agent.close_server();


agent.main(OuterAgent,gui=True)
