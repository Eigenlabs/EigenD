
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
import urllib
import terms
import tpg

class Delegate(piw.python_delegate):
    def __init__(self, subst={}, nosubst=False):
        self.__subst = subst
        self.__nosubst = nosubst
        piw.python_delegate.__init__(self)

    def find_word(self,key):
        if self.__nosubst:
            return terms.make_subst(key)
        if key not in self.__subst:
            print 'invalid substitution keyword %s' % key
            return None
        return self.__subst[key]

    def py_make_collection(self):
        return []

    def py_add_arg(self, obj1, obj2):
        obj1.append(obj2)

    def py_make_term(self,o,c):
        return terms.make_term(o,*c)

    def py_make_list(self,c):
        return tuple(c)

    def py_make_split(self, obj1, obj2):
        return terms.make_split(obj1,obj2)

    def py_make_long(self, value):
        return value

    def py_make_float(self, value):
        return value

    def py_make_bool(self, value):
        return value

    def py_make_none(self):
        return None

    def py_make_string(self, obj):
        return urllib.unquote(obj)

    def py_make_variable(self, obj):
        return terms.make_variable(obj)

    def py_make_subst1(self, obj):
        return self.find_word(obj)

    def py_make_subst2(self, obj1, obj2):
        return self.find_word(obj1)+obj2

def p__(p,i,s,n):
    r = p(Delegate(s,n),i)
    if r is None:
        raise tpg.Error((0,0),'parsing %s' % i)
    return r

def parse_clause(input,subst={},nosubst=False): return p__(piw.parse_clause,input,subst,nosubst)
def parse_clauselist(input,subst={},nosubst=False): return p__(piw.parse_clauselist,input,subst,nosubst)
def parse_term(input,subst={},nosubst=False): return p__(piw.parse_term,input,subst,nosubst)
def parse_termlist(input,subst={},nosubst=False): return p__(piw.parse_termlist,input,subst,nosubst)
