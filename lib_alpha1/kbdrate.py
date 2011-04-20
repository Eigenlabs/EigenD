
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

from lib_alpha1 import passive
from time import time,sleep
from picross import enumerate, make_string_functor, find

def get_rate(k):
    c1=k.count()
    t1=time()
    sleep(1)
    c2=k.count()
    t2=time()
    return (c2-c1)/(t2-t1)

def cli():
    p=find(0x049f,0x505a)

    print "measuring pot %s" % p

    k=passive(p,1)
    k.start()
    k.set_rawkeyrate(1)

    while True:
        print "rate=%f" % get_rate(k)
