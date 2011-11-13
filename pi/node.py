
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
import const
import picross
import utils
import traceback

"""
This module contains utility classes for creating hierarchies
of state variables.
"""

class server(piw.server):
    __slots__ = ('__children','__wrecker','__creator','__extension')

    """
    A node in a hierarchy.  Node behaves like a standard dict,
    but keys are constrained to be integers between 1 and 255
    and values must be piw.server's

    If change is callable, node will be read/write, and change
    will be called when a change request is received.

    if change is false, node will be readonly, otherwise it
    will be read/write but no change functor will be installed.
    """

    def __init__(self,value=None,change=None,rtransient=False,transient=False,creator=None,wrecker=None,extension=None,dynlist=False):
        self.__children=None
        self.__creator=utils.weaken(creator or getattr(self,'dynamic_create',None))
        self.__wrecker=utils.weaken(wrecker or self.dynamic_destroy)
        self.__extension=extension

        flags = 0

        if self.__creator is not None or dynlist:
            flags = flags | const.server_list

        if rtransient:
            flags = flags | const.server_rtransient

        if transient:
            flags = flags | const.server_transient

        piw.server.__init__(self,flags)
        self.set_change_handler(change)
        self.set_data(value or piw.data())

    def load_state(self,state,delegate,phase):
        if state.arity()==0 or phase!=1:
            return

        children = [state.arg(n) for n in range(1,state.arity())]

        if children and children[0].type()==1:
            d=children[0].value()
            self.load_value(delegate,d)
            children = children[1:]

        children = [(c.arg(0).value().as_long(),c) for c in children if c.type()==3 and (c.pred()=='n' or c.pred()=='p')]

        if self.__creator is not None:
            childlist = [c[0] for c in children]
            self.populate(childlist)

        r = None

        while children:
            (i,c) = children.pop(0)

            if c.pred()=='p':
                continue

            n = self.get_internal(i)
            if n is not None:
                if hasattr(n,'load_state'):
                    n.load_state(c,delegate,phase)
                continue
            else:
                if r is None:
                    r = piw.term('n',0)
                    r.append_arg(state.arg(0))
                r.append_arg(c)

        if r:
            delegate.set_residual(self,r)

    def load_value(self,delegate,value):
        self.server_change(value)

    def __make_extension(self):
        def creator(k):
            return self.__creator(k+self.__extension-1)
        def wrecker(k,v):
            return self.__wrecker(k+self.__extension-1,v)
        e = server(creator=creator,wrecker=wrecker,extension=255)
        return e

    def isinternal(self,k):
        if self.__extension and k==self.__extension:
            return True
        return False

    def set_change_handler(self,c):
        if c is None:
            self.set_readonly()
            piw.server.clear_change_handler(self)
            return

        piw.server.set_change_handler(self,utils.changify(c))
        self.set_readwrite()

    def __nonzero__(self):
        return True

    def find_hole(self):
        """
        Find an unused index
        """
        for i in xrange(1,255):
            if not self.isinternal(i) and (self.__children is None or not self.__children.has_key(i)):
                return i

        ex = self.__extension
        if ex:
            en = self.get_internal(ex)
            if en is None:
                en=self.__make_extension()
                self.set_internal(ex,en)
            i = ex-1+en.find_hole()
            return i

        return 0

    def remove(self,child):
        """
        remove child from node
        """
        for (k,v) in self.iteritems():
            if v is child:
                del self[k]
                return True
        return False

    def add(self,child):
        """
        Add a child at an unused index, returning that index
        """
        b=self.find_hole()
        if b==0:
            raise OverflowError('node is full')
        self[b]=child
        return b

    def extend(self,child):
        """
        Add a list of children at unused indices, returning the
        node object.

            node.extend(child1).extend(child2).extend(child3)
        """
        for c in child:
            self.add(c)
        return self

    def append(self,*child):
        """
        Add a child at an unused index, returning the
        node object.

            node.append(child1).append(child2).append(child3)
        """
        for c in child:
            self.add(c)
        return self

    def get_internal(self,key):
        if self.__children is None:
            return None
        else:
            return self.__children.get(key)

    def __getitem__(self,key):
        ex = self.__extension

        if not ex:
            if self.isinternal(key):
                raise KeyError('invalid internal access')
        else:
            if key>=ex:
                en = self.get_internal(ex)
                if en is None:
                    raise KeyError('invalid key')
                return en.__getitem__(key-ex+1)

        if self.__children is None:
            raise KeyError('invalid key')

        return self.__children.__getitem__(key)


    def set_internal(self,key,val):
        if val is None:
            return

        if not isinstance(val,piw.server):
            raise TypeError('%d:%s is not a server' % (key,val))

        if not isinstance(key,int):
            raise TypeError('key not integer')

        if key<1 or key>255:
            raise OverflowError('key %d not in range'%key)

        if self.__children is not None and self.__children.has_key(key):
            oldval=self.__children[key]
            del self.__children[key]
            oldval.close_server()

        if self.__children is None:
            self.__children={}

        self.__children[key]=val
        if self.open():
            self.child_add(key,val)

    def __setitem__(self,key,val):
        ex = self.__extension

        if not ex:
            if self.isinternal(key):
                raise KeyError('invalid internal access')
        else:
            if key>=ex:
                en = self.get_internal(ex)
                if en is None:
                    en=self.__make_extension()
                    self.set_internal(ex,en)
                return en.__setitem__(key-ex+1,val)

        self.set_internal(key,val)

    def del_internal(self,key):
        if self.__children is not None and self.__children.has_key(key):
            oldval=self.__children[key]
            del self.__children[key]
            if len(self.__children) == 0:
                self.__children=None
            if self.open(): oldval.close_server()

    def __delitem__(self,key):
        ex = self.__extension

        if not ex:
            if self.isinternal(key):
                raise KeyError('invalid internal access')
        else:
            if key>=ex:
                en = self.get_internal(ex)
                if en is not None:
                    return en.__delitem__(key-ex+1)
                return

        self.del_internal(key)

    def values(self):
        return list(self.itervalues())

    def items(self):
        return list(self.iteritems())

    def keys(self):
        return list(self.iterkeys())

    def __iter__(self):
        return self.iterkeys()

    def iterkeys(self):
        if self.__children is not None:
            for k in self.__children:
                if not self.isinternal(k):
                    yield k

        ex = self.__extension
        en = self.get_internal(ex)

        if en is None:
            return

        for k in en:
            yield k-1+ex

    def itervalues(self):
        if self.__children is not None:
            for (k,v) in self.__children.iteritems():
                if not self.isinternal(k):
                    yield v

        ex = self.__extension
        en = self.get_internal(ex)

        if en is None:
            return

        for v in en.itervalues():
            yield v

    def iteritems(self):
        if self.__children is not None:
            for (k,v) in self.__children.iteritems():
                if not self.isinternal(k):
                    yield (k,v)

        ex = self.__extension
        en = self.get_internal(ex)

        if en is None:
            return

        for (k,v) in en.iteritems():
            yield (k-1+ex,v)

    def has_key(self,k):
        if self.__children is not None and self.__children.has_key(k):
            return True

        ex = self.__extension
        en = self.get_internal(ex)

        if en is None:
            return False

        return en.has_key(k+1-ex)

    def setdefault(self,key,default=None):
        try:
            return self[key]
        except KeyError:
            self[key] = default
        return default

    def pop(self, key, *args):
        if len(args) > 1:
            raise TypeError, "pop expected at most 2 arguments, got "\
                              + repr(1 + len(args))
        try:
            value = self[key]
        except KeyError:
            if args:
                return args[0]
            raise
        del self[key]
        return value

    def popitem(self):
        try:
            k, v = self.iteritems().next()
        except StopIteration:
            raise KeyError, 'container is empty'
        del self[k]
        return (k, v)

    def update(self, other=None, **kwargs):
        # Make progressively weaker assumptions about "other"
        if other is None:
            pass
        elif hasattr(other, 'iteritems'):  # iteritems saves memory and lookups
            for k, v in other.iteritems():
                self[k] = v
        elif hasattr(other, 'keys'):
            for k in other.keys():
                self[k] = other[k]
        else:
            for k, v in other:
                self[k] = v
        if kwargs:
            self.update(kwargs)

    def __contains(self,k):
        return self.has_key(k)

    def get(self, key, default=None):
        try:
            return self[key]
        except KeyError:
            return default

    def __len__(self):
        return len(self.keys())

    def clear(self):
        for k in self.keys():
            del self[k]

    def attached(self):
        if self.__children is not None and self.open():
            for (k,v) in self.__children.iteritems():
                if not v.open():
                    self.child_add(k,v)

    def server_opened(self):
        piw.server.server_opened(self)
        self.attached()

    def clear_source(self):
        self.set_source(None)

    def close_server(self):
        piw.server.close_server(self)
        if self.__children is not None:
            for (k,v) in self.__children.iteritems():
                v.close_server()

    def populate(self,child_list):
        if self.__creator is None:
            return

        want = set(child_list)
        ditch = []

        if self.__children is not None:
            for k in self.__children:
                if k in want:
                    want.discard(k)
                else:
                    ditch.append(k)

        for k in ditch:
            if self.isinternal(k):
                if self.__extension and k==self.__extension:
                    self.del_internal(k)
                continue

            c=self.__children[k]
            self.__wrecker(k,c)
            self.del_internal(k)

        for k in want:
            if self.isinternal(k):
                if self.__extension and k==self.__extension:
                    c=self.__make_extension()
                    if c is not None: self.set_internal(k,c)
                continue
            else:
                c=self.__creator(k)
                if c is not None: self[k]=c

    def dynamic_destroy(self,index,node):
        pass

class client(piw.client):

    def __init__(self, flags=0, creator=None, wrecker=None, sync=True, auto=True, change=None, initial=False,extension=None):
        if sync:
            flags = flags | const.client_sync

        self.__children=None
        self.__dynamic=None
        self.__sync=sync
        self.__auto=auto
        self.__creator=creator or self.dynamic_create
        self.__wrecker=wrecker or self.dynamic_destroy
        self.__sink=None
        self.__initial=initial
        self.__extension = extension

        piw.client.__init__(self,flags)

        self.set_change_handler(change)

    def __nonzero__(self):
        return True

    def __make_extension(self):
        def creator(k):
            return self.__creator(k+self.__extension-1)
        def wrecker(k,v):
            return self.__wrecker(k+self.__extension-1,v)
        return client(extension=255,creator=creator,wrecker=wrecker)

    def isinternal(self,k):
        if self.__extension and self.__extension==k:
            return True
        return False

    def dynamic_create(self,k):
        return None

    def dynamic_destroy(self,k,v):
        pass

    def __child_exists(self,name):
        return self.enum_child(name-1)==name

    def __child(self,key):
        if self.__children is None:
            return None
        else:
            return self.__children.get(key,None)

    def __dyn(self,key):
        if self.__dynamic is None:
            return None
        else:
            return self.__dynamic.get(key,None)

    def get_internal(self,key):
        s=self.__child(key)
        if s is not None: return s
        return self.__dyn(key)

    def __getitem__(self,key):
        ex = self.__extension

        if not ex:
            if self.isinternal(key):
                raise KeyError('invalid internal access')
            s=self.__child(key)
            if s is not None: return s
            if self.__dynamic is None:
                raise KeyError('invalid key')
            return self.__dynamic.__getitem__(key)

        if key>=ex:
            en = self.__child(ex)
            if en is None:
                raise KeyError('invalid key')
            return en.__getitem__(key-ex+1)

        s=self.__child(key)
        if s is not None: return s
        if self.__dynamic is None:
            raise KeyError('invalid key')
        return self.__dynamic.__getitem__(key)

    def keys(self):
        ks=[]

        if self.__children is not None:
            ks.extend([k for k in self.__children.keys() if not self.isinternal(k)])
        if self.__dynamic is not None:
            ks.extend([k for k in self.__dynamic.keys() if not self.isinternal(k)])

        ex = self.__extension
        if ex:
            en = self.get_internal(ex)
            if en:
                ks.extend([k+ex-1 for k in en.keys()])

        return ks

    def __contains__(self,key):
        if self.isinternal(key):
            return False

        if self.__children is not None and self.__children.__contains__(key):
            return True
            
        if self.__dynamic is not None and self.__dynamic.__contains__(key):
            return True

        ex = self.__extension
        if ex:
            en = self.get_internal(ex)
            if en:
                return en.__contains__(k-ex+1)

        return False

    def __iter__(self):
        if self.__children is not None:
            for k in self.__children:
                if not self.isinternal(k):
                    yield k

        if self.__dynamic is not None:
            for k in self.__dynamic:
                if not self.isinternal(k):
                    yield k

        ex = self.__extension
        if ex:
            en = self.get_internal(ex)
            if en:
                for k in en:
                    yield k+ex-1

    def iteritems(self):
        if self.__children is not None:
            for i in self.__children.iteritems():
                if not self.isinternal(i[0]):
                    yield i
        if self.__dynamic is not None:
            for i in self.__dynamic.iteritems():
                if not self.isinternal(i[0]):
                    yield i

        ex = self.__extension
        if ex:
            en = self.get_internal(ex)
            if en:
                for (k,v) in en.iteritems():
                    yield (k+ex-1,v)

    def set_internal(self,key,val):
        if not isinstance(val,piw.client):
            raise TypeError('not a server')
        if not isinstance(key,int):
            raise TypeError('key not integer')
        if key<1 or key>255:
            raise OverflowError('key not in range')

        old = self.__dyn(key)
        if old is not None:
            self.__wreck(key,old)
            del self.__dynamic[key]
            if len(self.__dynamic) == 0:
                self.__dynamic = None

        old = self.__child(key)
        if old is not None:
            self.safe_close(old)
            del self.__children[key]

        if self.__children is None:
            self.__children = {}
        self.__children[key]=val
        if self.open():
            try:
                self.child_add(key,val)
            except:
                print 'child',key,'of',self.id(),'disappeared'

    def __setitem__(self,key,val):
        ex = self.__extension
        if not ex:
            if self.isinternal(key):
                raise KeyError('invalid internal access')
        else:
            if key>=ex:
                en = self.get_internal(ex)
                if not en:
                    en=self.__make_extension()
                    self.set_internal(ex,en)
                return en.__setitem__(key-ex+1,val)

        return self.set_internal(key,val)

    def del_internal(self,key):
        if self.__children is not None and self.__children.has_key(key):
            oldval=self.__children[key]
            self.safe_close(oldval)
            del self.__children[key]
            if len(self.__children) == 0:
                self.__children = None
            return

        if self.__dynamic is not None and self.__dynamic.has_key(key):
            oldval=self.__dynamic[key]
            self.__wreck(key,oldval)
            del self.__dynamic[key]
            if len(self.__dynamic) == 0:
                self.__dynamic = None
            return

        raise KeyError(key)

    def __delitem__(self,key):
        ex = self.__extension
        if not ex:
            if self.isinternal(key):
                raise KeyError('invalid internal access')
        else:
            if key>=ex:
                en = self.get_internal(ex)
                if not en:
                    return en.__delitem__(key-ex+1,val)

        return self.del_internal(key)

    def populate(self, dynamic=True):
        if self.open():
            for k in utils.client_iter(self):
                v = self.__child(k)
                if v is None:
                    v = self.__dyn(k)

                if v is None:
                    if not dynamic: continue
                    if self.__extension and k==self.__extension:
                        v=self.__make_extension()
                    else:
                        v=self.__creator(k)
                    if v is None: continue
                    try:
                        self.child_add(k,v)
                        if self.__dynamic is None:
                            self.__dynamic={}
                        self.__dynamic[k]=v
                    except:
                        #utils.log_trace()
                        print 'child',k,'of',self.id(),'disappeared'
                        self.__wreck(k,v)
                else:
                    if not v.open():
                        try:
                            self.child_add(k,v)
                        except:
                            #utils.log_trace()
                            print 'child',k,'of',self.id(),'disappeared'

            rep=None 
            if self.__dynamic is not None:
                for (k,v) in self.__dynamic.iteritems():
                    if not v.open():
                        if self.__extension and k==self.__extension:
                            v.close_client()
                        else:
                            self.__wreck(k,v)
                    else:
                        if rep is None:
                            rep={}
                        rep[k]=v

            self.__dynamic=rep

    def client_tree(self):
        piw.client.client_tree(self)
        self.populate(self.__auto)

    def client_child(self):
        piw.client.client_child(self)
        self.populate(self.__auto)

    def client_opened(self):
        piw.client.client_opened(self)
        if self.__sink:
            piw.client.set_sink(self,self.__sink)
        if self.__initial:
            piw.client.client_data(self,self.get_data())       

    def client_sync(self):
        piw.client.client_sync(self)

    def set_sink(self,sink):
        self.__sink=sink
        if self.open():
            piw.client.set_sink(self,sink)

    def clear_sink(self):
        self.set_sink(None)

    def close_client(self):
        if self.__children is not None:
            for (k,v) in self.__children.iteritems():
                self.safe_close(v)
        if self.__dynamic is not None:
            for (k,v) in self.__dynamic.items():
                self.__wreck(k,v)
            self.__dynamic.clear()
            self.__dynamic=None
        piw.client.close_client(self)

    def __wreck(self,k,v):
        try: v.close_client()
        except: pass
        try: self.__wrecker(k,v)
        except: pass

    @utils.nothrow
    def safe_close(self,v):
        v.close_client()

    def set_change_handler(self,c):
        if c is None:
            piw.client.clear_change_handler(self)
            return

        piw.client.set_change_handler(self,utils.changify(c))

    def clear_change_handler(self):
        self.set_change_handler(None)

    def is_readonly(self):
        return bool(self.get_host_flags()&const.server_ro)

    def is_transient(self):
        return bool(self.get_host_flags()&const.server_transient)

    def is_fast(self):
        return bool(self.get_host_flags()&const.server_fast)

    def has_key(self, key):
        try:
            value = self[key]
        except KeyError:
            return False
        return True

    def iterkeys(self):
        return self.__iter__()

    # fourth level uses definitions from lower levels
    def itervalues(self):
        for _, v in self.iteritems():
            yield v
    def values(self):
        return [v for _, v in self.iteritems()]
    def items(self):
        return list(self.iteritems())
    def clear(self):
        for key in self.keys():
            del self[key]

    def setdefault(self, key, default=None):
        try:
            return self[key]
        except KeyError:
            self[key] = default
        return default

    def pop(self, key, *args):
        if len(args) > 1:
            raise TypeError, "pop expected at most 2 arguments, got "\
                              + repr(1 + len(args))
        try:
            value = self[key]
        except KeyError:
            if args:
                return args[0]
            raise
        del self[key]
        return value

    def popitem(self):
        try:
            k, v = self.iteritems().next()
        except StopIteration:
            raise KeyError, 'container is empty'
        del self[k]
        return (k, v)

    def update(self, other=None, **kwargs):
        # Make progressively weaker assumptions about "other"
        if other is None:
            pass
        elif hasattr(other, 'iteritems'):  # iteritems saves memory and lookups
            for k, v in other.iteritems():
                self[k] = v
        elif hasattr(other, 'keys'):
            for k in other.keys():
                self[k] = other[k]
        else:
            for k, v in other:
                self[k] = v
        if kwargs:
            self.update(kwargs)
    def get(self, key, default=None):
        try:
            return self[key]
        except KeyError:
            return default
    def __len__(self):
        return len(self.keys())

Server = server
Client = client

def static(value=None,**children):
    a = Server(value)

    for(k,v) in children.iteritems():
        if k[0:1]=='_':
            a[int(k[1:])] = v

    return a

def string(val, **children):
    return static(piw.makestring(val,0), **children)
