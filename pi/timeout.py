
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

from pi import async,utils
import piw

class Timeout(async.Deferred):

    def __init__(self, deferred, timeout, status, *args, **kwds):
        async.Deferred.__init__(self)

        self.__original = deferred
        self.__status = status
        self.__args = args
        self.__kwds = kwds
        self.__thing = None

        self.__original.setCallback(self.__callback).setErrback(self.__errback)

        if self.__original.status() is None and timeout > 0:
            self.__thing = piw.thing()
            piw.tsd_thing(self.__thing)
            self.__thing.set_slow_timer_handler(utils.notify(self.__timeout))
            self.__thing.timer_slow(timeout)

    def original(self):
        return self.__original

    def __errback(self, *args, **kwds):
        self.__cancel()
        self.failed(*args,**kwds)

    def __callback(self, *args, **kwds):
        self.__cancel()
        self.succeeded(*args,**kwds)

    def __cancel(self):
        if self.__thing:
            self.__thing.clear_slow_timer_handler()
            self.__thing.close_thing()
            self.__thing=None

    def __timeout(self):
        self.__cancel()
        self.completed(self.__status,*self.__args,**self.__kwds)

class Timer(Timeout):
    def __init__(self,timeout):
        Timeout.__init__(self,async.Deferred(),timeout,True)

class Watchdog(async.Deferred):

    def __init__(self, deferred, status, *args, **kwds):
        async.Deferred.__init__(self)

        self.__original = deferred
        self.__status = status
        self.__args = args
        self.__kwds = kwds
        self.__thing = None

        if self.__original.status() is None:
            self.__original.setCallback(self.__callback).setErrback(self.__errback)
            self.__thing = piw.thing()
            piw.tsd_thing(self.__thing)
            self.__thing.set_slow_timer_handler(utils.notify(self.__timeout))
        else:
            self.completed(self.__original.status(),*self.__original.args(),**self.__original.kwds())

    def enable(self,timeout):
        if self.__thing is not None:
            self.__thing.timer_slow(timeout)

    def disable(self):
        if self.__thing is not None:
            self.__thing.cancel_timer_slow()

    def original(self):
        return self.__original

    def __errback(self, *args, **kwds):
        self.__cancel()
        self.failed(*args,**kwds)

    def __callback(self, *args, **kwds):
        self.__cancel()
        self.succeeded(*args,**kwds)

    def __cancel(self):
        if self.__thing:
            self.__thing.clear_slow_timer_handler()
            self.__thing.close_thing()
            self.__thing=None

    def __timeout(self):
        self.__cancel()
        self.completed(self.__status,*self.__args,**self.__kwds)

