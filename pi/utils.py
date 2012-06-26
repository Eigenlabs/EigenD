
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

import os.path
import piw
import struct
import traceback
import picross
import weakref
import types
import sys

class WeakFunctionCallback:
    def __init__(self,callback,default):
        self.callback = callback

    def __call__(self,*args,**kwds):
        try:
            return self.callback(*args,**kwds)
        except:
            log_exception()
            return self.default

class WeakMethodCallback:
    def __init__(self,callback,default):
        self.default = default
        self.im_class = callback.im_class
        self.im_self = weakref.ref(callback.im_self)
        self.im_func = callback.im_func

    def __call__(self,*args,**kwds):
        try:
            o = self.im_self()
            if o is not None:
                return types.MethodType(self.im_func,o,self.im_class)(*args,**kwds)

            return self.default
        except:
            log_exception()
            return self.default

def weaken(callback,default=None):
    if callback is None:
        return None

    if isinstance(callback,types.MethodType):
        try:
            return WeakMethodCallback(callback,default)
        except:
            pass

    return WeakFunctionCallback(callback,default)

def log_exception():
    print 'start traceback:'
    traceback.print_exc(limit=None)
    print 'end traceback:'

def log_trace():
    print 'start traceback:'
    traceback.print_stack(limit=None)
    print 'end traceback:'

def __nothrow(func,ret):
    def __do_nothrow(*args,**kwds):
        try:
            return func(*args,**kwds)
        except:
            print 'ignored:'
            traceback.print_exc(limit=None)
            return ret
    return __do_nothrow

def nothrow(func):
    return __nothrow(func,None)

def nothrow_ret(ret):
    def __do_nothrow(func):
        return __nothrow(func,ret)
    return __do_nothrow

@nothrow
def safe(func, *args, **kwds):
    return func(*args, **kwds)

def client_iter(client, p=None):
    path = p or ''
    if client.open():
        i=client.child_enum_child_str(path,len(path),0)
        while i>0:
            yield i
            i=client.child_enum_child_str(path,len(path),i)

def index_iter(index):
    old_count = None
    members = set()

    while True:
        new_count = index.member_count()

        if new_count == old_count:
            break

        if old_count != None:
            print 'reiterating',new_count,old_count

        for i in range(0,new_count):
            m = index.member_name(i)
            if m.is_string(): members.add(m.as_string())

        old_count = new_count

    return iter(members)

def decode_array(data):
    return [ data.as_array_member(i) for i in range(0,data.as_arraylen()) ]

def strsplit(obj):
    if obj is None: return []
    if isinstance(obj,list): return obj
    if isinstance(obj,tuple): return list(obj)
    if not isinstance(obj,str): return [obj]
    return obj.split()

def notify(c):
    if c is None:
        return picross.notify()
    if not isinstance(c,picross.notify):
        c = picross.make_notify_functor(c)
    return c

def stringify(c):
    if c is None:
        return picross.f_string()
    if not isinstance(c,picross.f_string):
        c = picross.make_string_functor(c)
    return c

def make_change_nb(c):
    if c is None:
        return piw.change_nb()
    if not isinstance(c,piw.change_nb):
        c = piw.make_change_nb(c)
    return c

def changify(c):
    if c is None:
        return piw.change()
    if isinstance(c,piw.change_nb):
       raise Exception("Can't changify a change_nb type "+repr(c)) 
    if not isinstance(c,piw.change):
        c = piw.make_change_functor(c)
    return c

def changify_nb(c):
    if c is None:
        return piw.change_nb()
    if isinstance(c,piw.change):
        c = make_change_nb(c)
    if not isinstance(c,piw.change_nb):
        c = piw.make_change_nb_functor(c)
    return c

def statusify(c):
    if c is None:
        return picross.status()
    if not isinstance(c,picross.status):
        c = picross.make_status_functor(c)
    return c

def slowchange(c):
    return piw.slowchange(changify(c))

def fastchange(c):
    return piw.fastchange(changify_nb(c))

def call_locked_callable(snapshot, callable, *args):
    """
    call a callable object in the context of a mainloop lock
    """
    snapshot.install()
    piw.tsd_lock()
    try:
        return callable(*args)
    finally:
        piw.tsd_unlock()

def make_locked_callable(callable):
    """
    Convert a callable into one which runs in a locked loop
    """
    snapshot = piw.tsd_snapshot()
    return lambda *args: call_locked_callable(snapshot,callable,*args)

def pack_str(*l):
    ll=len(l)
    return struct.pack('BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB'[0:ll],*l)

def makedict(items,ts):
    d = piw.dictnull(ts)
    for (k,v) in items.iteritems():
        d = piw.dictset(d,k,v)
    return d

def makedict_nb(items,ts):
    d = piw.dictnull_nb(ts)
    for (k,v) in items.iteritems():
        d = piw.dictset_nb(d,k,v)
    return d

def dict_items(d):
    return [ (d.as_dict_key(i),d.as_dict_value(i)) for i in range(0,d.as_dict_nkeys()) ]

def dict_keys(d):
    return [ d.as_dict_key(i) for i in range(0,d.as_dict_nkeys()) ]

def maketuple(items,ts):
    d = piw.tuplenull(ts)
    for v in items:
        d = piw.tupleadd(d,v)
    return d

def maketuple_longs(items,ts):
    d = piw.tuplenull(ts)
    for v in items:
        d = piw.tupleadd(d,piw.makelong(v,ts))
    return d

def maketuple_floats(items,ts):
    d = piw.tuplenull(ts)
    for v in items:
        d = piw.tupleadd(d,piw.makefloat(v,ts))
    return d

def tuple_items(d):
    return [ (d.as_tuple_value(i)) for i in range(0,d.as_tuplelen()) ]

def key_to_lists(d):
    if not piw.is_key(d): return None

    column = d.as_tuple_value(0).as_tuple_value(0).as_float()
    row = d.as_tuple_value(0).as_tuple_value(1).as_float()
    course = d.as_tuple_value(1).as_tuple_value(0).as_float()
    key = d.as_tuple_value(1).as_tuple_value(1).as_float()
    hardness = d.as_tuple_value(2).as_long()

    return [[column,row],[course,key],hardness]
