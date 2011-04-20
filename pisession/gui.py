
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

import piw, piagent, wx, picross
import wx.lib.newevent
import sys

from pi import utils, async
from pisession import session

(PiaServiceEvent, EVT_PIA_SERVICE) = wx.lib.newevent.NewEvent()
(PiaShutdownEvent, EVT_PIA_SHUTDOWN) = wx.lib.newevent.NewEvent()
(PiaAsyncEvent, EVT_PIA_ASYNC) = wx.lib.newevent.NewEvent()

class App(wx.App):
    def __init__(self,name = None, logfunc=None, user=None, **kwds):
        wx.App.__init__(self, redirect=False, **kwds)

        self.Bind(EVT_PIA_SERVICE, self.__service_ctx)
        self.Bind(EVT_PIA_ASYNC, self.__service_async)
        
        self.mgr = piagent.scaffold_gui(user or session.get_username(), utils.notify(self.__service), utils.notify(self.__service_shutdown), utils.stringify(logfunc), utils.stringify(None), False, False)

        self.__env_bg = self.mgr.bgcontext(utils.statusify(None), utils.stringify(logfunc), name or 'App')
        self.__env_fg = self.mgr.context(utils.statusify(None), utils.stringify(logfunc), name or 'App')

        piw.setenv(self.__env_fg.getenv())

        self.__stdio = (sys.stdout,sys.stderr)
        sys.stdout = session.Logger()
        sys.stderr = sys.stdout

        self.__thing = piw.thing()
        self.__thing.set_slow_trigger_handler(utils.notify(self.__trigger))
        self.__queue = []
        self.__lock = picross.mutex()

        self.run_bg_sync(self.__setup)

    def run_bg_sync(self,func,*args,**kwds):
        s = piw.tsd_snapshot()
        try:
            piw.setenv(self.__env_bg.getenv())
            piw.tsd_lock()
            try:
                return func(*args,**kwds)
            finally:
                piw.tsd_unlock()
        finally:
            s.install()

    def run_fg_async(self,func,*args,**kwds):
        wx.PostEvent(self, PiaAsyncEvent(e_func=func,e_args=args,e_kwds=kwds))

    def run_bg_async(self,func,*args,**kwds):
        self.__lock.lock()
        try:
            self.__queue.append((func,args,kwds))
            self.__thing.trigger_slow()
        finally:
            self.__lock.unlock()

    def __dequeue(self):
        self.__lock.lock()
        try:
            if len(self.__queue):
                return self.__queue.pop(0)
            else:
                return None
        finally:
            self.__lock.unlock()

    def __setup(self):
        piw.tsd_thing(self.__thing)

    def __trigger(self):
        while True:
            e = self.__dequeue()
            if e is None:
                break
            utils.safe(e[0],*e[1],**e[2])

    def __service(self):
        wx.PostEvent(self, PiaServiceEvent())

    def __service_shutdown(self):
        wx.PostEvent(self, PiaShutdownEvent())

    def __service_ctx(self,evt):
        self.mgr.process_ctx()
        piw.setenv(self.__env_fg.getenv())

    def __service_async(self,evt):
        func = evt.e_func
        args = evt.e_args
        kwds = evt.e_kwds
        utils.safe(func,*args,**kwds)

def call_bg_sync(func,*args,**kwds):
    return wx.GetApp().run_bg_sync(func,*args,**kwds)

def call_bg_async(func,*args,**kwds):
    wx.GetApp().run_bg_async(func,*args,**kwds)

def call_fg_async(func,*args,**kwds):
    wx.GetApp().run_fg_async(func,*args,**kwds)

def defer_bg(func,*args,**kwds):
    d=async.Deferred()
    a=wx.GetApp()

    def bg_ok(*args,**kwds):
        a.run_fg_async(d.succeeded,*args,**kwds)

    def bg_failed(*args,**kwds):
        a.run_fg_async(d.failed,*args,**kwds)

    def doit():
        try:
            r=utils.safe(func,*args,**kwds)
            if not isinstance(r,async.Deferred):
                a.run_fg_async(d.succeeded,r)
                return
            r.setCallback(bg_ok).setErrback(bg_failed)
        except:
            utils.log_exception()
            a.run_fg_async(d.failed)

    a.run_bg_async(doit)
    return d

def defer_fg(func,*args,**kwds):
    d=async.Deferred()
    a=wx.GetApp()

    def bg_ok(*args,**kwds):
        a.run_bg_async(d.succeeded,*args,**kwds)

    def bg_failed(*args,**kwds):
        a.run_bg_async(d.failed,*args,**kwds)

    def doit():
        try:
            r=utils.safe(func,*args,**kwds)
            if not isinstance(r,async.Deferred):
                a.run_bg_async(d.succeeded,r)
                return
            r.setCallback(bg_ok).setErrback(bg_failed)
        except:
            utils.log_exception()
            a.run_bg_async(d.failed)

    a.run_fg_async(doit)
    return d
