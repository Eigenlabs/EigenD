
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

import os

def guid():
    return ''.join([ '%02x' % ord(c) for c in os.urandom(6)])

def address(tail):
    return '<%s>' % ('%s/%s' % (guid(),tail))[:26]

def address2(head,tail):
    return '<%s>' % ('%s/%s' % (head,tail))[:26]

def isguid(id):
    return id.startswith('<') and id.endswith('>')

def toguid(id):
    if isguid(id):
        return id
    return '<%s>' % (id[:26])

def split(id):
    assert isguid(id)
    x = id[1:-1].split('/',1)
    if len(x) == 1:
        return (x[0],'')
    return x
