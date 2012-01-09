
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
from . import noun,interpreter

import piw
import copy
import sys


def __extend_scope(db,scope):
    working_scope = set()

    join_cache = db.get_joincache()
    assoc_cache = db.get_assoccache()
    host_cache = db.get_propcache('host')

    for o in scope:
        working_scope.update(join_cache.extended_lefts(o))
        working_scope.update(assoc_cache.extended_lefts(o))
        working_scope.update(host_cache.get_idset(o))

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


@async.coroutine('internal error')
def run(interp,verb,mods,roles,args):

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

    matches = []

    # cmdline_roles = set(None,to)
    # verb's role set = set((None,to), (None), (None,with))

    for v in verb_set:
        if v.fixed_roles() is not None and not cmdline_roles.issuperset(v.fixed_roles()):
            continue

        if not cmdline_mods.issuperset(v.mods()):
            continue

        mod_set = set(cmdline_mods)
        mod_set.difference_update(v.mods())
        # mod_set holds modifiers from command line but not supported by verb v

        role_set = set(cmdline_roles)
        if v.fixed_roles() is not None:
            role_set.difference_update(v.fixed_roles())
        if v.option_roles() is not None:
            role_set.difference_update(v.option_roles())
        # role_set holds roles from command line but not supported by verb v

        args_tmp = arg_values.copy()
        verb_args = []
        for r in v.order():
            a = args_tmp.get(r)
            verb_args.append(a)
            if a is not None: del args_tmp[r]

        if not role_set and not mod_set:
            matches.append((v,verb_args))

    if not matches:
        yield async.Coroutine.failure('inappropriate use',user_errors=((errors.inappropriate_use(verb),''),))

    checked = {}

    for (v,va) in matches:
        d = v.disambiguator()
        p = checked.get(d)

        if p is not None:
            # we have a matched direct verb
            continue

        vr = __check_constraints(db,v.order(),v.constraints(),va)
        yield vr
        if not vr.status():
            continue

        args = vr.args()[0]
        checked[d] = (v,args)

    if not checked:
        yield async.Coroutine.failure('inappropriate arguments',user_errors=((errors.inappropriate_arguments(verb),''),))

    yield async.Coroutine.success(checked.values())
