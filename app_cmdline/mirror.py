
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
from pi import async,paths,const,index,proxy

import optparse
import sys
import piw
import picross
import traceback

class MirrorProxy(proxy.AtomProxy):
    def __init__(self,name):
        proxy.AtomProxy.__init__(self)

    def node_added(self,index):
        return self.__class__(None)

    def node_ready(self):
        print 'added',self.id()

    def node_removed(self):
        print 'removed',self.id()

class Mirror:
    def __init__(self,name):
        self.__index = index.Index(MirrorProxy,False)
        piw.tsd_index(name,self.__index)

def main():
    def startup(dummy):
        return Mirror('<main>')

    session.run_session(startup,clock=False)
