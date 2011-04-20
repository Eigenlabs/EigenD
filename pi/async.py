
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

import traceback
import sys

from pi import utils
import piw

def success(*args, **kwds):
    d=Deferred()
    d.succeeded(*args,**kwds)
    return d

def failure(*args, **kwds):
    d=Deferred()
    d.failed(*args,**kwds)
    return d

def completion(status,*args, **kwds):
    d=Deferred()
    d.completed(status,*args,**kwds)
    return d

class Deferred:
    """
    Twisted style deferred result
    """

    def __init__(self):
        self.__callback = None
        self.__errback = None
        self.__callargs = ()
        self.__errargs = ()
        self.__result = None
        self.__triggered = False

    @utils.nothrow
    def __doit(self):
        if self.__triggered:
            return

        r = self.__result
        if r is not None:
            if r[0]:
                if self.__callback is not None:
                    self.__triggered = True
                    utils.safe(self.__callback,*(self.__callargs+r[1]),**r[2])
            else:
                if self.__errback is not None:
                    self.__triggered = True
                    utils.safe(self.__errback,*(self.__errargs+r[1]),**r[2])

    def completed(self, status, *args, **kwds):
        if self.__result is None:
            self.__result = (status, args, kwds)
            self.__doit()

    def failed(self, *args, **kwds):
        self.completed(False,*args,**kwds)

    def succeeded(self, *args, **kwds):
        self.completed(True,*args,**kwds)

    def status(self):
        r = self.__result
        if r is not None:
            return r[0]
        return None

    def kwds(self):
        r = self.__result
        if r is not None:
            return r[2]
        return None

    def args(self):
        r = self.__result
        if r is not None:
            return r[1]
        return None

    def clear_callbacks(self):
        self.__callback = None
        self.__callargs = None
        self.__errback = None
        self.__errargs = None
        self.__triggered = False

    def setCallback(self, callback, *args):
        self.__callback = callback
        self.__callargs = args
        self.__doit()
        return self
            
    def setErrback(self, errback, *args):
        self.__errback = errback
        self.__errargs = args
        self.__doit()
        return self

class Aggregate(Deferred):
    def __init__(self,accumulate=False,progress=None):
        Deferred.__init__(self)
        self.__outstanding = set()
        self.__results = {}
        self.__status = True
        self.__enabled = False
        self.__progress = progress;

        self.__successes = None
        self.__failures = None

        if accumulate:
            self.__successes = {}
            self.__failures = {}

    def __completed(self):
        if self.__successes is None:
            self.completed(self.__status)
            return

        if self.__status:
            self.completed(True,self.__successes,self.__failures)
        else:
            self.completed(False,self.__successes,self.__failures)

    def successes(self):
        return self.__successes

    def failures(self):
        return self.__failures

    def enable(self):
        self.__enabled = True
        if not self.__outstanding:
            self.__completed()
        self.__callprogress()

    def add(self,tag,result):
        assert self.__enabled == False
        self.__outstanding.add(tag)
        self.__results[tag] = result
        result.setCallback(lambda *a,**k: self.__callback(tag,True,result,a))
        result.setErrback(lambda *a,**k: self.__callback(tag,False,result,a))

    def get_outstanding(self):
        return self.__outstanding

    def get(self,tag):
        return self.__results.get(tag)

    def __callprogress(self):
        if self.__progress:
            t=len(self.__results)
            o=len(self.__outstanding)
            self.__progress(self,t-o,t)

    def __callback(self,tag,status,result,args):
        if self.status() is None:
            if not status:
                self.__status = False
            self.__outstanding.discard(tag)
            if self.__successes is not None:
                if status:
                    self.__successes[tag] = args
                else:
                    self.__failures[tag] = args
            self.__callprogress()
            if not self.__outstanding and self.__enabled:
                self.__completed()

class DeferredDecoder:
    def __init__(self,deferred):
        self.deferred = deferred
    def decode(self):
        return self.deferred

class Arg0(DeferredDecoder):
    def __init__(self,deferred,klass,*args,**kwds):
        DeferredDecoder.__init__(self,deferred)
        self.args = args
        self.kwds = kwds
        self.klass = klass
    def decode(self):
        if self.deferred.status() is True:
            return None
        else:
            return Coroutine.exception(self.klass(*self.args,**self.kwds))

class Arg1(DeferredDecoder):
    def __init__(self,deferred,klass,*args,**kwds):
        DeferredDecoder.__init__(self,deferred)
        self.args = args
        self.kwds = kwds
        self.klass = klass
    def decode(self):
        if self.deferred.status() is True:
            return self.deferred.args()[0]
        else:
            return Coroutine.exception(self.klass(*self.args,**self.kwds))

class Coroutine(Deferred):
    class __Result:
        def __init__(self,status,args,kwds):
            self.status=status
            self.args=args
            self.kwds=kwds

    class __Error:
        def __init__(self,value):
            self.value = value

    def __init__(self,generator,exc_handler=None):
        Deferred.__init__(self)
        self.__generator = generator
        self.__handler = exc_handler or (lambda ei: Coroutine.failure())
        self.__result = None
        self.__doit()

    @staticmethod
    def success(*args,**kwds):
        return Coroutine.__Result(True,args,kwds)

    @staticmethod
    def failure(*args,**kwds):
        return Coroutine.__Result(False,args,kwds)

    @staticmethod
    def completion(status,*args,**kwds):
        return Coroutine.__Result(status,args,kwds)

    @staticmethod
    def exception(value):
        return Coroutine.__Error(value)

    def __doit(self,*a_,**k_):
        while True:
            try:
                if isinstance(self.__result,DeferredDecoder):
                    result = self.__result.decode()
                else:
                    result = self.__result

                if isinstance(result,Coroutine.__Result):
                    self.completed(result.status,*result.args,**result.kwds)
                    if self.__generator:
                        self.__generator.close()
                    self.__generator = None
                    return

                if isinstance(result,Coroutine.__Error):
                    self.__result = self.__generator.throw(type(result.value),result.value)
                    continue
                
                result = self.__generator.send(result)

            except GeneratorExit:
                print 'generator exit'
                raise
            except StopIteration:
                result = self.success()
            except:
                traceback.print_exc(limit=None)
                result = self.__handler(sys.exc_info())

            if isinstance(result,Deferred):
                result = DeferredDecoder(result)

            self.__result  = result

            if isinstance(result,DeferredDecoder):
                if result.deferred.status() is None:
                    result.deferred.setCallback(self.__doit).setErrback(self.__doit)
                    return

def deferred(*fargs,**fkwds):
    def decorator(func):
        def newfunc(*gargs,**gkwds):
            try:
                return func(*gargs,**gkwds)
            except:
                traceback.print_exc(*ei)
                return failure(*fargs,**fkwds)

        return newfunc

    return decorator

def coroutine(*fargs,**fkwds):
    def handler(ei):
        traceback.print_exception(*ei)
        return Coroutine.failure(*fargs,**fkwds)

    def decorator(func):
        def newfunc(*gargs,**gkwds):
            return Coroutine(func(*gargs,**gkwds),handler)
        setattr(newfunc,'__doc__',func.__doc__)
        return newfunc

    return decorator

def deferred_call(func,*args,**kwds):
    r = Deferred()

    def call():
        try:
            v = func(*args,**kwds)
            r.succeeded(v)
        except:
            utils.log_trace()
            r.failed('internal error')
        

    r.thing = piw.thing()
    piw.tsd_thing(r.thing)
    r.thing.set_slow_trigger_handler(utils.notify(call))
    r.thing.trigger_slow()

    return r
