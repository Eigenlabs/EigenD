
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
import sys
import os
import time

ihx_eof = 1
usb_type_vendor = 0x40
firmware_load = 0xa0

# devasys
#cpucs_addr = 0x7f92
#vendor = 0x0abf
#product = 0x03ed

# cypress 7c64713 
cpucs_addr = 0xe600 
vendor = 0x04b4
product = 0x6473

new_vendor = 0x2139
new_product = 0x0001 

def find_release_resource(category,name):
    res_root = picross.release_resource_dir()
    reldir = os.path.join(res_root,category)

    if not os.path.exists(reldir):
        return None

    filename = os.path.join(reldir,name)

    if os.path.exists(filename):
        return filename

    return None

def firmware(vendor,product):
    print "using version 5 of firmware"
    return find_release_resource('firmware','pico.ihx')

def ez_cpucs(ez, x):
    ez.control_out(usb_type_vendor, firmware_load, cpucs_addr, 0, chr(x))

def ez_reset(ez):
    ez_cpucs(ez, 1)

def ez_run(ez):
    ez_cpucs(ez, 0)

def ez_poke(ez, addr, data):
    ez.control_out(usb_type_vendor, firmware_load, addr, 0, data)

def download(devicename,filename):
    code = open(filename,'r').read()
    device = picross.usbdevice(devicename, 0)

    ez_reset(device)

    for line in code.splitlines():
        len,offset,type = int(line[1:3], 16), int(line[3:7], 16), int(line[7:9], 16)

        if type == ihx_eof:
            break

        if type != 0:
            raise RuntimeError('unexpected ihx record type %d' % type)

        hex = line[9:]
        bytes = [int(hex[x:x+2],16) for x in range(0,len*2,2)]
        ez_poke(device, offset, ''.join(map(chr,bytes)))

    ez_run(device)

def main():
    if len(sys.argv) < 2:
        code = firmware(vendor,product)
    else:
        code = sys.argv[1]

    def doit(s):
        print 'downloading to',s
        download(s, code)

    picross.enumerate(new_vendor,new_product,picross.make_string_functor(doit))


