
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

import piw
import sys
import time

from pi import const,logic,paths,proxy,action,utils,index,async,constraints,rpc
from pi.logic.shortcuts import *

rules_database = """
    # synonym(A/bound,B) -> B is synonymous with A
        synonym(A,A).
        synonym(A,B) :- db_translation(A,B).
        synonym(A,B) :- db_translation(B,A).

    # classify(word,class)
        classify(W,CLS,S) :- synonym(W,S), @or(db_classification(S,CLS), db_classification2(S,CLS)).

    # translate(english,music)
        translate(E,M) :- db_translation(E,M).
        toenglish(M,E) :- @or(db_translation(E,M),@is(E,M)).

    db_relation(connect,cnc(FROMID),[role(to,[instance(TOID)])]) :- @ssmaster(TOID,FROMID).
    """

def popset(s):
    if s:
        for i in s:
            return i
    return None

class VerbProxy:
    def __init__(self,id,schema):
        self.__id = id
        self.__index = schema.args[0]

        schema = schema.args[1]

        if not logic.is_term(schema) or not logic.is_list(schema.args[0]):
            raise RuntimeError("invalid verb schema")

        self.__mods = set(schema.args[0])
        self.__options = set()
        self.__fixed = set()
        self.__verbname = schema.pred
        self.__constraints = dict()
        self.__order = []
        self.__dis = schema.args[1] or id

        if self.__dis.startswith('global_'):
            self.__subject = None
        else:
            self.__subject = paths.id2server(id)

        for r in schema.args[2:]:
            if logic.is_pred(r,'role'):
                role = r.args[0]
                self.__fixed.add(role)
                self.__constraints[role] = r.args[1]
                self.__order.append(role)
            if logic.is_pred(r,'option'):
                role = r.args[0]
                self.__options.add(role)
                self.__constraints[role] = r.args[1]
                self.__order.append(role)

    def name(self):
        return self.__verbname

    def fullname(self):
        if self.__mods:
            return '%s %s' % (' '.join(self.__mods),self.__verbname)
        else:
            return self.__verbname

    def order(self):
        return self.__order

    def disambiguator(self):
        return self.__dis

    def constraints(self):
        return self.__constraints

    def mods(self):
        return self.__mods

    def fixed_roles(self):
        return self.__fixed

    def option_roles(self):
        return self.__options

    def subject(self):
        return self.__subject

    def __hash__(self):
        return hash((self.__id,self.__index))

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,VerbProxy): return 1
        return cmp((self.__id,self.__index),(other.__id,other.__index))

    def invoke(self, interp, *args):
        interpid = str(id(interp))
        return action.adapt_callbackn(rpc.invoke_rpc(self.__id,'vinvoke',action.marshal((interpid,self.__index)+args)))

    def defer(self, interp, *args):
        return action.adapt_callback1(rpc.invoke_rpc(self.__id,'vdefer',action.marshal((self.__index,)+args)))

    def find(self, interp, *args):
        return action.adapt_callback1(rpc.invoke_rpc(self.__id,'vfind',action.marshal((self.__index,)+args)))

    def cancel(self,interp,*args):
        return action.adapt_callback1(rpc.invoke_rpc(self.__id,'vcancel',action.marshal((self.__index,)+args)))

    def __str__(self):
        return "<verb %s:%s>" % (self.__id,self.__index)

    def __repr__(self):
        return "<verb %s:%s>" % (self.__id,self.__index)

    def id(self):
        return self.__id


class ModeProxy:
    def __init__(self,id,schema):
        self.__id = id
        self.__index = schema.args[0]

        schema = schema.args[1]

        if not logic.is_term(schema) or not logic.is_list(schema.args[0]):
            raise RuntimeError("invalid mode schema")

        self.__mods = set(schema.args[0])
        self.__options = set()
        self.__fixed = set()
        self.__constraints = dict()
        self.__order = []

        for r in schema.args[1:]:
            if logic.is_pred(r,'role'):
                role = r.args[0]
                self.__fixed.add(role)
                self.__constraints[role] = r.args[1]
                self.__order.append(role)
            if logic.is_pred(r,'option'):
                role = r.args[0]
                self.__options.add(role)
                self.__constraints[role] = r.args[1]
                self.__order.append(role)

    def order(self):
        return self.__order

    def constraints(self):
        return self.__constraints

    def mods(self):
        return self.__mods

    def fixed_roles(self):
        return self.__fixed

    def option_roles(self):
        return self.__options

    def subject(self):
        return paths.id2server(self.__id)

    def __hash__(self):
        return hash((self.__id,self.__index))

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,ModeProxy): return 1
        return cmp((self.__id,self.__index),(other.__id,other.__index))

    def invoke(self, interp, *args):
        return action.adapt_callback1(rpc.invoke_rpc(self.__id,'minvoke',action.marshal((self.__index,)+args)))

    def find(self, interp, *args):
        return action.adapt_callback1(rpc.invoke_rpc(self.__id,'mfind',action.marshal((self.__index,)+args)))

    def cancel(self,interp,*args):
        return action.adapt_callback0(rpc.invoke_rpc(self.__id,'mcancel',action.marshal((self.__index,)+args)))

    def attach(self,interp,*args):
        return action.adapt_callback0(rpc.invoke_rpc(self.__id,'mattach',action.marshal((self.__index,)+args)))

    def __repr__(self):
        return "<mode %s:%s>" % (self.__id,self.__index)

    def __str__(self):
        return "<mode %s:%s>" % (self.__id,self.__index)

    def id(self):
        return self.__id

class Relation:
    def __init__(self,r,o,c):
        self.r = r
        self.o = o
        self.c = c
    def __cmp__(self,other):
        return cmp((self.r.name,self.o,self.c),other)
    def __hash__(self):
        return hash((self.r.name,self.o,self.c))
    def __str__(self):
        return "%s(%s,%s)" % (self.r.name,self.o,self.c)
    def __repr__(self):
        return self.__str__()

class RelationCache:

    def __init__(self,name):
        self.name = name
        self.__rules = dict()
        self.__rights = dict()
        self.__lefts = dict()
        self.__trights = dict()
        self.__tlefts = dict()
        self.__erights = dict()
        self.__elefts = dict()

    @staticmethod
    def __add(d,k,o):
        d[k] = d.get(k,frozenset()).union((o,))

    @staticmethod
    def __discard(d,k,o):
        x = d.get(k,frozenset())
        x = x.difference((o,))

        if x:
            d[k]=x
        else:
            del d[k]

    def iterrules(self, filter=''):
        return iter(self.__rules)

    def assert_relation(self,rel):
        r = self.__rules.get(rel)
        if r is not None:
            r[0] = r[0]+1
            return rel

        self.__rules[rel] = [1]
        self.__add(self.__lefts,rel.c,rel.o)
        self.__add(self.__rights,rel.o,rel.c)

        dirty = set()
        dirty.update(self.__tlefts.get(rel.o,set()))
        dirty.update(self.__trights.get(rel.c,set()))
        dirty.add(rel.o)
        dirty.add(rel.c)
        self.__rebuild(dirty)

        return rel

    def retract_relation(self,rel):
        r = self.__rules.get(rel)
        r[0] = r[0]-1
        if r[0] > 0:
            return

        del self.__rules[rel]
        self.__discard(self.__rights,rel.o,rel.c)
        self.__discard(self.__lefts,rel.c,rel.o)

        dirty=set()
        dirty.update(self.__tlefts.get(rel.o,set()))
        dirty.update(self.__trights.get(rel.c,set()))
        dirty.add(rel.o)
        dirty.add(rel.c)
        self.__rebuild(dirty)

    def __rebuild(self,dirty):
        for o in dirty:
            self.__trights[o] = self.__rebuild1(self.__rights,o)
            self.__tlefts[o] = self.__rebuild1(self.__lefts,o)
            self.__erights[o] = self.__trights[o].union((o,))
            self.__elefts[o] = self.__tlefts[o].union((o,))

    def __rebuild1(self,rel,o):
        x = set()
        p = set((o,))

        while p:
            p1 = p.pop()
            n = rel.get(p1,set()).difference(x)
            x.update(n)
            p.update(n)

        return frozenset(x)

    def extended_relation(self,o,c):
        return c in self.__erights.get(o,frozenset([o]))

    def extended_rights(self,o):
        return self.__erights.get(o,frozenset([o]))

    def extended_lefts(self,c):
        return self.__elefts.get(c,frozenset([c]))

    def direct_relation(self,o,c):
        return c in self.__rights.get(o,frozenset())

    def direct_rights(self,o):
        return self.__rights.get(o,frozenset())

    def direct_lefts(self,c):
        return self.__lefts.get(c,frozenset())

    def relation(self,o,c):
        return c in self.__trights.get(o,frozenset())

    def rights(self,o):
        return self.__trights.get(o,frozenset())

    def lefts(self,c):
        return self.__tlefts.get(c,frozenset())

    def pred_relation_extended(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)
        left = term.args[0]
        right = term.args[1]

        if logic.is_bound(left):
            if logic.is_bound(right):
                return self.extended_relation(left,right)

            r = []
            for c in self.extended_rights(left):
                env_copy = env.copy()
                if logic.unify(c,env,right,env_copy):
                    r.append(env_copy)
            return r

        if logic.is_bound(right):
            r = []

            for c in self.extended_lefts(right):
                env_copy = env.copy()
                if logic.unify(c,env,left,env_copy):
                    r.append(env_copy)
            return r

        return False

    def pred_relation_direct(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)
        left = term.args[0]
        right = term.args[1]

        if logic.is_bound(left):
            if logic.is_bound(right):
                return self.direct_relation(left,right)

            r = []
            for c in self.direct_rights(left):
                env_copy = env.copy()
                if logic.unify(c,env,right,env_copy):
                    r.append(env_copy)
            return r

        if logic.is_bound(right):
            r = []
            for c in self.direct_lefts(right):
                env_copy = env.copy()
                if logic.unify(c,env,left,env_copy):
                    r.append(env_copy)
            return r

        return False

    def pred_relation(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)
        left = term.args[0]
        right = term.args[1]

        if logic.is_bound(left):
            if logic.is_bound(right):
                return self.relation(left,right)

            r = []
            for c in self.rights(left):
                env_copy = env.copy()
                if logic.unify(c,env,right,env_copy):
                    r.append(env_copy)
            return r

        if logic.is_bound(right):
            r = []
            for c in self.lefts(right):
                env_copy = env.copy()
                if logic.unify(c,env,left,env_copy):
                    r.append(env_copy)
            return r

        return False


class ConnectionCache:
    def __init__(self,db):
        self.__master = {} # [os,obs,slaves,rules]
        self.__db = db
        self.__cn = {}
        self.__mss = {} # { mss -> { sss -> [ mid,sid] } }
        self.__sss = {} # { sss -> { mss -> [ mid,sid] } }
        self.__inputs = {} # { sid -> { mid -> [inputs] } }

    def __rebuild(self,mid,m):
        rules = m[3]
        for r in rules:
            self.__db.retract_relation(r)

        rules = []
        midss = paths.id2server(mid)

        if m[0]:
            for s in m[2]:
                sidss = paths.id2server(s)
                rules.append(Relation(self.__db,sidss,midss))

        if m[1]:
            for s in m[2]:
                sidss = paths.id2server(s)
                rules.append(Relation(self.__db,midss,sidss))

        m[3] = rules
        for r in rules:
            self.__db.assert_relation(r)

    def get_masters(self,sid):
        ml = self.__cn.get(sid)
        if ml is None:
            return ()
        return tuple(ml)

    def get_slaves(self,mid):
        m = self.__master.get(mid)
        if m is None:
            return set()
        return m[2]

    def add_master(self,mid,os,obs):
        m = self.__master.get(mid)

        if m is None:
            m = [os,obs,set(),[]]
            self.__master[mid]=m
            return

        m[0]=os
        m[1]=obs

        self.__rebuild(mid,m)

    def remove_master(self,mid):
        m = self.__master.get(mid)
        if m is not None:
            m[0]=False
            m[1]=False
            self.__rebuild(mid,m)
            if not m[2]:
                del self.__master[mid]

    def get_inputs(self,sid,mid):
        ssd = self.__inputs.get(sid)
        if ssd:
            msd = ssd.get(mid)
            if msd:
                return msd
        return set()

    def connect(self,sid,sss,mids):
        wanted_mids = set([id for (id,ss,inp) in mids])
        old_mids = self.__cn.get(sid,set())
        self.__cn[sid] = wanted_mids
        unwanted_mids = old_mids.difference(wanted_mids)
        new_mids = wanted_mids.difference(old_mids)

        if sid in self.__inputs:
            del self.__inputs[sid]

        if mids:
            self.__inputs[sid] = {}
            for (id,ss,inp) in mids:
                self.__inputs[sid].setdefault(id,set()).add(inp)

        for mid in unwanted_mids:
            m = self.__master.get(mid)
            m[2].discard(sid)
            self.__rebuild(mid,m)
            if not m[2] and not m[0] and not m[1]:
                del self.__master[mid]

        for mid in new_mids:
            m = self.__master.get(mid)
            if m is None:
                m = [False,False,set(),[]]
                self.__master[mid]=m
                m[2].add(sid)
            else:
                m[2].add(sid)
                self.__rebuild(mid,m)

        sssdict1 = self.__sss.setdefault(sss,dict())

        for mid in old_mids:
            mss = paths.id2server(mid)
            mssdict1 = self.__mss.setdefault(mss,dict())
            mssdict2 = mssdict1.setdefault(sss,dict())
            sssdict2 = sssdict1.setdefault(mss,dict())
            try: del mssdict2[sid]
            except KeyError: pass
            try: del sssdict2[sid]
            except KeyError: pass

        for (mid,mss,inp) in mids:
            mssdict1 = self.__mss.setdefault(mss,dict())
            mssdict2 = mssdict1.setdefault(sss,dict())
            sssdict2 = sssdict1.setdefault(mss,dict())
            mssdict2[sid] = wanted_mids
            sssdict2[sid] = wanted_mids


    def dump_subsys(self):
        for (sss,ssd) in self.__sss.items():
            print 'slave',sss
            for (mss,cd) in ssd.items():
                print '-master',mss
                for (sid,mids) in cd.items():
                    for mid in mids:
                        print '--',sid,'<-',mid

        for (mss,msd) in self.__mss.items():
            print 'master',mss
            for (sss,cd) in msd.items():
                print '-slave',sss
                for (sid,mids) in cd.items():
                    for mid in mids:
                        print '--',sid,'<-',mid

    def get_subsys_masters(self,ss):
        msd = self.__mss.get(ss)

        if not msd:
            return {}

        cn={}

        for (sss,d) in msd.iteritems():
            for (sid,mids) in d.iteritems():
                cn.setdefault(sss,[]).extend([(mid,sid) for mid in mids])

        return cn

    def get_subsys_slaves(self,ss):
        ssd = self.__sss.get(ss)

        if not ssd:
            return {}

        cn={}

        for (mss,d) in ssd.iteritems():
            for (sid,mids) in d.iteritems():
                cn.setdefault(mss,[]).extend([(mid,sid) for mid in mids])

        return cn

    def pred_ssmaster(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)

        slave = term.args[0]
        master = term.args[1]

        if logic.is_bound(slave):
            m = self.__sss.get(slave,{})
            if logic.is_bound(master):
                return master in m

            r = []
            for mm in m:
                env_copy = env.copy()
                if logic.unify(mm,env,master,env_copy):
                    r.append(env_copy)

            return r

        if logic.is_bound(master):
            m = self.__mss.get(master,{})
            if m is None:
                return False

            r = []
            for s in m:
                env_copy = env.copy()
                if logic.unify(s,env,slave,env_copy):
                    r.append(env_copy)
            return r

        return False

    def pred_master(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)

        slave = term.args[0]
        master = term.args[1]

        if logic.is_bound(slave):
            m = self.__cn.get(slave)
            if logic.is_bound(master):
                return master in m

            r = []
            for mm in m:
                env_copy = env.copy()
                if logic.unify(mm,env,master,env_copy):
                    r.append(env_copy)

            return r

        if logic.is_bound(master):
            m = self.__master.get(master)
            if m is None:
                return False

            r = []
            for s in m[2]:
                env_copy = env.copy()
                if logic.unify(s,env,slave,env_copy):
                    r.append(env_copy)
            return r

        return False

class PropertyCache:
 
    def __init__(self):
        self.__id2value = dict() # id -> frozenset(values)
        self.__value2id = dict() # value -> frozenset(ids)

    def iterrules(self,pred):
        for (id,props) in self.__id2value.iteritems():
            yield "%s(%s,[%s])" % (pred,id,','.join(props))
        for (prop,ids) in self.__value2id.iteritems():
            yield "%s_reverse(%s,[%s])" % (pred,prop,','.join(ids))

    def set_id(self,id,values):
        t = self.__id2value.get(id)

        if t is not None:
            self.remove_id(id)

        if values:
            self.__id2value[id]=frozenset(values)
            vs = frozenset((id,))

            for v in values:
                self.__value2id[v] = self.__value2id.get(v,frozenset()).union(vs)

    def remove_id(self,id):
        t = self.__id2value.get(id)

        if t is None:
            return

        del self.__id2value[id]

        vs = frozenset((id,))
        for v in t:
            np = self.__value2id.get(v,frozenset()).difference(vs)
            if np:
                self.__value2id[v] = np
            else:
                del self.__value2id[v]

    def change_id(self,id,add,remove):
        add = frozenset(add)
        remove = frozenset(remove)
        vs = frozenset((id,))

        t = self.__id2value.get(id)

        if t is None:
            if not add:
                return
            t = frozenset()

        n = t.union(add).difference(remove)

        for v in n.difference(t):
            self.__value2id[v] = self.__value2id.get(v,frozenset()).union(vs)

        for v in t.difference(n):
            np = self.__value2id.get(v,frozenset()).difference(vs)
            if np:
                self.__value2id[v] = np
            else:
                del self.__value2id[v]

        if n:
            self.__id2value[id] = n
        else:
            try: del self.__id2value[id]
            except: pass

    def get_valueset(self,id):
        return self.__id2value.get(id,frozenset())

    def get_idset(self,value):
        return self.__value2id.get(value,frozenset())

    def pred_property(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)

        value = term.args[0]
        id = term.args[1]

        if logic.is_bound(id):
            vs = self.__id2value.get(id,frozenset())
            if logic.is_bound(value):
                return value in vs

            r = []
            for v in vs:
                env_copy = env.copy()
                if logic.unify(v,env,value,env_copy):
                    r.append(env_copy)

            return r

        if logic.is_bound(value):
            ids = self.__value2id.get(value,frozenset())

            r = []
            for i in ids:
                env_copy = env.copy()
                if logic.unify(i,env,id,env_copy):
                    r.append(env_copy)

            return r

        return False
        
    def pred_filter(self,engine,term,env):
        if term.arity != 3: return False
        list_in = term.args[0]
        filter = term.args[1]
        list_out = term.args[2]

        if not logic.is_bound(list_in) or not logic.is_bound(filter): return False
        if type(list_in) != frozenset: return False

        fs = self.__value2id.get(filter,frozenset()).intersection(list_in)

        if logic.is_bound(list_out):
            return list_in==list_out

        return logic.unify(fs,{},list_out,env)

    def pred_fetch(self,engine,term,env):
        if term.arity != 2: return False
        filter = term.args[0]
        list_out = term.args[1]

        if not logic.is_bound(filter): return False

        fs = self.__value2id.get(filter,frozenset())

        if logic.is_bound(list_out):
            return list_in==list_out

        return logic.unify(fs,{},list_out,env)

    def pred_fetchid(self,engine,term,env):
        if term.arity != 2: return False
        filter = term.args[0]
        list_out = term.args[1]

        if not logic.is_bound(filter): return False

        fs = self.__id2value.get(filter,frozenset())

        if logic.is_bound(list_out):
            return list_in==list_out

        return logic.unify(fs,{},list_out,env)

class DatabaseProxy(proxy.AtomProxy):

    def __init__(self,database,parent = None):
        proxy.AtomProxy.__init__(self)

        self.database=database
        self.parent=parent
        self.rules={}

        self.__root = parent.root() if parent else self
        self.__changed = False
        self.__timestamp = 0
        
    def set_timestamp(self,ts):
        self.__timestamp = ts

    def get_timestamp(self):
        return self.__timestamp

    def parent_id(self):
        return self.parent.id() if self.parent else None

    def root(self):
        return self.__root

    def client_sync(self):
        proxy.AtomProxy.client_sync(self)
        if self.__changed:
            self.__changed = False
            utils.safe(self.database.subsys_sync,self)

    def node_added(self,index):
        return self.__class__(self.database,self)

    def node_ready(self):
        self.__root.__changed = True

        self.rules,props,modes,verbs = self.database.make_rules(self,True,self.monitor)

        myid = self.id()

        if modes is not None:
            self.database.get_verbcache().set_modes(myid,modes)

        if verbs is not None:
            self.database.get_verbcache().set_verbs(myid,verbs)

        for v in self.rules.itervalues():
            self.database.assert_rules(v)

        for c,(a,r) in props.iteritems():
            self.database.get_propcache(c).set_id(myid,a);

        utils.safe(self.database.object_added,self)

        #print 'node_ready',self.id(),'protocols=',self.database.get_propcache('protocol').get_valueset(self.id())
        if 'nostage' not in self.database.get_propcache('protocol').get_valueset(self.id()):
            self.__root.__timestamp = piw.tsd_time()
            self.database.set_timestamp(self.__root.__timestamp)


    def node_changed(self,parts):
        self.__root.__changed = True
        
        #print 'node_changed protocols=',self.database.get_propcache('protocol').get_valueset(self.id())
        if 'nostage' not in self.database.get_propcache('protocol').get_valueset(self.id()):
            self.__root.__timestamp = piw.tsd_time()
            self.database.set_timestamp(self.__root.__timestamp)
        
        newrules,newprops,newmodes,newverbs = self.database.make_rules(self,False,parts)

        myid = self.id()
        
        if newmodes is not None:
            self.database.get_verbcache().set_modes(myid,newmodes)

        if newverbs is not None:
            self.database.get_verbcache().set_verbs(myid,newverbs)

        oldrules = self.rules
        for (k,v) in newrules.iteritems():
            o = oldrules.get(k)
            if o is not None:
                self.database.retract_rules(o)
            self.database.assert_rules(v)
        self.rules.update(newrules)

        for c,(a,r) in newprops.iteritems():
            if r is None:
                self.database.get_propcache(c).set_id(myid,a);
            else:
                self.database.get_propcache(c).change_id(myid,a,r);

        utils.safe(self.database.object_changed,self,parts)

    def node_removed(self):
        self.__root.__changed = True

        #print 'node_removed protocols=',self.database.get_propcache('protocol').get_valueset(self.id())
        if 'nostage' not in self.database.get_propcache('protocol').get_valueset(self.id()):
            self.__root.__timestamp = piw.tsd_time()
            self.database.set_timestamp(self.__root.__timestamp)
        
        #if self.__root==self:
        #    print 'node_removed ',self.id()


        myid = self.id()

        self.database.get_verbcache().retract_verbs(myid)
        self.database.get_verbcache().retract_modes(myid)

        for v in self.rules.itervalues():
            self.database.retract_rules(v)

        for c in self.database.get_propcaches():
            self.database.get_propcache(c).remove_id(myid)

        utils.safe(self.database.object_removed,self)

class VerbCache:

    def __init__(self):
        self.__id2verbs = dict() # id -> list of VerbProxy objects
        self.__subject2verbs = dict() # id -> set of VerbProxy objects
        self.__name2verbs = dict() # name -> set of VerbProxy objects
        self.__id2modes = dict() # id -> list of ModeProxy objects
        self.__mod2modes = dict()
        self.__role2modes = dict()
        self.__modes = set()

    def find_verbs_by_name(self,verbname):
        return self.__name2verbs.get(verbname,frozenset())

    def find_verbs_by_subject(self,id):
        return self.__subject2verbs.get(id,frozenset())

    def find_all_modes(self):
        return self.__modes

    def find_modes_by_role(self,rolename):
        return self.__role2modes.get(rolename,frozenset())

    def find_modes_by_mod(self,modname):
        return self.__mod2modes.get(modname,frozenset())

    def set_modes(self,id,modes):
        self.retract_modes(id)

        if modes:
            self.__id2modes[id] = modes

            for m in modes:
                self.__modes.add(m)
                for mm in m.mods():
                    self.__mod2modes.setdefault(mm,set()).add(m)
                for mm in m.fixed_roles():
                    self.__role2modes.setdefault(mm,set()).add(m)
                for mm in m.option_roles():
                    self.__role2modes.setdefault(mm,set()).add(m)

    def retract_modes(self,id):
        modes = self.__id2modes.get(id)

        if modes is not None:
            del self.__id2modes[id]

            for m in modes:
                self.__modes.discard(m)
                for mm in m.mods():
                    ms = self.__mod2modes[mm]
                    ms.discard(m)
                    if not ms: del self.__mod2modes[mm]
                for mm in m.fixed_roles():
                    ms = self.__role2modes[mm]
                    ms.discard(m)
                    if not ms: del self.__role2modes[mm]
                for mm in m.option_roles():
                    ms = self.__role2modes[mm]
                    ms.discard(m)
                    if not ms: del self.__role2modes[mm]

    def set_verbs(self,id,verbs):
        self.retract_verbs(id)

        if verbs:
            self.__id2verbs[id] = verbs

            for v in verbs:
                self.__name2verbs.setdefault(v.name(),set()).add(v)
                self.__subject2verbs.setdefault(v.subject(),set()).add(v)


    def retract_verbs(self,id):
        verbs = self.__id2verbs.get(id)

        if verbs is not None:
            del self.__id2verbs[id]

            for v in verbs:
                vs = self.__name2verbs[v.name()]
                vs.discard(v)
                if not vs: del self.__name2verbs[v.name()]

                vs = self.__subject2verbs[v.subject()]
                vs.discard(v)
                if not vs: del self.__subject2verbs[v.subject()]


class Database(logic.Engine):

    proxy = DatabaseProxy

    def __init__(self):
        logic.Engine.__init__(self)
        self.__partof = RelationCache('partof')
        self.__assocwith = RelationCache('assocwith')
        self.__jointo = RelationCache('jointo')
        self.__index = None
        self.__jointo_testc = logic.parse_clause('join(cnc(A),role(to,[instance(B)]))')
        self.__jointo_testv = logic.parse_clause('join(virtual(A),role(to,[instance(B)]))')
        self.__ccache = ConnectionCache(self.__assocwith)
        self.__propcache = PropertyCache()
        self.__helpcache = PropertyCache()
        self.__namecache = PropertyCache()
        self.__ordcache = PropertyCache()
        self.__procache = PropertyCache()
        self.__protocache = PropertyCache()
        self.__idealcache = PropertyCache()
        self.__bindcache = PropertyCache()
        self.__desccache = PropertyCache()
        self.__canonical = PropertyCache()
        self.__verbcache = VerbCache()

        #print 'database timestamp = 1'
        self.__timestamp = 1

        self.parse_rules(rules_database)
        self.add_module(constraints)

        self.__properties = { 'props': self.__propcache,
                              'name': self.__namecache,
                              'ordinal': self.__ordcache,
                              'pronoun': self.__procache,
                              'protocol': self.__protocache,
                              'ideal': self.__idealcache,
                              'bind': self.__bindcache,
                              'desc': self.__desccache,
                              'canonical': self.__canonical,
                              'help': self.__helpcache,
                            }

    def set_timestamp(self,ts):
        if ts > self.__timestamp:
            self.__timestamp = ts
            #print 'database timestamp updated to',ts
            
            
    def get_timestamp(self):
        return self.__timestamp

    def update_all_agents(self):
        new_timstamp = piw.tsd_time()
        self.__timestamp = new_timstamp
        for ap in self.__index.members():
            ap.set_timestamp(new_timstamp)
        
    def changed_agents(self,since_time):
        if since_time<self.__timestamp and self.__index is not None:
            agents = []
            for ap in self.__index.members():
                ats = ap.get_timestamp()
                if ats > since_time:
                    #print 'change: database time=',ats,' > stage server time=',since_time
                    #master = self.find_joined_master(ap.id)
                    #print ap.id(), 'master=',master,'slaves=',self.find_joined_slaves(ap.id)
                    agents += [ap.id()]
                else:
                    agent_slaves = self.find_joined_slaves(ap.id())
                    # if any subsystem has updated then add the agent to the change list
                    for slave in agent_slaves:
                        item = self.find_item(slave)
                        if item is not None:
                            sts = item.get_timestamp()
                            #print slave,sts,since_time
                            if sts > since_time:
                                agents += [ap.id()]
                                break
            return agents
        else:
            return []
                
                    
    def num_agents(self):
        if self.__index is not None:
            agents = [id for id in self.__index.members()]
            print 'num agents=',len(agents)
        
    def get_propcaches(self):
        return self.__properties.iterkeys()

    def get_partcache(self):
        return self.__partof

    def get_joincache(self):
        return self.__jointo

    def get_assoccache(self):
        return self.__assocwith

    def get_propcache(self,name):
        return self.__properties[name]

    def get_verbcache(self):
        return self.__verbcache

    def get_subsys_slaves(self,ss):
        return self.__ccache.get_subsys_slaves(ss)

    def get_subsys_masters(self,ss):
        return self.__ccache.get_subsys_masters(ss)

    def iterrules(self,filter=''):
        for r in logic.Engine.iterrules(self,filter):
            yield r

        for r in self.__partof.iterrules(filter):
            yield r

        for r in self.__jointo.iterrules(filter):
            yield r

        for r in self.__assocwith.iterrules(filter):
            yield r

        for (n,c) in self.__properties.iteritems():
            for r in c.iterrules('@'+n):
                yield r

    def assert_rule(self, rule):
        if isinstance(rule,Relation):
            rule.r.assert_relation(rule)
            return
        logic.Engine.assert_rule(self,rule)

    def retract_rule(self, rule):
        if isinstance(rule,Relation):
            rule.r.retract_relation(rule)
            return
        logic.Engine.retract_rule(self,rule)

    def pred_jointo_extended(self,engine,term,env):
        return self.__jointo.pred_relation_extended(engine,term,env)

    def pred_jointo_direct(self,engine,term,env):
        return self.__jointo.pred_relation_direct(engine,term,env)

    def pred_jointo(self,engine,term,env):
        return self.__jointo.pred_relation(engine,term,env)

    def pred_assocwith_extended(self,engine,term,env):
        return self.__assocwith.pred_relation_extended(engine,term,env)

    def pred_assocwith_direct(self,engine,term,env):
        return self.__assocwith.pred_relation_direct(engine,term,env)

    def pred_assocwith(self,engine,term,env):
        return self.__assocwith.pred_relation(engine,term,env)

    def pred_partof_extended(self,engine,term,env):
        return self.__partof.pred_relation_extended(engine,term,env)

    def pred_partof_direct(self,engine,term,env):
        return self.__partof.pred_relation_direct(engine,term,env)

    def pred_partof(self,engine,term,env):
        return self.__partof.pred_relation(engine,term,env)

    def pred_ssmaster(self,engine,term,env):
        return self.__ccache.pred_ssmaster(engine,term,env)

    def pred_master(self,engine,term,env):
        return self.__ccache.pred_master(engine,term,env)

    def pred_name_filter(self,engine,term,env): return self.__namecache.pred_property_filter(engine,term,env)
    def pred_pronoun_filter(self,engine,term,env): return self.__procache.pred_property_filter(engine,term,env)
    def pred_ordinal_filter(self,engine,term,env): return self.__ordcache.pred_property_filter(engine,term,env)
    def pred_protocol_filter(self,engine,term,env): return self.__protocache.pred_property_filter(engine,term,env)
    def pred_bind_filter(self,engine,term,env): return self.__bindcache.pred_property_filter(engine,term,env)
    def pred_ideal_filter(self,engine,term,env): return self.__idealcache.pred_property_filter(engine,term,env)
    def pred_property_filter(self,engine,term,env): return self.__propcache.pred_property_filter(engine,term,env)

    def pred_name_fetch(self,engine,term,env): return self.__namecache.pred_property_fetch(engine,term,env)
    def pred_pronoun_fetch(self,engine,term,env): return self.__procache.pred_property_fetch(engine,term,env)
    def pred_ordinal_fetch(self,engine,term,env): return self.__ordcache.pred_property_fetch(engine,term,env)
    def pred_protocol_fetch(self,engine,term,env): return self.__protocache.pred_property_fetch(engine,term,env)
    def pred_bind_fetch(self,engine,term,env): return self.__bindcache.pred_property_fetch(engine,term,env)
    def pred_ideal_fetch(self,engine,term,env): return self.__idealcache.pred_property_fetch(engine,term,env)
    def pred_property_fetch(self,engine,term,env): return self.__propcache.pred_property_fetch(engine,term,env)

    def pred_name(self,engine,term,env): return self.__namecache.pred_property(engine,term,env)
    def pred_pronoun(self,engine,term,env): return self.__procache.pred_property(engine,term,env)
    def pred_ordinal(self,engine,term,env): return self.__ordcache.pred_property(engine,term,env)
    def pred_protocol(self,engine,term,env): return self.__protocache.pred_property(engine,term,env)
    def pred_bind(self,engine,term,env): return self.__bindcache.pred_property(engine,term,env)
    def pred_ideal(self,engine,term,env): return self.__idealcache.pred_property(engine,term,env)
    def pred_property(self,engine,term,env): return self.__propcache.pred_property(engine,term,env)

    def pred_subsystem(self,engine,term,env):
        if term.arity != 2: return False
        term = logic.expand(term,env)

        id = term.args[0]
        ss = term.args[1]

        if not logic.is_bound(id):
            return False

        ssid = paths.id2server(id)
        if logic.is_bound(ss):
            return ss==ssid

        return logic.unify(ssid,env,ss,env)

    def add_lexicon(self,lex):
        rules = []

        for (e,(m,c)) in lex.iteritems():
            rules.append(R(T('db_classification',e,c)))
            if m is not None:
                rules.append(R(T('db_translation',e,'!'+m)))

        self.assert_rules(rules)

    def make_relation_rule(self,id,rlist):
        rules = []

        for rel in rlist:
            e={}
            if logic.match(rel,self.__jointo_testc,e) or logic.match(rel,self.__jointo_testv,e):
                rules.append(Relation(self.__assocwith,e['A'],e['B']))
                rules.append(Relation(self.__jointo,e['A'],e['B']))
            else:
                verb = rel.pred
                subject = rel.args[0]
                objects = rel.args[1:]
                rules.append(R(T('db_relation',verb,subject,objects)))

        return rules

    def make_verb_proxy(self,id,schema):
        try:
            return VerbProxy(id,schema)
        except:
            print 'malformed verb',id,schema
            return None

    def make_mode_proxy(self,id,schema):
        try:
            return ModeProxy(id,schema)
        except:
            print 'malformed mode',id,schema
            return None

    @staticmethod
    def __master2id(master):
        terms = logic.parse_termlist(master or '')
        ids = []

        for t in terms:
            if logic.is_pred_arity(t,'conn',4,4):
                ids.append((t.args[2],t.args[0]))

        return ids

    def make_rules(self,ap,init,parts):
        rules = {}
        props = {}

        id = ap.id()
        pid = ap.parent_id()
        desc = False
        cdesc = False
        ss = paths.id2server(id)

        prop_add = []
        prop_del = []

        if init:
            r=[]

            r.append(R(T('db_item',id,ap)))

            if pid:
                r.append(Relation(self.__partof,id,pid))

            rules['init']=r

        mode_return = None
        if 'modes' in parts:
            mode_return = []
            for s in ap.modes():
                p = self.make_mode_proxy(id,s)
                if p is not None:
                    mode_return.append(p)

        verb_return = None
        if 'verbs' in parts:
            verb_return = []
            for s in ap.verbs():
                p = self.make_verb_proxy(id,s)
                if p is not None:
                    verb_return.append(p)

        if 'master' in parts:
            r=[]
            master = ap.get_master()
            if master:
                mids = self.__master2id(master)
                midmap = []
                for mid,inp in mids:
                    p=ap.protocols()
                    mss = paths.id2server(mid)
                    midmap.append((mid,mss,inp))
                    if 'obm' in p:
                        r.append(Relation(self.__assocwith,ss,mss))
                    if 'om' in p:
                        r.append(Relation(self.__assocwith,mss,ss))
                self.__ccache.connect(id,ss,midmap)
            else:
                self.__ccache.connect(id,ss,[])

            rules['master'] = r

        if 'name' in parts or 'protocols' in parts:
            names = ap.names()
            p=ap.protocols()

            if not pid and 'notagent' not in p:
                names = names+['agent']
                prop_add.append('agent')
            else:
                prop_del.append('agent')

            if names:
                if 'virtual' in p:
                    prop_add.append('virtual')
                    prop_del.append('concrete')
                else:
                    prop_del.append('virtual')
                    prop_add.append('concrete')

                prop_del.append('anon')
            else:
                prop_add.append('anon')
                prop_del.append('virtual')
                prop_del.append('concrete')

            props['name'] = (names,None)
            props['protocol'] = (p,None)

            if 'bind' in p:
                props['bind'] = ((ss,),None)
            else:
                props['bind'] = ((),None)

            desc = True

        if 'cname' in parts or 'cordinal' in parts:
            cdesc = True

        if 'ideals' in parts:
            props['ideal'] = (ap.ideals(),None)

        if  'domain' in parts:
            r=[]
            control = ap.domain().hint('control')
            if control is not None:
                r.append(R(T('db_control',control[0],id)))
            rules['domain'] = r

        if 'ordinal' in parts:
            ordinal = ap.ordinal()
            if ordinal is not None:
                props['ordinal'] = ((str(ordinal),),None)
            else:
                props['ordinal'] = ((),None)
            desc = True

        if 'pronoun' in parts:
            pronoun = ap.pronoun()
            if pronoun:
                props['pronoun'] = ((pronoun,),None)
            else:
                props['pronoun'] = ((),None)
            desc = True

        if 'frelation' in parts:
            r=[]
            frelations = ap.frelations()
            if frelations:
                r.extend(self.make_relation_rule(id,frelations))
            rules['frelation'] = r

        if 'relation' in parts:
            r=[]
            relations = ap.relations()
            if relations:
                r.extend(self.make_relation_rule(id,relations))
            rules['relation'] = r

        if cdesc:
            d=[]
            d.extend(ap.cnames())
            ordinal = ap.cordinal()
            if ordinal: d.append(str(ordinal))
            if d:
                props['canonical'] = ((' '.join(d),),None)
            else:
                props['canonical'] = (('',),None)

        if desc:
            d=[]
            d.extend(ap.names())
            ordinal = ap.ordinal()
            if ordinal: d.append(str(ordinal))
            if d:
                props['desc'] = ((' '.join(d),),None)
            else:
                props['desc'] = (('',),None)

        if 'help' in parts:
            r=[]
            hlp = ap.help_text()
            if hlp:
                props['help'] = ((hlp,),None)
            else:
                props['help'] = ((),None)

        props['props'] = (prop_add,prop_del)

        return rules,props,mode_return,verb_return

    def start(self,name):
        if not self.__index:
            self.__index = index.Index(lambda name: self.proxy(self),False)
            piw.tsd_index(name,self.__index)

    def stop(self):
        if self.__index:
            self.__index.close_index()
            self.__index=None

    def sync(self, *args):
        if self.__index:
            return self.__index.sync(*args)
        return async.success()

    def classify(self,word):
        result = self.search_any(T('classify',word,V('C'),V('X')))
        if result is not None:
            return (result['C'], result['X'])
        return None

    def find_joined_master(self,item):
        return self.__jointo.direct_rights(item)

    def find_joined_slaves(self,item):
        return self.__jointo.direct_lefts(item)

    def find_masters(self,item):
        return self.__ccache.get_masters(item)

    def get_inputs(self,slave_id,master_id):
        return self.__ccache.get_inputs(slave_id,master_id)

    def find_slaves(self,item):
        return self.__ccache.get_slaves(item)

    def find_descendants(self,item):
        return self.__partof.lefts(item)

    def find_children(self,item):
        return self.__partof.direct_lefts(item)

    def find_parents(self,item):
        return self.__partof.direct_rights(item)

    def find_by_name(self, name):
        item = self.search_any_key('P',T('@name',name,V('P')))
        return item

    def find_item(self, item):
        proxy = self.search_any_key('P',T('db_item',item,V('P')))
        return proxy

    def find_help(self, pid):
        return ' '.join(self.__helpcache.get_valueset(pid))

    def find_full_canonical(self, pid, as_string=True):
        aid = paths.id2server(pid)
        if aid==pid:
            adesc = self.find_canonical(pid)
            sdesc = []
            pdesc = []
        else:
            adesc = self.find_canonical(aid)
            sdesc = []
            pdesc = self.find_canonical_path(pid,aid)

        if 'agent' not in self.__propcache.get_valueset(aid):
            mid = self.find_joined_master(aid)
            if mid:
                sdesc = adesc
                adesc = self.find_full_canonical(popset(mid),False)

        full = adesc
        full.extend(sdesc)
        full.extend(pdesc)

        if as_string:
            return ' '.join(full)
        else:
            return full

    def find_canonical_path(self, pid, aid):
        path = []

        while pid is not None and pid != aid:
            path = self.find_canonical(pid) + path
            pid = popset(self.find_parents(pid))

        return path

    def find_canonical(self, id):
        desc = popset(self.__canonical.get_valueset(id))
        return [desc] if desc else []

    def find_full_desc(self, id):
        aid = paths.id2server(id)
        if aid==id:
            adesc = self.find_desc(id) or id
            sdesc = ''
            pdesc = ''
        else:
            adesc = self.find_desc(aid) or aid
            sdesc = ''
            pdesc = self.find_desc(id) or id

        if 'agent' not in self.__propcache.get_valueset(aid):
            mid = self.find_joined_master(aid)
            if mid:
                sdesc = adesc
                adesc = self.find_full_desc(set(mid).pop())

        return ' '.join((adesc,sdesc,pdesc)).replace('  ',' ')

    def find_desc(self, id):
        desc = ' '.join(self.__desccache.get_valueset(id))
        return desc or ''

    def translate_to_english(self,word):
        if not word.startswith('!'):
            return word

        result = self.search_any_key('C',T('translate',V('C'),word))
        return result

    def __isnumber(self,word):
        for w in word:
            if not w.isdigit() and w!='.':
                return False
        return True

    def translate_to_music2(self,word):
        if word.startswith('!'):
            return [word]

        if not self.__isnumber(word):
            return [ self.search_any_key('C',T('translate',word,V('C'))) or word ]

        return [ self.search_any_key('C',T('translate',w,V('C'))) for w in word ]

    def translate_to_music(self,word):
        if word.startswith('!'):
            return word

        result = self.search_any_key('C',T('translate',word,V('C')))
        return result or word

    def subsys_sync(self,proxy):
        pass

    def object_added(self,proxy):
        p = proxy.protocols()
        obsflag = 'obs' in p
        osflag = 'os' in p
        if obsflag or osflag:
            self.__ccache.add_master(proxy.id(),osflag,obsflag)

    def object_removed(self,proxy):
        id=proxy.id()
        self.__ccache.remove_master(id)
        self.__ccache.connect(id,paths.id2server(id),[])

    def object_changed(self,proxy,parts):
        pass
