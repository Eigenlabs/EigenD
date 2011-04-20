
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
from pi import rpc,async

import optparse
import sys
import piw
import picross
import traceback

def main():
    picross.pic_set_interrupt()
    picross.pic_init_time()

    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] id rpc arg')
    parser.add_option('--quiet',action='store_true',dest='quiet',default=False,help='quiet')
    parser.add_option('--timeout',action='store',type='int',dest='timeout',default=300000,help='rpc timeout (300000 ms)')
    parser.add_option('--verbose',action='store_true',dest='verbose',default=False,help='verbose')

    (opts,args) = parser.parse_args(sys.argv)

    if len(args) < 3:
        parser.error('wrong number of arguments')

    id = args[1]
    name = args[2]
    arg = ' '.join(args[3:])

    def coroutine():

        inv = rpc.invoke_rpc(id,name,arg,opts.timeout)
        yield inv
        yield async.Coroutine.completion(inv.status(),inv.args()[0])

    def handler(ei):
        traceback.print_exception(file=sys.stderr,*ei)
        return async.Coroutine.failure('internal error')

    def failed(msg):
        if opts.verbose:
            print >>sys.stderr,'rpc failed:',msg
        picross.exit(-1)

    def succeeded(msg):
        if opts.verbose:
            print >>sys.stderr,'rpc succeeded:',msg
        print msg
        picross.exit(0)

    def startup(dummy):
        result = async.Coroutine(coroutine(),handler)
        result.setErrback(failed).setCallback(succeeded)
        return result

    session.run_session(startup,clock=False)
