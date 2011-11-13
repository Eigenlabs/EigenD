
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
from pi import async,paths,const,policy

import optparse
import sys
import piw
import picross
import traceback

class Plumber:
    def __init__(self,name,address,abs,wav):
        self.__cdomain = piw.clockdomain_ctl()
        self.__backend = piw.capture_backend(name,1,abs,wav)
        self.__correlator = piw.correlator(self.__cdomain,chr(1),piw.null_filter(),self.__backend.cookie(),0,0)
        self.__filter = piw.signal_cnc_filter(0,0)
        self.__plumber = policy.Plumber(self.__correlator,1,1,-1,policy.Plumber.input_input,policy.AnisoStreamPolicy(),address,self.__filter,False,self.__connected)

    def __connected(self,plumber):
        print 'connected to',plumber.id(),plumber.domain()

def main():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] tag=id ..')
    parser.add_option('-n','--name',action='store',dest='name',default='',help='file name')
    parser.add_option('-a','--absolute',action='store_true',dest='abs',default=False,help='absolute timestamps')
    parser.add_option('-r','--wav',action='store_true',dest='wav',default=False,help='create .wav audio file')

    (opt,args) = parser.parse_args(sys.argv)

    picross.pic_set_interrupt()

    if len(args) < 2:
        parser.error('wrong number of arguments')

    prefix=opt.name+'-' if opt.name else ''

    map = {}
    for arg in args[1:]:
        spl = arg.split('=')
        if len(spl)!=2:
            parser.error('invalid argument %s' % arg)
        map[spl[1]]=prefix+spl[0]

    def coroutine():
        r = async.Deferred()
        d = {}

        for (id,tag) in map.items():
            d[id] = Plumber(tag,id,opt.abs,opt.wav)

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
