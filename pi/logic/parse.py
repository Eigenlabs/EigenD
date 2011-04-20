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


import terms
import tpg
import urllib
import threading
import parse_new

class PrologParser(tpg.Parser):
    r"""
        separator space '\s+' ;
        separator comment '#.*\n' ;

        token SQSTRING    '"[^"]*"'                                 {{ lambda x: urllib.unquote(x[1:-1]) }}
        token DQSTRING    "'[^']*'"                                 {{ lambda x: urllib.unquote(x[1:-1]) }}
        token WORD        '[\!\$@a-z#<>][A-Za-z0-9#<>_\.]*';
        token VARIABLE    '[A-Z_][A-Za-z0-9_]*';
        token NUMBER      '[+\-0-9\.][+\-eE0-9\.]*';

        START/e           -> clause/e ;

        clause/r          -> VARIABLE/p		                        {{ r = self.make_variable(p) }}
                          |  term/t                                 {{ r = t }}
                          |  WORD/p		                            {{ r = p }}
                          |  '~' WORD/p		                        {{ r = self.find_word(p) }}
                          |  '~' '\(' WORD/p '\)' WORD/s                  {{ r = self.find_word(p)+s }}
                          |  '~' '\(' WORD/p '\)' NUMBER/s                {{ r = self.find_word(p)+s }}
                          |  '~' '\(' WORD/p '\)' VARIABLE/s              {{ r = self.find_word(p)+s }}
                          |  '~' '\(' WORD/p '\)' SQSTRING/s              {{ r = self.find_word(p)+s }}
                          |  '~' '\(' WORD/p '\)' DQSTRING/s              {{ r = self.find_word(p)+s }}
                          |  NUMBER/p		                        {{ r = self.make_number(p) }}
                          |  '%' DQSTRING/p		                    {{ r = terms.make_variable(p) }}
                          |  '%' SQSTRING/p		                    {{ r = terms.make_variable(p) }}
                          |  DQSTRING/p		                        {{ r = p }}
                          |  SQSTRING/p		                        {{ r = p }}
                          |  '\[' clause/a '\|' clause/b ']'		{{ r = terms.make_split(a,b) }}
                          |  '\[' clauselist_opt/l ']'		        {{ r = tuple(l) }}
                          ;

        term/r            -> WORD/p '\(' clauselist_opt/b '\)'	    {{ r = terms.make_term(p,*b) }}
                          ;

        rule/r            -> term/h ':-' termlist_opt/g '\.'	    {{ r = terms.make_rule(h,*g) }}
                          |  term/h '\.'		                    {{ r = terms.make_rule(h) }}
                          ;

        command/c         -> rule/r                                 {{ c = terms.make_term('assert', r.head, *r.goals) }}
                          |  termlist/t                             {{ c = terms.make_term('query', *t) }}
                          ;

        clauselist_opt/l  -> {{ l=() }} clauselist/l ? ;
        clauselist/l      -> clause/e {{ l=(e,) }} ( ',' clause/e {{ l=l+(e,) }} )* ;
        termlist_opt/l    -> {{ l=() }} termlist/l ? ;
        termlist/l        -> term/e {{ l=(e,) }} ( ',' term/e {{ l=l+(e,) }} )* ;
        rulelist_opt/l    -> {{ l=() }} rulelist/l ? ;
        rulelist/l        -> {{ l=() }} ( rule/e {{ l=l+(e,) }} )+ ;

    """

    def make_number(self,word):
        try: return int(word)
        except: pass
        try: return float(word)
        except: pass
        raise tpg.Error((0,0),'invalid number %s' % word)

    def make_variable(self,word):
        if word == 'True': return True
        if word == 'False': return False
        if word == 'None': return None
        return terms.make_variable(word)

    def find_word(self,key):
        if self.nosubst:
            return terms.make_subst(key)
        if key not in self.dict:
            raise tpg.Error((0,0),'invalid substitution %s' % key)
        return self.dict[key]

    dict = {}
    nosubst = False

_tlocal = threading.local()

def _parse(start, input, dict, nosubst):
    p=getattr(_tlocal,'parser',None)
    if p is None:
        p = PrologParser()
        _tlocal.parser = p
    p.dict = dict
    p.nosubst = nosubst
    return p.parse(start,input)

def parse_rule(input,subst={},nosubst=False): return _parse('rule',input,subst,nosubst)
def parse_command(input,subst={},nosubst=False): return _parse('command',input,subst,nosubst)
def parse_rulelist(input,subst={},nosubst=False): return _parse('rulelist_opt',input,subst,nosubst)
def parse_rulefile(input,subst={},nosubst=False): return _parse('rulelist_opt',file(input).read(),subst,nosubst)

parse_clause = parse_new.parse_clause
parse_clauselist = parse_new.parse_clauselist
parse_term = parse_new.parse_term
parse_termlist = parse_new.parse_termlist

def old_parse_clause(input,subst={},nosubst=False): return _parse('clause',input,subst,nosubst)
def old_parse_clauselist(input,subst={},nosubst=False): return _parse('clauselist_opt',input,subst,nosubst)
def old_parse_term(input,subst={},nosubst=False): return _parse('term',input,subst,nosubst)
def old_parse_termlist(input,subst={},nosubst=False): return _parse('termlist_opt',input,subst,nosubst)

ParseError = tpg.Error
