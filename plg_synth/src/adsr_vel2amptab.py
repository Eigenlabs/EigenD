
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

from math import *
import sys

tablesize=100

def func(x):
    #x1=-20.0/96.0
    #x2=x1*log(x*x)
    #x3=x2/log(10.0)
    #return x3
    #return -20.0/96.0*log(f*f)/log(10.0)
    #return ((-20.0/96.0)*log(x*x))/log(10.0)
    return (-20.0/(96.0*log(10.0)))*log(x*x)

for x in range(0,tablesize):
    xx = float(x)/float(tablesize)
    f = 1.0-xx
    ff = func(1.0-xx)
    sys.stdout.write('%.10f, ' % max(0.0,min(1.0,(ff))))
    if (x+1)%8 == 0: sys.stdout.write('\n')
        
