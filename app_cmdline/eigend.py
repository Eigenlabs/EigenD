
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

import piw
import picross
import sys
import os
import optparse
import subprocess

from pisession import session,agentd,version
from pi import resource,guid,utils

def bracketify(addr):
    if not addr.startswith('<'):
        addr = '<%s>' % addr
    return addr

def make_logger(prefix,logfunc):
    def logger(msg):
        logfunc("%s: %s" % (prefix,msg))
    return utils.stringify(logger)

class Agent(agentd.Agent):
    def __init__(self,scaffold,ordinal,path,icon=None,logger=None):
        self.__scaffold = scaffold
        self.__logger = logger
        agentd.Agent.__init__(self,ordinal,path,icon=icon)
        piw.tsd_server(self.uid, self)

    def run_in_gui(self,func,*args,**kwds):
        return func(*args,**kwds)

    def load_complete(self,errors):
        if errors:
            print 'problems with load:'
            for e in errors:
                print e

    def stop_gc(self):
        pass

    def start_gc(self):
        pass

    def load_started(self,setup):
        print 'Started load of',setup

    def load_ended(self):
        print 'Finished load'

    def load_status(self,msg,progress):
        print 'PROGRESS: %s %d' % (message,progress)

    def create_context(self,name,callback,gui):
        return self.__scaffold.context(utils.statusify(callback),make_logger(name,self.__logger),name)

    def setups_changed(self,file=None):
        pass

def main():
    """
    bad main function
    """

    #sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_LOCAL)

    picross.pic_set_interrupt()
    picross.pic_mlock_code()
    picross.pic_init_time()

    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] [address=agent ...]')
    parser.add_option('--stdout',action='store_true',dest='stdout',default=False,help='log to stdout')
    parser.add_option('--path',action='append',dest='path',default=[],help='add to search path')
    parser.add_option('--mt',action='store',type='int',dest='mt',default=1,help='number of threads (1)')
    parser.add_option('--target',action='store',dest='target',default=None,help='state to load')
    parser.add_option('--name',action='store',type='int',dest='name',default=1,help='address')

    (opts,args) = parser.parse_args(sys.argv)

    lock = resource.LockFile('eigend-%d' % opts.name)
    if not lock.lock():
        print 'cannot run more than one eigend at a time'
        sys.exit(-1)

    preload = {}

    logfilename = 'eigend'

    for a in args[1:]:
        sa = a.split('=')
        if len(sa) != 2:
            parser.error('invalid preload %s' % a)
        preload[bracketify(sa[0])] = sa[1]

    if not opts.path:
        zd=os.path.join(picross.release_root_dir(),'plugins')
        opts.path.append(zd)

    if opts.stdout:
        logfile = sys.__stdout__
    else:
        logfile = resource.open_logfile(logfilename)


    result = [None]

    def load_ok(errs):
        print 'load ok',errs

    def load_failed(errs):
        print 'load failed',errs

    def hostlogger(msg):
        print >>logfile,msg
        logfile.flush()

    def startup(mgr):
        a=Agent(mgr,opts.name,opts.path,logger=hostlogger, icon='app_cmdline/agentd.png')

        try:
            if opts.target:
                x = a.load_file(opts.target)
                path,result[0] = x
                result[0].setCallback(load_ok).setErrback(load_failed)
            else:
                for (name,plugin) in preload.iteritems():
                    a.add_agent(name,plugin)

        except Exception,e:
            utils.log_exception()

        return a

    session.run_session(startup,mt=opts.mt,name='agentd',logger=make_logger('eigend',hostlogger))
