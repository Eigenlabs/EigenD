
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

from pi import resource
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

def find_release_resource(category,name):
    res_root = picross.release_resource_dir()
    reldir = os.path.join(res_root,category)

    if not resource.os_path_exists(reldir):
        return None

    filename = os.path.join(reldir,name)

    if resource.os_path_exists(filename):
        return filename

    return None


def firmware(vendor,product):
    return find_release_resource('firmware','micro.ihx')

def ez_cpucs(ez, x):
    ez.control_out(usb_type_vendor, firmware_load, cpucs_addr, 0, chr(x))

def ez_reset(ez):
    ez_cpucs(ez, 1)

def ez_run(ez):
    ez_cpucs(ez, 0)

def ez_poke(ez, addr, data):
    ez.control_out(usb_type_vendor, firmware_load, addr, 0, data)

def download(device, filename):
    code = resource.file_open(filename,'r').read()
    ez = picross.usbdevice(device, 0)
    ez_reset(ez)

    for line in code.splitlines():
        len,offset,type = int(line[1:3], 16), int(line[3:7], 16), int(line[7:9], 16)

        if type == ihx_eof:
            break

        if type != 0:
            raise RuntimeError('unexpected ihx record type %d' % type)

        hex = line[9:]
        bytes = [int(hex[x:x+2],16) for x in range(0,len*2,2)]
        ez_poke(ez, offset, ''.join(map(chr,bytes)))

    ez_run(ez)

def main():
    if len(sys.argv) < 2:
        code = firmware(vendor,product)
    else:
        code = sys.argv[1]

    def doit(s):
        print 'downloading to',s
        download(s, code)

    picross.enumerate(vendor,product,picross.make_string_functor(doit))

