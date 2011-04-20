
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


# offsets in ADC read lengths from start (500/(16*6) us each)
roffsets = [ 6*(k%4) + (k/4) for k in range(0,16) ]
roffsets.append(4) # key 17 is weird
print ','.join(map(str,roffsets))

dt = 500.0/(16.0*6.0)
toffsets = [int(0.5+(roffsets[k]+36.0)*dt) for k in range(0,16)]
toffsets.append(int(0.5+(4.0+9.0)*dt)) # key 17 is weird
print ','.join(map(str,toffsets))
