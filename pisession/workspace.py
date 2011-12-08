
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

from pi import atom,agent,action,errors,node,utils,async,index,guid,logic,files,resource,state,rpc,async,timeout,version,container,database
from pisession import registry,upgrade
from pi.logic.shortcuts import *
from pi.logic.terms import *

"""
import cherrypy
import dowser
"""

import picross
import piw
import sys
import random
import time
import os
import re
import glob
import urllib
import binascii
import traceback
import gc

"""
from guppy import hpy; h=hpy()
"""

rpc_chunksize = 1200

def all_agents(snap):
    agents = []

    for i in range(0,snap.agent_count()):
        a = snap.get_agent_index(i)
        if a.get_checkpoint():
            agents.append(a.get_address())

    return agents

def merge_snapshot(trunk,snapshot):
    mapping = state.Mapping()
    trunk.copy(snapshot,mapping,True)

class Controller(state.Manager):

    def add_sync(self):
        r = async.Deferred()
        if not self.__syncers and self.open():
            self.sync()
        self.__syncers.append(r)
        return r

    def __init__(self, workspace, address):
        self.address = address
        self.__workspace = workspace
        self.__volatile = 1
        self.__syncers = []
        self.__first_sync = True
        self.__saving = False

        (self.__agent,self.__created) = workspace.create_sink(address)

        sink = self.__agent.get_root()
        state.Manager.__init__(self,sink)

    def enable_save(self,e=True):
        self.__saving = e

    def client_sync(self):
        if self.get_data().is_dict() and not self.get_data().as_dict_lookup('volatile').is_null():
            self.__volatile = 1
        else:
            self.__volatile = 0

        state.Manager.client_sync(self)

        while self.__syncers:
            syncers = self.__syncers
            self.__syncers = []

            for s in syncers:
                s.succeeded()

        if self.__first_sync:
            self.__first_sync = False
            self.__workspace.agent_connected(self)

    def client_opened(self):
        state.Manager.client_opened(self)

    def close_client(self):
        if not self.__first_sync:
            self.__workspace.agent_disconnected(self)

        # turn off saving, and checkpoint the setup
        # as of right now to avoid tracking the
        # tear down of the agent

        if self.__saving:
            self.__saving = False
            self.__agent.set_checkpoint()
            checkpoint = self.__agent.checkpoint()
            checkpoint.set_type(self.__volatile)
            self.__workspace.set_agent(checkpoint)

        if not self.__volatile or (self.__created and not self.__first_sync):
            print 'erasing agent',self.address
            self.__workspace.erase_agent(self.__agent)

        state.Manager.close_client(self)

    def manager_checkpoint(self):
        if self.__saving:
            if self.__agent.isdirty() or not self.__agent.get_checkpoint():
                self.__agent.set_type(self.__volatile)
                self.__agent.set_checkpoint()
                self.__workspace.set_agent(self.__agent)

    @async.coroutine('internal error')
    def reload(self,snap):
        t = time.time()
        yield self.add_sync()

        myid = self.servername_fq().as_string()

        version=snap.get_checkpoint()

        if not version:
            print self.address,'nothing to load'
            yield async.Coroutine.success([])

        diff = self.get_diff(snap.get_root(),state.Mapping()).render()

        spl = []
        while diff:
            s = diff[:rpc_chunksize]
            diff = diff[len(s):]
            spl.append(s)

        rve = []

        for (i,s) in enumerate(spl):
            r = rpc.invoke_rpc(myid,'loadstate','%d:%d:%s' % (i,len(spl),s))
            yield r
            if r.status() and len(r.args())>0:
                v = logic.parse_clause(r.args()[0])
                rve.extend(v)

        yield async.Coroutine.success(rve)

class AgentLoader:
    def __init__(self,module):
        self.agent = None
        self.context = None
        self.module = __import__(module,fromlist=['main','unload','isgui'])

    def __run(self,func,*args,**kwds):
        current_context = piw.tsd_snapshot()

        try:
            self.context.install()
            piw.tsd_lock()
            try:
                return func(*args,**kwds)
            finally:
                piw.tsd_unlock()
        finally:
            current_context.install()

    def is_gui(self):
        return self.module.isgui

    def __load(self,name,ordinal):
        self.agent = self.module.main(self.context.getenv(),name,ordinal)

    def __quit(self):
        if self.agent:
            self.module.on_quit(self.agent)

    def __unload(self,destroy):
        if self.agent:
            ss = self.module.unload(self.context.getenv(),self.agent,destroy)
            self.context.kill()
            self.context.clear()
            self.context = None
            self.agent = None
            return ss

    def load(self,scope,name,ordinal):
        if self.context is not None and self.agent is not None:
            self.unload(False)

        self.context = piw.tsd_subcontext(self.is_gui(),scope,name)
        self.__run(self.__load,name,ordinal)

    def unload(self,destroy):
        if self.context is not None and self.agent is not None:
            return self.__run(self.__unload,destroy)

    def on_quit(self):
        if self.context is not None and self.agent is not None:
            self.__run(self.__quit)

class AgentFactory:
    def __init__(self,name,version,cversion,module):
        self.name=name
        self.version=version
        self.cversion=cversion
        self.module=module

    def dump(self):
        return self.module

class WorkspaceBackend:
    def start_gc(self):
        pass

    def stop_gc(self):
        pass

    def load_started(self,setup):
        pass

    def load_status(self,message,progress):
        pass

    def load_ended(self,errors=[]):
        pass


class Workspace(atom.Atom):
    def __init__(self,name,backend,registry):
        atom.Atom.__init__(self)
        self.__backend = backend
        self.__registry = registry
        self.__meta = container.PersistentMetaData(self,'agents',asserted=self.__asserted,retracted=self.__retracted)
        self.__name = name

        self.__load_queue = []
        self.__load_result = None
        self.__plugin_count = 0
        self.__load_errors = None

        dbfile = resource.user_resource_file('global',"%s-%s" % (resource.current_setup,name))

        if os.path.exists(dbfile):
            os.remove(dbfile)

        self.database = state.open_database(dbfile,True)
        self.trunk = self.database.get_trunk()
        upgrade.set_version(self.trunk,version.version)
        self.flush()

        self.index = index.Index(lambda a: Controller(self,a),False)

    def listmodules_rpc(self,arg):
        modules = []

        for mslug in self.__registry.modules():
            mname = ' '.join([s.capitalize() for s in mslug.split('_')])
            mordinals = tuple(self.find_all_ordinals(mslug))
            modules.append(logic.make_term('module',mname,'%s Agent' % mname,mordinals))

        return logic.render_termlist(tuple(modules))


    def addmodule_rpc(self,plugin_def):
        plugin_def = logic.parse_term(plugin_def)

        assert(logic.is_pred(plugin_def,'module') and plugin_def.arity>0)

        plugin_name = plugin_def.args[0]
        plugin_slug = '_'.join([w.lower() for w in plugin_name.split()])
        factory = self.__registry.get_module(plugin_slug)

        if not factory:
            return async.failure('no such agent')

        if plugin_def.arity>1:
            plugin_ordinal = plugin_def.args[1]
        else:
            plugin_ordinal = 0

        plugin_addr = self.create(factory,ordinal=plugin_ordinal)

        if not plugin_addr:
            return async.failure('ordinal in use')

        print 'created',plugin_addr,'as',plugin_slug
        return async.success(plugin_addr)

    def flush(self,tag=''):
        cp=self.trunk.save(piw.tsd_time(),tag)
        self.database.flush()
        return cp

    def __doload(self):
        while self.__load_queue:
            f = self.__load_queue[0]
            s = self.find_agent(f.address)

            if not s:
                print 'skipped',f.address
                self.__load_queue = self.__load_queue[1:]
                f.enable_save()
                continue

            def ok(*args,**kwds):
                if self.__load_queue and self.__load_queue[0]==f:
                    self.__load_queue = self.__load_queue[1:]

                if self.__load_result:
                    self.__load_result(True,n,f.address)

                if self.__load_errors is not None:
                    self.__load_errors.extend(args[0])

                f.enable_save()
                self.__doload()

            def not_ok(*args,**kwds):
                if self.__load_queue and self.__load_queue[0]==f:
                    self.__load_queue = self.__load_queue[1:]

                if self.__load_result:
                    self.__load_result(False,n,f.address)

                f.enable_save()
                self.__doload()

            n = s.get_name()
            r = f.reload(s)
            r.setCallback(ok).setErrback(not_ok)

            if self.__load_result:
                self.__load_result(None,n)

            break

    @async.coroutine('internal error')
    def load_file(self,path,upgrade_flag = False):

        """
        cherrypy.config.update({'server.socket_port': 8088})
        cherrypy.tree.mount(dowser.Root())
        cherrypy.engine.autoreload.unsubscribe()
        cherrypy.engine.start()
        """
        """
        print h.heap()
        print h.heapu()

        h.setref()
        """

        label = upgrade.split_setup(os.path.basename(path))[1]

        self.__backend.load_started(label)
        self.__backend.stop_gc()
        self.__backend.load_status('Preparing',0)

        snapshot = self.__backend.run_foreground_sync(upgrade.prepare_file,path,version.version)

        if upgrade.get_upgrade(snapshot):
            upgrade_flag = True

        if upgrade_flag:
            setup_signature = upgrade.get_setup_signature(snapshot)
            print 'loading from version',snapshot.version(),'in',path,'sig',setup_signature
        else:
            print 'loading from version',snapshot.version(),'in',path

        agents = set(all_agents(snapshot))

        for m in self.index.members():
            ma = m.address
            m.enable_save(False)
            if ma in agents:
                qa = self.index.to_absolute(ma)
                r = rpc.invoke_rpc(qa,'preload',path)
                yield r

        r = self.__load1(snapshot,label)
        yield r
        yield self.index.sync()
        e = r.args()[0]

        for m in self.index.members():
            ma = m.address
            if ma in agents:
                qa = self.index.to_absolute(ma)
                yield rpc.invoke_rpc(qa,'postload',path)

        if upgrade_flag and r.status():
            self.__backend.load_status('Upgrading',100)
            r = rpc.invoke_rpc('<interpreter>','upgrade',setup_signature)
            yield r

        self.__backend.load_status('Cleaning up',100)
        yield timeout.Timer(1000)

        """
        o = gc.collect()
        if o: print 'gc collected',o
        o = gc.collect()
        if o: print 'gc collected',o
        yield timeout.Timer(1000)

        x=h.heap()
        print "Total Heap"
        print "=========="
        print x

        print "dict"
        print "===="
        xd = x[0]
        print xd.byid
        print xd.byvia
        print xd.rp
        print xd.rp.more
        print xd.rp.more.more
        print xd.rp.more.more.more
        print xd.shpaths
        print xd.shpaths.more
        print xd.shpaths.more.more
        print xd.shpaths.more.more.more

        print "DatabaseProxy"
        print "===="
        xp = (x&database.DatabaseProxy)
        print xp.byid
        print xp.byvia
        print xp.rp
        print xp.rp.more
        print xp.rp.more.more
        print xp.rp.more.more.more
        print xp.shpaths
        print xp.shpaths.more
        print xp.shpaths.more.more
        print xp.shpaths.more.more.more

        print "str"
        print "==="
        xs = (x&str)
        xs_ = xs.byid
        #for i in range(100):
        #   print xs_
        #   xs_ = xs_.more
        print xs.byvia
        print xs.rp
        print xs.rp.more
        print xs.rp.more.more
        print xs.rp.more.more.more
        print xs.shpaths
        print xs.shpaths.more
        print xs.shpaths.more.more
        print xs.shpaths.more.more.more

        print "Term"
        print "===="
        xt = (x&Term)
        print xt.byid
        print xt.byvia
        print xt.rp
        print xt.rp.more
        print xt.rp.more.more
        print xt.rp.more.more.more
        print xt.shpaths
        print xt.shpaths.more
        print xt.shpaths.more.more
        print xt.shpaths.more.more.more

        print "Not reachable from root"
        print "======================="
        print h.heapu()

        hpy().heap().stat.dump("/Users/gbevin/Desktop/heap.txt")
        """ 
        self.__backend.start_gc()

        if e:
            self.__backend.load_ended(e)
        else:
            self.__backend.load_ended()

        yield async.Coroutine.completion(r.status(),e)

    def __load1(self,snapshot,label):
        merge_snapshot(self.trunk,snapshot)

        self.flush(label)

        self.__load_queue = []
        self.__load_errors = []
        pending = set()
        parked = set()
        total = 0

        r = async.Deferred()
        r2 = async.Deferred()
        w = timeout.Watchdog(r2,False,'load timeout')

        for i in range(0,self.trunk.agent_count()):
            a = self.trunk.get_agent_index(i)
            addr = a.get_address()
            if a.get_type()==0:
                pending.add(a.get_address())
            if a.get_type()==1:
                parked.add(a.get_address())

        total = len(pending)
        start = time.time()

        def progress(status,n,*args,**kwds):
            if status is not None:
                try: pending.discard(args[0])
                except: pass
                w.enable(60000)
            else:
                w.disable()

            p = len(pending)
            print 'loaded:',total-p,total,'in',time.time()-start,'s',n
            if not p:
                r2.succeeded()
            else:
                self.__backend.load_status(n,100*(total-p)/total)

        def watchdog(status,*args,**kwds):
            self.__load_result = None

            if status:
                r.succeeded(self.__load_errors)
            else:
                print 'watchdog fired; load failed'
                print pending
                r.failed(self.__load_errors)

            self.__load_errors = None

        w.setCallback(watchdog,True).setErrback(watchdog,False)
        self.__load_result = progress

        for c in self.index.members():
            if c.address in pending or c.address in parked:
                self.__load_queue.append(c)

        self.__doload()
        return r

    def __findversion(self,ver):
        snap = self.database.get_trunk()

        while True:
            p = snap.previous()
            v = snap.version()
            if v==ver: return snap
            if not p: break
            snap = self.database.get_version(p)

        return None


    @async.coroutine('internal error')
    def save_file(self,filename,description=''):
        yield self.index.sync()

        agents = set(all_agents(self.trunk))

        for m in self.index.members():
            ma = m.address
            if ma in agents:
                qa = self.index.to_absolute(ma)
                r = rpc.invoke_rpc(qa,'presave',filename)
                yield r


        yield self.index.sync()

        m = [ c.address for c in self.index.members() ]

        for i in range(0,self.trunk.agent_count()):
            agent = self.trunk.get_agent_index(i)
            address = agent.get_address()


            if address in m or agent.get_type()!=0:
                continue
            
            self.trunk.erase_agent(agent)
            checkpoint = agent.checkpoint()
            checkpoint.set_type(1)
            self.trunk.set_agent(checkpoint)

        cp = self.flush('saved')
        snap = self.database.get_version(cp)

        def save_tweaker(snap,src_snap):
            upgrade.set_upgrade(snap,False)
            upgrade.set_description(snap,description)

        upgrade.copy_snap2file(snap,filename,tweaker=save_tweaker)

        yield async.Coroutine.success()

    def server_opened(self):
        atom.Atom.server_opened(self)
        piw.tsd_index('<%s:main>'%self.__name,self.index)

    def close_server(self):
        self.index.close_index()
        atom.Atom.close_server(self)

    def find_agent(self,address):
        for i in range(0,self.trunk.agent_count()):
            a = self.trunk.get_agent_index(i)

            if a.get_address() == address and a.get_checkpoint():
                return a.checkpoint()

        return None

    def agent_disconnected(self,controller):
        if controller in self.__load_queue:
            self.__load_queue.remove(controller)

    def agent_connected(self,controller):
        if controller not in self.__load_queue:
            self.__load_queue.append(controller)
            if len(self.__load_queue)==1:
                self.__doload()

    def create_sink(self,address):
        for i in range(0,self.trunk.agent_count()):
            a = self.trunk.get_agent_index(i)

            if a.get_address() == address:
                return (a,False)

        return (self.trunk.get_agent_address(0,address,True),True)


    @staticmethod
    def __relation(address):
        return 'create(cnc("%s"),role(by,[instance(~a)]))' % address

    def erase_agent(self,a):
        self.trunk.erase_agent(a)
        self.flush()

    def set_agent(self,a):
        self.trunk.set_agent(a)
        self.flush()

    def check_address(self,address):
        found = [False]

        def visitor(v,s):
            if v.args[0]==address:
                found[0] = True

        self.__meta.visit(visitor)
        return found[0]

    def check_ordinal(self,name,ordinal):
        found = [False]

        def visitor(v,s):
            if v.args[1]==name and v.args[4]==ordinal:
                found[0] = True

        self.__meta.visit(visitor)
        return found[0]

    def find_all_ordinals(self,name):
        ordinals = []

        def visitor(v,s):
            if v.args[1]==name:
                ordinals.append(v.args[4])

        self.__meta.visit(visitor)
        return ordinals

    def find_new_ordinal(self,name):
        ordinal = [0]

        def visitor(v,s):
            if v.args[1]==name:
                ordinal[0] = max(v.args[4],ordinal[0])

        self.__meta.visit(visitor)
        return ordinal[0]+1

    def __make_unloader(self,name):
        def u(status):
            self.__plugin_count -= 1
            print 'unloaded',name,'plg count:',self.__plugin_count
        return u

    def __asserted(self,signature):
        (address,plugin,version,cversion,ordinal) = signature.args
        factory = self.__registry.get_compatible_module(plugin,cversion)
        self.add_frelation(self.__relation(address))
        agent = AgentLoader(factory.module)
        self.__plugin_count += 1
        agent.load(self.__name,address,ordinal)
        return agent

    def __retracted(self,signature,plugin,destroy):
        self.del_frelation(self.__relation(signature.args[0]))
        return plugin.unload(destroy)

    def unload(self,address,destroy=False):
        # unload address and return ss or None
        ss = self.__meta.retract_state(lambda v,s: v.args[0]==address,destroy)
        return ss

    def on_quit(self):
        # call on_quit for all plugins
        self.__meta.visit(lambda v,s: s.on_quit())

    def create(self,factory,address=None,ordinal=0):
        if ordinal:
            if self.check_ordinal(factory.name,ordinal):
                return None
        else:
            ordinal = self.find_new_ordinal(factory.name)

        if address:
            if self.check_address(address):
                return None
        else:
            address = guid.toguid("%s%d" % (factory.name,ordinal))

        print 'assigned ordinal',ordinal,'to',factory.name
        signature = logic.make_term('a',address,factory.name,factory.version,factory.cversion,ordinal)
        plugin = self.__meta.assert_state(signature)
        return address


def create_registry():
    r=registry.Registry()
    r.scan_path(os.path.join(picross.release_root_dir(),'plugins'),AgentFactory)
    r.dump(lambda m: m.dump())
    return r
