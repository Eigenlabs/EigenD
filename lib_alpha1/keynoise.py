
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
import picross
import lib_alpha1
import math

def cli():
    pot=picross.find(0x049f,0x505a)
    samples = 1000

    key=4

    print "measuring pot %s key %d samples %d" % (pot,key,samples)

    kbd=lib_alpha1.passive(pot)
    kbd.start()
    kbd.set_rawkeyrate(1)

    sensors = [ key*4+i for i in (0,1,2,3) ]

    bins = [ dict() for s in sensors ]
    minv = [ 100000 for s in sensors ]
    maxv = [ 0 for s in sensors ]
    maxt = [ 0 for s in sensors ]

    for i in xrange(samples):
        kbd.wait()

        for (i,s) in enumerate(sensors):
            value = kbd.get_rawkey(s/4,s%4)
            total = bins[i].get(value,0)+1
            maxv[i] = max(maxv[i],value)
            minv[i] = min(minv[i],value)
            maxt[i] = max(maxt[i],total)
            bins[i][value] = total

    for (i,s) in enumerate(sensors):
        print "key %d sensor %d min %d max %d range %d" % (s/4,s%4,minv[i],maxv[i],maxv[i]-minv[i])

        range = bins[i].keys()
        range.sort()

        for k in range:
            v = bins[i][k]
            r = int(60.0*v/maxt[i])
            print '%04d %4d %s' % (k,v,''.center(60,'*')[0:r])
