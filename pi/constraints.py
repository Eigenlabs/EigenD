
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

from pi import async,logic,paths,rpc,action
from logic.shortcuts import *

rules_constraints = """

# array_match(LIST1,LIST2)
    array_match([],[]).
    array_match([A|ALEFT],BLIST) :- @any(synonym(A,ASYN),@extract(ASYN,BLIST,BLEFT)), array_match(ALEFT,BLEFT).

    array_head([],BLIST,BLIST).
    array_head([A1|ALEFT],[B1|BLEFT],BREM) :- synonym(A1,B1), array_head(ALEFT,BLEFT,BREM).

    dscmatch([],[]).
    dscmatch([S|SL],[dsc(S,D2)|DL2]) :- dscmatch(SL,DL2).

    make_descriptor(dsc(ID,P),dsc(ID,P)).
    make_descriptor(cnc(ID),dsc(ID,'')).

    make_composite(cmp(ILIST),cmp(OLIST),SUBC) :- constraints_check(SUBC,ILIST,OLIST).
    make_composite(cnc(ID),cmp(OLIST),SUBC) :- @is(ILIST,[cnc(ID)]), constraints_check(SUBC,ILIST,OLIST).
    make_composite(dsc(ID,P),cmp(OLIST),SUBC) :- @is(ILIST,[dsc(ID,P)]), constraints_check(SUBC,ILIST,OLIST).

    timespec(MAPPING,[],[],[]).
    timespec(MAPPING,WLIN,WLOUT,[N|[LABEL|[False|TSPEC]]]) :- @in([WORDS,LABEL],MAPPING), array_head(WORDS,WLIN,[N|WLREM]), @numeric(N), timespec(MAPPING,WLREM,WLOUT,TSPEC).
    timespec(MAPPING,[N|WLIN],WLOUT,[N|[LABEL|[True|TSPEC]]]) :- @in([WORDS,LABEL],MAPPING), array_head(WORDS,WLIN,WLREM), @numeric(N), timespec(MAPPING,WLREM,WLOUT,TSPEC).

# constraint(CONSTRAINT,VALUE)

    constraint(agent,cnc(O)) :- @property(agent,O).
    constraint(concrete,cnc(O)).
    constraint(abstract,abstract(O)).
    constraint(instance(OBJECT),cnc(OBJECT)).
    constraint(partof(PARENT),cnc(OBJECT)) :- @partof_direct(OBJECT,PARENT).

    constraint(ideal(TAG),ideal(TAG,ANY)).
    constraint(gideal(TAG),ideal([ANYTAG1,TAG],ANY2)).
    constraint(tagged_ideal(TAG,W),tagged_ideal(TAG,ANY,W),ideal(TAG,ANY)).
    constraint(tagged_gideal(TAG,W),tagged_ideal([ANYTAG1,TAG],ANY2,W),ideal([ANYTAG1,TAG],ANY2)).
    constraint(virtual,virtual(ID)).
    constraint(atomic,cnc(O)).
    constraint(atomic,virtual(ID)).
    constraint(implements(P),cnc(O)) :- @protocol(P,O).
    constraint(implements(P),virtual(ID)) :- @protocol(P,ID).
    constraint(cmpdsc(DL1),cmp(DL2)) :- dscmatch(DL1,DL2).

    constraint(descriptor,I,O) :- make_descriptor(I,O).
    constraint(composite(SUBC),I,O) :- make_composite(I,O,SUBC).

    constraint(matches(XLIST),abstract(WLIST)) :- array_match(XLIST,WLIST).
    constraint(numeric,abstract([W])) :- @numeric(W).

    constraint(matches(XLIST,LABEL),abstract(WLIST),abstract([LABEL])) :- array_match(XLIST,WLIST).
    constraint(mass(XLIST),abstract([W|WLIST])) :- @numeric(W), array_match(XLIST,WLIST).
    constraint(mass(XLIST),abstract(ILIST),abstract([W|WLIST])) :- @last(ILIST,W,WLIST), @numeric(W), array_match(XLIST,WLIST).

    constraint(timespec(MAPPING),abstract(WLIST),abstract(TSPEC)) :- timespec(MAPPING,WLIST,[],TSPEC).
    constraint(issubject(VERB,CROLES),O) :- db_relation(VERB,O,RROLES), @applies(role(CPRED,CNOUN),CROLES, @in(role(CPRED,RCONS),RROLES), constraints(RCONS,CNOUN,X)).


    constraint(not(C),I) :- @not(constraint(C,I,TMP)).
    constraint(or([C1|CTAIL]),I,O) :- @or(constraints_filter(C1,[I],[O]), constraint(or(CTAIL),I,O)).
    constraint(or(C1,C2),I,O) :- @or(constraints_filter(C1,[I],[O]), constraints_filter(C2,[I],[O])).

    constraint(C,I,I) :- constraint(C,I).

# constraints(ARG,SCHEMA)

    constraints(CLIST,ILIST,OLIST) :- constraints_filter(CLIST,ILIST,OLIST).

    constraint_filter(C,ILIST,OLIST) :- @is(OLIST,$extend(I,ILIST,O,constraint(C,I,O))), @nonempty(OLIST).
    constraint_filter(singular,[O],[O]).

    constraints_filter([],ILIST,ILIST).
    constraints_filter([C|CLIST],ILIST,OLIST) :-
        constraint_filter(C,ILIST,TLIST),
        constraints_filter(CLIST,TLIST,OLIST).

    constraint_check(C,ILIST,OLIST) :- @map(I,ILIST,O,OLIST,constraint(C,I,O)).
    constraint_check(singular,[O],[O]).

    constraints_check([],ILIST,ILIST).
    constraints_check([C|CLIST],ILIST,OLIST) :-
        constraint_check(C,ILIST,TLIST),
        constraints_check(CLIST,TLIST,OLIST).

"""

def is_number(n):
    try:
        m = float(n)
        return True
    except:
        return False

def __match_mapping(words,mapping):
    for (sig,label) in mapping:
        l = len(sig)
        if words[0:l] == sig:
            return (words[l:],label)
    return (words,None)

def __get_timespec(words,mapping):
    matches = []

    while words:
        if is_number(words[0]):
            n = words[0]
            (w2,l) = __match_mapping(words[1:],mapping)
            if l:
                words = w2
                matches.extend((n,l,True))
                continue

        (w2,l) = __match_mapping(words,mapping)
        if not l: return []
        if not w2 or not is_number(w2[0]): return []
        matches.extend((w2[0],l,False))
        words = w2[1:]

    return [logic.make_term('timespec',tuple(matches))]


def constraint_timespec_1(db,c,objects):
    mapping = c.args[0]
    matches = []

    for r in objects:
        if logic.is_pred(r,'abstract'):
            words = r.args[0]
            matches.extend(__get_timespec(words,mapping))

    return matches

def __partition(words):
    numbers = []
    text = [[]]

    for w in words:
        if is_number(w):
            numbers.append(w)
            if text[-1]:
                text.append([])
        else:
            text[-1].append(w)

    if not text[-1]:
        text.pop()

    return (text,numbers)

def __constraint_coord(db,c,objects):
    matches = []
    tag = c.args[0]
    dimensions = len(c.args[1:])
    dimensionsets = [ (i,set(a)) for i,a in enumerate(c.args[1:]) ]
    dimensionresults = [ None for i in dimensionsets ]

    for r in objects:
        if not logic.is_pred(r,'abstract'):
            continue

        (w,n) = __partition(r.args[0])

        if len(w) != dimensions or len(n) != dimensions:
            continue

        for (wx,nx) in zip(w,n):
            wxs = set(wx)
            for (di,ds) in dimensionsets:
                if ds == wxs:
                    dimensionresults[di] = nx
                    break

        if None in dimensionresults:
            continue

        matches.append(logic.make_term('abstract',(tag,)+tuple(dimensionresults)))

    return matches

def constraint_coord_2(db,c,objects):
    return __constraint_coord(db,c,objects)

def constraint_coord_3(db,c,objects):
    return __constraint_coord(db,c,objects)

def constraint_coord_4(db,c,objects):
    return __constraint_coord(db,c,objects)

def constraint_coord_5(db,c,objects):
    return __constraint_coord(db,c,objects)

def constraint_mass_1(db,c,objects):
    wordlist = set(c.args[0])
    matches = []
    nwords = len(wordlist)+1

    for r in objects:
        if logic.is_pred(r,'abstract'):
            owords = r.args[0]
            if len(owords)==nwords:
                if is_number(owords[0]) and set(owords[1:])==wordlist:
                    matches.append(r)
                    continue
                if is_number(owords[-1]) and set(owords[:-1])==wordlist:
                    matches.append(logic.make_term('abstract',(owords[-1],)+owords[:-1]))
                    continue

    return matches

def constraint_tagged_1(db,c,objects):
    wordlist = set(c.args[0])
    wlen = len(wordlist)+1
    matches = []

    for r in objects:
        if logic.is_pred(r,'abstract') and len(r.args[0])==wlen:
            if wordlist==set(r.args[0][:-1]):
                matches.append(logic.make_term('abstract',r.args[0][-1:]))

    return matches

def constraint_matches_1(db,c,objects):
    wordlist = set(c.args[0])
    matches = []

    for r in objects:
        if logic.is_pred(r,'abstract'):
            if wordlist==set(r.args[0]):
                matches.append(r)

    return matches

def constraint_matches_2(db,c,objects):
    wordlist = set(c.args[0])
    label = c.args[1]
    matches = []

    for r in objects:
        if logic.is_pred(r,'abstract'):
            if wordlist==set(r.args[0]):
                matches.append(logic.make_term('abstract',(label,)))

    return matches

def constraint_descriptor(db,c,objects):
    matches = []

    for r in objects:
        if logic.is_pred(r,'dsc'):
            matches.append(r)
            continue
        if logic.is_pred(r,'cnc'):
            matches.append(T('dsc',r.args[0],''))
            continue

    return matches

def constraint_cmpdsc_1(db,c,objects):
    matches = []
    req_id = c.args[0]

    for r in objects:
        if logic.is_pred_arity(r,'cmp',1,1):
            dsc = r.args[0]
            if logic.is_list(dsc) and len(dsc)==1:
                dsc = dsc[0]
                if logic.is_pred_arity(dsc,'dsc',2,2) and dsc.args[0]==req_id:
                    matches.append(r)

    return matches

@async.coroutine('internal error')
def constraint_composite_1(db,c,objects):
    subc = c.args[0]
    matches = []

    for r in objects:
        if logic.is_pred(r,'cnc') or logic.is_pred(r,'dsc'):
            r = T('cmp',(r,))
        elif not logic.is_pred(r,'cmp'):
            continue

        result = resolve_constraints(db,subc,r.args[0])
        yield result

        if not result.status():
            yield async.Coroutine.failure(result.args()[0])

        if result.args()[0]:
            matches.append(T('cmp',result.args()[1]))

    yield async.Coroutine.success(matches)

@async.coroutine('internal error')
def constraint_or_2(db,c,objects):
    alternatives1 = c.args[0]
    alternatives2 = c.args[1]

    r = resolve_constraints(db,alternatives1,objects)
    yield r
    if not r.status():
        yield async.Coroutine.failure(r.args()[0])
    if r.args()[0]:
        yield async.Coroutine.success(r.args()[1])

    r = resolve_constraints(db,alternatives2,objects)
    yield r
    if not r.status():
        yield async.Coroutine.failure(r.args()[0])
    if r.args()[0]:
        yield async.Coroutine.success(r.args()[1])

    yield async.Coroutine.failure(())

@async.coroutine('internal error')
def constraint_or_1(db,c,objects):
    alternatives = c.args[0]

    for subc in alternatives:
        r = resolve_constraints(db,subc,objects)
        yield r
        if not r.status():
            yield async.Coroutine.failure(r.args()[0])
        if r.args()[0]:
            yield async.Coroutine.success(r.args()[1])

    yield async.Coroutine.failure(())

def constraint_numeric(db,c,objects):
    matches = []

    for r in objects:
        if logic.is_pred(r,'abstract') and len(r.args[0])==1:
            try: 
                n = float(r.args[0][0])
                matches.append(r)
            except: pass

    return matches

def constraint_singular(db,c,objects):
    if len(objects) == 1:
        return objects
    return []

def constraint_virtual(db,c,objects):
    matches = []

    for r in objects:
        if logic.is_pred(r,'virtual'):
            matches.append(r)

    return matches

def constraint_concrete(db,c,objects):
    matches = []

    for r in objects:
        if logic.is_pred(r,'cnc'):
            matches.append(r)

    return matches

def constraint_abstract(db,c,objects):
    matches = []

    for r in objects:
        if logic.is_pred(r,'abstract'):
            matches.append(r)

    return matches

def constraint_instance_1(db,c,objects):
    matches = []
    id = c.args[0]

    for r in objects:
        if logic.is_pred(r,'cnc') and r.args[0]==id:
            matches.append(r)

    return matches

def constraint_partof_1(db,c,objects):
    matches = []
    id = c.args[0]
    parts = db.get_partcache().extended_lefts(id)

    for r in objects:
        if logic.is_pred(r,'cnc') and r.args[0] in parts:
            matches.append(r)

    return matches

def constraint_proto_1(db,c,objects):
    matches = []
    proto = db.get_propcache('protocol').get_idset(c.args[0])
    print 'protocol constraint',c.args[0],objects

    for r in objects:
        if logic.is_pred(r,'virtual') and r.args[0] in proto:
            matches.append(r)
        if logic.is_pred(r,'cnc') and r.args[0] in proto:
            matches.append(r)

    return matches

def constraint_notproto_1(db,c,objects):
    matches = []
    proto = db.get_propcache('protocol').get_idset(c.args[0])

    for r in objects:
        if logic.is_pred(r,'cnc') and r.args[0] not in proto:
            matches.append(r)

    return matches

def remove_word(tup,word):
    if word in tup:
        lst = list(tup)
        lst.remove(word)
        tup = tuple(lst)
    return tup

@async.coroutine('internal error')
def constraint_tagged_ideal_2(db,c,objects):
    matches = []
    t = c.args[0]
    srv = t[0]
    typ = t[1]
    wrds = set(c.args[1])
    lwrds = len(wrds)

    idc = db.get_propcache('ideal')

    for r in objects:
        if logic.is_pred(r,'tagged_ideal'):
            if r.args[0][0]==srv and r.args[0][1]==typ and set(r.args[2])==wrds:
                matches.append(T('ideal',(srv,typ),r.args[1]))
            continue

        if not logic.is_pred(r,'abstract'):
            continue

        words = r.args[0]
        if not words: continue
        if len(words)-lwrds <1: continue
        if set(words[-lwrds:]) != wrds: continue

        words = words[:lwrds]
        words = remove_word(words,typ)

        if not words: continue

        resolvers = (srv,)
        if not srv:
            resolvers = idc.get_idset(typ)

        print 'resolvers for',t,'=',resolvers
            
        for srv in resolvers:
            result = rpc.invoke_rpc(srv,'resolve_ideal',action.marshal((typ,words)))
            yield result

            if result.status():
                sub = paths.make_subst(srv)
                i = logic.parse_clause(result.args()[0],sub)
                print words,'resolved to',i,'via',t
                matches.extend(i)
                break

            print 'resolution error',result.args(),'resolving',typ,words,'on',s

    yield async.Coroutine.success(matches)


@async.coroutine('internal error')
def constraint_ideal_1(db,c,objects):
    matches = []
    t = c.args[0]
    srv = t[0]
    typ = t[1]
    idc = db.get_propcache('ideal')

    for r in objects:
        if logic.is_pred(r,'ideal'):
            if r.args[0][0]==srv and r.args[0][1]==typ:
                matches.append(r)
            continue

        if not logic.is_pred(r,'abstract'):
            continue

        words = r.args[0]

        if not words: continue
        words = remove_word(words,typ)
        if not words: continue


        resolvers = (srv,)
        if not srv:
            resolvers = idc.get_idset(typ)

        print 'resolvers for',t,'=',resolvers
            
        for srv in resolvers:
            result = rpc.invoke_rpc(srv,'resolve_ideal',action.marshal((typ,words)))
            yield result

            if result.status():
                sub = paths.make_subst(srv)
                i = logic.parse_clause(result.args()[0],sub)
                print words,'resolved to',i,'via',t
                matches.extend(i)
                break

            print 'resolution error',result.args(),'resolving',typ,words,'on',srv

    yield async.Coroutine.success(matches)
            
@async.coroutine('internal error')
def constraint_issubjectextended_3(db,c,objects):
    verb = c.args[0]   # ie 'create'
    croles = dict((a.args[0],a.args[1]) for a in c.args[2])  # ie '[role(by,[cnc(~self)])]'
    rr = c.args[1]
    matches = []

    for o in objects:
        print 'issubjectextended',verb,o
        for r in db.search_key('RROLES',T('db_relation',verb,o,V('RROLES'))):
            print 'r=',r
            rroles = dict((a.args[0],a.args[1]) for a in r)
            cr = (yield resolve_constraints_dict(db,croles,rroles))
            print 'found',o,croles,rroles,cr.status()
            if not cr.status() or not cr.args()[0]:
                continue

            if rr not in cr.args()[1]:
                continue

            matches.append(T('cmp',(o,)+cr.args()[1][rr]))

    yield async.Coroutine.success(matches)

@async.coroutine('internal error')
def constraint_issubject_2(db,c,objects):
    verb = c.args[0]   # ie 'create'
    croles = dict((a.args[0],a.args[1]) for a in c.args[1])  # ie '[role(by,[cnc(~self)])]'
    matches = []

    for o in objects:
        print 'issubject',verb,o
        for r in db.search_key('RROLES',T('db_relation',verb,o,V('RROLES'))):
            rroles = dict((a.args[0],a.args[1]) for a in r)
            cr = (yield resolve_constraints_dict(db,croles,rroles))
            if not cr.status() or not cr.args()[0]:
                continue
            print 'found',o,croles,rroles,cr.args()[1]
            matches.append(o)
            break

    yield async.Coroutine.success(matches)

def __get_constraint(c):
    if isinstance(c,str):
        return globals().get('constraint_'+c)
    if logic.is_term(c):
        return globals().get('constraint_%s_%d' % (c.pred,c.arity))
    print 'invalid constraint',c
    return None

@async.coroutine('internal error')
def resolve_constraints(db,constraints, objects):
    for c in constraints:
        cfunc = __get_constraint(c)
        if cfunc is None:
            yield async.Coroutine.failure('invalid constraint: %s'%c)

        r = cfunc(db,c,objects)

        if isinstance(r,async.Deferred):
            yield r
            if not r.status():
                yield async.Coroutine.failure(r.args()[0])
            objects = r.args()[0]
        else:
            objects = r

        if not objects:
            break

    if objects:
        yield async.Coroutine.success(True,tuple(objects))

    yield async.Coroutine.success(False,())

@async.coroutine('internal error')
def resolve_constraints_dict(db,constraints,objects):
    out_dict = dict()

    for (cr,cc) in constraints.iteritems():
        oc = objects.get(cr)
        if oc is None:
            yield async.Coroutine.success(False,())
        r = (yield resolve_constraints(db,cc,oc))
        if not r.status():
            yield async.Coroutine.failure(r.args()[0])
        if not r.args()[0]:
            yield async.Coroutine.success(False,{})
        out_dict[cr] = r.args()[1]

    yield async.Coroutine.success(True,out_dict)

def filter_result_ids(results,filt):
    r2 = []

    for r in results:
        if logic.is_pred(r,'abstract'):
            r2.append(r)
            continue
        if logic.is_pred_arity(r,'cnc',1):
            r2.append(logic.make_term('cnc',filt(r.args[0]),*r.args[1:]))
            continue
        if logic.is_pred_arity(r,'virtual',1):
            r2.append(logic.make_term('virtual',filt(r.args[0]),*r.args[1:]))
            continue
        if logic.is_pred_arity(r,'dsc',1):
            r2.append(logic.make_term('dsc',filt(r.args[0]),*r.args[1:]))
            continue
        if logic.is_pred_arity(r,'ideal',1):
            r2.append(logic.make_term('ideal',(filt(r.args[0][0]),r.args[0][1]),*r.args[1:]))
            continue
        if logic.is_pred(r,'cmp'):
            r2.append(logic.make_term('cmp',*filter_result_ids(r.args)))
            continue
        r2.append(r)

    return tuple(r2)

