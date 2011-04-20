
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


# usage: python keypress.py pressurestart pressureend rollstart rollend yawstart yawend time(ms)

import sys

pstart = float(sys.argv[1])
pend = float(sys.argv[2])
rstart = float(sys.argv[3])
rend = float(sys.argv[4])
ystart = float(sys.argv[5])
yend = float(sys.argv[6])
time = float(sys.argv[7])

steps = int(time*2)
pinc = (pend-pstart)/(steps-1)
rinc = (rend-rstart)/(steps-1)
yinc = (yend-ystart)/(steps-1)

pcurrent = pstart
rcurrent = rstart
ycurrent = ystart
for x in range(0,steps):
    print '%d %d %d'%(pcurrent,rcurrent,ycurrent)
    pcurrent += pinc
    rcurrent += rinc
    ycurrent += yinc


