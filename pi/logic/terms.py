
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

import string

__tx = string.maketrans('','')

def make_subst(word):
    return Subst(word)

def render_termlist(tl):
    return ','.join(map(render_term,tl))

def render_result(e):
    return ','.join([ "%s=%s" % (v, render_term(e[v])) for v in e ])

def render_term(t):
    if t is None: return 'None'
    if is_term(t): return str(t)
    if is_variable(t): return str(t)
    if is_split(t): return str(t)
    if is_list(t): return '[%s]' % render_termlist(t)
    if isinstance(t,Subst): return '~%s' % t.word
    if isinstance(t,str): return quotesimplename(t)
    if isinstance(t,int): return str(t)
    if isinstance(t,bool): return str(t)
    if isinstance(t,long): return str(t)
    if isinstance(t,float): return str(t)
    if isinstance(t,Expansion): return t.pred
    return '<%s>' % repr(t)

def make_rule(h,*g):
    return Rule(h,g)

def make_term(t,*a):
    return Term(str(t),*a)

def make_expansion(p):
    return Expansion(p)

def make_variable(v):
    return Variable(str(v))

def make_split(h,t):
    if is_bound(h) and is_bound(t):
        return (h,)+t
    return Split(h,t)

def is_pred(term,pred):
    return is_term(term) and term.pred==pred

def is_pred_arity(term,pred,min,max=None):
    if is_pred(term,pred):
        if term.arity >= min:
            if max is None or term.arity<=max:
                return True
    return False

def is_term(term):
    return isinstance(term,Term)

def is_split(term):
    return isinstance(term,Split)

def is_bound(term):
    if isinstance(term,Variable): return False
    if isinstance(term,Term): return term.bound
    if isinstance(term,Split): return False
    if is_list(term):
        for i in term:
            if not is_bound(i):
                    return False
    return True

def is_unbound(term):
    return not is_bound(term)

def is_variable(term):
    return isinstance(term,Variable)

def is_atom(term):
    if isinstance(term,Variable): return False
    if isinstance(term,Term): return False
    if isinstance(term,Split): return False
    if is_list(term): return False
    return True

def is_const(term):
    if isinstance(term,Variable): return False
    if isinstance(term,Term): return False
    if isinstance(term,Split): return False
    if is_list(term): return False
    return True

def is_list(term):
    return isinstance(term,tuple)

def expand(term,env):
    if is_bound(term):
        return term

    if is_variable(term):
        if term.pred in env:
            return expand(env[term.pred],env)
        return term

    if is_split(term):
        return make_split(expand(term.head,env),expand(term.tail,env))

    if is_list(term):
        return tuple( expand(arg,env) for arg in term )

    if term.arity==0:
        return term

    args = [ expand(arg,env) for arg in term.args ]
    return make_term(term.pred, *args)

def match(left,right,env):
    env.clear()
    return unify(left,{},right,env)

def unify(src, src_env, dest, dest_env):
    while True:
        if is_variable(src):
            if not src.pred in src_env:
                return True
            src = src_env[src.pred]
            continue

        if is_variable(dest):
            if not dest.pred in dest_env:
                dest_env[dest.pred] = expand(src,src_env)
                return True
            dest = dest_env[dest.pred]
            continue

        if is_bound(src) and is_bound(dest):
            return src==dest

        if is_const(src):
            if not is_const(dest):
                return False
            return src==dest

        if is_const(dest):
            if not is_const(src):
                return False
            return src==dest

        if is_split(src):
            if is_split(dest):
                dest_copy = dest_env.copy()
                if not unify(src.head,src_env,dest.head,dest_copy): return False
                if not unify(src.tail,src_env,dest.tail,dest_copy): return False
                dest_env.update(dest_copy)
                return True
            if is_list(dest) and len(dest)>0:
                dest_copy = dest_env.copy()
                if not unify(src.head,src_env,dest[0],dest_copy): return False
                if not unify(src.tail,src_env,dest[1:],dest_copy): return False
                dest_env.update(dest_copy)
                return True
            return False

        if is_split(dest):
            if is_list(src) and len(src)>0:
                dest_copy = dest_env.copy()
                if not unify(src[0],src_env,dest.head,dest_copy): return False
                if not unify(src[1:],src_env,dest.tail,dest_copy): return False
                dest_env.update(dest_copy)
                return True
            return False

        if is_list(src):
            if not is_list(dest): return False
            if len(src) != len(dest): return False
            dest_copy = dest_env.copy()
            for (s,d) in zip(src,dest):
                if not unify(s,src_env,d,dest_copy): return False
            dest_env.update(dest_copy)
            return True

        if is_list(dest):
            return False

        if src.signature() != dest.signature():
            return False

        dest_copy = dest_env.copy()
        for i in range(src.arity) :
            if not unify(src.args[i],src_env,dest.args[i],dest_copy) : return False
        dest_env.update(dest_copy)
        return True

def quotevarname(name):
    h=name[0].translate(__tx,'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')
    t=name[1:].translate(__tx,'ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789')
    if not h and not t: return name
    return "%%'%s'" % name.replace('%','%25').replace("'","%27")

def quotesimplename(name):
    if name.startswith('~'): return name
    if name is True: return 'True'
    if name is False: return 'False'
    if isinstance(name,int): return str(name)
    if isinstance(name,long): return str(name)
    if isinstance(name,float): return str(name)
    if not isinstance(name,str): return '<%s>' % str(name)
    if not name: return "''"
    h=name[0].translate(__tx,'abcdefghijklmnopqrstuvwxyz$@!#')
    t=name[1:].translate(__tx,'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789#_.')
    if not h and not t: return name
    return "'%s'" % name.replace('%','%25').replace("'","%27")

class Expansion(object):
    __slots__ = ('pred')

    def __init__(self, pred):
        self.pred = pred

    def __str__(self):
        return self.pred

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash(self.pred)

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,Expansion): return 1
        return cmp(self.pred,other.pred)


class Variable(object):
    __slots__ = ('pred')

    def __init__(self, pred):
        self.pred = pred

    def __str__(self):
        return quotevarname(self.pred)

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash(self.pred)

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,Variable): return 1
        return cmp(self.pred,other.pred)

class Split(object):
    __slots__ = ('head','tail','hash')

    def __init__(self,head,tail):
        self.head = head
        self.tail = tail
        self.hash = hash(head)^hash(tail)

    def __str__(self):
        return '[%s|%s]' % (self.head,self.tail)

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return self.hash

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,Split): return 1
        x = cmp(self.head,other.head)
        if x != 0: return x
        return cmp(self.tail,other.tail)

class Term(object):
    __slots__ = ('pred','args','bound','hash','arity','isig')

    def __init__(self, pred, *args):
        self.pred = pred
        self.args = args
        self.bound = True
        self.hash=13

        self.isig="%s/%u" % (pred,len(args))
        self.arity=len(args)

        if pred.startswith('$'):
            self.bound = False

        for a in args:
            self.hash=self.hash^hash(a)
            if is_unbound(a):
                self.bound=False

        self.hash=self.hash^hash(pred)
        self.hash=self.hash^hash(self.bound)

    def __ne__(self,other):
        return not self.__eq__(other)

    def __eq__(self,other):
        if self is other: return True
        if not isinstance(other,Term): return False
        if self.arity != other.arity: return False
        if self.pred != other.pred: return False
        if not self.args: return True
        for (s,o) in zip(self.args,other.args):
            x = (s==o)
            if not x:
                return False
        return True

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,Term): return 1
        if self.arity < other.arity: return -1
        if self.arity > other.arity: return 1
        c = cmp(self.pred,other.pred)
        if c!=0: return c
        if not self.args: return 0
        for (s,o) in zip(self.args,other.args):
            x = cmp(s,o)
            if x != 0: return x
        return 0

    def __str__(self):
        p = self.pred
        a = self.arity
        return "%s(%s)" % (p,','.join(map(render_term,self.args)))

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return self.hash

    def signature(self):
        return self.isig

class Rule:

    __slots__ = ('head','goals','hash','count')

    def __init__(self, head, goals=()):
        self.head = head
        self.goals = goals
        self.count = 0

        h=hash(head)
        for a in goals: h=h^hash(a)
        self.hash=h

    def __hash__(self):
        return self.hash

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,Rule): return -1
        if self.head != other.head: return -1
        if len(self.goals) != len(other.goals): return -1
        for (s,o) in zip(self.goals,other.goals):
            x=cmp(s,o)
            if x != 0:
                return x
        return 0

    def __str__(self):
        if len(self.goals)==0: return "%s." % self.head
        return "%s:-%s." % (self.head,','.join(map(str,self.goals)))

    def __repr__(self):
        return self.__str__()

class Subst:
    def __init__(self,word):
        self.word=word

    def __str__(self):
        return self.word

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash(self.word)

    def __cmp__(self,other):
        if self is other: return 0
        if not isinstance(other,Subst): return 1
        return cmp(self.word,other.word)

