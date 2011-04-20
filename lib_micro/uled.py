
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

import picross
import micro_native
import sys
import time

colours={"off":0,"red":0x20,"green":0x08,"orange":0x28}

def main():
    dev = picross.find(0xbeca,0x0101)
    kbd = micro_native.passive(dev,500)
    kbd.wait()

    for k in range(0,17): kbd.set_ledcolour(k,0)

    if len(sys.argv)==3:
        kbd.set_ledcolour(int(sys.argv[1])-1,colours[sys.argv[2]])
        print ['%x'%ord(x) for x in kbd.debug()]
    else:
        for k in range(0,17):
            for c in ("red","green","orange","off"):
                kbd.set_ledcolour(k,colours[c])
                print k,c,['%x'%ord(x) for x in kbd.debug()]
                time.sleep(0.5)
