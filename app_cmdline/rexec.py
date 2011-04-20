
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

class Finder(piw.index,async.Deferred):
    def __init__(self):
        async.Deferred.__init__(self)
        piw.index.__init__(self)
        piw.tsd_index('<language>',self)

    def index_changed(self):
        piw.index.index_changed(self)
        if self.member_count()==0: return
        name = self.member_name(0).as_string()
        self.succeeded(name)

def main():
    picross.pic_set_interrupt()
    user_default = session.get_username()
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] command')
    parser.add_option('--user',action='store',dest='user',default=user_default,help='user (%s)' % user_default)
    parser.add_option('--quiet',action='store_true',dest='quiet',default=False,help='quiet')
    parser.add_option('--ctimeout',action='store',type='int',dest='ctimeout',default=20000,help='con timeout (20000 ms)')
    parser.add_option('--rtimeout',action='store',type='int',dest='rtimeout',default=60000,help='rpc timeout (60000 ms)')

    (opts,args) = parser.parse_args(sys.argv)

    cmdline = ' '.join(args[1:])

    finder = [None]
    language = [None]
    rpc = [None]
    agent = [None]

    def rpc_ok(*a,**kw):
        if not opts.quiet: print 'rpc completed'
        language[0].original().cancel()
        picross.exit(0)

    def rpc_failed(msg):
        if not opts.quiet: print 'rpc failed: ',msg
        language[0].original().cancel()
        picross.exit(-1)

    def lang_connected():
        if not opts.quiet: print 'connected to',agent[0]
        rpc[0] = language[0].original().invoke_rpc('exec',cmdline,time=opts.rtimeout)
        rpc[0].setCallback(rpc_ok).setErrback(rpc_failed)

    def lang_discon():
        if not opts.quiet: print 'language agent not connected'
        language[0].original().cancel()

    def lang_found(name):
        agent[0] = name
        language[0] = timeout.Timeout(Connector(name),opts.ctimeout,False)
        language[0].setCallback(lang_connected).setErrback(lang_discon)
        finder[0].original().close_index()

    def lang_notfound():
        if not opts.quiet: print 'language agent not found'
        finder[0].original().close_index()

    def doexec(manager):
        finder[0] = timeout.Timeout(Finder(),opts.ctimeout,False)
        finder[0].setCallback(lang_found).setErrback(lang_notfound)
        return finder[0]

    if cmdline:
        session.run_session(doexec,user=opts.user,clock=False)
