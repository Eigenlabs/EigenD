
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

import terms
import exceptions

class LogicError(exceptions.Exception):
    def __init__(self,term):
        self.term = term
    def __str__(self):
        return str(self.term)

def pred_nonempty(engine,term,env):
    if term.arity!=1: return False
    t = terms.expand(term.args[0],env)
    if not terms.is_list(t): return False
    if not t: return False
    return True

def pred_meta(engine,term,env):
    if term.arity!=3: return False

    t = terms.expand(term.args[0],env)
    p = terms.expand(term.args[1],env)
    a = terms.expand(term.args[2],env)

    if terms.is_unbound(t):
        if terms.is_unbound(p) or terms.is_unbound(a):
            return False
        if not isinstance(p,str) or not terms.is_list(a):
            return False
        x = terms.make_term(p,*a)
        return terms.unify(x,{},t,env)

    e = env.copy()
    if terms.unify(t.pred,{},p,e) and terms.unify(t.args,{},a,e):
        env.update(e)
        return True

    return False

def pred_fail(engine,term,env):
    return False

def pred_print(engine,term,env):
    term = terms.expand(term,env)
    print ' '.join(terms.render_term(a) for a in term.args)
    return True

def func_cat(rules,term,env):
    term = terms.expand(term,env)
    result=' '.join(str(a) for a in term.args)
    return result

def func_lt(rules,term,env):
    if term.arity!=2: return None
    left = rules.evaluate(term.args[0],env)
    if terms.is_unbound(left): return term
    right = rules.evaluate(term.args[1],env)
    if terms.is_unbound(right): return term
    return left<right

def pred_isnot(rules,term,env):
    if term.arity!=2: return False

    left = rules.evaluate(term.args[0],env)
    right  = rules.evaluate(term.args[1],env)

    if terms.is_unbound(left): return False
    if terms.is_unbound(right): return False

    return left!=right

def pred_is(rules,term,env):
    if term.arity!=2: return False

    leftv = rules.evaluate(term.args[0],env)
    rightv = rules.evaluate(term.args[1],env)

    if terms.is_bound(leftv):
        if terms.is_bound(rightv):
            return leftv==rightv
        return terms.unify(leftv,{},rightv,env)

    if terms.is_bound(rightv):
        return terms.unify(rightv,{},leftv,env)

    return False

def func_ifthenelse(rules,term,env):
    if term.arity!=3: return term

    for result in rules.search(term.args[0]):
        e=env.copy()
        e.update(result)
        return rules.evaluate(term.args[1],e)

    return rules.evaluate(term.args[2],env)

def pred_not(rules,term,env):
    if term.arity < 1: return False
    goals = [ terms.expand(g,env) for g in term.args ]
    for result in rules.search(*goals):
        return False
    return True

def pred_bound(rules,term,env):
    return terms.is_bound(terms.expand(term,env))

def pred_unbound(rules,term,env):
    return not terms.is_bound(terms.expand(term,env))

def func_any(rules,term,env):
    if term.arity < 2: return term
    key = term.args[0]
    goals = [ terms.expand(g,env) for g in term.args[1:] ]
    for result in rules.search(*goals):
        return rules.evaluate(key,result)
    return term

def pred_numeric(rules,term,env):
    term=terms.expand(term,env)
    for a in term.args:
        try:
            if not terms.is_const(a): return False
            f = float(a)
        except ValueError:
            return False
    return True

def pred_any(rules,term,env):
    for r in rules.search_env(env,*term.args):
        env.update(r)
        return True
    return False

def pred_and(rules,term,env):
    return [ r for r in rules.search_env(env,*term.args) ]

def pred_or(rules,term,env):
    for t in term.args:
        e=env.copy()
        q=[ r for r in rules.search_env(e,t) ]
        if q: return q
    return False

def func_anyelse(rules,term,env):
    if term.arity < 3: return term
    key = term.args[0]
    alt = term.args[-1]
    goals = [ terms.expand(g,env) for g in term.args[1:-1] ]
    for result in rules.search(*goals):
        return rules.evaluate(key,result)
    return rules.evaluate(alt,env)

def func_all(rules,term,env):
    if term.arity < 2: return term
    key = term.args[0]
    goals = [ terms.expand(g,env) for g in term.args[1:] ]
    results = [ terms.expand(key,r) for r in rules.search(*goals) ]
    return tuple(results)

def func_allset(rules,term,env):
    if term.arity < 2: return term
    key = term.args[0]
    goals = [ terms.expand(g,env) for g in term.args[1:] ]
    results = [ terms.expand(key,r) for r in rules.search(*goals) ]

    return frozenset(results)

def func_alluniq(rules,term,env):
    if term.arity < 2: return term
    key = term.args[0]
    goals = [ terms.expand(g,env) for g in term.args[1:] ]
    results = [ terms.expand(key,r) for r in rules.search(*goals) ]

    results.sort()
    if len(results) >= 2:
        i=0
        while i< len(results)-1:
            if results[i]==results[i+1]: del results[i]
            else: i=i+1

    return tuple(results)

def func_allelse(rules,term,env):
    if term.arity < 3: return term
    key = term.args[0]
    alt = term.args[-1]
    goals = [ terms.expand(g,env) for g in term.args[1:-1] ]
    results = [ terms.expand(key,r) for r in rules.search(*goals) ]
    if len(results)>0:
        return tuple(results)
    return rules.evaluate(alt,env)

def func_extend(rules,term,env):
    if term.arity<3: return ()

    var1 = terms.expand(term.args[0],env)
    list = terms.expand(term.args[1],env)
    var2 = terms.expand(term.args[2],env)
    clauses = [ terms.expand(t,env) for t in term.args[3:] ]

    if terms.is_unbound(list) or not terms.is_list(list): return ()

    r=[]

    for e in list:
        e2={}
        if not terms.unify(e,{},var1,e2): continue
        for r2 in rules.search_env(e2,*clauses):
            r.append(terms.expand(var2,r2))

    return tuple(r)

def func_filter(rules,term,env):
    if term.arity<2: return ()

    var = terms.expand(term.args[0],env)
    list = terms.expand(term.args[1],env)
    clauses = [ terms.expand(t,env) for t in term.args[2:] ]

    if terms.is_unbound(list) or not terms.is_list(list): return ()

    r=[]

    for e in list:
        e2={}
        if not terms.unify(e,{},var,e2): continue
        if rules.search_any_env(e2,*clauses) is None: continue
        r.append(terms.expand(var,e2))

    return tuple(r)

def pred_map(rules,term,env):
    if term.arity<4: return False

    var = terms.expand(term.args[0],env)
    list = terms.expand(term.args[1],env)
    var2 = terms.expand(term.args[2],env)
    list2 = terms.expand(term.args[3],env)
    clauses = [ terms.expand(t,env) for t in term.args[4:] ]

    if terms.is_unbound(list) or not terms.is_list(list): return False

    r=[]

    for e in list:
        e2={}
        if not terms.unify(e,{},var,e2): return False
        r2 = rules.search_any_env(e2,*clauses)
        if r2 is None: return False
        r.append(terms.expand(var2,r2))

    return terms.unify(tuple(r),env,list2,env)

def pred_applies(rules,term,env):
    if term.arity<2: return False

    var = terms.expand(term.args[0],env)
    list = terms.expand(term.args[1],env)
    clauses = [ terms.expand(t,env) for t in term.args[2:] ]

    if terms.is_unbound(list) or not terms.is_list(list): return False

    for e in list:
        e2={}
        if not terms.unify(e,{},var,e2): return False
        if rules.search_any_env(e2,*clauses) is None: return False

    return True

def pred_last(rules,term,env):
    if term.arity!=3: return False

    left = terms.expand(term.args[0],env)
    middle = terms.expand(term.args[1],env)
    right = terms.expand(term.args[2],env)

    if terms.is_bound(left):
        if not terms.is_list(left) or not left:
            return False
        env_copy = env.copy()
        if not terms.unify(left[-1],env,middle,env_copy):
            return False
        if not terms.unify(left[:-1],env,right,env_copy):
            return False
        return [env_copy]

    if terms.is_unbound(middle) or terms.is_unbound(right) or not terms.is_list(right):
        return False

    l2 = right+(middle,)
    return terms.unify(l2,env,left,env)
        
def pred_extract(rules,term,env):
    if term.arity!=3: return False

    left = terms.expand(term.args[0],env)
    middle = terms.expand(term.args[1],env)
    right = terms.expand(term.args[2],env)

    if terms.is_unbound(middle):
        return False

    if not terms.is_list(middle): return False

    if terms.is_bound(right):
        # l,m,r const
        if not terms.is_list(right): return False
        if terms.is_bound(left):
            for (i,e) in enumerate(middle):
                if e==left:
                    rl=middle[0:i]+middle[i+1:]
                    return rl==right
            return False
        # r,m const l variable
        r = []
        for (i,e) in enumerate(middle):
            env_copy = env.copy()
            if terms.unify(e,env,left,env_copy):
                rl=middle[0:i]+middle[i+1:]
                if rl==right:
                    r.append(env_copy)
        return r

    # right variable
    if terms.is_unbound(left):
        # r,l var m const
        r = []
        for (i,e) in enumerate(middle):
            env_copy = env.copy()
            if terms.unify(e,env,left,env_copy):
                rl=middle[0:i]+middle[i+1:]
                if terms.unify(rl,env,right,env_copy):
                    r.append(env_copy)
        return tuple(r)

    # l,m const r var
    r = []
    for (i,e) in enumerate(middle):
        if e==left:
            rl=middle[0:i]+middle[i+1:]
            env_copy = env.copy()
            if terms.unify(rl,env,right,env_copy):
                r.append(env_copy)

    return tuple(r)
        
def pred_in(rules,term,env):
    if term.arity!=2: return False

    right = terms.expand(term.args[1],env)
    if terms.is_unbound(right) or not terms.is_list(right): return False

    left = terms.expand(term.args[0],env)

    if terms.is_bound(left):
        return left in right

    r = []
    for e in right:
        env_copy = env.copy()
        if terms.unify(e,env,term.args[0],env_copy):
            r.append(env_copy)

    return r
        
def pred_reverse(rules,term,env):
    if term.arity!=2: return False

    left_term = terms.expand(term.args[0],env)
    right_term = terms.expand(term.args[1],env)

    if terms.is_bound(left_term) and terms.is_bound(right_term):
        if not terms.is_list(left_term): return False
        left_list = list(left_term)
        left_list.reverse()
        left_term = tuple(left_list)
        return left_term == right_term

    if terms.is_bound(left_term):
        if not terms.is_list(left_term): return False
        left_list = list(left_term)
        left_list.reverse()
        left_term = tuple(left_list)
        return terms.unify(left_term,{},right_term,env)

    if terms.is_bound(right_term):
        if not terms.is_list(right_term): return False
        right_list = list(right_term)
        right_list.reverse()
        right_term = tuple(right_list)
        return terms.unify(right_term,{},left_term,env)

    return False

def func_sort(rules,term,env):
    if term.arity != 1: return term
    term = rules.evaluate(term.args[0],env)
    if term is None: return term
    term = list(term)
    term.sort()
    return tuple(term)
        
def func_uniq(rules,term,env):
    if term.arity != 1: return term
    l = rules.evaluate(term.args[0],env)
    if not terms.is_list(l): return term

    r=[]
    s=set()

    for e in l:
        if e not in s:
            r.append(e)
            s.add(e)

    return tuple(r)
