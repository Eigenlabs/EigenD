
from pi import logic,async,rpc,domain,paths

class Endpoint:
    def __init__(self,db,id,channel=None):
        proxy = db.find_item(id)

        self.id=id
        self.qid=db.to_usable_id(id)
        self.words = {}
        self.ordinal=proxy.ordinal()
        self.domain=proxy.domain() or domain.Null()
        self.channel=channel
        self.connect_static="connect-static" in proxy.protocols()

        self.__add_words(*[n for n in proxy.names() if n not in ['input','output']])
        self.__add_words(*proxy.fuzzy())

    def __add_words(self,*words):
        for w in words:
            w,s = self.__score_word(w)
            self.words[w] = self.words.get(w,0) + s

    def connect(self,ostuff,u):
        oid = paths.to_relative(ostuff.qid,scope=paths.id2scope(self.qid))
        d=logic.render_term(logic.make_term('conn',u,self.channel,oid,ostuff.channel,ostuff.connect_static and 'ctl' or None))
        print 'connecting',d,'->',self.id,'using',u
        rpc.invoke_rpc(self.qid,'connect',d)

    def __repr__(self):
        return '<%s %s %s>' % (self.id,self.channel,self.words)

    def __str__(self):
        return repr(self)

    @staticmethod
    def __score_word(w):
        mw,ml = Endpoint.__countstart(w,'+')
        lw,ll = Endpoint.__countstart(w,'-')

        if ml:
            return mw,ml

        if ll:
            return lw,-ll

        return w,1

    @staticmethod
    def __countstart(w,c):
        s = w.lstrip(c)
        return s,len(w)-len(s)

    @staticmethod
    def score(ostuff,istuff):
        ws = Endpoint.__score_words(ostuff.words,istuff.words)
        os = Endpoint.__score_ordinals(ostuff.ordinal,istuff.ordinal)
        ds = Endpoint.__score_domains(ostuff.domain,istuff.domain)
        return (ws,os,ds)

    @staticmethod
    def __score_words(owords,iwords):
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

    @staticmethod
    def __score_domains(odomain,idomain):
        if isinstance(odomain,domain.Null):
            return -10
        if isinstance(idomain,domain.Null):
            return -10
        if odomain.iso() and idomain==domain.BoundedFloat(-1,1):
            return 2
        return 0

    @staticmethod
    def __score_ordinals(oord,iord):
        if oord is not None and iord is not None and oord==iord:
            return 1
        return 0

@async.coroutine('internal error')
def plumber(db,to_descriptor,from_descriptors,using=None):
    assoc_cache = db.get_assoccache()
    partof_cache = db.get_partcache()
    proto_cache = db.get_propcache('protocol')
    name_cache = db.get_propcache('name')

    (ti,tp) = to_descriptor

    if using is None:
        if 'using' in proto_cache.get_valueset(ti):
            using=True

    print 'connect2 from:',from_descriptors
    print '-          to:',ti,tp
    print '-       using:',using

    if 1 == len(from_descriptors) and 0 == len(partof_cache.direct_lefts(ti)) and 0 == len(partof_cache.direct_lefts(from_descriptors[0][0])):
        (fi,fp) = from_descriptors[0]
        print 'direct connect from:',fi,fp
        print '-                to:',ti,tp
        fe = Endpoint(db,fi,channel=fp)
        te = Endpoint(db,ti,channel=tp)
        if using is True:
            iix = set()
            iim = db.find_masters(te.id)
            for ii2 in iim:
                iixm = db.get_inputs(te.id,ii2)
                iix = iix.union(iixm)
            using = 1+reduce(max,iix,0)
            print 'using',using,iix
        if using == 0:
            using = None
        te.connect(fe,using)
        yield async.Coroutine.success([])

    normoutputs = []
    revoutputs = []
    norminputs = []
    revinputs = []

    all_output = proto_cache.get_idset('output').union(name_cache.get_idset('output'))
    all_input = proto_cache.get_idset('input').union(name_cache.get_idset('input'))
    all_reverse = proto_cache.get_idset('revconnect')
    all_explicit = proto_cache.get_idset('explicit')

    if tp is None:
        contains = assoc_cache.direct_lefts(ti).union(partof_cache.lefts(ti)).difference(all_explicit).union(frozenset([ti]))
        contains_forward = contains.intersection(all_input).difference(all_reverse)
        contains_reverse = contains.intersection(all_output).intersection(all_reverse)
        norminputs.extend([Endpoint(db,i) for i in contains_forward])
        revoutputs.extend([Endpoint(db,i) for i in contains_reverse])
    else:
        if ti in all_reversed:
            if ti in all_output:
                revoutputs.append(Endpoint(db,ti,channel=tp))
        else:
            if ti in all_input:
                norminputs.append(Endpoint(db,ti,channel=tp))

    for (fi,fp) in from_descriptors:
        if fp is None:
            contains = assoc_cache.direct_lefts(fi).union(partof_cache.lefts(fi)).difference(all_explicit).union(frozenset([fi]))
            contains_forward = contains.intersection(all_output).difference(all_reverse)
            contains_reverse = contains.intersection(all_input).intersection(all_reverse)
            normoutputs.extend([Endpoint(db,i) for i in contains_forward])
            revinputs.extend([Endpoint(db,i) for i in contains_reverse])
        else:
            if fi in all_reversed:
                if fi in all_input:
                    revinputs.append(Endpoint(db,fi,channel=fp))
            else:
                if fi in all_output:
                    normoutputs.append(Endpoint(db,fi,channel=fp))

    print 'ni=',norminputs
    print 'ro=',revoutputs
    print 'no=',normoutputs
    print 'ri=',revinputs
    print 'u=',using

    allinputs = norminputs+revinputs
    alloutputs = normoutputs+revoutputs

    if using is True:
        iix = set()
        for ii in allinputs:
            iim = db.find_masters(ii.id)
            for ii2 in iim:
                iixm = db.get_inputs(ii.id,ii2)
                iix = iix.union(iixm)
        using = 1+reduce(max,iix,0)
        print 'using',using,iix

    if using == 0:
        using = None

    if len(allinputs)==1 and len(alloutputs)==1:
        print 'direct connect'
        allinputs[0].connect(alloutputs[0],using)
        yield async.Coroutine.success([])

    connections = []

    for i in norminputs:
        scores = {}
        for o in normoutputs:
            s = Endpoint.score(o,i)
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
            s = Endpoint.score(o,i)
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

    for (ostuff,istuff) in connections:
        istuff.connect(ostuff,using)

    yield async.Coroutine.success([])
