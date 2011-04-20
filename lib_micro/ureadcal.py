
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

from picross import find
from micro_native import passive
import sys

def cli():
    kbd=passive(find(0xbeca, 0x0101),20)

    for k in range(0,18):
        for c in range(0,4):
            sys.stdout.write('%d %d ' % (k,c))
            sys.stdout.flush()
            kbd.start_calibration_row(k,c)
            kbd.read_calibration_row()
            mn = kbd.get_calibration_min()
            mx = kbd.get_calibration_max()
            pts = [ kbd.get_calibration_point(p) for p in range(0,30) ]
            sys.stdout.write('%d %d %s\n' % (mn,mx,' '.join(map(str,pts))))
            sys.stdout.flush()





