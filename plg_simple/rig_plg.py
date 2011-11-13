
from pi import agent,atom,domain,utils,bundles,upgrade,paths,audio,async,collection
from pisession import workspace
import piw
from plg_simple import rig_version as version
import os

class Output(bundles.Output):
    def __init__(self,ordinal):
        bundles.Output.__init__(self,1,False,ordinal=ordinal)

class Input(atom.Atom):
    def __init__(self,d,peer,index):
        self.__peer = peer
        self.__index = index
        self.__output = atom.Atom(ordinal=index)
        atom.Atom.__init__(self,ordinal=index)
        self.__peer[self.__index] = self.__output

    def destroy_input(self):
        del self.__peer[self.__index]

class InputList(collection.Collection):
    def __init__(self,d,peer):
        self.__peer = peer
        self.__domain = d
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
        return Input(self.__domain,self.__peer,i)

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

        self.set_property_string('rig',self.inner_name)

        self[2] = OutputList()
        self[3] = InputList(self.__domain,self.__inner_agent[2])
        self.__inner_agent[3] = InputList(self.__domain,self[2])

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
        piw.tsd_server('<%s:eigend1>' % self.inner_name,self.__inner_agent)

    def close_server(self):
        agent.Agent.close_server(self)
        __inner_agent.close_server();


agent.main(OuterAgent,gui=True)
