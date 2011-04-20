
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

"""
Module for talking to a standard scheduler
"""

import piw
from pi import const,proxy,node,action,logic
from logic.shortcuts import *

class SchedProxy(proxy.AtomProxy):

    monitor = set(['protocols'])

    def __init__(self, delegate = None):
        proxy.AtomProxy.__init__(self)
        self.__delegate = delegate or self
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)

    def set_address(self,address):
        self.__anchor.set_address_str(address)

    def get_address(self):
        return self.__anchor.get_address()

    def node_ready(self):
        self.__delegate.scheduler_ready()

    def node_removed(self):
        self.__delegate.scheduler_gone()

    def create_trigger(self,schema):
        assert(self.is_ready())
        return self.invoke_rpc('create_trigger',schema)

    def delete_trigger(self,trigger):
        assert(self.is_ready())
        return self.invoke_rpc('delete_trigger',action.marshal(trigger))

    def scheduler_ready(self):
        pass

    def scheduler_gone(self):
        pass

def get_constraints():
    return "[timespec([[[beat],2],[[song,beat],3],[[bar],4]])]"

def make_schema(at,until,every,delta=0,prefix=None,desc=None):
    s=[]

    for (k,(q,v)) in at.items():
        if k==2: 
            at[k]=(q,v+delta)

    for (k,(q,v)) in every.items():
        if k==2 and not q: 
            every[k]=(q,v+delta)

    for (k,(q,v)) in at.items():
        s.append(T('l',k,v))
        if not k in until and not k in every:
            s.append(T('u',k,v+1))

    for (k,(q,v)) in until.items():
        s.append(T('u',k,v))

    for (k,(q,v)) in every.items():
        if q:
            if k in at:
                akv = at[k][1] % v
                s.append(T('z',k,v,akv,akv+1))
            else:
                s.append(T('z',k,v,0,1))
        else:
            s.append(T('l',k,v))
            s.append(T('u',k,v+1))

    if desc:
        text=desc
    else:
        text=describe_schema_list(s)
        if prefix:
            text=prefix+' '+text

    return action.marshal(T('schema',text,tuple(s)))

def describe_schema_list(schema):
    d=[]
    clks = ['time','bar beat','song beat','bar']

    for c in schema:
        if logic.is_pred_arity(c,'u',2):
            clk=clks[c.args[0]-1]
            val=c.args[1]
            d.append('until %s %g' % (clk,val))

        if logic.is_pred_arity(c,'l',2):
            clk=clks[c.args[0]-1]
            val=c.args[1]
            d.append('at %s %g' % (clk,val))

        if logic.is_pred_arity(c,'m',3):
            clk=clks[c.args[0]-1]
            mod=c.args[1]
            rem=c.args[2]
            d.append('every %g %s' % (mod,clk))

        if logic.is_pred_arity(c,'z',4):
            clk=clks[c.args[0]-1]
            mod=c.args[1]
            rem1=c.args[2]
            rem2=c.args[3]
            d.append('every %g %s' % (mod,clk))

    return ' '.join(d)

def describe_schema(schema):
    schema = action.unmarshal(schema)
    return describe_schema_list(schema.args[1])
