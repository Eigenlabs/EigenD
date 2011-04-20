
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
import math

fadelength=1024
factor=math.pi/(float(fadelength)-1.0)
curve=[(1.0-math.cos(float(x)*factor))/2 for x in range(0,fadelength)]

sys.stdout.write('static float fadein__[FADELENGTH] = {\n')
for x in range(0,fadelength):
    sys.stdout.write('%.10f, ' % curve[x])
    if (x+1)%4 == 0:
        sys.stdout.write('\n')
sys.stdout.write('};\n')

sys.stdout.write('static float fadeout__[FADELENGTH] = {\n')
for x in range(0,fadelength):
    sys.stdout.write('%.10f, ' % (1.0-curve[x]))
    if (x+1)%4 == 0:
        sys.stdout.write('\n')

sys.stdout.write('};\n')
