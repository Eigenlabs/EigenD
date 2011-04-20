
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
import sys
import os
import time
import optparse

from pisession import session
from pi import utils

class Indexer:
    def __init__(self,name):
        self.__timer = piw.thing()
        self.__index = piw.index()

        piw.tsd_index(name,self.__index)
        piw.tsd_thing(self.__timer)

        self.__timer.set_slow_timer_handler(utils.notify(self.__timercallback))
        self.__timer.timer_slow(8000)

    def __timercallback(self):
        for i in range(0,self.__index.member_count()):
            print self.__index.member_name(i)

        self.__timer.close_thing()
        self.__index.close_index()

def main(name):
    return Indexer(name)

def cli():
    name=sys.argv[1]
    session.run_session(lambda m: main(name),name='bstlist')
