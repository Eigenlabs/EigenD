
from pi import agent,atom,domain,utils,bundles,upgrade,paths,audio,async,collection,policy,proxy,node,container,logic
from pisession import workspace
import piw
from plg_rig import rig_version as version
import rig_native
import os

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
    def __init__(self,ordinal):
        atom.Atom.__init__(self,ordinal=ordinal,policy=RigOutputPolicy())

        self.__inputs = {}

        self.__clockdom = piw.clockdomain_ctl()
        self.__clockdom.set_source(piw.makestring('*',0))
        self.__clock = piw.clocksink()
        self.__clockdom.sink(self.__clock,"rig input")
        self.__output = self.get_policy().data_node()
        self.__output.set_clock(self.__clock)

        self.set_domain(domain.Aniso())

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

    def plumb_clocks(self):
        for v in self.__inputs.values():
            v.set_downstream(self.__clock)

    def unplumb_clocks(self):
        for v in self.__inputs.values():
            v.clear_downstream()

    def add_input(self,iid,inp):
        print 'add input',inp.servername(),inp.path(),inp.domain()

        if iid in self.__inputs:
            self.__inputs[iid].clear_downstream()
            del self.__inputs[iid]
        self.__inputs[iid] = inp
        self.__setup()

    def del_input(self,iid):
        if iid in self.__inputs:
            self.__inputs[iid].clear_downstream()
            del self.__inputs[iid]
            self.__setup()

class RigInputPlumber(proxy.AtomProxy):

    monitor = set(['latency','domain'])

    def __init__(self,output,iid,address,filt):
        proxy.AtomProxy.__init__(self)
        self.__output = output
        self.__iid = iid
        self.__filt = filt
        self.__connector = None
        self.__mainanchor = piw.canchor()
        self.__mainanchor.set_client(self)
        self.__mainanchor.set_address_str(address)

    def disconnect(self):
        self.set_data_clone(None)
        self.__mainanchor.set_address_str('')
        self.__connector = None

    def node_ready(self):
        self.__output.add_input(self.__iid,self)
        self.__connector = rig_native.connector(self.__output.get_policy().data_node(),self.__iid,self.__filt)
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
        pass

    def set_value(self,v,t=0):
        pass

    def get_value(self):
        return None

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

            id=paths.qualify(stream.args[2],self.__scope)
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

    def __add_connection(self,src):
        iid = (max(self.__connection_iids)+1 if self.__connection_iids else 1)

        (a,f) = self.make_filter(src,iid)

        if not paths.valid_id(a):
            return None

        self.__connection_iids.add(iid)
        return policy.PlumberSlot(iid,src,RigInputPlumber(self.__output,iid,a,f))

    def __del_connection(self,src,slot):
        self.__connection_iids.discard(slot.iid)
        slot.plumber.disconnect()


def RigInputPolicy(*args,**kwds):
    return policy.PolicyFactory(RigInputPolicyImpl,*args,**kwds)


class Input(atom.Atom):
    def __init__(self,scope,peer,index):
        self.__peer = peer
        self.__index = index
        self.__scope = scope
        self.__output = RigOutput(ordinal=index)
        atom.Atom.__init__(self,ordinal=index,domain=domain.Aniso(),policy=RigInputPolicy(self.__scope,self.__output))
        self.__peer[self.__index] = self.__output

    def destroy_input(self):
        del self.__peer[self.__index]

class InputList(collection.Collection):
    def __init__(self,scope,peer):
        self.__peer = peer
        self.__scope = scope
        collection.Collection.__init__(self,names='input')

    @async.coroutine('internal error')
    def instance_create(self,name):
        j = self.dynamic_create(name)
        self[name] = j
        yield async.Coroutine.success(j)

    @async.coroutine('internal error')
    def instance_wreck(self,k,e,name):
        del self[k]
        e.destroy_input()

    def dynamic_create(self,i):
        return Input(self.__scope,self.__peer,i)

    def dynamic_destroy(self,i,v):
        v.destroy_input()
        
class OutputList(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,names='output')

class InnerAgent(agent.Agent):
    def __init__(self,outer_agent):
        agent.Agent.__init__(self,signature=version,names='eigend',ordinal=1)

        self.__registry = workspace.create_registry()
        self.__outer_agent = outer_agent
        self.__name = outer_agent.inner_name
        self.__workspace = workspace.Workspace(self.__name,self,self.__registry)

        self[1] = self.__workspace
        self[2] = OutputList()

    def save(self,filename):
        print 'saving rig',self.__name,'to',filename
        return self.__workspace.save_file(filename)

    def load(self,filename):
        return self.__workspace.load_file(filename)

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.advertise('<%s:main>'%self.__name)

    def rpc_listmodules(self,arg):
        return self.__workspace.listmodules_rpc(arg)

    def rpc_addmodule(self,arg):
        return self.__workspace.addmodule_rpc(arg)

    def rpc_destroy(self,arg):
        self.__workspace.unload(arg)

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

class OuterAgent(agent.Agent):
    def __init__(self,address,ordinal):
        agent.Agent.__init__(self,signature=version,names='rig',ordinal=ordinal)

        self.file_name = 'rig%d' % ordinal
        self.inner_name = '%s.%s' % (piw.tsd_user(),self.file_name)
        self.__inner_agent = InnerAgent(self)
        self.__domain = piw.clockdomain_ctl()

        self.set_property_string('rig',self.file_name)

        self[2] = OutputList()
        self[3] = InputList(None,self.__inner_agent[2])
        self.__inner_agent[3] = InputList(self.inner_name,self[2])

    def rig_file(self,filename):
        return "%s#%s" % (filename,self.file_name)

    def property_veto(self,key,value):
        if agent.Agent.property_veto(self,key,value):
            return True
        return key == 'rig'

    @async.coroutine('internal error')
    def agent_postload(self,filename):
        if os.path.exists(filename):
            print 'loading rig',self.inner_name,'from',filename
            r = self.__inner_agent.load(self.rig_file(filename))
            yield r
            print 'rig load errors',r.args()[0]

    def agent_presave(self,filename):
        print 'starting presave',filename
        return self.__inner_agent.save(self.rig_file(filename))

    def server_opened(self):
        agent.Agent.server_opened(self)
        piw.tsd_server(paths.qualify('<eigend1>',self.inner_name),self.__inner_agent)

    def close_server(self):
        agent.Agent.close_server(self)
        __inner_agent.close_server();


agent.main(OuterAgent,gui=True)
