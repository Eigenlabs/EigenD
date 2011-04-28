
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
from pi import action,logic,async,domain,agent
from plg_language import interpreter, interpreter_version as version

rules_conn = """
    connect_contains(A,A).
    connect_contains(A,B) :- @partof(B,A), @not(@protocol(explicit,B)).
    connect_contains(A,B) :- @assocwith_direct(B,A), @not(@protocol(explicit,B)).

    connect_isinput(O) :- @name(input,O).
    connect_isinput(O) :- @protocol(input,O).
    connect_isoutput(O) :- @name(output,O).
    connect_isoutput(O) :- @protocol(output,O).

    connect_inputs(W,OL)  :- @is(OL,$alluniq(O,connect_contains(W,O),connect_isinput(O))).
    connect_outputs(W,OL) :- @is(OL,$alluniq(O,connect_contains(W,O),connect_isoutput(O))).

    connect_revoutputs(T,OL) :- @is(OL, $alluniq(O,connect_contains(T,O),connect_isoutput(O),@protocol(revconnect,O))).
    connect_revinputs(F,OL) :- @is(OL, $alluniq(O,connect_contains(F,O),connect_isinput(O),@protocol(revconnect,O))).

    connect_tosplit(T,OOUT,OIN) :- connect_inputs(T,OIN), connect_revoutputs(T,OOUT).
    connect_fromsplit(F,OOUT,OIN) :- connect_outputs(F,OOUT), connect_revinputs(F,OIN).

    find_connections(F,T,TL) :- @is(TL,$alluniq(O,connect_contains(T,O),@master(O,OM),connect_contains(F,OM))).

    dump_contains(O,P) :- @assocwith_extended(PP,O), @partof(P,PP).
    dump_connections0(P,T,P) :- @master(P,T).
    dump_connections0(P,P,T) :- @master(T,P).
    dump_connections(O,F,T) :- dump_contains(O,P),dump_connections0(P,F,T).
    dump_connections(O,F,T) :- dump_connections0(O,F,T).

    unconnect_candidate(T,[O,OM]) :- connect_contains(T,O),connect_isinput(O),@master(O,OM).
    unconnect_candidate(T,[OM,O]) :- connect_contains(T,O),connect_isoutput(O),@protocol(revconnect,O),@master(OM,O).
    unconnect_list(T,L) :- @is(L,$alluniq(O,unconnect_candidate(T,O))).

    input_list_candidate(T,O) :- connect_contains(T,O),connect_isinput(O).
    input_list_candidate(T,O) :- connect_contains(T,O),connect_isoutput(O),@protocol(revconnect,O).
    input_list(T,L) :- @is(L,$alluniq([O,None],input_list_candidate(T,O))).

    unconnect_from_candidate(T,F,[O,OM]) :- connect_contains(T,O),connect_isinput(O),@master(O,OM), connect_contains(F,OM).
    unconnect_from_candidate(T,F,[OM,O]) :- connect_contains(T,O),connect_isoutput(O),@protocol(revconnect,O),@master(OM,O), connect_contains(F,OM).
    unconnect_from_list(T,F,L) :- @is(L,$alluniq(O,unconnect_from_candidate(T,F,O))).
"""

class Stuff:
    def __init__(self,id,words,ordinal=None,domain=None,channel=None,**extra):
        self.id=id
        self.words=words
        self.ordinal=ordinal
        self.domain=domain or domain.Null()
        self.channel=channel
        self.extra=extra

    def get(self,key,val=None):
        return self.extra.get(key,val)

    def target(self,src,u):
        return logic.render_term(logic.make_term('conn',u,self.channel,src.id,src.channel))

    def __repr__(self):
        return '<%s %s %s>' % (self.id,self.channel,self.words)

    def __str__(self):
        return repr(self)

class WordTable(dict):

    def __init__(self,*words):
        dict.__init__(self)
        self.add(*words)

    def add(self,*words):
        for w in words:
            w,s = self.__score(w)
            self[w] = self.setdefault(w,0) + s

    def __score(self,w):
        mw,ml = self.__countstart(w,'+')
        lw,ll = self.__countstart(w,'-')

        if ml:
            return mw,ml

        if ll:
            return lw,-ll

        return w,1

    def __countstart(self,w,c):
        s = w.lstrip(c)
        return s,len(w)-len(s)


def find_conn(aproxy,id):
    cnxs = logic.parse_clauselist(aproxy.get_master())
    r = []
    for cnx in cnxs:
        if logic.is_pred_arity(cnx,'conn',4) and cnx.args[2]==id:
            r.append(logic.render_term(cnx))

    return r

class Plumber(agent.Agent):

    def __init__(self,master_agent,database,ordinal):
        self.database = database
        self.master_agent = master_agent
        agent.Agent.__init__(self,names='plumber',subsystem='plumber',signature=version,container=1,protocols='is_subsys',ordinal=ordinal)

        self.add_verb2(20,'connect([],None,role(None,[or([concrete],[composite([descriptor])])]),role(to,[concrete,singular]),option(using,[numeric,singular]))',self.verb_20_connect)
        self.add_verb2(25,'connect([],None,role(None,[concrete,singular]))',self.verb_25_connect)
        self.add_verb2(18,'connect([un],None,role(None,[concrete]))',self.verb_18_unconnect)
        self.add_verb2(19,'connect([un],None,role(None,[concrete]),role(from,[concrete,singular]))',self.verb_19_unconnect_from)
        self.add_verb2(12,'insert([],None,role(None,[concrete,singular]),role(to,[concrete,singular]))',self.verb_12_insert)
        self.add_verb2(13,'insert([un],None,role(None,[concrete,singular]))',self.verb_13_uninsert)

    def verb_12_insert(self,subject,i,o):
        i = action.concrete_object(i)
        o = action.concrete_object(o)

        proxy = self.database.find_item(o)
        if not proxy:
            return async.failure('internal error: no proxy')

        proxy.set_insert(i)

    def verb_13_uninsert(self,subject,o):
        o = action.concrete_object(o)
        proxy = self.database.find_item(o)
        if not proxy:
            return async.failure('internal error: no proxy')

        proxy.set_insert('')

    @async.coroutine('internal error')
    def __unconnect_from(self,t,tproxy,f):
        objs = self.database.search_any_key('W',T('unconnect_from_list',t,f,V('W')))

        for (s,m) in objs:
            sproxy = self.database.find_item(s)
            sconn = find_conn(sproxy,m)
            for c in sconn:
                print 'disconnect',c,'from',s
                yield interpreter.RpcAdapter(sproxy.invoke_rpc('disconnect',c))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __unconnect(self,t,tproxy):
        print '__unconnect',t
        objs = self.database.search_any_key('W',T('input_list',t,V('W')))
        print '__unconnect',t,objs

        for (s,m) in objs:
            sproxy = self.database.find_item(s)
            #sconn = self.find_conn(sproxy,m)
            #for c in sconn:
            #    print 'disconnect',c,'from',s
            #    yield interpreter.RpcAdapter(sproxy.invoke_rpc('disconnect',c))
            yield interpreter.RpcAdapter(sproxy.invoke_rpc('clrconnect',''))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __unconnect_inputlist_from(self,t,tproxy,f):
        print 'deleting',f,'inputs from',t

        inputs = yield interpreter.RpcAdapter(tproxy.invoke_rpc('lstinput',''))
        inputs = action.unmarshal(inputs)
        print 'candidate inputs are:',inputs

        for input in inputs:
            print 'unconnecting',input
            iproxy = self.database.find_item(input)
            objs = self.database.search_any_key('W',T('unconnect_from_list',input,f,V('W')))

            if objs:
                yield self.__unconnect(input,iproxy)
                print 'deleting input',input
                yield interpreter.RpcAdapter(tproxy.invoke_rpc('delinput',input))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __unconnect_inputlist(self,t,tproxy):
        print 'deleting all inputs from',t

        inputs = yield interpreter.RpcAdapter(tproxy.invoke_rpc('lstinput',''))
        inputs = action.unmarshal(inputs)
        print 'candidate inputs are:',inputs

        for input in inputs:
            print 'unconnecting',input
            iproxy = self.database.find_item(input)
            yield self.__unconnect(input,iproxy)
            print 'deleting input',input
            yield interpreter.RpcAdapter(tproxy.invoke_rpc('delinput',input))

        yield async.Coroutine.success()

    def verb_25_connect(self,subject,t):
        t = action.concrete_object(t)
        print 'dump connect',t
        rv = self.get_connections(t)
        for r in rv:
            print r

    def get_connections(self,id):
        rv = []
        db = self.database

        for e in db.search(T('dump_connections',id,V('F'),V('T'))):
            f = e['F']
            t = e['T']

            fd = db.find_full_desc(f) or f
            td = db.find_full_desc(t) or t

            us = db.get_inputs_channels(t,f)
            for (u,c) in us:
                d = '%s -> %s' % (fd,td)
                if c: d = d+' channel %s' % (c)
                if u: d = d+' using %s' % (u)
                rv.append(d)

        return rv

    def verb_18_unconnect(self,subject,t):
        print 'un connect',t
        t = action.concrete_object(t)
        tproxy = self.database.find_item(t)

        if 'inputlist' in tproxy.protocols():
            return self.__unconnect_inputlist(t,tproxy)
        else:
            return self.__unconnect(t,tproxy)

    def verb_19_unconnect_from(self,subject,t,f):
        f = action.concrete_object(f)
        t = action.concrete_object(t)
        print 'un connect',t,'from',f
        tproxy = self.database.find_item(t)

        if 'inputlist' in tproxy.protocols():
            return self.__unconnect_inputlist_from(t,tproxy,f)
        else:
            return self.__unconnect_from(t,tproxy,f)


    @async.coroutine('internal error')
    def __connect_inner(self,t,f,u=None):
        normoutputs = []
        revoutputs = []

        norminputs = []
        revinputs = []

        d = self.database.search_any(T('connect_tosplit',t,V('OOUT'),V('OIN')))
        norminputs.extend([ self.__stuff(o) for o in d['OIN'] ])
        revoutputs.extend([ self.__stuff(o) for o in d['OOUT'] ])


        if action.is_concrete(f):
            f = action.crack_concrete(f)
            d = self.database.search_any(T('connect_fromsplit',f,V('OOUT'),V('OIN')))
            revinputs.extend([ self.__stuff(o) for o in d['OIN'] ])
            normoutputs.extend([ self.__stuff(o) for o in d['OOUT'] ])
        else:
            f = action.crack_composite(f,action.crack_descriptor)
            print 'from=',f
            for o,p in f:
                d =self.database.search_any(T('connect_fromsplit',o,V('OOUT'),V('OIN')))
                if d.get('OIN'):
                    revinputs.append(self.__stuff(o,channel=p))
                else:
                    normoutputs.append(self.__stuff(o,channel=p))

        print 'ni=',norminputs
        print 'ro=',revoutputs
        print 'no=',normoutputs
        print 'ri=',revinputs
        print 'u=',u

        allinputs = norminputs+revinputs
        alloutputs = normoutputs+revoutputs

        if u is True:
            iix = set()
            for ii in allinputs:
                iim = self.database.find_masters(ii.id)
                for ii2 in iim:
                    iixm = self.database.get_inputs(ii.id,ii2)
                    iix = iix.union(iixm)
            u = 1+reduce(max,iix,0)
            print 'using',u,iix

        if u == 0:
            u = None

        if len(allinputs)==1 and len(alloutputs)==1:
            print 'direct connect'
            self.__connect(alloutputs[0],allinputs[0],u)
            yield async.Coroutine.success()

        connections = []

        for i in norminputs:
            scores = {}
            for o in normoutputs:
                s = self.__score(o,i)
                print o.id,'->',i.id,'score',s
                if s[0]>0:
                    scores[s] = o

            if not scores:
                print i.id,'no inputs'
                continue

            best = max(scores.keys())
            o = scores[best]
            connections.append((o,i))

        for i in revinputs:
            scores = {}
            for o in revoutputs:
                s = self.__score(o,i)
                print o.id,'->',i.id,'score',s
                if s[0]>0:
                    scores[s] = o

            if not scores:
                print i.id,'no inputs'
                continue

            best = max(scores.keys())
            o = scores[best]
            connections.append((o,i))

        if not connections:
            yield async.Coroutine.failure('incompatible')

        for (o,i) in connections:
            self.__connect(o,i,u)

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __connect_outer(self,tproxy,f,u=None):
        ret = [ ]

        if 'inputlist' in tproxy.protocols() and u is not None:
            yield async.Coroutine.failure('using and inputlist incompatible')

        for f in action.arg_objects(f):
            if 'inputlist' in tproxy.protocols():
                ar = tproxy.invoke_rpc('addinput','')
                yield ar
                if not ar.status():
                    yield async.Coroutine.failure('addinput failed')
                (t,n) = action.unmarshal(ar.args()[0])
                print 'input list connect: new input is',t,n,'resyncing'
                yield self.database.sync()
                print 'resync complete'
                if n:
                    ret.append(action.initialise_return(t))
                etproxy = self.database.find_item(t)
            else:
                t = tproxy.id()
                etproxy = tproxy

            if u is None:
                if 'using' in etproxy.protocols():
                    u=True

            print 'connect2 from:',f
            print '-          to:',t
            print '-       using:',u
            r = self.__connect_inner(t,f,u)
            yield r
            if not r.status():
                yield async.Coroutine.failure(*r.args(),**r.kwds())

        yield async.Coroutine.success(tuple(ret))

    def verb_20_connect(self,subject,f,t,u):
        if u is not None:
            u=int(action.abstract_string(u))

        t = action.concrete_object(t)
        tproxy = self.database.find_item(t)
        return self.__connect_outer(tproxy,f,u)

    def __stuff(self,o,**extra):
        proxy = self.database.find_item(o)
        words = WordTable()

        words.add(*[n for n in proxy.names() if n not in ['input','output']])
        words.add(*proxy.fuzzy())

        return Stuff(o,words,ordinal=proxy.ordinal(),domain=proxy.domain(),proxy=proxy,**extra)

    def __score(self,ostuff,istuff):
        ws = self.__score_words(ostuff.words,istuff.words)
        os = self.__score_ordinals(ostuff.ordinal,istuff.ordinal)
        ds = self.__score_domains(ostuff.domain,istuff.domain)
        return (ws,os,ds)

    def __score_words(self,owords,iwords):
        s = 0

        iw = set(iwords.keys())
        ow = set(owords.keys())

        if iw==ow:
            return 5

        both = ow.intersection(iw)

        for w in both:
            so = owords[w]
            si = iwords[w]
            ws = cmp(so,0)*cmp(si,0)*(abs(so)+abs(si))
            s += ws

        return s

    def __score_domains(self,odomain,idomain):
        if isinstance(odomain,domain.Null):
            return -10
        if isinstance(idomain,domain.Null):
            return -10
        if odomain.iso() and idomain==domain.BoundedFloat(-1,1):
            return 2
        return 0

    def __score_ordinals(self,oord,iord):
        if oord is not None and iord is not None and oord==iord:
            return 1
        return 0

    def __connect(self,ostuff,istuff,u):
        channel = ostuff.get('channel',None)
        d=istuff.target(ostuff,u)
        print 'connecting',d,'->',istuff.id,'using',u
        istuff.get('proxy').invoke_rpc('connect',d)


@async.coroutine('internal error')
def unconnect_set(db,objs):
    cnx=[]

    for o in objs:
        cn = db.get_subsys_masters(o)
        for (ss,cl) in cn.items():
            print o,ss,cl
            if ss not in objs:
                for c in cl:
                    cnx.append(c)

    for (m,s) in cnx:
        print m,'->',s
        sproxy = db.find_item(s)
        sconn = find_conn(sproxy,m)
        for c in sconn:
            print 'disconnect',c,'from',s
            yield interpreter.RpcAdapter(sproxy.invoke_rpc('disconnect',c))
