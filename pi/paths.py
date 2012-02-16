
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

import struct
import piw
from pi import utils

def valid_id(id):
    id = id2server(id)
    return True if len(id)>2 and id.startswith('<') and id.endswith('>') else False

def make_subst(id):
    server = id2server(id)
    subst = { 'self':id, 'server':server, 'a':server, 's':id }

    id = id2parent(id)

    if id is not None:
        subst['parent']=id
        subst['p']=id
        id = id2parent(id)
        if id is not None:
            subst['grandparent']=id
            subst['pp']=id

    return subst

def id2parent(a):
    (a,p) = breakid_list(a)
    if len(p)==0:
        return None
    return makeid_list(a,*p[0:-1])

def makeid_list(server,*path):
    if not path:
        return server
    return server+'#'+'.'.join([str(p) for p in path])

def id2server(id):
    if '#' not in id:
        return id
    (a,p) = id.split('#')
    return a

def breakid_list(path):
    if '#' not in path:
        return (path,[])
    (a,p) = path.split('#')
    return (a,[int(c) for c in p.split('.') if c])

def breakid(path):
    if '#' not in path:
        return (piw.makestring(path,0),piw.pathnull(0))
    (a,p) = path.split('#')
    return (piw.makestring(a,0),piw.parsepath(p,0))

def id2child(id,*c):
    (s,p) = breakid_list(id)
    p.extend(c)
    return makeid_list(s,*p)

def to_absolute(id,scope=None):
    if '#' not in id:
        (a,p) = (id,'')
    else:
        (a,p) = id.split('#')
        
    if len(a)<3 or a[0]!='<' or a[-1] != '>':
        return id

    a2 = a[1:-1]

    if a2.find(':') >= 0:
        return id

    s = scope or piw.tsd_scope()

    if p:
        aq = '<%s:%s>#%s' % (s,a2,p)
    else:
        aq = '<%s:%s>' % (s,a2)

    return aq

def to_relative(id,scope=None):
    if '#' not in id:
        (a,p) = (id,'')
    else:
        (a,p) = id.split('#')

    a2 = a[1:-1]
    cp = a2.find(':')

    if cp < 0:
        u = piw.tsd_scope()
        n = a2
    else:
        u = a2[:cp]
        n = a2[cp+1:]

    s = scope or piw.tsd_scope()

    if u == s:
        if p:
            return '<%s>#%s' % (n,p)
        else:
            return '<%s>' % n
    else:
        if p:
            return '<%s:%s>#%s' % (u,n,p)
        else:
            return '<%s:%s>' % (u,n)


def id2scope(id,scope=None):
    return splitid(id,scope)[0]

def splitid(id,scope=None):
    if '#' not in id:
        (a,p) = (id,'')
    else:
        (a,p) = id.split('#')

    a2 = a[1:-1]
    cp = a2.find(':')

    s = scope or piw.tsd_scope()

    if cp < 0:
        return (s,a2,p)

    u = a2[:cp]
    n = a2[cp+1:]

    return (u,n,p)
