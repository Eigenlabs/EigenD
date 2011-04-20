
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

from pi import async,rpc
from pi.logic.shortcuts import *
from plg_language import interpreter,builtin_conn

@async.coroutine('internal error')
def uninitialise_set(database,objs):
    print 'uninitialise_set',objs
    yield builtin_conn.unconnect_set(database,objs)

def autonumber_set(db,objs):
    names = {}

    name_cache = db.get_propcache('name')
    ord_cache = db.get_propcache('ordinal')
    world = db.get_propcache('props').get_idset('agent')

    for o in objs:
        p = db.find_item(o)
        n = p.names()
        n.sort()
        n = tuple(n)
        names.setdefault(n,[]).append(o)

    bases = {}

    for (n,os) in names.iteritems():
        a = world
        for nn in n:
            a = a.intersection(name_cache.get_idset(nn))
        a = a.difference(set(os))
        d = [int(o) for oo in a for o in ord_cache.get_valueset(oo)]
        bases[n]=reduce(lambda a,b: a if a>b else b,d,0)

    def co():
        for (n,os) in names.iteritems():
            x = bases[n]
            for o in os:
                x+=1
                print 'autonumber',o,n,'with',x
                yield interpreter.RpcAdapter(rpc.invoke_rpc(o,'set_ordinal',str(x)))
        
    return async.Coroutine(co(),interpreter.rpcerrorhandler)

def initialise_set(database,objs):
    print 'initialise_set',objs
    return autonumber_set(database,objs)
