#!/usr/bin/env python
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


import unittest

import engine
import terms
import parse
import builtin
import fixture
from shortcuts import *

PR = parse.parse_rule
PT = parse.parse_term
PC = parse.parse_clause
PCMD = parse.parse_command
PTL = parse.parse_termlist
PRL = parse.parse_rulelist
RE = terms.render_result

class empty_test(fixture.Fixture):

    def __init__(self,*a,**k):
        fixture.Fixture.__init__(self,None,*a,**k)

    def test_empty(self):
        self.checkFalse('@nonempty([])')
        self.checkTrue('@nonempty([a])')

class meta_test(fixture.Fixture):

    def __init__(self,*a,**k):
        fixture.Fixture.__init__(self,None,*a,**k)

    def test_unboundleft(self):
        self.checkFalse('@meta(A,B,C)')
        self.checkFalse('@meta(A,andrew,C)')
        self.checkFalse('@meta(A,B,[])')
        self.checkFalse('@meta(A,andrew,[A,B,C])')
        self.checkFalse('@meta(A,andrew,[owen,B])')

        self.checkResult('@meta(A,andrew,[])',A='andrew()')
        self.checkResult('@meta(A,andrew,[owen])',A='andrew(owen)')
        self.checkResult('@meta(A,andrew,[owen,jim])',A='andrew(owen,jim)')
        self.checkResult('@meta(andrew(A,B),andrew,[owen,jim])',A='owen',B='jim')

    def test_boundleft(self):
        self.checkFalse('@meta(andrew(owen,jim),andy,C)')
        self.checkFalse('@meta(andrew(owen,jim),andrew,[])')
        self.checkTrue('@meta(andrew(owen,jim),andrew,[owen,jim])')
        self.checkResult('@meta(andrew(owen,jim),A,[owen,jim])',A='andrew')
        self.checkResult('@meta(andrew(owen,jim),A,[B,C])',A='andrew',B='owen',C='jim')
        self.checkResult('@meta(andrew(owen,jim),A,C)',A='andrew',C='[owen,jim]')

class compare_test(unittest.TestCase):

    def test_clause_cmp(self):
        self.failUnless(PC('1') < PC('2'))
        self.failUnless(PC('2') > PC('1'))
        self.failUnless(PC('1') == PC('1'))
        self.failUnless(PC('2') == PC('2'))
        self.failUnless(PC('2') != PC('1'))
        self.failUnless(PC('1') != PC('2'))

    def test_term_pred_cmp(self):
        self.failUnless(PT('a1()') < PT('a2()'))
        self.failUnless(PT('a2()') > PT('a1()'))
        self.failUnless(PT('a1()') == PT('a1()'))
        self.failUnless(PT('a2()') == PT('a2()'))
        self.failUnless(PT('a1()') != PT('a2()'))
        self.failUnless(PT('a2()') != PT('a1()'))

    def test_term_arg1_cmp(self):
        self.failUnless(PT('a(1)') < PT('a(2)'))
        self.failUnless(PT('a(2)') > PT('a(1)'))
        self.failUnless(PT('a(1)') == PT('a(1)'))
        self.failUnless(PT('a(2)') == PT('a(2)'))
        self.failUnless(PT('a(1)') != PT('a(2)'))
        self.failUnless(PT('a(2)') != PT('a(1)'))

    def test_term_arg2_cmp(self):
        self.failUnless(PT('a(1,1)') < PT('a(1,2)'))
        self.failUnless(PT('a(1,2)') > PT('a(1,1)'))
        self.failUnless(PT('a(1,1)') == PT('a(1,1)'))
        self.failUnless(PT('a(1,2)') == PT('a(1,2)'))
        self.failUnless(PT('a(1,1)') != PT('a(1,2)'))
        self.failUnless(PT('a(1,2)') != PT('a(1,1)'))

    def test_term_deep_cmp(self):
        self.failUnless(PT('a(1,b(1))') < PT('a(1,b(2))'))
        self.failUnless(PT('a(1,b(2))') > PT('a(1,b(1))'))
        self.failUnless(PT('a(1,b(1))') == PT('a(1,b(1))'))
        self.failUnless(PT('a(1,b(2))') == PT('a(1,b(2))'))
        self.failUnless(PT('a(1,b(1))') != PT('a(1,b(2))'))
        self.failUnless(PT('a(1,b(2))') != PT('a(1,b(1))'))

class render_test(unittest.TestCase):

    def setUp(self):
        self.const = 'andrew'
        self.variable = V('Andrew')
        self.term1 = T('andrew')
        self.term2 = T('andrew',self.const,self.variable)
        self.split = S(self.variable, self.variable)
        self.rule1 = R(self.term1)
        self.rule2 = R(self.term2,self.term2,self.term2)
        self.list0 = ()
        self.list1 = (self.const,)
        self.list2 = (self.const,self.variable)
        self.list3 = (self.const,self.variable,self.split)
        self.list4 = (self.const,self.const)

    def test_const(self):
        self.assertEquals(str(self.const),"andrew")
        
    def test_variable(self):
        self.assertEquals(str(self.variable),"Andrew")
        
    def test_term(self):
        self.assertEquals(str(self.term1),"andrew()")
        self.assertEquals(str(self.term2),"andrew(andrew,Andrew)")
        
    def test_split(self):
        self.assertEquals(str(self.split),"[Andrew|Andrew]")
        
    def test_rule(self):
        self.assertEquals(str(self.rule1),"andrew().")
        self.assertEquals(str(self.rule2),"andrew(andrew,Andrew):-andrew(andrew,Andrew),andrew(andrew,Andrew).")

    def test_list(self):
        self.assertEquals(terms.render_term(self.list0),"[]")
        self.assertEquals(terms.render_term(self.list1),"[andrew]")
        self.assertEquals(terms.render_term(self.list2),"[andrew,Andrew]")
        self.assertEquals(terms.render_term(self.list3),"[andrew,Andrew,[Andrew|Andrew]]")

class list_test(unittest.TestCase):

    def setUp(self):
        self.term1 = PC('[a,b,c,d,e]')
        self.term2 = PC('[f,g,h,i,j]')
        self.terme = PC('[]')
        self.list1 = ( 'a', 'b', 'c', 'd', 'e' )
        self.list2 = ( 'f', 'g', 'h', 'i', 'j' )
        self.liste = ( )

    def check_invariants(self):
        self.assertEquals(PC('[a,b,c,d,e]'),self.term1)
        self.assertEquals(PC('[f,g,h,i,j]'),self.term2)
        self.assertEquals(PC('[]'),self.terme)

    def test_length(self):
        self.assertEquals(len(self.term1),5)
        self.assertEquals(len(self.term2),5)
        self.assertEquals(len(self.terme),0)

    def test_make_decode(self):
        self.assertEquals(self.term1,self.list1)
        self.assertEquals(self.term2,self.list2)
        self.assertEquals(self.terme,self.liste)
        self.assertEquals(self.term1,self.list1)
        self.assertEquals(self.term2,self.list2)
        self.assertEquals(self.terme,self.liste)
        self.check_invariants()

    def test_join(self):
        self.assertEquals(self.term1+self.term2,(self.list1+self.list2))
        self.assertEquals(self.term1+self.term2,self.list1+self.list2)
        self.assertEquals(self.terme+self.term2,(self.liste+self.list2))
        self.assertEquals(self.terme+self.term2,self.liste+self.list2)
        self.assertEquals(self.term1+self.terme,(self.list1+self.liste))
        self.assertEquals(self.term1+self.terme,self.list1+self.liste)
        self.check_invariants()

    def test_iter(self):
        self.assertEquals(tuple([ e for e in self.term1]), self.list1)
        self.assertEquals(tuple([ e for e in self.term2]), self.list2)
        self.assertEquals(tuple([ e for e in self.terme]), self.liste)
        self.check_invariants()
        
class construction_test(unittest.TestCase):

    def setUp(self):
        self.const = 'andrew'
        self.variable = V('Andrew')
        self.term1 = T('andrew')
        self.term2 = T('andrew',self.const,self.variable)
        self.term3 = T('andrew',self.const)
        self.split = S(self.variable, self.variable)
        self.rule1 = R(self.term1)
        self.rule2 = R(self.term2,self.term2,self.term2)
        self.list0 = ()
        self.list1 = (self.const,)
        self.list2 = (self.const,self.variable)
        self.list3 = (self.const,self.variable,self.split)
        self.list4 = (self.const,self.const)

    def check_list0(self,list):
        self.assertEquals(True, terms.is_list(list))
        self.assertEquals(True, terms.is_bound(list))
        self.assertEquals(False, terms.is_unbound(list))
        self.assertEquals(True, terms.is_list(list))
        self.assertEquals(False, terms.is_split(list))

    def check_list1(self,list,e1):
        self.assertEquals(terms.is_bound(e1), terms.is_bound(list))
        self.assertEquals(tuple, list.__class__)
        self.assertEquals(1, len(list))
        self.assertEquals(e1, list[0])
        self.check_list0(list[1:])

    def check_list2(self,list,e1,e2):
        self.assertEquals(terms.is_bound(e1) and terms.is_bound(e2), terms.is_bound(list))
        self.assertEquals(tuple, list.__class__)
        self.assertEquals(2, len(list))
        self.assertEquals(e1, list[0])
        self.check_list1(list[1:],e2)

    def check_list3(self,list,e1,e2,e3):
        self.assertEquals(terms.is_bound(e1) and terms.is_bound(e2) and terms.is_bound(e3), terms.is_bound(list))
        self.assertEquals(tuple, list.__class__)
        self.assertEquals(3, len(list))
        self.assertEquals(e1, list[0])
        self.check_list2(list[1:],e2,e3)

    def test_const(self):
        self.assertEquals(True, terms.is_const(self.const))
        self.assertEquals(True, terms.is_bound(self.const))
        self.assertEquals(False, terms.is_variable(self.const))
        self.assertEquals(False, terms.is_term(self.const))
        self.assertEquals(False, terms.is_unbound(self.const))
        self.assertEquals(False, terms.is_list(self.const))

        self.assertEquals(True, terms.is_const(True))
        self.assertEquals(True, terms.is_bound(True))
        self.assertEquals(False, terms.is_variable(True))
        self.assertEquals(False, terms.is_term(True))
        self.assertEquals(False, terms.is_unbound(True))
        self.assertEquals(False, terms.is_list(True))

        self.assertEquals(True, terms.is_const(False))
        self.assertEquals(True, terms.is_bound(False))
        self.assertEquals(False, terms.is_variable(False))
        self.assertEquals(False, terms.is_term(False))
        self.assertEquals(False, terms.is_unbound(False))
        self.assertEquals(False, terms.is_list(False))

    def test_variable(self):
        self.assertEquals(False, terms.is_const(self.variable))
        self.assertEquals(False, terms.is_bound(self.variable))
        self.assertEquals(True, terms.is_variable(self.variable))
        self.assertEquals(False, terms.is_term(self.variable))
        self.assertEquals(True, terms.is_unbound(self.variable))
        self.assertEquals(False, terms.is_list(self.variable))

    def test_term(self):
        self.assertEquals('andrew', self.term1.pred)
        self.assertEquals('andrew', self.term2.pred)
        self.assertEquals('andrew', self.term3.pred)

        self.assertEquals(False, terms.is_const(self.term1))
        self.assertEquals(True, terms.is_bound(self.term1))
        self.assertEquals(False, terms.is_variable(self.term1))
        self.assertEquals(True, terms.is_term(self.term1))
        self.assertEquals(False, terms.is_unbound(self.term1))
        self.assertEquals(False, terms.is_list(self.term1))
    
        self.assertEquals(False, terms.is_const(self.term2))
        self.assertEquals(False, terms.is_bound(self.term2))
        self.assertEquals(False, terms.is_variable(self.term2))
        self.assertEquals(True, terms.is_term(self.term2))
        self.assertEquals(True, terms.is_unbound(self.term2))
        self.assertEquals(False, terms.is_list(self.term2))
    
        self.assertEquals(False, terms.is_const(self.term3))
        self.assertEquals(True, terms.is_bound(self.term3))
        self.assertEquals(False, terms.is_variable(self.term3))
        self.assertEquals(True, terms.is_term(self.term3))
        self.assertEquals(False, terms.is_unbound(self.term3))
        self.assertEquals(False, terms.is_list(self.term3))
    
    def test_split(self):
        self.assertEquals(True, terms.is_split(self.split))
        self.assertEquals(False, terms.is_term(self.split))
        self.assertEquals(False, terms.is_bound(self.split))
        self.assertEquals(self.variable, self.split.head)
        self.assertEquals(self.variable, self.split.tail)

    def test_rule(self):
        self.assertEquals(self.term1, self.rule1.head)
        self.assertEquals((),self.rule1.goals)
        self.assertEquals(self.term2, self.rule2.head)
        self.assertEquals((self.term2,self.term2),self.rule2.goals)

    def test_list(self):
        self.check_list0(self.list0)
        self.check_list1(self.list1, self.const)
        self.check_list2(self.list2, self.const, self.variable)
        self.check_list3(self.list3, self.const, self.variable, self.split)
        self.check_list2(self.list4, self.const, self.const)

    def test_join(self):
        self.check_list0(self.list0+self.list0)
        self.check_list1(self.list1+self.list0,self.const)
        self.check_list1(self.list0+self.list1,self.const)
        self.check_list2(self.list1+self.list1,self.const,self.const)

class parse_test(unittest.TestCase):

    def test_constant(self):
        self.assertEquals('andrew',PC('andrew'))
        self.assertEquals(1,PC('1'))
        self.assertEquals(1.1,PC('1.1'))
        self.assertEquals(True,PC('True'))
        self.assertEquals(False,PC('False'))
        self.assertEquals(None,PC('None'))
        
    def test_variable(self):
        self.assertEquals(V('Andrew'),PC('Andrew'))
        
    def test_clause(self):
        self.assertEquals('andrew',PC('andrew'))
        self.assertEquals(T('andrew'),PC('andrew()'))
        self.assertEquals(T('andrew','owen'),PC('andrew(owen)'))
        self.assertEquals(T('andrew','owen','nicholas'),PC('andrew(owen,nicholas)'))
        self.assertEquals(T('andrew','owen',V('Nicholas'),('a',S(V('B'),V('C')))),PC('andrew(owen,Nicholas,[a,[B|C]])'))
        self.assertEquals('a string',PC("'a string'"))
        self.assertEquals('a string',PC("'a string'"))
        self.assertEquals(T('andrew',True),PC("andrew(True)"))
        self.assertEquals(T('andrew',False),PC("andrew(False)"))
        self.assertEquals(T('andrew',None),PC("andrew(None)"))
        
    def test_split(self):
        self.assertEquals(S(V('A'),V('B')),PC('[A|B]'))
        
    def test_list(self):
        self.assertEquals((),PC('[]'))
        self.assertEquals(('a',),PC('[a]'))
        self.assertEquals(('a','b'),PC('[a,b]'))
        self.assertEquals(('a',V('B'),T('t','a')),PC('[a,B,t(a)]'))
        self.assertEquals((('a',V('B'),T('t','a')),),PC('[[a,B,t(a)]]'))
        
    def test_term(self):
        self.assertEquals(T('andrew'),PT('andrew()'))
        self.assertRaises(parse.ParseError,PT,'andrew')
        self.assertRaises(parse.ParseError,PT,'[]')
        self.assertRaises(parse.ParseError,PT,'Andrew')
        self.assertRaises(parse.ParseError,PT,'[A|B]')
        self.assertEquals(T('andrew','owen'),PT('andrew(owen)'))
        self.assertEquals(T('andrew',T('owen')),PT('andrew(owen())'))

    def test_clauselist(self):
        self.assertEquals((),PCL(''))

    def test_termlist(self):
        self.assertEquals((),PTL(''))
        self.assertEquals((T('andrew'),),PTL('andrew()'))
        self.assertRaises(parse.ParseError,PTL,'andrew')
        self.assertEquals((T('andrew'),T('owen')),PTL('andrew(),owen()'))

    def test_rule(self):
        self.assertEquals(R(T('andrew')), PR('andrew().'))
        self.assertEquals(R(T('andrew',V('A'))), PR('andrew(A).'))
        self.assertEquals(R(T('andrew',V('A')),T('andy',V('A'))), PR('andrew(A):-andy(A).'))
        self.assertEquals(R(T('andrew',V('A')),T('andy',V('A')),T('laidback',V('A'))), PR('andrew(A):-andy(A),laidback(A).'))
        self.assertRaises(parse.ParseError,PR,'andrew.')

    def test_rulelist(self):
        self.assertEquals((R(T('andrew')),), PRL('andrew().'))
        self.assertEquals((R(T('andrew')),R(T('owen'),T('andrew'))), PRL('andrew().owen():-andrew().'))
        self.assertEquals((),PRL(''))

    def test_command(self):
        self.assertEquals(T('assert',T('andrew')),PCMD('andrew().'))
        self.assertEquals(T('query',T('andrew')),PCMD('andrew()'))
        self.assertEquals(T('query',T('andrew'),T('owen')),PCMD('andrew(),owen()'))

    def test_insert(self):
        self.assertEquals(T('andrew','andrew'), PC('andrew(~insert)',dict(insert='andrew')))
        self.assertEquals(T('andrew','~insert'), PC('andrew("~insert")',dict(insert='andrew')))
        self.assertEquals(T('andrew','~insert'), PC("andrew('~insert')",dict(insert='andrew')))
        self.assertEquals(T('andrew','nicholas'), PC('andrew(~andrew)',dict(andrew='nicholas')))
        self.assertEquals(T('andrew','nicholaskate'), PC('andrew(~(andrew)kate)',dict(andrew='nicholas')))
        self.assertRaises(parse.ParseError,PC,'andrew(~andrew)')

    def test_nosubst(self):
        self.assertEquals('andrew(~insert)', terms.render_term(PC('andrew(~insert)',nosubst=True)))

class assert_retract_test(fixture.Fixture):

    def __init__(self,*a,**k):
        fixture.Fixture.__init__(self,None,*a,**k)

    def test_assert_twice(self):
        r1 = R(T('rule','value'))
        r2 = R(T('rule','value'))
        self.checkResults('rule(X)', [])
        self.engine.assert_rule(r1)
        self.checkResults('rule(X)', [ { 'X':'value' } ])
        self.engine.assert_rule(r2)
        self.checkResults('rule(X)', [ { 'X':'value' } ])
        self.engine.retract_rule(r2)
        self.checkResults('rule(X)', [ { 'X':'value' } ])
        self.engine.retract_rule(r1)
        self.checkResults('rule(X)', [])

class logic_test(fixture.Fixture):

    rules_test = """
      parent(jim,andrew).
      parent(joe,nicholas).
      parent(steve,jim).
      ancestor(X,Y):-parent(X,Y).
      ancestor(X,Y):-parent(X,Z),ancestor(Z,Y).
      append([], L, L).
      append([W|X],Y,[W|Z]) :- append(X,Y,Z).
      reverse([],[]).
      reverse([H|T], R) :- reverse(T, RT), append(RT, [H], R).
    """

    def __init__(self,*a,**k):
        fixture.Fixture.__init__(self,None,*a,**k)


    def test_fact(self):
        self.checkTrue('parent(jim,andrew)')
        self.checkFalse('parent(jim,nicholas)')
        self.checkResults('parent(X,Y)', [ { 'X': 'jim', 'Y': 'andrew' }, { 'X': 'joe', 'Y': 'nicholas' }, { 'X': 'steve', 'Y': 'jim' } ])

    def test_derived(self):
        self.checkResults('ancestor(X,Y)', [ { 'X': 'jim', 'Y': 'andrew' }, { 'X': 'joe', 'Y': 'nicholas' }, { 'X': 'steve', 'Y': 'jim' }, { 'X': 'steve', 'Y': 'andrew' } ])
        self.checkResults('ancestor(steve,Y)', [ { 'Y': 'jim' }, { 'Y': 'andrew' } ])
        self.checkTrue('ancestor(steve,andrew)')
        self.checkFalse('ancestor(steve,nicholas)')

    def test_complicated(self):
        self.checkResults('append([],[],A)', [{ 'A': '[]' }])
        self.checkResults('append([a],[],A)', [{ 'A': '[a]' }])
        self.checkResults('append([],[a],A)', [{ 'A': '[a]' }])
        self.checkResults('append([a],[b],A)', [{ 'A': '[a,b]' }])
        self.checkResults('append([b],[a],A)', [{ 'A': '[b,a]' }])
        self.checkResults('append([a,b],[c,d],A)', [{ 'A': '[a,b,c,d]' }])
        self.checkTrue('append([a,b],[c,d],[a,b,c,d])')
        self.checkFalse('append([a,b],[c,d],[d,c,b,a])')
        self.checkResults('reverse([],R)', [{ 'R': '[]' }])
        self.checkResults('reverse([a,b,c,d],R)', [{ 'R': '[d,c,b,a]' }])
        self.checkTrue('reverse([a,b,c,d],[d,c,b,a])')
        self.checkFalse('reverse([a,b,c,d],[a,b,c,d])')
        self.checkTrue('reverse([a],[a])')

class builtin_test(fixture.Fixture):

    rules_test = """
      parent(jim,andrew).
      parent(joe,nicholas).
      parent(steve,jim).
      ancestor(X,Y):- parent(X,Y).
      ancestor(X,Y):-parent(X,Z),ancestor(Z,Y).
      anc(Child,Ancestors):-@is(Ancestors,$all(Ancestor,ancestor(Ancestor,Child))).
      anc2(C1,C2,Ancestors):-@is(Ancestors,$all(Ancestor,ancestor(Ancestor,C1),ancestor(Ancestor,C2))).
    """

    def __init__(self,*a,**k):
        fixture.Fixture.__init__(self,None,*a,**k)

    def test_is(self):
        self.checkResults('@is(1,1)', [{}])
        self.checkResults('@is(0,1)', [])
        self.checkResults('@is(a,a)', [{}])
        self.checkResults('@is(a(b),a(b))', [{}])
        self.checkResults('@is(a,b)', [])
        self.checkResults('@is(R,1)', [{'R':'1'}])
        self.checkResults('@is(R,a)', [{'R':'a'}])
        self.checkResults('@is(R,V)', [])
        self.checkResults('@is(1,V)', [{'V':'1'}])
        self.checkResults('@is([a,b],[a,b])', [{}])
        self.checkResults('@is([H|T],[a,b,c])', [{'H':'a','T':'[b,c]'}])
        
    def test_lt(self):
        self.checkResults('@is(True,$lt(1,3))',[{}])
        self.checkResults('@is(False,$lt(3,1))',[{}])
        self.checkResults('@is(True,$lt(1,a))',[{}])
        self.checkResults('@is(False,$lt(a,1))',[{}])
        self.checkResults('@is(False,$lt(a,a))',[{}])
        self.checkResults('@is(Truth,$lt(1,3))',[{'Truth':'True'}])
        self.checkResults('@is(Truth,$lt(3,1))',[{'Truth':'False'}])

    def test_any_pred(self):
        self.checkResults('@any(ancestor(A,andrew))', [{'A':'jim'}])
        self.checkResults('@any(gronk(A,andrew))', [])

    def test_or(self):
        self.checkResults('@or(ancestor(A,andrew),ancestor(A,jim))', [{'A':'jim'},{'A':'steve'}])
        self.checkResults('@or(ancestor(A,fred),ancestor(A,andrew))', [{'A':'jim'},{'A':'steve'}])
        self.checkTrue('@or(ancestor(jim,andrew),ancestor(fred,jim))')
        self.checkTrue('@or(ancestor(jim,fred),ancestor(steve,jim))')
        self.checkFalse('@or(ancestor(jim,fred),ancestor(fred,jim))')

    def test_any_func(self):
        self.checkResults('@is(Any,$any(A,ancestor(A,andrew)))', [{'Any':'jim'}])
        self.checkResults('@is(Any,$any(A,gronk(A,andrew)))', [])

    def test_all(self):
        self.checkResults('@is(All,$all(A,ancestor(A,andrew)))', [{'All':'[jim,steve]'}])
        self.checkResults('@is(All,$all(A,gronk(A,andrew)))', [{'All':'[]'}])
        self.checkResults('@is([jim,steve],$all(A,ancestor(A,andrew)))', [{}])
        self.checkResults('@is([jim],$all(A,ancestor(A,andrew)))', [])
        self.checkResults('anc(andrew,A)',[{'A':'[jim,steve]'}])
        self.checkResults('anc2(andrew,jim,A)',[{'A':'[steve]'}])

    def test_extend(self):
        self.checkResult('@is(Any,$sort($extend(A,[andrew,nicholas],B,ancestor(B,A))))', Any='[jim,joe,steve]')
        self.checkResult('@is(Any,$sort($extend(A,[andrew,nicholas],x(B),ancestor(B,A))))', Any='[x(jim),x(joe),x(steve)]')

    def test_applies(self):
        self.checkTrue('@applies(Q,[jim,steve],ancestor(Q,andrew))')
        self.checkFalse('@applies(Q,[jim,steve,andrew],ancestor(Q,andrew))')
        self.checkTrue('@applies(Q,[jim,steve],ancestor(Q,A))')
        self.checkFalse('@applies(Q,[jim,steve,andrew],ancestor(Q,A))')
        self.checkFalse('@applies(Q,[jim,X,andrew],ancestor(Q,andrew))')
        self.checkFalse('@applies(jim,[jim,steve],ancestor(Q,A))')

    def test_map(self):
        self.checkResult('@map(I,[andrew,nicholas],O,OLIST,parent(O,I))', OLIST='[jim,joe]')
        self.checkTrue('@map(I,[andrew,nicholas],O,[jim,joe],parent(O,I))')
        self.checkFalse('@map(I,[andrew,nicholas],O,[joe,jim],parent(O,I))')
        self.checkFalse('@map(I,UNBOUND,O,[joe,jim],parent(O,I))')

    def test_filter(self):
        self.checkResults('@is(F,$filter(Q,[jim,steve,andrew,joe],ancestor(Q,andrew)))', [{'F':'[jim,steve]'}])
        self.checkTrue('@is([jim,steve],$filter(Q,[jim,steve,andrew,joe],ancestor(Q,andrew)))')
        self.checkTrue('@is([],$filter(Q,[jim,steve,andrew,joe],ancestor(Q,billgates)))')

    def test_ifthenelse(self):
        self.checkTrue('@is(yes,$ifthenelse(ancestor(jim,andrew),yes,no))')
        self.checkTrue('@is(no,$ifthenelse(ancestor(jim,nicholas),yes,no))')
        self.checkTrue('@is(jim,$ifthenelse(ancestor(A,andrew),A,no))')
        self.checkTrue('@is(no,$ifthenelse(ancestor(A,bastard),A,no))')

    def test_allelse(self):
        self.checkResults('@is(N,$allelse(A,ancestor(A,andrew),gronk))',[{'N':'[jim,steve]'}])
        self.checkResults('@is(N,$allelse(A,ancestor(A,steve),gronk))',[{'N':'gronk'}])

    def test_anyelse(self):
        self.checkResult('@is(Any,$anyelse(A,ancestor(A,andrew),gronk))', Any='jim')
        self.checkResult('@is(Any,$anyelse(A,gronk(A,andrew),gronk))', Any='gronk')

    def test_fail(self):
        self.checkFalse('@fail()')

    def test_in(self):
        self.checkTrue('@in(a,[a])')
        self.checkTrue('@in(a,[a,b])')
        self.checkTrue('@in(a,[b,a])')
        self.checkFalse('@in(a,[])')
        self.checkFalse('@in(a,[c])')
        self.checkFalse('@in(a,[c,d])')
        self.checkFalse('@in(A,[])')
        self.checkResults('@in(A,[a])', [{'A':'a'}])
        self.checkResults('@in(A,[a,b])', [{'A':'a'},{'A':'b'}])
        self.checkResults('@in(A,[b,a])', [{'A':'b'},{'A':'a'}])
        self.checkResults('@in(a(A),[a(a),b(a),a(c)])', [{'A':'a'},{'A':'c'}])
        self.checkResults('@in(a(A,B),[a(a,b),b(a),a(c,d)])', [{'A':'a','B':'b'},{'A':'c','B':'d'}])

    def test_extract(self):
        self.checkFalse('@extract(b,UNBOUND,[a,c])')
        self.checkTrue('@extract(b,[a,b,c],[a,c])')
        self.checkResults('@extract(E,[a,b,c],R)',[ {'E':'a','R':'[b,c]'}, {'E':'b','R':'[a,c]'}, {'E':'c','R':'[a,b]'} ])
        self.checkResults('@extract(E,[a,b,c],[a,c])',[{'E':'b'}])
        self.checkResults('@extract(E,[a,b,c],[a,d])',[])
        self.checkResults('@extract(b,[a,b,c],R)',[{'R':'[a,c]'}])
        self.checkResults('@extract(b,[a,b,c,b],R)',[{'R':'[a,c,b]'},{'R':'[a,b,c]'}])

    def test_reverse(self):
        self.checkResults('@reverse([],[])',[{}])
        self.checkResults('@reverse([a],[a])',[{}])
        self.checkResults('@reverse([a,b],[a,b])',[])
        self.checkResults('@reverse([a,b],[b,a])',[{}])
        self.checkResults('@reverse([b,a],[a,b])',[{}])
        self.checkResults('@reverse([a,b],R)',[{'R':'[b,a]'}])
        self.checkResults('@reverse([b,a],R)',[{'R':'[a,b]'}])
        self.checkResults('@reverse(R,[b,a])',[{'R':'[a,b]'}])
        self.checkResults('@reverse([H|T],[c,b,a])',[{'H':'a','T':'[b,c]'}])

    def test_last(self):
        self.checkTrue('@last([1,2,3,4],4,[1,2,3])')
        self.checkResult('@last(L,4,[1,2,3])',L='[1,2,3,4]')
        self.checkResult('@last([H|T],4,[1,2,3])',H='1',T='[2,3,4]')
        self.checkResult('@last([1,2,3,4],L,R)',L='4',R='[1,2,3]')
        self.checkResult('@last([1,2,3,4],4,R)',R='[1,2,3]')
        self.checkResult('@last([1,2,3,4],L,[1,2,3])',L='4')

    def test_sort(self):
        self.checkResults('@is([],$sort([]))',[{}])
        self.checkResults('@is([a],$sort([a]))',[{}])
        self.checkResults('@is([a,b],$sort([a,b]))',[{}])
        self.checkResults('@is([b,a],$sort([a,b]))',[])
        self.checkResults('@is([a,b],$sort([b,a]))',[{}])
        self.checkResults('@is(R,$sort([a,b]))',[{'R':'[a,b]'}])
        self.checkResults('@is(R,$sort([b,a]))',[{'R':'[a,b]'}])
        self.checkResults('@is([a(a),a(c)],$sort([a(c),a(a)]))',[{}])
        self.checkResults('@is([a(a(a)),a(a(c))],$sort([a(a(c)),a(a(a))]))',[{}])

    def test_uniq(self):
        self.checkResults('@is([],$uniq([]))',[{}])
        self.checkResults('@is([a],$uniq([a,a,a,a]))',[{}])
        self.checkResults('@is(U,$uniq([c,a,d,a,f,b,a,a]))',[{'U':'[c,a,d,f,b]'}])

    def test_isnot(self):
        self.checkResults('@isnot(a,b)',[{}])
        self.checkResults('@isnot(a,a)',[])
        self.checkResults('@isnot(A,a)',[])
        self.checkResults('@is(A,1),@isnot(A,1)',[])
        self.checkResults('@is(A,1),@isnot(A,2)',[{'A':'1'}])
        self.checkResults('@is(A,1),@is(B,1),@isnot(A,B)',[])
        self.checkResults('@is(A,1),@is(B,2),@isnot(A,B)',[{'A':'1','B':'2'}])

    def test_not(self):
        self.checkResults('@not(ancestor(jim,andrew))',[])
        self.checkResults('@not(ancestor(jim,nicholas))',[{}])
        self.checkResults('@not(ancestor(jim,Child))',[])
        self.checkResults('@not(ancestor(andrew,Child))',[{}])

    def test_bound(self):
        self.checkTrue('@bound(1,a(1),b(a()),c())')
        self.checkFalse('@bound(a(B),C,1,a(1))')
        self.checkFalse('@bound(a(B),C,d(D),d(e(F)))')
        self.checkFalse('@unbound(1,a(1),b(a()),c())')
        self.checkTrue('@unbound(a(B),C,1,a(1))')
        self.checkTrue('@unbound(a(B),C,d(D),d(e(F)))')

if __name__ == '__main__':
    unittest.main()
