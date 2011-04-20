
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

cal_points=30
cal_len=cal_points+4

def cli():
    file=sys.argv[1]
    if len(sys.argv) != 2:
        print >>sys.stderr, 'usage: writecal filename'
        sys.exit(1)

    k=passive(find(0xbeca, 0x0101),20)
    for line in open(file).readlines():
        words = line.split()
        assert len(words)<=cal_len
        words.extend(['0'] * (cal_len-len(words)))

        key,corner,min,max = map(int,words[:4])
        points = map(int,words[4:])

        k.start_calibration_row(key,corner)
        k.set_calibration_range(min,max);
        for i,p in enumerate(points):
            k.set_calibration_point(i,p)

        print 'writing key %d corner %d (min=%d max=%d)' % (key,corner,min,max)
        k.write_calibration_row()

    print 'committing calibration to non volatile memory'
    k.commit_calibration()

