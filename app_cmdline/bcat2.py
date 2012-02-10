
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

import sys

from pisession import session
from pi import async,paths,const

import optparse
import sys
import piw
import picross
import traceback

class Opener(piw.client):
    def __init__(self,address,path,flags,deferred,fast):
        piw.client.__init__(self,const.client_sync)
        piw.tsd_client(address,self,fast)
        self.__deferred = deferred
        self.__flags = flags
        self.__path = path

    def client_sync(self):
        if self.child_exists_str(self.__path,len(self.__path)):
            c=piw.client(0)
            try:
                self.child_add_str(self.__path,len(self.__path),c)
                piw.dump_client(c,self.__flags,False)
            except:
                pass

        if self.__deferred is not None:
            self.__deferred.succeeded()

def main():
    picross.pic_set_interrupt()

    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] id')
    parser.add_option('-i','--id',action='store_const',dest='id',const=0x0020,default=0,help='fast id')
    parser.add_option('-I','--idtime',action='store_const',dest='idtime',const=0x2000,default=0,help='id time')
    parser.add_option('-D','--fdata',action='store_const',dest='fdata',const=0x0040,default=0,help='fast data')
    parser.add_option('-S','--fscalar',action='store_const',dest='fscalar',const=0x0080,default=0,help='fast scalar')
    parser.add_option('-V','--fvector',action='store_const',dest='fvector',const=0x0100,default=0,help='fast vector')
    parser.add_option('-T','--ftime',action='store_const',dest='ftime',const=0x0200,default=0,help='fast time')
    parser.add_option('-d','--sdata',action='store_const',dest='sdata',const=0x0001,default=0,help='slow data')
    parser.add_option('-s','--sscalar',action='store_const',dest='sscalar',const=0x0002,default=0,help='slow scalar')
    parser.add_option('-v','--svector',action='store_const',dest='svector',const=0x0004,default=0,help='slow vector')
    parser.add_option('-t','--stime',action='store_const',dest='stime',const=0x0008,default=0,help='slow time')
    parser.add_option('-x','--flags',action='store_const',dest='flags',const=0x0800,default=0,help='flags')
    parser.add_option('-a','--address',action='store_const',dest='flags',const=0x1000,default=0,help='full address')
    parser.add_option('-M','--monitor',action='store_true',dest='monitor',default=False,help='monitor mode')

    (opt,args) = parser.parse_args(sys.argv)

    if len(args) != 2:
        parser.error('wrong number of arguments')

    id = args[1]
    flags = opt.id|opt.fdata|opt.fscalar|opt.fvector|opt.ftime|opt.sdata|opt.sscalar|opt.svector|opt.stime|opt.flags|opt.idtime

    if flags==0:
        flags=0x0001

    fast=False
    if (flags&(0x20|0x40|0x80|0x100))!=0:
        fast=True

    def coroutine():
        (a,p) = paths.breakid_list(id)
        p = ''.join(chr(c) for c in p)
        r = async.Deferred()
        c = Opener(a,p,flags,r if not opt.monitor else None,fast)
        yield r

    def handler(ei):
        traceback.print_exception(file=sys.stderr,*ei)
        return async.Coroutine.failure('internal error')

    def failed(msg):
        print msg
        picross.exit(-1)

    def succeeded():
        picross.exit(0)

    def startup(dummy):
        result = async.Coroutine(coroutine(),handler)
        result.setErrback(failed).setCallback(succeeded)
        return result

    session.run_session(startup,clock=False,rt=False)
