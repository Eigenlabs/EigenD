
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
import traceback

from pi import logic,action,async,errors
from pi.logic.shortcuts import *
from . import interpreter,imperative,noun,referent

rules_verb = """
    # subject qualification
         srefiner(OLISTIN,OLISTOUT,VERB,NROLES) :-
            @is(OLISTOUT,
                $alluniq(O,
                    @in(O,OLISTIN),
                    db_relation(VERB,O,RROLES),
                    @applies(role(NRPRED,NRNOUN),NROLES,
                        @in(role(NRPRED,RRCONS),RROLES),
                        constraints(RRCONS,NRNOUN,XXX)
                    )
                )
            ).

    # object qualification
        orefiner(OLISTIN,OLISTOUT,VERB,SUBJECTS,NROLES,ROLE) :-
            @nonempty(SUBJECTS),
            @is(OLISTOUT,
                $alluniq(O,
                    @in(O,OLISTIN),
                    @is(R2,[role(ROLE,[O])|NROLES]),
                    @in(S,SUBJECTS),
                    db_relation(VERB,S,RCONS),
                        @applies(role(R,N),R2,
                            @in(role(R,C),RCONS),
                            constraints(C,N,XXXX)
                        )
                )
            ).
"""

def make_default_rule(word, value):
    rules = []
    rules.append(R(T('db_default',word,value)))
    return rules

class BgMarker(referent.StackObj):
    def __init__(self,word):
        self.word = word
    def __str__(self):
        return 'bg'
    def words(self):
        return (self.word,)

class RoleMarker(referent.StackObj):
    def __init__(self,word):
        self.word = word
    def __str__(self):
        return 'role: %s' % self.word
    def words(self):
        return (self.word,)

class ObjectRefiner(interpreter.VerbAction):
    def __init__(self,role,noun):
        self.role = role
        self.noun = noun

    def __str__(self):
        return "<<object: %s %s>>" % (self.role,self.noun)

    def run(self,interp,verb,mods,roles,args,flags,text):
        if not roles or roles[0] != 'none':
            return async.failure('garbled %s relationship' % verb)

        text = self.noun.words()+(self.role,'which')+text+(verb,)
        r = tuple( T('role',r,a.to_prolog()) for (r,a) in zip(roles[1:],args[1:]) )
        q = T('orefiner',self.noun.generic_objects(),V('XX'),verb,args[0].to_prolog(),r,self.role)
        print 'refining via',q
        o = interp.get_database().search_any_key('XX',q) or ()
        interp.push(referent.Referent(objects=o,words=text))
        return async.success()

class SubjectRefiner(interpreter.VerbAction):
    def __init__(self,noun):
        self.noun = noun

    def __str__(self):
        return "<<subject: %s>>" % self.noun

    def run(self,interp,verb,mods,roles,args,flags,text):
        text = self.noun.words()+('which',)+text+(verb,)
        r = tuple( T('role',r,a.to_prolog()) for (r,a) in zip(roles,args) )
        q = T('srefiner',self.noun.generic_objects(),V('XX'),verb,r)
        o = interp.get_database().search_any_key('XX',q) or ()
        interp.push(referent.Referent(objects=o,words=text))
        return async.success()

def primitive_background(interp,word):
    interp.push(BgMarker(word))
    return async.success()

def primitive_modifier(interp,word):
    interp.push(referent.ModMarker(word))
    return async.success()

def primitive_role(interp,word):
    interp.push(RoleMarker(word))
    return async.success()

def primitive_ify(interp,word):
    w = interp.undo()
    w = interp.undo()

    if w is None:
        interp.clear()
        return async.failure('invalid use of ify')

    interp.close()

    print w,'-ify'
    return primitive_verb(interp,w)

def primitive_which(interp,word):
    r = interp.pop(RoleMarker)
    n = interp.pop(referent.Referent)

    if n is None:
        return async.failure('which without a noun')

    if r is None:
        interp.push(SubjectRefiner(n))
    else:
        interp.push(ObjectRefiner(r.word,n))

    return async.success()


def primitive_verb(interp,verb):
    args=[]
    mods=[]
    roles=[]
    flags=[]
    action=interp.get_action() or run_imperative
    text=(verb,)

    print 'verb:',verb,map(str,interp.stack())

    while not interp.empty():
        m = interp.pop(referent.ModMarker)
        if m is not None:
            mods.append(m.word)
            text = m.words()+text
            continue
        m = interp.pop(BgMarker)
        if m is not None:
            flags.append('bg')
            text = m.words()+text
            continue
        break

    while not interp.empty():
        t = interp.pop(interpreter.VerbAction)
        if t is not None:
            action=t.run
            break

        t = interp.pop(referent.Referent)
        if t is None:
            print 'top stack is',interp.topany()
            return async.failure('garbled form of '+verb)

        text = t.words()+text

        r = interp.pop(RoleMarker)
        if r is None:
            r2=None
        else:
            r2=r.word
            text = r.words()+text

        args.insert(0,t)
        roles.insert(0,r2)

    args=tuple(args)
    roles=tuple(roles)

    return action(interp,verb,mods,roles,args,flags,text)


def run_imperative(interp,verb,mods,roles,args,flags,text):
    fg = ('bg' not in flags)
    coresult = async.Coroutine(run_imperative_co(interp,verb,mods,roles,args,flags,fg,text),interpreter.rpcerrorhandler)
    if fg: return coresult
    interp.add_job(coresult)
    return async.success()


def run_imperative_co(interp,verb,mods,roles,args,flags,fg,text):
    sresult = imperative.run(interp,verb,mods,roles,args)

    yield sresult

    if not sresult.status():
        yield async.Coroutine.failure(*sresult.args(),**sresult.kwds())

    (verbs,) = sresult.args()

    vresult = async.Aggregate(accumulate=True)

    for (verb,verb_args) in verbs:
        vresult.add(verb.subject(),verb.invoke(interp,verb.subject(),*verb_args))

    vresult.enable()

    yield vresult

    (vs,vf) = vresult.args()

    db = interp.get_database()

    for d in vs:
        print "%s: ok" % db.find_desc(d)

    for (d,m) in vf.iteritems():
        print "%s: %s" % (db.find_desc(d), m[0])

    context = set()
    sync = []
    removed = []
    dosync = True
    cancel = []
    errs=[]
    nsucceeded = 0
    nerr=0

    def result_iter(vs,vf):
        for (id,r) in vs.iteritems():
            yield (id,r)

        for (id,m) in vf.iteritems():
            yield ( id,(errors.message(m[0]),) )

    for (vid,vr) in result_iter(vs,vf):
        err=False
        for r in vr:
            if not logic.is_term(r):
                print 'duff return:',r
                continue
            c = r.pred

            if c == 'immediate':
                o = referent.Referent.from_prolog(r.args[0])
                sync.extend(o.concrete_ids())
                interp.push(o)
            elif c=='removed':
                o = referent.Referent.from_prolog(r.args[0])
                removed.extend(o.concrete_ids())
            elif c=='concrete':
                o = referent.Referent.from_prolog(r.args[0])
                sync.extend(o.concrete_ids())
                context.update(set(o.concrete_ids()))
            elif c=='nosync':
                dosync = False
            elif c=='cancel':
                cancel.append(r.args)
            elif c=='err':
                err = True
                errs.append((r.args,vid))
            elif c=='msg':
                msg = r.args[0]
                print 'message',vid,msg
        if err:
            nerr +=1
        else:
            nsucceeded += 1

    print 'after verb, succeeded=',nsucceeded,'errors=',nerr,'sync=',sync,'obj=',context

    if fg and context:
        print 'pushing',context,'to context stack'
        interp.get_context().push_stack(context)
        interp.get_context().extend_scope(context)

    if dosync:
        print 'starting sync after',verb,':',sync,[piw.address2server(o) for o in sync]
        yield interp.sync(*[piw.address2server(o) for o in sync])
        print 'sync done'

    if nsucceeded:
        yield async.Coroutine.success('%d verbs failed: %d verbs succeeded' % (nerr,nsucceeded),user_errors=tuple(errs))
    elif nerr:
        yield async.Coroutine.failure('%d verbs failed: %d verbs succeeded' % (nerr,nsucceeded),user_errors=tuple(errs))

    yield async.Coroutine.success()
