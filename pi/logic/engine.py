
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
import builtin
import parse

class Goal(object):
    __slots__ = ('rule','env','current','parent')

    def __init__(self, rule, parent, env, current):
        self.rule = rule
        self.env = env
        self.current = current
        if parent: 
            self.parent=Goal(parent.rule, parent.parent, parent.env.copy(), parent.current)
        else:
            self.parent = None

def indices(term):
    i1=None
    i2=None
    if term.arity>0 and terms.is_bound(term.args[0]):
        i1=hash(term.args[0])
    if term.arity>1 and terms.is_bound(term.args[1]):
        i2=hash(term.args[1])
    return (i1,i2)

def indexat(i,k):
    s=i.get(k)
    if not s:
        s=set()
        i[k]=s
    return s

class Ruleset(object):
    __slots__ = ('master','index1','index2','variable1','variable2','constant1','constant2')

    def __init__(self):
        self.master = {}
        self.index1 = {}
        self.index2 = {}
        self.variable1 = set()
        self.variable2 = set()
        self.constant1 = set()
        self.constant2 = set()

    def add(self,rule):
        if rule in self.master:
            self.master[rule] += 1
            return

        self.master[rule]=1
        (i1,i2) = indices(rule.head)

        if i1 is None:
            self.variable1.add(rule)
        else:
            self.constant1.add(rule)
            indexat(self.index1,i1).add(rule)

        if i2 is None:
            self.variable2.add(rule)
        else:
            self.constant2.add(rule)
            indexat(self.index2,i2).add(rule)

    def remove(self,rule):

        count = self.master[rule]
        if count > 1:
            self.master[rule]-=1
            return

        del self.master[rule]
        (i1,i2) = indices(rule.head)

        if i1 is None:
            self.variable1.remove(rule)
        else:
            self.constant1.remove(rule)
            indexat(self.index1,i1).remove(rule)

        if i2 is None:
            self.variable2.remove(rule)
        else:
            self.constant2.remove(rule)
            indexat(self.index2,i2).remove(rule)

    def find(self,term):
        (i1,i2) = indices(term)

        r1=self.variable1.copy()
        if i1 is None:
            r1.update(self.constant1)
        else:
            r1.update(self.index1.get(i1,set()))

        r2=self.variable2.copy()
        if i2 is None:
            r2.update(self.constant2)
        else:
            r2.update(self.index2.get(i2,set()))

        r1.intersection_update(r2)

        return iter(r1)

    def iter(self):
        return iter(self.master)

class Engine:
    def __init__(self):
        self.ruledict = {}
        self.predicates = {}
        self.functions = {}

        self.add_module(self)
        self.add_module(builtin)

    def iterrules(self, filter=''):
        if '/' in filter:
            ruleset = self.ruledict.get(filter)
            if ruleset:
                for r in ruleset.iter():
                    yield r
            return

        for (sig,rules) in self.ruledict.iteritems():
            if filter in sig:
                for r in rules.iter():
                    yield r

    def add_module(self,module):
        if hasattr(module,'ruleset_init'):
            module.ruleset_init(self)

        r=[ getattr(module,k) for k in dir(module) if k.startswith('rules_') ]
        p=dict([ (k[5:],getattr(module,k)) for k in dir(module) if k.startswith('pred_') ])
        f=dict([ (k[5:],getattr(module,k)) for k in dir(module) if k.startswith('func_') ])

        for r1 in r:
            self.parse_rules(r1)

        self.add_predicates(p)
        self.add_functions(f)

    def add_functions(self, builtin):
        self.functions.update(builtin)

    def add_predicates(self, builtin):
        self.predicates.update(builtin)

    def parse_rules(self,rules):
        self.assert_rules(parse.parse_rulelist(rules))

    def parse_file(self,file):
        self.assert_rules(parse.parse_rulefile(file))

    def find_rules(self, term, env):
        ruleset = self.ruledict.get(term.signature())
        if ruleset != None:
            for rule in ruleset.find(terms.expand(term,env)):
                yield rule

    def assert_rule(self, rule):
        ruleset = self.ruledict.get(rule.head.signature())
        if ruleset == None:
            ruleset = Ruleset()
            self.ruledict[rule.head.signature()] = ruleset
        ruleset.add(rule)

    def assert_rules(self, rules):
        for r in rules:
            self.assert_rule(r)

    def retract_rule(self, rule):
        ruleset = self.ruledict.get(rule.head.signature())
        if ruleset != None:
            ruleset.remove(rule)

    def retract_rules(self, rules):
        for r in rules:
            self.retract_rule(r)

    def search_any_env(self, env, *goals):
        for result in self.search_env(env,*goals):
            return result
        return None

    def search_any(self, *goals):
        for result in self.search(*goals):
            return result
        return None

    def search_key(self, key, *goals):
        for result in self.search(*goals):
            yield result.get(key)

    def search_any_key(self, key, *goals):
        result = self.search_any(*goals)

        if result is not None:
            return result.get(key)
            
        return None

    def search(self, *goals):
        return self.search_env({},*goals)

    def search_env(self, goal_env, *goals):
        queue = [Goal(terms.make_rule(terms.make_term('head'), *goals), None, goal_env, 0)]

        while queue:
            goal = queue.pop()
            if goal.current >= len(goal.rule.goals):
                if goal.parent == None:
                    yield goal.env
                else:
                    parent = goal.parent
                    terms.unify(goal.rule.head,goal.env,parent.rule.goals[parent.current],parent.env)
                    parent.current = parent.current+1
                    queue.insert(0,parent)
                continue

            term = goal.rule.goals[goal.current]

            if isinstance(term.pred,str) and term.pred.startswith('@'):
                builtin = self.predicates.get(term.pred[1:])
                if not builtin: continue

                results = builtin(self,term,goal.env)
                if results == False: continue

                goal.current = goal.current + 1
                if results == True:
                    queue.insert(0,goal)
                    continue

                for result in results:
                    queue.insert(0,Goal(goal.rule, goal.parent, result, goal.current))

                continue;

            for rule in self.find_rules(term, goal.env):
                rule_env = {}
                if self.unify(term, goal.env, rule.head, rule_env):
                    queue.insert(0,Goal(rule, goal, rule_env, 0))

    def unify(self, src, src_env, dest, dest_env):
        return terms.unify(src,src_env,dest,dest_env)

    def evaluate(self, term, env):
        if terms.is_bound(term):
            return term

        if terms.is_variable(term):
            if term.pred in env:
                return env[term.pred]
            return term

        if terms.is_split(term):
            return terms.make_split(self.evaluate(term.head,env),self.evaluate(term.tail,env))

        if terms.is_list(term):
            return tuple( self.evaluate(a,env) for a in term )

        if term.pred.startswith('$'):
            builtin = self.functions.get(term.pred[1:])
            if not builtin:
                raise ValueError('unknown function: %s' % term.pred)
            return builtin(self,term,env)

        if term.arity==0:
            return term

        args = [ self.evaluate(a,env) for a in term.args ]
        return terms.make_term(term.pred, *args)
