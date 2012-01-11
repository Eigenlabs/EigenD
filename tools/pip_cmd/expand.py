#!/usr/bin/python
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


import sys
import parse
import re
import string
import types
import copy

def fetch_val(dict,name):
    if type(dict)!= types.DictType or len(name) == 0:
        return None

    n = name[0]
    if not dict.has_key(n):
        return None

    v = dict[n]
    if len(name) == 1:
        return v

    return fetch_val(v,name[1:])

class expand_state:
    def __init__(self,tree_dict,parent, iteration):
        self.tree_dict = tree_dict
        self.parent = parent
        self.iteration = iteration

    def fetch_name(self,name):
        while name.startswith('parent.'):
            return self.parent.fetch_name(name[7:])

        if name == 'comma':
            if self.iteration > 0:
                return ','
            return ''

        if name == 'nl':
            return '\n'

        if name == 'index':
            return str(self.iteration)

        return fetch_val(self.tree_dict,name.split('.'))


    def macro_loop(self,arg,code):
        output=''
        var=self.fetch_name(arg)
        if var:
            if isinstance(var,types.ListType) or isinstance(var,types.TupleType):
                i=0;
                for iter in var:
                    if type(iter) is types.DictType:
                        state = expand_state(iter,self,i)
                        output += state.expand(code)
                        i+=1
        return output

    def macro_ifdef(self,arg,code):
        if self.fetch_name(arg):
            return self.expand(code)
        return ""

    def macro_ifndef(self,arg,code):
        if not self.fetch_name(arg):
            return self.expand(code)
        return ""

    def macro(self,match):
        code=match.group('code')
        tag=match.group('tag')
        arg=match.group('arg')
        if tag.startswith('loop'): return self.macro_loop(arg,code)
        if tag.startswith('ifdef'): return self.macro_ifdef(arg,code)
        if tag.startswith('ifndef'): return self.macro_ifndef(arg,code)
        return ""

    def variable(self,match):
        var=match.group('var')
        val=self.fetch_name(var)
        if val: return val
        return ""

    def expand(self,text):
        text = re.sub(r'(?s)<<(?P<tag>[a-zA-Z0-9_\.]+)(\((?P<arg>[a-zA-Z_0-9\.]+)\))?{(?P<code>.*?)}(?P=tag)>>',self.macro,text)
        text = re.sub(r'<<(?P<var>[a-zA-Z0-9_\.]+)>>',self.variable,text)
        return text

def expand(text,tree_dict):
    state = expand_state(tree_dict,None,0)
    return state.expand(text)
