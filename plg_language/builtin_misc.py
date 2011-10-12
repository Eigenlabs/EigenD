
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

from pi.logic.shortcuts import *
from pi import action,logic,async,domain,utils,resource,rpc,timeout
from plg_language import interpreter,noun,imperative,referent
import traceback

class SubDelegate(interpreter.Delegate):
    def __init__(self,agent):
        self.__agent = agent

    def error_message(self,err):
        return self.__agent.error_message(err)

    def buffer_done(self,*a,**kw):
        return 

class Builtins:

    rules_lexicon = """
    enumerate0([],O).
    enumerate0([W|WORDS],O) :- @name(W,O), enumerate0(WORDS,O).
    enumerate(IGNORE,WORDS,OL) :- @is(OL,$alluniq(ORD,enumerate0(WORDS,O),@ordinal(ORD,O),@not(@in(O,IGNORE)))).
    """

    def __init__(self,agent,database):
        self.database = database
        self.agent = agent

    def primitive_again(self,interp,word):
        return interp.again()

    def primitive_describe(self,interp,word):
        t = interp.pop(referent.Referent)

        if t is None:
            return async.failure('not object')

        obj = t.concrete_ids()

        if not obj:
            return async.failure('no objects')

        print obj

        for o in obj:
            d= self.database.find_full_desc(o)
            print o,d

        return async.success()

    def verb2_99_ruleset(self,subject):
        """
        ruleset([],None)
        """
        print "== ruleset =="
        r=resource.open_logfile('ruleset')
        for t in enumerate(self.database.iterrules()):
            print >>r,"%i: %s" % t
        r.flush()
        return async.success()

    def primitive_context(self,interp,word):
        if interp.get_context().stack_empty():
            print '== context empty =='
            return async.success()

        print "== context =="
        for t in enumerate(interp.get_context().iter_stack()):
            print "%i: %s" % t
        return async.success()

    def verb2_1_synchronise(self,subject):
        """
        synchronise([],None)
        """
        print "== syncing (verb) =="
        return self.database.sync()

    def verb2_3_name(self,subject,thing,value):
        """
        name([],global_name,role(None,[concrete]),role(to,[abstract]))
        """

        words = action.abstract_wordlist(value)
        thing = action.concrete_object(thing)
        ordinal = None

        try:
            ordinal = int(words[-1])
            words = words[:-1]
        except:
            pass

        print 'set type name of ',thing,' to ',words,ordinal

        proxy = self.database.find_item(thing)
        if not proxy:
            return async.failure('internal error: no proxy')

        def co():
            yield interpreter.RpcAdapter(proxy.invoke_rpc('set_names',' '.join(words)))
            if ordinal is not None:
                yield interpreter.RpcAdapter(proxy.invoke_rpc('set_ordinal',str(ordinal)))
            else:
                yield interpreter.RpcAdapter(proxy.invoke_rpc('clear_ordinal',''))

        return async.Coroutine(co(),interpreter.rpcerrorhandler)

    def verb2_5_number(self,subject,thing,value):
        """
        number([],global_number,role(None,[concrete,singular]),role(to,[abstract]))
        """

        value = action.abstract_string(value)
        thing = action.concrete_object(thing)

        try:
            value=int(value)
        except:
            return async.failure('internal error: non numeric ordinal')

        print 'set ordinal of ',thing,' to ',value

        proxy = self.database.find_item(thing)
        if not proxy:
            return async.failure('internal error: no proxy')

        def co():
            yield interpreter.RpcAdapter(proxy.invoke_rpc('set_ordinal',str(value)))

        return async.Coroutine(co(),interpreter.rpcerrorhandler)

    def verb2_6_renumber(self,subj,things,names):
        """
        renumber([],global_renumber,role(None,[concrete]),role(with,[abstract]))
        """

        things = tuple(action.concrete_objects(things))
        names = action.abstract_wordlist(names)

        name_cache = self.database.get_propcache('name')
        ord_cache = self.database.get_propcache('ordinal')
        world = self.database.get_propcache('props').get_idset('agent')

        a=world
        for nn in names:
            a = a.intersection(name_cache.get_idset(nn))
        a = a.difference(set(things))
        d = [int(o) for oo in a for o in ord_cache.get_valueset(oo)]
        x=reduce(lambda a,b: a if a>b else b,d,0)

        def co(x):
            for  thing in things:
                x+=1
                print 'renumbering',thing,'with',names,'ordinal',x
                yield interpreter.RpcAdapter(rpc.invoke_rpc(thing,'set_names',' '.join(names)))
                yield interpreter.RpcAdapter(rpc.invoke_rpc(thing,'set_ordinal',str(x)))

        return async.Coroutine(co(x),interpreter.rpcerrorhandler)

    def verb2_10_associate(self,subject,part,whole):
        """
        associate([],global_associate,role(None,[or([concrete],[virtual]),singular]),role(to,[concrete,singular]))
        """
        part = action.arg_objects(part)[0]
        whole = action.concrete_object(whole)
        rel = 'join(%s,role(to,[instance(~self)]))' % part

        proxy = self.database.find_item(whole)
        if not proxy:
            return async.failure('internal error: no proxy')

        def co():
            yield interpreter.RpcAdapter(proxy.invoke_rpc('add_relation',rel))

        return async.Coroutine(co(),interpreter.rpcerrorhandler)

    def verb2_11_unassociate(self,subject,part,whole):
        """
        associate([un],global_unassociate,role(None,[concrete]),role(from,[concrete,singular]))
        """
        part = action.arg_objects(part)[0]
        whole = action.concrete_object(whole)
        rel = 'join(%s,role(to,[instance(~self)]))' % part

        proxy = self.database.find_item(whole)
        if not proxy:
            return async.failure('internal error: no proxy')

        def co():
            yield interpreter.RpcAdapter(proxy.invoke_rpc('remove_relation',rel))

        return async.Coroutine(co(),interpreter.rpcerrorhandler)

    def verb2_14_find(self,subject,objects):
        """
        find([],None,role(None,[]))
        """

        print 'find:',objects

    def verb2_21_statemgr(self,subject,sm):
        """
        use([],None,role(None,[concrete,proto(setupmanager),singular]))
        """
        sm = action.concrete_object(sm)
        self.agent.set_statemgr(sm)
        print 'using',sm,'for checkpointing'


    def verb2_22_undo(self,subject):
        """
        undo([],None)
        """
        return self.agent.checkpoint_undo()

    def verb2_26_clear(self,subject,arg):
        """
        clear([],None,role(None,[abstract,matches([history])]))
        """

        self.agent.clear_history()
        #thing= action.abstract_wordlist(arg)
        #print 'clear',thing

    def verb2_29_say(self,subject,arg):
        """
        say([],None,role(None,[abstract]))
        """

        words = action.abstract_wordlist(arg)
        print 'injecting',words

        for w in words:
            self.agent.inject(w)

    @async.coroutine('internal error')
    def verb2_31_do(self,subject,arg):
        """
        do([],None,role(None,[abstract]))
        """

        words = action.abstract_wordlist(arg)
        interp = interpreter.Interpreter(self.agent,self.database, SubDelegate(self.agent))

        if not len(words):
            print 'nothing to do',arg
            return

        print 'doing',words
        self.agent.register_interpreter(interp)

        try:
            r = interp.process_block(words)
            yield r
            if r.status():
                print 'SUCCEEDED',words
            else:
                print 'FAILED',words
        finally:
            self.agent.unregister_interpreter(interp)


    def primitive_wait(self,interp,word):
        t = interp.pop(referent.Referent)

        if t is None:
            return interp.wait()

        if len(t.words())!=1:
            return async.failure('not number')

        n = None
        try:
            n = int(t.words()[0])
        except:
            return async.failure('not number')

        print 'waiting',n
        return timeout.Timeout(async.Deferred(),int(n*1000),True)
