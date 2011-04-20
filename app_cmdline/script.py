
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
from pi import index,async,timeout,proxy

import optparse
import sys
import piw
import picross
import traceback

class Connector(proxy.AtomProxy,async.Deferred):

    monitor = set()

    def __init__(self,address):
        async.Deferred.__init__(self)
        proxy.AtomProxy.__init__(self)
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)
        self.__anchor.set_address_str(address)

    def close_client(self):
        proxy.AtomProxy.close_client(self)

    def cancel(self):
        self.__anchor.set_address_str('')
        self.__anchor.set_client(None)
        self.__anchor=None

    def node_ready(self):
        self.succeeded()

class RpcAdapter(async.DeferredDecoder):
    def decode(self):
        if self.deferred.status() is False:
            return async.Coroutine.failure(self.deferred.args()[0])
        return self.deferred.args()[0]

def coroutine(lang,script,ctimeout=3000,rtimeout=3000,verbose=True):

    connector = Connector(lang)
    timer = timeout.Timeout(connector,ctimeout,False,'cant connect to language agent')

    yield timer
    if not timer.status():
        yield async.Coroutine.failure(*timer.args())
        return

    if verbose:
        print 'connected to',lang,connector.status()

    for line in script_reader(script):
        rpc = connector.invoke_rpc('exec',line,time=rtimeout)
        yield rpc

        if not rpc.status():
            print line,'failed:',rpc.args()[0]
            return

        if verbose:
            print line,'ok'

def script_reader(fp):
    for line in fp:
        line = line.strip()
        if not line or line.startswith('#'): continue
        yield line

def open_script(name):
    if name == '-':
        return sys.stdin

    try:
        return open(name,"r")
    except:
        return None

def main():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] agent script')
    parser.add_option('--quiet',action='store_true',dest='quiet',default=False,help='quiet')
    parser.add_option('--ctimeout',action='store',type='int',dest='ctimeout',default=5000,help='con timeout (5000 ms)')
    parser.add_option('--rtimeout',action='store',type='int',dest='rtimeout',default=300000,help='rpc timeout (300000 ms)')
    parser.add_option('--verbose',action='store_true',dest='verbose',default=False,help='verbose')

    (opts,args) = parser.parse_args(sys.argv)

    if len(args) != 3:
        parser.error('wrong number of arguments')

    lang = args[1]
    script = args[2]

    fp = open_script(script)

    if fp is None:
        parser.error('cant open %s' % script)

    def handler(ei):
        traceback.print_exception(*ei)
        return async.Coroutine.failure('internal error')

    def failed(msg):
        if opts.verbose:
            print 'script failed:',msg
        picross.exit(-1)

    def succeeded():
        if opts.verbose:
            print 'script finished'
        picross.exit(0)

    def startup(dummy):
        result = async.Coroutine(coroutine(lang,fp,opts.ctimeout,opts.rtimeout,opts.verbose),handler)
        result.setErrback(failed).setCallback(succeeded)
        return result

    picross.pic_set_interrupt()
    session.run_session(startup,clock=False)
