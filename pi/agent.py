
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

import sys
import os
import piw

from pi import atom,node,action,container,index,guid,files,utils,logic,rpc,async,const,paths,upgrade

import time
import types
import inspect
import weakref
import gc

class Logger:
    def __init__(self):
        self.__env = piw.tsd_snapshot()
        self.__buffer = []

    def write(self, msg):
        eol = (msg[-1:] == '\n')
        self.__buffer.append(str(msg).strip())

        if eol:
            self.__env.log(' '.join(self.__buffer))
            self.__buffer=[]

    def close(self):
        pass

    def flush(self):
        pass

def main(klass,upgrader=None,gui=False):
    """
    Use in an agent to define all the required boilerplate.  Define an agent like this:

        from pi import agent

        class agent(...):
            def __init__(self,path):
                ...

        agent.main(agent)

    """

    def __main(env,name,ordinal):
        piw.setenv(env)
        root = klass(name,ordinal)
        piw.tsd_server(name,root)
        root.advertise('<main>')
        return root

    def __unload(env,obj,destroy):

        ss=''
        if hasattr(obj,'unload'):
            ss = obj.unload(destroy)

        return ss

    def __on_quit(obj):
        obj.quit()

    def __upgrade(oldv,newv,tools,agent,phase):
        u = upgrader or upgrade.Upgrader
        return u().upgrade(oldv,newv,tools,agent,phase)

    def __fini():
        pass

    f = inspect.currentframe()
    f = f.f_back
    agent_globals = f.f_globals
    agent_globals['main'] = __main
    agent_globals['upgrade'] =  __upgrade
    agent_globals['unload'] =  __unload
    agent_globals['on_quit'] =  __on_quit
    agent_globals['fini'] =  __fini
    agent_globals['isgui'] =  gui

class RpcNode(piw.rpcserver):

    def __init__(self,root):
        piw.rpcserver.__init__(self)
        self.__root = root
        self.__running = set()

    def rpc_open(self,id):
        piw.tsd_rpcserver(self,id.as_string())

    def rpc_close(self):
        self.close_rpcserver()

    def __completed(self,token,status,arg):
        arg = piw.makestring_len(arg,len(arg),0)
        self.completed(token,status,arg)

    def get_rpc(self,path,name):
        a = self.__root
        p = path.as_pathstr()

        while len(p)>0:
            f = ord(p[0])
            p = p[1:]
            a = a.get_internal(f)
            if a is None:
                return None

        return a.get_rpc(name)

    def rpcserver_invoke(self,token,path,name,arg):
        arg = arg.as_stdstr()
        name = name.as_string()

        func = self.get_rpc(path,name)

        if func is None:
            self.__completed(token,False,'no such rpc %s:%s' % (path,name))
            return

        def result(deferred,status,msg=''):
            self.__completed(token,status,msg)
            self.__running.remove(deferred)

        try:
            r = func(arg)
            if r is None: r=async.success()
            if not isinstance(r,async.Deferred): r=async.success(r)
            self.__running.add(r)
        except:
            utils.log_exception()
            self.__completed(token,False,'internal rpc error %s' % name)
            return

        r.setCallback(result,r,True).setErrback(result,r,False)


class Agent(atom.Atom):
    """
    Main class for an Agent.  This should be used as the
    root server for an agent.
    """
    def __init__(self,*args,**kwds):

        if 'container' not in kwds:
            kwds['container'] = const.verb_node


        volatile = None
        signature = None
        subsystem = None

        if 'volatile' in kwds:
            volatile = kwds['volatile']
            del kwds['volatile']

        if 'subsystem' in kwds:
            subsystem = kwds['subsystem']
            del kwds['subsystem']

        if 'signature' in kwds:
            signature = kwds['signature']
            del kwds['signature']

        atom.Atom.__init__(self,*args,**kwds)

        if volatile:
            self.set_property_long('volatile',1)

        if signature:
            self.set_property_string('plugin',getattr(signature,'plugin'))
            self.set_property_string('version',getattr(signature,'version'))
            self.set_property_string('cversion',getattr(signature,'cversion'))

        if subsystem:
            self.set_property_string('subsystem',subsystem)

        self.__subsystems = {}
        self.__rpc = RpcNode(self)

        self.add_verb2(200,'set([],~a,role(None,[partof(~s),notproto(set)]),role(to,[abstract]))', self.__verb_builtin_set_value)
        self.add_verb2(201,'set([],~a,role(None,[partof(~s),notproto(set)]))', self.__verb_builtin_set)
        self.add_verb2(202,'set([un],~a,role(None,[partof(~s),notproto(set)]))', self.__verb_builtin_unset)
        self.add_verb2(203,'up([],~a,role(None,[partof(~s),notproto(up)]))', self.__verb_builtin_up)
        self.add_verb2(204,'down([],~a,role(None,[partof(~s),notproto(down)]))', self.__verb_builtin_down)
        self.add_verb2(205,'set([toggle],~a,role(None,[partof(~s),notproto(set)]))', self.__verb_builtin_toggle)
        self.add_verb2(206,'up([],~a,role(None,[partof(~s),notproto(up)]),role(by,[numeric]))', self.__verb_builtin_up_by)
        self.add_verb2(207,'down([],~a,role(None,[partof(~s),notproto(down)]),role(by,[numeric]))', self.__verb_builtin_down_by)

        self.__state_buffer = []

    def load_agent_state(self,delegate):
        pass

    def rpc_loadstate(self,arg):
        (i,c,a) = arg.split(':',2)

        c=int(c)
        i=int(i)

        while len(self.__state_buffer)<c:
            self.__state_buffer.append('')

        self.__state_buffer[i] = a

        if i!=c-1:
            return '[]'

        arg = ''.join(self.__state_buffer)
        self.__state_buffer = []
        state = piw.parse_state_term(arg)

        class LoadResults:
            def __init__(self):
                self.residual = {}
                self.deferred = {}
                self.errors = []
            def retval(self):
                return logic.render_term(tuple(self.errors))
            def set_residual(self,n,r):
                self.residual[n] = r
            def set_deferred(self,n,r):
                self.deferred[n] = r
            def add_error(self,msg):
                self.errors.append(msg)
                
        delegate = LoadResults()
        delegate.set_residual(self,state)

        self.load_agent_state(delegate)

        while delegate.residual:
            r = delegate.residual
            r2 = r.copy()
            delegate.residual = {}

            while r:
                (k,v) = r.popitem()
                k.load_state(v,delegate,1)

            if delegate.residual.keys() == r2.keys():
                break

        if delegate.residual:
            print 'didnt load after phase 1:',[(k,v.render()) for (k,v) in delegate.residual.items()]

        delegate.residual = delegate.deferred

        while delegate.residual:
            r = delegate.residual
            r2 = r.copy()
            delegate.residual = {}

            while r:
                (k,v) = r.popitem()
                k.load_state(v,delegate,2)

            if delegate.residual.keys() == r2.keys():
                break

        if delegate.residual:
            print 'didnt load after phase 2:',delegate.residual

        return delegate.retval()

    def rpc_preload(self,arg):
        return self.agent_preload(arg)

    def rpc_postload(self,arg):
        return self.agent_postload(arg)

    def rpc_presave(self,arg):
        return self.agent_presave(arg)

    def agent_preload(self,filename):
        pass

    def agent_postload(self,filename):
        pass

    def agent_presave(self,filename):
        pass

    def __get_child(self,path):
        c = self
        while path:
            p = path[0]
            path = path[1:]
            c = c.get(p)
            if c is None:
                return None
        return c

    @async.coroutine('internal error')
    def __verb_builtin(self,target,op):
        targets = action.concrete_objects(target)
        rv = []
        agg = async.Aggregate(accumulate=True)

        for t in targets:
            (a,p) = paths.breakid_list(t)
            c = self.__get_child(p)
            if c is not None:
                r = op(c)
                if r is not None:
                    if isinstance(r,async.Deferred):
                        agg.add(t,r)
                    else:
                        rv.extend(r)

        if agg.get_outstanding():
            agg.enable()
            yield agg
            for r in agg.successes().values(): rv.extend(r)
            for r in agg.failures().values(): rv.extend(r)

        yield async.Coroutine.success(rv)

    def __verb_builtin_set_value(self,subject,target,value):
        value = action.abstract_string(value)
        def op(c):
            return c.builtin_set_value(value)
        return self.__verb_builtin(target,op)

    def __verb_builtin_up(self,subject,target):
        def op(c):
            return c.builtin_up()
        return self.__verb_builtin(target,op)

    def __verb_builtin_down(self,subject,target):
        def op(c):
            return c.builtin_down()
        return self.__verb_builtin(target,op)

    def __verb_builtin_up_by(self,subject,target,inc):
        inc = float(action.abstract_string(inc))
        def op(c):
            return c.builtin_up_by(inc)
        return self.__verb_builtin(target,op)

    def __verb_builtin_down_by(self,subject,target,inc):
        inc = float(action.abstract_string(inc))
        def op(c):
            return c.builtin_down_by(inc)
        return self.__verb_builtin(target,op)

    def __verb_builtin_toggle(self,subject,target):
        def op(c):
            return c.builtin_toggle()
        return self.__verb_builtin(target,op)

    def __verb_builtin_set(self,subject,target):
        def op(c):
            return c.builtin_set()
        return self.__verb_builtin(target,op)

    def __verb_builtin_unset(self,subject,target):
        def op(c):
            return c.builtin_unset()
        return self.__verb_builtin(target,op)

    def shutdown(self):
        self.__thing = piw.thing()
        piw.tsd_thing(self.__thing)
        self.__thing.set_slow_timer_handler(utils.notify(self.__shutdown))
        self.__thing.timer_slow(500)

    def __shutdown(self):
        self.__thing.close_thing()
        self.__thing = None
        piw.tsd_exit()

    def rpc_unload(self,arg):
        self.shutdown()

    def __ssname(self,id):
        (myid,mypath) = guid.split(self.id())
        nid = ('%s/%s%s' % (myid,id,mypath))[:26]
        return '<%s>' % nid

    def __ssrelation(self,subsys,name):
        if hasattr(subsys,'subsys_relation'):
            rel = subsys.subsys_relation
            if not rel: return None
        else:
            rel=('join','to')

        return '%s(cnc("%s"),role(%s,[instance(~self)]))' % (rel[0],name,rel[1])

    def __ssopen(self,id,subsys):
        name = self.__ssname(id)
        rel = self.__ssrelation(subsys,name)
        if rel is not None:
            self.add_frelation(rel)
        piw.tsd_server(name,subsys)
        subsys.advertise('<main>')

    def __ssclose(self,id,subsys):
        name = self.__ssname(id)
        rel = self.__ssrelation(subsys,name)
        if rel is not None:
            self.del_frelation(rel)
        subsys.close_server()

    def subsystem_keys(self):
        return self.__subsystems.keys()

    def subsystem_ids(self):
        return tuple([self.__ssname(id) for id in self.__subsystems.keys()])

    def add_subsystem(self,id,subsys):
        v = self.__subsystems.get(id)
        if v is not None:
            self.__ssclose(id,v)
        self.__subsystems[id] = subsys
        if self.open():
            self.__ssopen(id,subsys)

    def remove_subsystem(self,id):
        v = self.__subsystems.get(id)
        if v is not None:
            del self.__subsystems[id]
            if self.open():
                self.__ssclose(id,v)

    def get_subsystem(self,id):
        return self.__subsystems.get(id)

    def rpc_subsystems(self,arg):
        ss=tuple(self.__subsystems.keys())
        if ss:
            return logic.render_term(ss)
        return None    

    def iter_subsys_items(self):
        return self.__subsystems.iteritems()

    def iter_subsystem(self):
        return iter(self.__subsystems)

    def server_opened(self):
        atom.Atom.server_opened(self)
        self.__rpc.rpc_open(self.servername())

        for (k,v) in self.__subsystems.items():
            self.__ssopen(k,v)

    def close_server(self):
        for (k,v) in self.__subsystems.items():
            self.__ssclose(k,v)
        self.__rpc.rpc_close()
        atom.Atom.close_server(self)

    def unload(self,destroy=False):
        ss = ''
        if self.open():
            ss = logic.render_term(tuple([ss.id() for ss in self.__subsystems.values()]))
        self.notify_destroy()
        self.close_server()

        return ss

    def quit(self):
        for v in self.__subsystems.itervalues():
            if hasattr(v,'on_quit'): v.on_quit()
        if hasattr(self,'on_quit'): self.on_quit()


