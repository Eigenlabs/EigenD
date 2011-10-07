
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

from pi import async,logic,action,errors,rpc,paths,constraints,utils,paths
from pi.logic.shortcuts import *
from plg_language import noun,interpreter

import piw
import copy
import sys

def __extend_scope(db,scope):
    working_scope = set()

    join_cache = db.get_joincache()
    assoc_cache = db.get_assoccache()

    for o in scope:
        working_scope.update(join_cache.extended_lefts(o))
        working_scope.update(assoc_cache.extended_lefts(o))

    return working_scope

def __filter_by_scope(obj_set, scope):
    obj_out = set()

    for o in obj_set:
        s = o.subject()
        if not s or s in scope:
            obj_out.add(o)

    return obj_out

def __filter_scope(obj_set, inner_scope, outer_scope):
    if inner_scope:
        return __filter_by_scope(obj_set,inner_scope)

    if outer_scope:
        return __filter_by_scope(obj_set,outer_scope)

    return set(obj_set)

@async.coroutine('internal error')
def run(interp,verb,mods,roles,args,text,build_result):

    db=interp.get_database()
    ctx=interp.get_context()
    inner_scope = __extend_scope(db,ctx.get_inner_scope())
    outer_scope = __extend_scope(db,ctx.get_verb_scope())
    verbcache = db.get_verbcache()
    cmdline_roles = set(roles)
    cmdline_mods = set(mods)

    arg_values = dict()

    for (r,a) in zip(roles,args):
        arg_values[r] = a.generic_objects()

    verb_set = verbcache.find_verbs_by_name(verb)
    verb_set = __filter_scope(verb_set,inner_scope,outer_scope)

    if not verb_set: 
        yield async.Coroutine.failure('nothing uses verb',user_errors=((errors.nothing_uses_verb(verb),''),))

    mode_set = verbcache.find_all_modes()
    mode_set = __filter_scope(mode_set,inner_scope,outer_scope)

    matches = []

    # cmdline_roles = set(None,to)
    # verb's role set = set((None,to), (None), (None,with))

    for v in verb_set:
        if not cmdline_roles.issuperset(v.fixed_roles()):
            continue

        if not cmdline_mods.issuperset(v.mods()):
            continue

        mod_set = set(cmdline_mods)
        mod_set.difference_update(v.mods())
        # mod_set holds modifiers from command line but not supported by verb v

        role_set = set(cmdline_roles)
        role_set.difference_update(v.fixed_roles())
        role_set.difference_update(v.option_roles())
        # role_set holds roles from command line but not supported by verb v

        args_tmp = arg_values.copy()
        verb_args = []
        for r in v.order():
            a = args_tmp.get(r)
            verb_args.append(a)
            if a is not None: del args_tmp[r]

        working_mode_set = mode_set.copy()

        for r in mod_set:
            m = verbcache.find_modes_by_mod(r)
            working_mode_set.intersection_update(m)

        for r in role_set:
            m = verbcache.find_modes_by_role(r)
            working_mode_set.intersection_update(m)

        vmatches = []
        for m in working_mode_set:
            if mod_set != m.mods():
                continue
            if not role_set.issuperset(m.fixed_roles()):
                continue
            if v.subject() == m.subject():
                continue

            mode_args = [args_tmp.get(r) for r in m.order()]
            vmatches.append((v,verb_args,m,mode_args))

        if vmatches:
            matches.extend(vmatches)
        else:
            if not role_set and not mod_set:
                matches.append((v,verb_args,None,None))

    if not matches:
        yield async.Coroutine.failure('inappropriate use',user_errors=((errors.inappropriate_use(verb),''),))

    checked = {}

    for (v,va,m,ma) in matches:
        d = v.disambiguator()
        p = checked.get(d)

        if p is not None and p[2]==None:
            # we have a matched direct verb
            continue

        vr = __check_constraints(db,v.order(),v.constraints(),va)
        yield vr
        if not vr.status():
            continue

        if m is None:
            checked[d] = (v,vr.args()[0],None,None)
            continue

        mr = __check_constraints(db,m.order(),m.constraints(),ma)
        yield mr
        if not mr.status(): continue
        checked[d] = (v,vr.args()[0],m,tuple(mr.args()[0]))

    grouped = {}
    if not checked:
        yield async.Coroutine.failure('inappropriate arguments',user_errors=((errors.inappropriate_arguments(verb),''),))

    for (v,va,m,ma) in checked.itervalues():
        grouped.setdefault((m,ma),[]).append((v,va))

    for ((m,ma),vl) in grouped.iteritems():
        invoker = direct_invoker if m is None else event_invoker
        r = invoker(interp,m,ma,vl,text,build_result)
        yield r

    yield async.Coroutine.success()

@async.coroutine('internal error')
def __check_constraints(db,order,clist,args):
    #
    # args is dict of role -> Referent
    # returns dict of role -> Prolog object representations
    #
    argsout = []

    for (r,a) in zip(order,args):
        if a is None:
            argsout.append(a)
            continue

        c = clist.get(r)
        sr = constraints.resolve_constraints(db,c,a)
        yield sr

        if not sr.status():
            yield async.Coroutine.failure()

        if not sr.args()[0]:
            yield async.Coroutine.failure()

        argsout.append(sr.args()[1])

    yield async.Coroutine.success(argsout)


def direct_invoker(interp,mode,mode_args,vlist,text,build_result):
    for (verb,verb_args) in vlist:
        result = verb.invoke(interp,verb.subject(),*verb_args)
        build_result(verb.subject(),result)
    return async.success()

@async.coroutine('internal error')
def event_invoker(interp,mode,mode_args,vlist,text,build_result):
    text = ' '.join(text)
    result = mode.invoke(interp, text, *mode_args)
    yield result

    if not result.status():
        yield async.Coroutine.failure(*result.args(),**result.kwds())

    mode_result = result.args()[0]
    col = async.Aggregate(accumulate=True)
    print 'mode returned',mode_result
    trigger,trigger_flags = logic.parse_clause(mode_result)
    print 'trigger is',trigger,'flags',trigger_flags

    if 'echo' in trigger_flags:
        try:
            interp.get_agent().create_echo(trigger,text)
        except:
            utils.log_exception()
            print 'cancelling mode'
            mode.cancel(interp,trigger)
            yield async.Coroutine.failure('internal error')

    status_returns = []

    for (verb,verb_args) in vlist:
        print '>>>>>> verb',verb,verb_args,type(verb)
        result0 = verb.defer(interp, trigger, None, *verb_args)
        result2 = async.Deferred()
        result3 = async.Deferred()

        def not_ok(r2,r3,*args,**kwds):
            print 'deferred action for',text,'not ok',result3,result0
            r2.failed(*args,**kwds)
            r3.failed(*args,**kwds)

        def ok(id,r2,r3,*args,**kwds):
            print 'deferred action for',text,'ok',args,kwds
            (event,status) = args[0]
            if status:
                status_returns.append(logic.parse_clause(status,paths.make_subst(id)))
            r2.succeeded()
            r3.succeeded()

        result0.setCallback(ok,verb.id(),result2,result3).setErrback(not_ok,result2,result3)
        col.add(result0,result3)
        build_result(verb.subject(),result2)

    col.enable()
    yield col

    if not col.successes():
        print 'cancelling mode'
        mode.cancel(interp,trigger)
    else:
        print 'status returns:',status_returns
        if status_returns:
            mode.attach(interp,trigger,*status_returns)
        yield async.Coroutine.success()

@async.coroutine('internal error')
def cancel_events(db,event_list):
    for (mode_id,mode_index,trigger_id) in event_list:
        print 'canceling event',trigger_id,'from',mode_id

        event_ids = db.find_slaves(trigger_id)
        event_ids.intersection_update(db.get_propcache('protocol').get_idset('deferred'))

        print 'event',event_ids,'mode',mode_id

        for id in event_ids:
            action.adapt_callback0(rpc.invoke_rpc(id,'cancel',''))

        yield rpc.invoke_rpc(mode_id,'mcancel',action.marshal((mode_index,trigger_id)))

    yield async.Coroutine.success()
