
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
import pico_native
import sys
import time

colours={"off":0,"red":0x20,"green":0x08,"orange":0x28}

def main():

    def doit( dev ):
        kbd = pico_native.passive(dev,500)
        kbd.start()
        kbd.wait()

        for k in range(0,22): kbd.set_ledcolour(k,0)

        if len(sys.argv)==3:
            kbd.set_ledcolour(int(sys.argv[1])-1,colours[sys.argv[2]])
            print ['%x'%ord(x) for x in kbd.debug()]
            picross.exit(0)

        cyc = [0x20,0x08,0x28,0]

        for k in range(0,1000):
            kbd.set_ledcolour((k-0)%22,cyc[0])
            kbd.set_ledcolour((k-1)%22,cyc[1])
            kbd.set_ledcolour((k-2)%22,cyc[2])
            kbd.set_ledcolour((k-3)%22,cyc[3])
            time.sleep(0.5)
    
    picross.enumerate(0xbeca,0x0101,picross.make_string_functor(doit))
    picross.enumerate(0x2139,0x0101,picross.make_string_functor(doit))
