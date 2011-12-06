
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

from pi import async,logic,rpc
from pi.logic.shortcuts import *
from plg_language import context,referent

import copy
import traceback


class Delegate:
    def buffer_done(self,status,msg,repeat):
        pass
    def error_message(self,err):
        print 'error_message',err

def rpcerrorhandler(ei):
    traceback.print_exception(*ei)
    return async.Coroutine.failure('internal error')

class RpcAdapter(async.DeferredDecoder):
    def decode(self):
        if self.deferred.status() is False:
            return async.Coroutine.failure(self.deferred.args()[0])
        return self.deferred.args()[0]

class Queue:
    def __init__(self,interpreter):
        self.__interp = interpreter
        self.__queue = []
        self.__busy = False

    def __ready(self,*args,**kwds):
        while self.__queue:
            self.__busy = True
            w = self.__queue.pop(0)
            r = self.__interp.interpret(w)
            if r.status() is None:
                r.setCallback(self.__ready).setErrback(self.__ready)
                return
        self.__busy = False

    def interpret(self,word):
        self.__queue.append(word)
        if not self.__busy:
            self.__ready()

class Interpreter:
    def __init__(self, agent, database, delegate, ctx = None, args = None):
        self.__agent = agent
        self.__database = database
        self.__delegate = delegate
        self.__context = ctx or context.throwaway_context()
        self.__stack = []
        self.__args = args or {}
        self.__digits = []
        self.__result = None
        self.__jobs = set([])
        self.__waiters = []
        self.__status = True
        self.__snapshot = []
        self.__lastcmd = None
        self.__history = []
        self.__statemgr = None
        self.__action = None

    def set_action(self,action):
        self.__action = action

    def get_action(self):
        return self.__action

    def set_statemgr(self,sm):
        print 'using',sm,'for undo'
        self.__statemgr = sm
        self.__laststate = None
        self.__undostate = None

    def process_block(self,words):

        def coroutine():
            for w in words:
                r = self.interpret(w)
                yield r
                if r.status() is False:
                    yield async.Coroutine.failure(r.args()[0])

            yield async.Coroutine.success()

        return async.Coroutine(coroutine(),rpcerrorhandler)

    def wait(self):
        if not self.__jobs:
            s = self.__status
            self.__status = True
            if s:
                return async.success()
            else:
                return async.failure('background job failed')

        result = async.Deferred()
        self.__waiters.append(result)
        return result

    def add_job(self,result):
        if result.status() is not None:
            if not result.status():
                self.__status = False
            return

        self.__jobs.add(result)

        def completed(status):
            print 'background job',status
            self.__jobs.discard(result)

            if not status:
                self.__status = False

            if self.__jobs:
                return

            waiters = self.__waiters
            self.__waiters = []

            if self.__status:
                for w in waiters: w.succeeded()
            else:
                for w in waiters: w.failed('background job failed')

        result.setCallback(lambda: completed(True))
        result.setErrback(lambda msg: completed(False))

    def set_result(self,r):
        self.__result = r

    def get_result(self):
        return self.__result

    def set_args(self,args):    
        self.__args = args

    def get_arg(self,key):
        return self.__args.get(key)

    def get_database(self):
        return self.__database

    def get_context(self):
        return self.__context

    def get_delegate(self):
        return self.__delegate

    def get_agent(self):
        return self.__agent

    def sync(self, *args):
        return self.__database.sync(*args)

    def classify(self,word):
        classification = self.__database.classify(word)
        if classification: return classification
        return ('noun',word)

    def interpret(self,word):
        (klass,word) = self.classify(word)
        return self.__interpret0(klass,word)

    def undo(self):
        if not self.__history:
            return None

        (w,k,s,d) = self.__history.pop()

        self.__stack = s
        self.__digits = d

        return w

    @async.coroutine('internal error')
    def close(self):
        if self.__digits:
            num = ''.join(self.__digits)
            self.__digits = []
            self.__interpret0('noun',num)
            self.__history.pop()

        top = self.topany()
        if top is not None:
            m = top.close(self)
            yield m
            if m is not None and not m.status():
                yield async.Coroutine.failure(*m.args(),**m.kwds())

    def line_clear(self):
        self.__stack = []
        self.__history = []
        self.__digits = []

    @async.coroutine('internal error')
    def __interpret0(self,klass,word):
        s = copy.deepcopy(self.__stack)
        d = self.__digits[:]

        if klass == 'digit':
            self.__history.append((word,klass,s,d))
            self.__digits.extend(word)
            yield async.Coroutine.success()

        if self.__digits:
            num = ''.join(self.__digits)
            self.__digits = []
            r = self.__interpret0('noun',num)
            yield r
            if not r.status():
                yield async.Coroutine.failure(*r.args(),**r.kwds())
            self.__history.pop()

        self.__history.append((word,klass,s,d))

        top = self.topany()
        if top:
            r = top.interpret(self,klass,word)
            yield r
            if not r.status():
                yield async.Coroutine.failure(*r.args(),**r.kwds())
            if r.args()[0]==True:
                yield async.Coroutine.success()

        action = self.__database.lookup_primitive(klass)

        if not action:
            yield async.Coroutine.failure('word ignored')

        s = (self.__context.get_snapshot(),copy.deepcopy(self.__stack))
        ar = action(self,word)

        yield ar

        if ar.status():
            if not self.__stack:
                if klass != 'again':
                    self.__lastcmd = (lambda: action(self,word),s)
                self.__history = []
                if self.__statemgr:
                    yield self.__checkpoint()
                self.__delegate.buffer_done(True,"",klass=='again')
                if 'user_errors' in ar.kwds():
                    for err in ar.kwds()['user_errors']:
                        self.__delegate.error_message(err)
               
                if klass != 'ahem' and klass != 'hey':
                    self.__context.clear_inner_scope()
            yield async.Coroutine.success()
        else:
            if not self.__stack:
                self.__history = []
                if self.__statemgr:
                    yield self.__checkpoint()
                self.__delegate.buffer_done(False,"",klass=='again')
                if 'user_errors' in ar.kwds():
                    for err in ar.kwds()['user_errors']:
                        self.__delegate.error_message(err)

                self.__context.clear_inner_scope()
            yield async.Coroutine.failure(*ar.args(),**ar.kwds())

    @async.coroutine('internal error')
    def __checkpoint(self):
        r = rpc.invoke_rpc(self.__statemgr,'get_checkpoint','')
        yield r

        if not r.status():
            print 'checkpoint failure',r.args()
            self.__laststate = None
            self.__undostate = None
        else:
            self.__undostate = self.__laststate
            self.__laststate = r.args()[0]
            print 'checkpointed at',self.__laststate

    def checkpoint_undo(self):
        if not self.__statemgr:
            return async.failure('no state manager')

        l = self.__undostate
        self.__undostate = None
        self.__laststate = None
        self.__lastcmd = None

        if l is None:
            return async.failure('no checkpoint')

        @async.coroutine('internal error')
        def __undo():
            r = rpc.invoke_rpc(self.__statemgr,'load_checkpoint',l)
            yield r

            if not r.status():
                print 'checkpoint load failure',r.args()
            else:
                print 'reverted to',l

        return __undo()
        

    def again(self):
        if not self.empty():
            print 'garbled again'
            return async.failure('garbled again command')

        if self.__lastcmd is None:
            return async.failure('no last command')

        (a,(c,s)) = self.__lastcmd

        self.__stack = []
        self.__context.set_snapshot(c)

        print 'again:',a
        print 'stack:',' '.join([str(ss) for ss in s])

        for ss in s:
            if isinstance(s,referent.Referent):
                ss = ss.reinterpret(self,self.__context.get_noun_scope())
            self.__stack.append(ss)

        print 'stack:',' '.join([str(ss) for ss in self.__stack])
        return a()

    def topany(self):
        if len(self.__stack):
            return self.__stack[-1]
        return None

    def top(self,type):
        if len(self.__stack):
            t = self.__stack[-1]
            if isinstance(t,type):
                return t
        return None

    def popany(self):
        if len(self.__stack)>0:
            return self.__stack.pop()
        return None

    def pop(self,type):
        t=self.topany()
        if t and isinstance(t,type):
            return self.__stack.pop()
        return None

    def clear(self):
        self.__stack=[]

    def empty(self):
        return not self.__stack

    def size(self):
        return len(self.__stack)

    def push(self,item):
        self.__stack.append(item)

    def stack(self):
        return self.__stack

    def popall(self,type):
        r=[]
        while True:
            t=self.pop(type)
            if not t: return r
            r.append(t)

    def iterstack(self):
        return reversed(self.__stack)

class VerbAction(referent.StackObj):
    def __str__(self):
        return '<<verbaction>>'
    def words(self):
        return ()
    def execute(self,interp,verb,mods,roles,args,bg,text):
        return async.failure('not implemented')

