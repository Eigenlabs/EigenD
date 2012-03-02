
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
from pi import paths, logic, async, action, rpc
from . import interpreter,referent

import piw

pri_global = 1
pri_inner = 3
pri_outer = 2

def disambiguate(db,ids,words,all):
    if all or len(ids)==1:
        return ids

    exact = set()
    name_cache = db.get_propcache('name')
    aws = frozenset(words)
    awsi = aws.union(('input',))
    awsa = aws.union(('agent',))

    for o in ids:
        iws = name_cache.get_valueset(o)
        if iws == aws or iws == awsi or iws == awsa:
            exact.add(o)

    if exact:
        return exact

    return ids

def disambiguate_virtual(db,ids,words):
    exact = set()
    name_cache = db.get_propcache('name')
    aws = frozenset(words)
    awsi = aws.union(('input',))

    for o in ids:
        iws = name_cache.get_valueset(o)
        if iws.issubset(aws) or iws.issubset(awsi):
            exact.add(o)

    if exact:
        return exact

    return ids

def get_parts(db,ids):
    val = set()

    join_cache = db.get_joincache()
    part_cache = db.get_partcache()
    assoc_cache = db.get_assoccache()
    bind_cache = db.get_propcache('bind')
    host_cache = db.get_propcache('host')

    for o in ids:
        val.update(part_cache.lefts(o))
        val.update(join_cache.direct_lefts(o))
        val.update(assoc_cache.direct_lefts(o))
        assoc = assoc_cache.lefts(o)
        for a in assoc:
            val.update(bind_cache.get_idset(a))
        val.update(host_cache.get_idset(o))

    return val

def get_number(s):
    try: return int(s)
    except: return None

@async.coroutine('internal error')
def refine_state(db,word,istate):
    ostate = []
    n = get_number(word)

    while istate:
        cstate = []

        if word=='all':
            for s in istate:
                r = s.refine_all(db,word)
                if r.status() is None: yield r
                if r.status():
                    (o,c) = r.args()
                    ostate.extend(o)
                    cstate.extend(c)

            istate = cstate
            continue

        if n is None:
            for s in istate:
                r = s.refine(db,word)
                if r.status() is None: yield r
                if r.status():
                    (o,c) = r.args()
                    ostate.extend(o)
                    cstate.extend(c)

            istate=cstate
            continue

        for s in istate:
            r = s.refine_number(db,word)
            if r.status() is None: yield r
            if r.status():
                (o,c) = r.args()
                ostate.extend(o)
                cstate.extend(c)

        istate = cstate

    yield async.Coroutine.success(ostate)

@async.coroutine('internal error')
def finalise_state(db,istate):
    objects = dict()
    max_pri = -1

    for s in istate:
        r = s.flush(db)
        if r.status() is None: yield r
        if r.status():
            rv = r.args()[0]
            for (pri,olist) in rv:
                objects.setdefault(pri,[]).extend(olist)
                if pri > max_pri: max_pri = pri
                
    if max_pri < 0:
        yield async.Coroutine.success([])
    
    yield async.Coroutine.success(objects[max_pri])


def make_state(pri,obj):
    if logic.is_pred(obj,'cnc'):
        return State_FinalAtom_Unknown(pri,set((obj.args[0],)))

    if logic.is_pred(obj,'ideal'):
        return State_Ideal(pri,obj)

    if logic.is_pred(obj,'dsc'):
        return State_Descriptor(pri,obj.args[0],obj.args[1])

    if logic.is_pred(obj,'cmp'):
        return State_Composite(pri,obj.args[0])

    return None


class State_Initial:
    def __init__(self,db,outer = None,inner = None,all = False):
        self.__outer = outer or frozenset()
        self.__inner = inner or frozenset()
        self.__all = all

    def __repr__(self):
        return '<initial %s:%s:%s>' % (self.__inner,self.__outer,self.__all)

    def refine_number(self,db,w):
        return async.success([],[])

    def refine_all(self,db,w):
        return async.success([State_Initial(db,self.__outer,self.__inner,True)],[])

    def refine(self,db,w):
        vids = db.get_propcache('protocol').get_idset('virtual')
        world = db.get_propcache('props').get_idset('agent')
        raids = db.get_propcache('props').get_idset('inrig')
        wids = db.get_propcache('name').get_idset(w)

        states = []
        cstates = []

        o_world_agent = world.intersection(wids).difference(raids)
        o_outer_agent = self.__outer.intersection(wids).difference(raids)

        inner_parts = get_parts(db,self.__inner)
        outer_parts = get_parts(db,self.__outer)

        o_outer_part = outer_parts.intersection(wids)
        o_inner_part = inner_parts.intersection(wids)

        if o_world_agent:
            states.append(State_Atom(pri_global,[w],o_world_agent,self.__all))

        if o_outer_agent:
            states.append(State_Atom(pri_outer,[w],o_outer_agent,self.__all))

        if o_outer_part:
            states.append(State_Atom(pri_outer,[w],o_outer_part,self.__all))

        if o_inner_part:
            states.append(State_Atom(pri_inner,[w],o_inner_part,self.__all))

        ovids = outer_parts.intersection(vids)
        ivids = inner_parts.intersection(vids)

        if ovids:
            cstates.append(State_BufferedChild(db,pri_outer,[],self.__all,ovids))

        if ivids:
            cstates.append(State_BufferedChild(db,pri_inner,[],self.__all,ivids))

        return async.success(states,cstates)

    def flush(self,db):
        return async.success([])


class State_Atom:
    def __init__(self,pri,words,ids,all):
        self.__ids = ids
        self.__words = words
        self.__all = all
        self.__pri = pri

    def __repr__(self):
        return '<atom %s:%s:%s>' % (self.__words,self.__ids,self.__all)

    def refine_all(self,db,w):
        ids = disambiguate(db,self.__ids,self.__words,self.__all)

        if not ids:
            return async.success([],[])

        if not self.__all and len(ids)!=1:
            return async.success([],[])

        return async.success([State_FinalAtom_All(self.__pri,ids)],[])

    def refine_number(self,db,w):
        nids = db.get_propcache('ordinal').get_idset(w)
        ids = self.__ids.intersection(nids)

        if not ids:
            return async.success([],[])

        ids = disambiguate(db,ids,self.__words,self.__all)

        if not self.__all and len(ids)!=1:
            return async.success([],[])

        return async.success([State_FinalAtom_Unknown(self.__pri,ids)],[])

    def refine(self,db,w):
        states = []

        wids = db.get_propcache('name').get_idset(w)
        ids = self.__ids.intersection(wids)

        if w not in self.__words and ids:
            return async.success([State_Atom(self.__pri,self.__words+[w],ids,self.__all)],[])

        ids = disambiguate(db,self.__ids,self.__words,self.__all)

        if not self.__all and len(ids)!=1:
            return async.success([],[])

        return async.success([],[State_FinalAtom_NoAll(self.__pri,ids)])

    def flush(self,db):
        print 'pre-dis',self.__ids
        ids = disambiguate(db,self.__ids,self.__words,self.__all)
        print 'post-dis',ids

        if not self.__all and len(ids)!=1:
            return async.success([])

        vids = db.get_propcache('protocol').get_idset('virtual')
        cnc = ids.difference(vids)
        vrt = ids.intersection(vids)
        all = [T('cnc',o) for o in cnc]
        all.extend([T('virtual',o) for o in vrt])
        return async.success([(self.__pri,all)])


class State_FinalAtom_NoAll:
    def __init__(self,pri,ids):
        self.__pri = pri
        self.__ids = ids

    def __repr__(self):
        return '<final_atom_noall %s>' % (self.__ids)

    def refine_all(self,db,w):
        return async.success([],[])

    def refine_number(self,db,w):
        return async.success([],[])

    def refine(self,db,w):
        return async.success([],[State_FinalAtom(self.__pri,set([i]),False) for i in self.__ids])

    def flush(self,db):
        return async.success([])


class State_FinalAtom_All:
    def __init__(self,pri,ids):
        self.__pri = pri
        self.__ids = ids

    def __repr__(self):
        return '<final_atom_all %s>' % (self.__ids)

    def refine_all(self,db,w):
        return async.success([],[])

    def refine_number(self,db,w):
        return async.success([],[])

    def refine(self,db,w):
        return async.success([],[State_FinalAtom(self.__pri,self.__ids,True)])

    def flush(self,db):
        return async.success([])


class State_FinalAtom_Unknown:
    def __init__(self,pri,ids):
        self.__pri = pri
        self.__ids = ids

    def __repr__(self):
        return '<final_atom_unknown %s>' % (self.__ids)

    def refine_all(self,db,w):
        return async.success([State_FinalAtom_All(self.__pri,self.__ids)],[])

    def refine_number(self,db,w):
        return async.success([],[])

    def refine(self,db,w):
        return async.success([],[State_FinalAtom(self.__pri,set([i]),False) for i in self.__ids])

    def flush(self,db):
        return State_FinalAtom(self.__pri,self.__ids,False).flush(db)


class State_FinalAtom:
    def __init__(self,pri,ids,all):
        self.__pri = pri
        self.__ids = ids
        self.__all = all

    def __repr__(self):
        return '<final_atom %s:%s:%s>' % (self.__pri,self.__ids,self.__all)

    def refine_all(self,db,w):
        return async.success([],[])

    def refine_number(self,db,w):
        return async.success([],[])

    def refine(self,db,w):
        ostates = []
        cstates = []

        wids = db.get_propcache('name').get_idset(w)
        vids = db.get_propcache('protocol').get_idset('virtual')

        children = get_parts(db,self.__ids)

        vchildren = children.intersection(vids)
        cchildren = children.intersection(wids)

        if cchildren:
            ostates.append(State_Atom(self.__pri,[w],cchildren,self.__all))

        if vchildren:
            cstates.append(State_BufferedChild(db,self.__pri,[],self.__all,vchildren))

        return async.success(ostates,cstates)

    def flush(self,db):
        ids = self.__ids
        vids = db.get_propcache('protocol').get_idset('virtual')
        cnc = ids.difference(vids)
        vrt = ids.intersection(vids)
        all = [T('cnc',o) for o in cnc]
        all.extend([T('virtual',o) for o in vrt])
        return async.success([(self.__pri,all)])

class State_BufferedChild:
    def __init__(self,db,pri,words,all,children):
        self.__pri = pri
        self.__words = words
        self.__all = all
        self.__children = children

    def __repr__(self):
        return '<buffered_child %s:%s:%s>' % (self.__pri,self.__words,self.__children)

    def refine_all(self,db,w):
        return async.success([],[])

    def refine_number(self,db,w):
        return async.success([],[])

    def refine(self,db,w):
        states = []
        wids = db.get_propcache('name').get_idset(w)

        vchildren = self.__children.intersection(wids)

        if vchildren:
            return async.success([State_Virtual(self.__pri,self.__words,vchildren,self.__all)],[])

        if not self.__children:
            return async.success([],[])

        return async.success([State_BufferedChild(db,self.__pri,self.__words+[w],self.__all,self.__children)],[])

    def flush(self,db):
        return async.success([])

class State_Virtual:
    def __init__(self,pri,words,ids,all):
        self.__pri = pri
        self.__words = words
        self.__ids = ids
        self.__all = all

    def __repr__(self):
        return '<virtual %s:%s:%s:%s>' % (self.__pri,self.__words,self.__ids,self.__all)

    @async.coroutine('internal error')
    def resolve(self,db,ids,words,ordinal):
        states = []

        for id in ids:
            id_words = db.get_propcache('name').get_valueset(id)
            id_working = tuple(set(words).difference(id_words))
            print 'resolving',id_working,ordinal,'on',id
            result = rpc.invoke_rpc(id,'resolve',action.marshal((id_working,ordinal)))
            yield result
            if not result.status():
                print 'resolution error',result.args()
                continue

            result = logic.parse_clause(result.args()[0],paths.make_subst(id))
            print 'resolved to',result

            for r in result:
                s = make_state(self.__pri,r)
                if s is not None:
                    states.append(s)

        yield async.Coroutine.success(states)

    def refine_all(self,db,w):
        return async.success([],[])

    @async.coroutine('internal error')
    def refine_number(self,db,w):
        print 'refining',self.__ids,self.__words,w
        ids = disambiguate_virtual(db,self.__ids,self.__words)
        print 'refining',ids,self.__words,w
        r = self.resolve(db,ids,self.__words,w)

        yield r

        if not r.status():
            print 'resolution error',r.args()
            yield async.Coroutine.success([])

        yield async.Coroutine.success(r.args()[0],[])

    def refine_all(self,db,w):
        return async.success([],[])

    @async.coroutine('internal error')
    def refine(self,db,w):
        states = []

        if not self.__all:
            yield async.Coroutine.success([],[])

        wids = db.get_propcache('name').get_idset(w)
        refined_ids = self.__ids.intersection(wids)

        if refined_ids:
            yield async.Coroutine.success([State_Virtual(self.__pri,self.__words+[w],refined_ids,self.__all)],[])

        ids = disambiguate_virtual(db,self.__ids,self.__words)
        r = self.resolve(db,ids,self.__words,None)
        yield r

        if not r.status():
            print 'resolution error',r.args()
            yield async.Coroutine.success([])

        yield async.Coroutine.success([],r.args()[0])

    @async.coroutine('internal error')
    def flush(self,db):
        if not self.__all and not self.__words:
            yield async.Coroutine.success([])

        ids = disambiguate_virtual(db,self.__ids,self.__words)
        r = self.resolve(db,ids,self.__words,None)
        yield r

        if not r.status():
            print 'resolution error',r.args()
            yield async.Coroutine.success([])

        s = finalise_state(db,r.args()[0])
        yield s

        if not s.status():
            print 'resolution error',s.args()
            yield async.Coroutine.success([])

        print 'finalisation returns',s.args()

        yield async.Coroutine.success([(self.__pri,s.args()[0])])


class State_TaggedIdeal:
    def __init__(self,pri,ideal,words):
        self.__pri = pri
        self.__ideal = ideal
        self.__words = words

    def __repr__(self):
        return '<ideal %s>' % (self.__ideal)

    def refine_number(self,db,w):
        return async.success([],[])

    def refine_all(self,db,w):
        return async.success([State_TaggedIdeal(self.__pri,self.__ideal,self.__words+[w])],[])

    def refine(self,db,w):
        return async.success([State_TaggedIdeal(self.__pri,self.__ideal,self.__words+[w])],[])

    def flush(self,db):
        i=self.__ideal
        w=tuple(self.__words)
        return async.success([(self.__pri,[T('tagged_ideal',i.args[0],i.args[1],w)])])


class State_Ideal:
    def __init__(self,pri,ideal):
        self.__pri = pri
        self.__ideal = ideal

    def __repr__(self):
        return '<ideal %s>' % (self.__ideal)

    def refine_number(self,db,w):
        return async.success([],[])

    def refine_all(self,db,w):
        return async.success([State_TaggedIdeal(self.__pri,self.__ideal,[w])],[])

    def refine(self,db,w):
        return async.success([State_TaggedIdeal(self.__pri,self.__ideal,[w])],[])

    def flush(self,db):
        return async.success([(self.__pri,[self.__ideal])])



class State_Descriptor:
    def __init__(self,pri,id,path):
        self.__pri = pri
        self.__id = id
        self.__path = path

    def __repr__(self):
        return '<descriptor %s:%s:%s>' % (self.__pri,self.__id,self.__path)

    def refine_all(self,db,w):
        return async.success([],[])

    def refine_number(self,db,w):
        nids = db.get_propcache('ordinal').get_idset(w)

        if self.__id not in nids:
            return async.success([],[])

        return async.success([State_Descriptor(self.__pri,self.__id,self.__path)],[])

    def refine(self,db,w):
        wids = db.get_propcache('name').get_idset(w)

        if self.__id not in wids:
            return async.success([],[])

        return async.success([State_Descriptor(self.__pri,self.__id,self.__path)],[])

    def flush(self,db):
        return async.success([(self.__pri,[T('dsc',self.__id,self.__path)])])


class State_Composite:
    def __init__(self,pri,components):
        self.__pri = pri
        self.__components = components
        self.__all = all

    def __repr__(self):
        return '<composite %s:%s>' % (self.__pri,self.__components)

    @async.coroutine('internal error')
    def refine_number(self,db,w):
        yield async.Coroutine.success([],[make_state(self.__pri,c) for c in self.__components])

    @async.coroutine('internal error')
    def refine(self,db,w):
        yield async.Coroutine.success([],[make_state(self.__pri,c) for c in self.__components])

    @async.coroutine('internal error')
    def refine_all(self,db,w):
        yield async.Coroutine.success([],[make_state(self.__pri,c) for c in self.__components])

    @async.coroutine('internal error')
    def flush(self,db):
        yield async.Coroutine.success([(self.__pri,[T('cmp',tuple(self.__components))])])

class ConcreteReferent(referent.Referent):
    
    def __init__(self,interp=None,outer=None,inner=None,state=None,words=[],open=True):
        referent.Referent.__init__(self)
        self.__open = open
        self.__words = words[:]

        if state is not None:
            self.__state = state
            return

        if outer is None:
            out_scope = interp.get_context().get_noun_scope()
        else:
            out_scope = outer

        if inner is None:
            in_scope = interp.get_context().get_inner_scope()
        else:
            in_scope = inner

        self.__state = self.init_state(interp,out_scope,in_scope)
        print 'initial state:',self.__state,in_scope,out_scope

    def stack_copy(self):
        return ConcreteReferent(state=self.__state,words=self.__words,open=self.__open)

    def init_state(self,interp,oscope,iscope):
        db = interp.get_database()
        return [State_Initial(db,outer=oscope,inner=iscope)]

    @async.coroutine('internal error')
    def interpret(self,interp,klass,word):
        if not self.__open:
            yield async.Coroutine.success(False)

        if klass == 'noun':
            s = (yield ResolvHandler(refine_state(interp.get_database(),word,self.__state)))
            self.__words.append(word)
            self.__state = s
            print 'after',word,'state:',self.__state
            yield async.Coroutine.success(True)

        if self.__state is None:
            yield async.Coroutine.failure('need noun')

        yield async.Arg0(self.close(interp),ResolutionError)
        yield async.Coroutine.success(False)

    def finalised(self,interp,objects):
        if len(objects)==0:
            print "%s: doesn't exist" % ' '.join(self.__words)

    @async.coroutine('internal error')
    def reinterpret(self,interp,scope):
        words = self.__words[:]

        ref = ConcreteReferent(interp=interp,outer=scope,inner=[])

        print 'reinterpreting',words,'in',scope

        if not words:
            yield async.Coroutine.success()

        for w in words:
            r = ref.interpret(interp,'noun',w)
            yield r
            if not r.status():
                yield async.Coroutine.failure(r.args[0])

        r = ref.close(interp)
        yield r
        if not r.status():
            yield async.Coroutine.failure(r.args[0])

        yield async.Coroutine.success(ref)

    @async.coroutine('internal error')
    def close(self,interp):
        if self.__open:
            s = self.__state
            self.__open=False
            self.__state = None

            s = (yield ResolvHandler(finalise_state(interp.get_database(),s)))
            s = s+[logic.make_term('abstract',tuple(self.__words))]
            print 'after flush, state=',s
            self.set_referent(words=self.__words,objects=s)
            self.finalised(interp,s)

        yield async.Coroutine.success()


class QuoteReferent(referent.Referent):

    def __init__(self,words,open=True):
        referent.Referent.__init__(self)
        self.__open = open
        self.__words = words

    def stack_copy(self):
        return QuoteReferent(self.__words[:],self.__open)

    def interpret(self,interp,klass,word):
        if not self.__open:
            return async.success(False)
        self.__words.append(word)
        self.set_referent(words=self.__words,objects=(logic.make_term('abstract',tuple(self.__words)),))
        self.__open = False
        return async.success(True)

class BlockReferent(referent.Referent):

    def __init__(self,delim,words,open=True):
        referent.Referent.__init__(self)
        self.__delim = delim[:]
        self.__open = open
        self.__words = words[:]

    def stack_copy(self):
        return BlockReferent(self.__delim,self.__words,self.__open)

    def interpret(self,interp,klass,word):
        if not self.__open:
            return async.success(False)

        self.__words.append(word)

        if klass == 'block':
            if word==self.__delim[-1]:
                self.__delim.pop()
                if not self.__delim:
                    self.__open = False
                    self.set_referent(words=self.__words,objects=(logic.make_term('abstract',tuple(self.__words[1:-1])),))
            else:
                self.__delim.append(word)

        return async.success(True)

class IdReferent(referent.Referent):

    def __init__(self,words,open=True):
        referent.Referent.__init__(self)
        self.__open = open
        self.__words = words[:]

    def stack_copy(self):
        return IdReferent(self.__words,self.__open)

    def interpret(self,interp,klass,word):
        if not self.__open:
            return async.success(False)

        self.__open = False
        self.__words.append(word)

        olist = interp.get_database().get_propcache('pronoun').get_idset(word)
        olist = [T('cnc',o) for o in olist]

        self.set_referent(words=self.__words,objects=olist)
        return async.success(True)

class AddrReferent(referent.Referent):

    def __init__(self,words,open=True):
        referent.Referent.__init__(self)
        self.__open = open
        self.__words = words[:]

    def stack_copy(self):
        return AddrReferent(self.__words,self.__open)

    def interpret(self,interp,klass,word):
        if not self.__open:
            return async.success(False)

        self.__open = False
        self.__words.append(word)
        self.set_referent(words=self.__words,objects=(T('cnc',word),))
        return async.success(True)

class ItReferent(referent.Referent):
    def __init__(self,objs,word):
        self.__word = word
        referent.Referent.__init__(self,objects=objs)

class VarReferent(referent.Referent):

    def __init__(self,words,open=True):
        referent.Referent.__init__(self)
        self.__open = open
        self.__words = words[:]

    def stack_copy(self):
        return VarReferent(self.__words,self.__open)

    def interpret(self,interp,klass,word):
        if not self.__open:
            return async.success(False)

        if klass == 'noun':
            self.__words.append(word)
            return async.success(True)

        self.__open = False

        var = interp.get_agent().get_variable(' '.join(self.__words[1:]))
        obj = ()
        if var is not None:
            obj = logic.parse_clause(var)

        print 'resolved variable', self.__words,obj

        self.set_referent(words=self.__words,objects=obj)
        return async.success(False)

@async.coroutine('internal error')
def primitive_noun(interp,word):
    prefix = [word]

    while not interp.empty():
        m = interp.pop(referent.ModMarker)
        if m is None:
            break
        prefix.append(m.word)

    prefix.reverse()

    r = ConcreteReferent(interp=interp)
    interp.push(r)

    for w in prefix:
        result = r.interpret(interp,'noun',w)
        yield result
        if not result.status():
            yield async.Coroutine.failure(*r.args(),**r.kwds())

    yield async.Coroutine.success()

def primitive_possessive(interp,word):
    n = interp.pop(referent.Referent)

    if n is None:
        return async.failure('no noun for it')

    scope = n.concrete_ids()

    if not scope:
        return async.failure('no noun for it')

    print 'possesive:',scope

    r = ConcreteReferent(interp=interp,inner=set(scope))
    interp.push(r)
    return async.Coroutine.success()

def primitive_quote(interp,word):
    interp.push(QuoteReferent([word]))
    return async.success()

def primitive_block(interp,word):
    interp.push(BlockReferent([word],[word]))
    return async.success()

def primitive_it(interp,word):
    for c in interp.get_context().iter_stack():
        if len(c) == 1:
            interp.push(ItReferent(tuple(T('cnc',id) for id in c),word))
            return async.success()
    return async.failure('no it')

def primitive_comma(interp,word):
    return async.success()

def primitive_addr(interp,word):
    interp.push(AddrReferent([word]))
    return async.success()

def primitive_var(interp,word):
    interp.push(VarReferent([word]))
    return async.success()

def primitive_id(interp,word):
    interp.push(IdReferent([word]))
    return async.success()

class ResolutionError(Exception):
    pass

class ResolvHandler(async.Arg1):
    def __init__(self,deferred):
        async.Arg1.__init__(self,deferred,ResolutionError)

@async.coroutine('internal error')
def interpret(interp,scope,words):
    print 'reinterpreting',words,'in',scope

    ref = ConcreteReferent(interp=interp,outer=scope,inner=[])

    for w in words:
        r = ref.interpret(interp,'noun',w)
        yield r
        if not r.status():
            yield async.Coroutine.failure(r.args[0])

    r = ref.close(interp)
    yield r
    if not r.status():
        yield async.Coroutine.failure(r.args[0])

    yield async.Coroutine.success(ref)

