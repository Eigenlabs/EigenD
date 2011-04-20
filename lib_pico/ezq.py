
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
import optparse

def main():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options]')
    parser.add_option('--req',action='store',type='int',dest='req',default=0xc0,help='request')
    parser.add_option('--val',action='store',type='int',dest='val',default=0,help='value')
    parser.add_option('--idx',action='store',type='int',dest='idx',default=0,help='index')
    parser.add_option('--len',action='store',type='int',dest='len',default=0,help='number of bytes to return')

    (opts,args) = parser.parse_args(sys.argv)

    def query(s):
        print 'querying',s
        ez = picross.usbdevice(s,0)

        if opts.len>0:
            r = ez.control_in(0x40|0x80,opts.req,opts.val,opts.idx,opts.len)
            print ' '.join(['%x'%ord(x) for x in r])

        else:
            ez.control(0x40,opts.req,opts.val,opts.idx)


    picross.enumerate(0xbeca,0x0101,picross.make_string_functor(query))
    picross.enumerate(0x2139,0x0101,picross.make_string_functor(query))

