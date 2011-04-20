
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
import time
import optparse


def decode_mcs(file):
    code = []
    for l in open(file,'U').readlines():
        if len(l)==44:
            code.extend([ int(l[2*x+9:2*x+11],16) for x in range(0,16) ])
    return ''.join(map(chr,code))


def load_firmware(dev,fw):
    device = picross.usbdevice(dev,0)
    code = decode_mcs(fw)
    load(device,code)
    verify(device,code)
    fpga_reboot(device)


def code_iterator(code):
    index = 0
    remain = len(code)
    paddr = 0
    while remain>0:
        progress = (100*index)/len(code)
        for baddr,length in enumerate((64,64,64,64,8)):
            length = min(length,remain)
            block = code[index:index+length]
            yield paddr,baddr,block,length,progress
            remain -= length
            index += length
        time.sleep(0.01)
        paddr += 1


def message(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()


def _hex(bytes):
    return ' '.join(map(lambda x:hex(ord(x))[2:].zfill(2),bytes))


def load(device,code):
    for paddr,baddr,block,blen,prog in code_iterator(code):
        message('load progress: %d\r' % prog)
        device.control_out(0x40,0xc9,paddr,baddr,block+'\0'*(64-blen))
    message('load done.          \n')


class VerifyError(RuntimeError):
    pass

def verify0(got,exp,blen):
    if got[:blen]!=exp[:blen]:
        message('\nexpected (p%d b%d): %s' % (paddr,baddr,_hex(exp)))
        message('\ngot (p%d b%d): %s\n' % (paddr,baddr,_hex(got)))
        raise VerifyError()

def verify(device,code):
    for paddr,baddr,block,blen,prog in code_iterator(code):
        got=device.control_in(0x40|0x80,0xca,paddr,baddr,64)
        message('verify progress: %d\r' % prog)
        verify0(got,block,blen)
    message('verify done.          \n')


def fpga_reboot(device):
    message('rebooting FPGA\r')
    time.sleep(0.1)
    device.control_out(0x40,0xcb,0,0,'')
    message('\ndone.\n')


def main():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options]')
    parser.add_option('-f','--firmware',action='store',dest='firmware',default='',help='firmware file to load')
    parser.add_option('-S','--serial',action='store',dest='serial',default=0,help='serial number')
    parser.add_option('-v','--vid',action='store',dest='vendor',default=0x2139,help='vendor ID')
    parser.add_option('-p','--pid',action='store',dest='product',default=0x0002,help='product ID')

    (opts,args) = parser.parse_args(sys.argv)

    dev = picross.find(opts.vendor,opts.product)

    if opts.firmware:
        load_firmware(dev,opts.firmware)

    if opts.serial:
        set_serial(dev,opts.serial)

