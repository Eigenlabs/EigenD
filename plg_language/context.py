
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

from pi import async,node,action,logic
from plg_language import referent
import piw

class ResolutionError(Exception):
    pass

class ResolvHandler(async.Arg1):
    def __init__(self,deferred):
        async.Arg1.__init__(self,deferred,ResolutionError)

class Context:
    def __init__(self, manager, auto = False, lurkers = None, listeners = None, stack=None, inner = None):
        self.__stack = stack or []
        self.__listeners = listeners or set()
        self.__lurkers = lurkers or set()
        self.__inner = inner or set()
        self.__noun = set()
        self.__manager = manager
        self.__auto = auto
        self.name = 'default'

    def get_snapshot(self):
        return (self.__listeners,self.__lurkers,self.__inner)

    def set_snapshot(self,snap):
        (li,lu,i) = snap
        self.__inner = i
        self.__listeners = li
        self.__lurkers = lu
        self.__update()

    def stack_empty(self):
        return not self.__stack

    def clear_stack(self):
        self.__stack = []

    def iter_stack(self):
        return iter(self.__stack)

    def push_stack(self,obj):
        self.__stack.insert(0,obj)
        self.__stack=self.__stack[:20]

    def isauto(self):
        return self.__auto

    def set_auto(self,manager):
        self.__auto = auto

    def clear_inner_scope(self):
        self.__inner = set()

    def set_inner_scope(self,objs):
        self.__inner = objs

    def get_inner_scope(self):
        return self.__inner

    def setup_empty(self):
        self.__listeners = set()
        self.__lurkers = set()
        self.__auto = False
        self.__update()

    def __update(self):
        self.__noun = self.__listeners.union(self.__lurkers)
        if self.__manager: self.__manager.autosave()

    def setup(self,auto,li,lu):
        self.__listeners = li
        self.__lurkers = lu
        self.__auto = auto
        self.__update()

    def extend_scope(self,objs):
        if self.__listeners:
            self.__listeners = self.__listeners.union(objs)
            self.__update()

    def set_lurker_scope(self,objs):
        self.__lurkers = objs
        self.__update()

    def set_listener_scope(self,objs):
        self.__listeners = objs
        self.__update()

    def get_lurker_scope(self):
        return self.__lurkers

    def get_listener_scope(self):
        return self.__listeners

    def get_verb_scope(self):
        return self.__listeners

    def get_noun_scope(self):
        return self.__noun

    def get_stack(self):
        return self.__stack[:]


class ContextManager(node.Server):
    def __init__(self,agent):
        node.Server.__init__(self,extension=255)
        self.__storage = node.Server(extension=255,creator=self.__create)
        self[3] = self.__storage
        self[1] = node.Server()
        self.__mainctx = Context(self)
        self.__mainctx.name = 'default'
        self.__agent = agent
        self.autosave()

    def get_context(self,interpid):
        interp = self.__agent.get_interpreter(interpid)
        return interp.get_context()

    def __create(self,k):
        return node.Server(change=lambda v: self.__changed(k,v))

    def default_context(self):
        return self.__mainctx

    def __changed(self,k,v):
        print 'conv change',k,v
        n = v.as_string().split(':',1)
        print 'conversation',n[0],'changed'

        if n[0] == self.__mainctx.name:
            (auto,li,lu) = logic.parse_clause(n[1])
            print 'update current conversation',li,lu
            self.__mainctx.setup(auto,set(li),set(lu))

        self.__storage[k].set_data(v)

    def __find_value(self,name):
        name = name

        for (k,n) in self.__storage.iteritems():
            v = n.get_data()
            if not v.is_string(): continue
            v = v.as_string()
            v = v.split(':',1)
            if not v[0]==name: continue
            return v[1]

        if name == 'default':
            return '[False,[],[]]'

        return None
            
    def __find(self,name):
        name = name

        for (k,n) in self.__storage.iteritems():
            v = n.get_data()
            if not v.is_string(): continue
            v = v.as_string()
            v = v.split(':',1)
            if not v[0]==name: continue
            return (k,n)

        return None
            
    def autosave(self):
        name = self.__mainctx.name
        ctx = self.__mainctx
        if ctx.isauto():
            self.__save(ctx,name)
        v = '%s:%s' % (name,logic.render_term((ctx.isauto(),tuple(ctx.get_listener_scope()),tuple(ctx.get_lurker_scope()))))
        self[1].set_data(piw.makestring(v,0))

    def __save(self,ctx,name):
        print 'remember',name

        t = self.__find(name)

        if t is None:
            k = self.__storage.find_hole()
            n = self.__create(k)
            self.__storage[k]=n
        else:
            (k,n) = t

        v = piw.makestring('%s:%s' % (name,logic.render_term((ctx.isauto(),tuple(ctx.get_listener_scope()),tuple(ctx.get_lurker_scope())))),0)
        n.set_data(v)
        self[1].set_data(v)

    def iverb2_59_remember(self,interp,subject,conv):
        """
        remember([],global_remember,role(None,[matches([conversation])]))
        """

        ctx = self.get_context(interp)
        
        if ctx.name=='all' or ctx.name=='empty':
            return async.failure("can't remember special conversations")

        self.__save(ctx,ctx.name)


    def iverb2_50_remember(self,interp,subject,conv):
        """
        remember([],global_remember,role(None,[tagged([conversation])]))
        """
        
        name = action.abstract_string(conv)
        ctx = self.get_context(interp)

        if name=='all' or name=='empty':
            return async.failure("can't remember special conversations")

        self.__save(ctx,name)
        ctx.name = name


    def iverb2_49_join(self,interp,subject,name):
        """
        join([],global_join,option(None,[abstract]))
        """

        ctx = self.get_context(interp)

        if name is None:
            name = 'default'
        else:
            name = action.abstract_string(name)

        if name == 'empty' or name == 'all':
            ctx.name = name
            ctx.setup_empty()
            print 'joining all'
            return

        t = self.__find_value(name)

        if not t:
            return async.failure("no such conversation")

        (auto,li,lu) = logic.parse_clause(t)
        ctx.name = name
        ctx.setup(auto,set(li),set(lu))


    @async.coroutine('internal error')
    def iverb2_51_listen(self,interpid,subject,agent):
        """
        listen([],global_listen,role(None,[abstract]))
        """

        interp = self.__agent.get_interpreter(interpid)
        words = action.abstract_wordlist(agent)
        r = self.__agent.interpret(interp,[],words)

        yield r

        if not r.status():
            yield async.Coroutine.failure('no agents')

        (ref,) = r.args()
        agents = set(ref.concrete_ids())

        if len(agents)==0:
            yield async.Coroutine.failure('no agents')

        ctx = interp.get_context()

        li = ctx.get_listener_scope().union(agents)
        lu = ctx.get_lurker_scope().difference(agents)

        ctx.clear_stack()
        ctx.set_lurker_scope(lu)
        ctx.set_listener_scope(li)
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_52_unlisten(self,interp,subject,agent):
        """
        listen([un],global_listen,role(None,[concrete]))
        """

        agent = action.concrete_objects(agent)
        ctx = self.get_context(interp)

        ctx.clear_stack()
        ctx.set_listener_scope(ctx.get_listener_scope().difference(set(agent)))
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_53_lurk(self,interp,subject,agent):
        """
        lurk([],global_lurk,role(None,[concrete]))
        """

        ctx = self.get_context(interp)
        agents = set(action.concrete_objects(agent))

        lu = ctx.get_lurker_scope().union(agents)
        li = ctx.get_listener_scope().difference(agents)

        ctx.clear_stack()
        ctx.set_lurker_scope(lu)
        ctx.set_listener_scope(li)
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_54_unlurk(self,interp,subject,agent):
        """
        lurk([un],global_listen,role(None,[concrete]))
        """

        agent = action.concrete_objects(agent)
        ctx = self.get_context(interp)

        ctx.clear_stack()
        ctx.set_lurker_scope(ctx.get_lurker_scope().difference(set(agent)))
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_55_ctxlisten(self,interp,subject,name):
        """
        listen([],global_listen,role(None,[tagged([conversation])]))
        """

        name = action.abstract_string(name)

        t = self.__find_value(name)

        if not t:
            return async.failure("no such conversation")

        (auto,li,lu) = logic.parse_clause(t)

        ctx = self.get_context(interp)

        ctx.clear_stack()
        ctx.set_listener_scope(ctx.get_listener_scope().union(set(li)))
        ctx.set_lurker_scope(ctx.get_lurker_scope().union(set(lu)))
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_56_ctxunlisten(self,interp,subject,name):
        """
        listen([un],global_listen,role(None,[tagged([conversation])]))
        """

        name = action.abstract_string(name)

        t = self.__find_value(name)

        if not t:
            return async.failure("no such conversation")

        (auto,li,lu) = logic.parse_clause(t)

        ctx = self.get_context(interp)

        ctx.clear_stack()
        ctx.set_listener_scope(ctx.get_listener_scope().difference(set(li)))
        ctx.set_lurker_scope(ctx.get_lurker_scope().difference(set(lu)))
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_57_ctxlurk(self,interp,subject,name):
        """
        lurk([],global_lurk,role(None,[tagged([conversation])]))
        """

        name = action.abstract_string(name)

        t = self.__find_value(name)

        if not t:
            return async.failure("no such conversation")

        (auto,li,lu) = logic.parse_clause(t)

        ctx = self.get_context(interp)

        ctx.clear_stack()
        ctx.set_lurker_scope(ctx.get_lurker_scope().union(set(lu)).union(set(li)))
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

    def iverb2_58_ctxunlurk(self,interp,subject,name):
        """
        lurk([un],global_lurk,role(None,[tagged([conversation])]))
        """

        name = action.abstract_string(name)

        t = self.__find_value(name)

        if not t:
            return async.failure("no such conversation")

        (auto,li,lu) = logic.parse_clause(t)

        ctx = self.get_context(interp)

        ctx.clear_stack()
        ctx.set_lurker_scope(ctx.get_lurker_scope().difference(set(lu)))
        ctx.set_inner_scope(set())

        print 'scope:',ctx.get_listener_scope(),ctx.get_lurker_scope()

def throwaway_context():
    return Context(None)

@async.coroutine('internal error')
def primitive_hey(interp,word):
    scope = set()

    while not interp.empty():
        n = interp.pop(referent.Referent)

        if n is None:
            yield async.Coroutine.failure('bad noun')

        n = (yield ResolvHandler(n.reinterpret(interp,[])))

        o = n.concrete_ids()
        if not o:
            yield async.Coroutine.failure('empty noun')

        scope.update(set(o))

    interp.get_context().set_inner_scope(scope)
    print 'inner scope:',interp.get_context().get_inner_scope()

@async.coroutine('internal error')
def primitive_ahem(interp,word):
    scope = set()

    while not interp.empty():
        n = interp.pop(referent.Referent)

        if n is None:
            yield async.Coroutine.failure('bad noun')

        o = n.concrete_ids()
        if not o:
            yield async.Coroutine.failure('empty noun')

        scope.update(set(o))

    interp.get_context().set_inner_scope(scope)
    print 'inner scope:',interp.get_context().get_inner_scope()

def primitive_scope(interp,word):
    ctx = interp.get_context()

    print "== noun scope =="
    for t in enumerate(ctx.get_noun_scope()):
        print "%i: %s" % t

    print "== verb scope =="
    for t in enumerate(ctx.get_verb_scope()):
        print "%i: %s" % t

    print "== inner scope =="
    for t in enumerate(ctx.get_inner_scope()):
        print "%i: %s" % t

    print "== argument stack =="
    for t in enumerate(interp.iterstack()):
        print "%i: %s" % t

    return async.success()
