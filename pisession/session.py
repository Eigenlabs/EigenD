
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
import piw
import piagent
import picross
import time
import os
import gc

from pi import utils,async,resource

class Logger:
    def __init__(self):
        self.__buffer = []

    def write(self, msg):
        eol = (msg[-1:] == '\n')
        self.__buffer.append(str(msg).strip())

        if eol:
            piw.tsd_log(' '.join(self.__buffer))
            self.__buffer=[]

    def close(self):
        pass

    def flush(self):
        pass

def get_username():
    return resource.user_name()

def run_session(session,user=None,mt=1,name='ctx',logger=None,clock=True,rt=True):
    u = user or get_username()

    def logfunc(msg):
        if logger:
            logger(msg)
        else:
            print 

    context = None

    def ctxdun(status):
        context.release()

    scaffold = piagent.scaffold_mt(mt,utils.stringify(logfunc),utils.stringify(None),clock,rt)
    context = scaffold.context(u,utils.statusify(ctxdun),utils.stringify(logfunc),name)
    stdio = (sys.stdout,sys.stderr)
    x = None

    try:
        if logger:
            sys.stdout = Logger()
            sys.stderr = sys.stdout

        piw.setenv(context.getenv())
        piw.tsd_lock()

        try:
            x = session(scaffold)
            context.trigger()
        finally:
            piw.tsd_unlock()

        scaffold.wait()

    finally:
        sys.stdout,sys.stderr = stdio

    return x
