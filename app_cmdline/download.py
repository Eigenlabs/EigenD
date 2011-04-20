
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

from pisession import session
from pi import files,async

import optparse
import sys
import piw
import picross
import traceback
import os

def main():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] ideal filename')
    parser.add_option('--quiet',action='store_true',dest='quiet',default=False,help='quiet')
    parser.add_option('--verbose',action='store_true',dest='verbose',default=False,help='verbose')

    (opts,args) = parser.parse_args(sys.argv)
    args = args[1:]

    if len(args) != 2:
        parser.error('wrong number of arguments')

    def failed(msg):
        if opts.verbose:
            print >>sys.stderr,'rpc failed:',msg
        picross.exit(-1)

    def succeeded(msg):
        if opts.verbose:
            print >>sys.stderr,'rpc succeeded: ',msg
        picross.exit(0)

    def startup(dummy):
        result = files.copy_file(args[0],args[1])
        result.setErrback(failed).setCallback(succeeded)
        return result

    session.run_session(startup,clock=False)
