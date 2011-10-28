
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

from pi import const, index, node, utils, domain, paths, async, timeout, logic, rpc
from pi.logic.shortcuts import *

import piw
import random
import traceback
import struct
import copy

class DataProxy(node.Client):

    __slots__ = ('__clone','__cloned')

    def __init__(self,clone=None):
        node.Client.__init__(self)
        self.__clone = clone
        self.__cloned = False

    def client_opened(self):
        node.Client.client_opened(self)
        if not self.__cloned and self.__clone is not None:
            self.__cloned=True
            self.clone(self.__clone)

    def close_client(self):
        node.Client.close_client(self)
        if self.__clone is not None:
            self.__cloned=False
            self.__clone.close_client()

    def set_clone(self,data):
        if self.__clone is not None:
            self.__clone.close_client()
        self.__clone=data
        self.__cloned=False
        if self.open() and self.__clone is not None:
            self.__cloned=True
            self.clone(self.__clone)

class AtomProxy(node.Client):

    __slots__ = ('__ready','__domain','__syncers','__data','__meta','__current_meta')

    monitor = set(['domain','master','protocols','ordinal','name',
                   'latency','frelation','fuzzy','relation','insert',
                   'ideals','verbs','modes','cname','cordinal','help'])

    def __init__(self):
        self.__ready=False
        self.__domain=None
        self.__syncers=None
        self.__data=DataProxy()
        self.__meta=DataProxy()
        self.__current_meta=None

        flags = const.client_sync

        if hasattr(self,'node_added'):
            node.Client.__init__(self,flags=flags,creator=self.node_added,extension=253)
        else:
            node.Client.__init__(self,flags=flags,extension=253)

        self.set_internal(const.data_node,self.__data)

    def set_data_clone(self,clone):
        self.__data.set_clone(clone)

    def set_meta_clone(self,clone):
        if clone:
            if self.get_internal(const.meta_node) is None:
                self.set_internal(const.meta_node,self.__meta)
            self.__meta.set_clone(clone)
        else:
            self.del_internal(const.meta_node)

    def invoke_rpc(self,name,arg,time=30000):
        return rpc.RpcInvocation(self.servername(),self.path(),name,arg,timeout=time)

    def isinternal(self,k):
        return k==const.meta_node or k==const.data_node

    def is_fast(self):
        return (self.__data.get_host_flags() & const.server_fast) != 0

    def is_ready(self):
        return self.__ready

    def node_ready(self):
        pass

    def node_changed(self,parts):
        pass

    def node_removed(self):
        pass

    def __as_stringlist(self,key,default=[]):
        if self.__current_meta is None:
            return default
        v = self.__current_meta.as_dict_lookup(key)
        return v.as_string().split() if v.is_string() else copy.copy(default)

    def __as_long(self,key,default=0):
        if self.__current_meta is None:
            return default
        v = self.__current_meta.as_dict_lookup(key)
        return v.as_long() if v.is_long() else default

    def __as_string(self,key,default=''):
        if self.__current_meta is None:
            return default
        v = self.__current_meta.as_dict_lookup(key)
        return v.as_string() if v.is_string() else default

    def __as_termlist(self,key,default=[]):
        v = self.get_meta_data().as_dict_lookup(key)
        if v.is_string(): return logic.parse_termlist(v.as_string(),paths.make_subst(self.id()))
        return default

    def protocols(self):
        return self.__as_stringlist('protocols')

    def ideals(self):
        return self.__as_stringlist('ideals')

    def fuzzy(self):
        return self.__as_stringlist('fuzzy')

    def domain(self):
        if not self.__domain:
            dom = self.__as_string('domain','null()')
            self.__domain = domain.traits(dom)
        return self.__domain

    def cnames(self):
        return self.__as_stringlist('cname')

    def names(self):
        return self.__as_stringlist('name')

    def pronoun(self):
        return self.__as_string('pronoun')

    def cordinal(self):
        return self.__as_long('cordinal')

    def ordinal(self):
        return self.__as_long('ordinal')

    def frelations(self):
        return self.__as_termlist('frelation')

    def relations(self):
        return self.__as_termlist('relation')

    def latency(self):
        return self.__as_long('latency')

    def icon(self):
        return self.__as_string('icon')

    def modes(self):
        return self.__as_termlist('modes')

    def verbs(self):
        return self.__as_termlist('verbs')

    def help_text(self):
        return self.__as_string('help')

    def control(self):
        return self.__as_string('control')

    def get_insert(self):
        return self.__as_string('insert')

    def get_master(self):
        return self.__as_string('master')

    def set_meta_data(self,value):
        node.Client.set_data(self,value)

    def get_meta_data(self):
        return node.Client.get_data(self)

    def get_data(self):
        return self.__data.get_data()

    def get_value(self):
        d = self.__data.get_data()
        v = self.domain().data2value(d)
        return v

    def set_sink(self,sink):
        self.__data.set_sink(sink)

    def clear_sink(self):
        self.__data.clear_sink()

    def set_change_handler(self,c):
        self.__data.set_change_handler(c)

    def __nodeready(self):  
        self.__domain=None

        try:
            self.node_ready()
        except:
            utils.log_exception()

    def __noderemoved(self):
        try:
            self.node_removed()
        except:
            utils.log_exception()

    def __nodechanged(self,parts):
        if 'domain' in parts:
            self.__domain=None

        try:
            self.node_changed(parts)
        except:
            utils.log_exception()

    def add_sync(self):
        r = async.Deferred()
        if not self.__syncers and self.open():
            self.sync()
        if self.__syncers is None:
            self.__syncers = []
        self.__syncers.append(r)
        return r

    def __meta_changed(self):
        old_value = self.__current_meta
        new_value = self.get_meta_data()

        old_keys = set(utils.dict_keys(old_value)) if old_value is not None else set()
        new_keys = set(utils.dict_keys(new_value))
        all_keys = old_keys.union(new_keys)
        common_keys = old_keys.intersection(new_keys)

        all_keys.intersection_update(self.monitor)
        common_keys.intersection_update(self.monitor)

        for k in common_keys:
            if old_value.as_dict_lookup(k) == new_value.as_dict_lookup(k):
                all_keys.remove(k)

        self.__current_meta = new_value
        return all_keys

    def client_sync(self):
        node.Client.client_sync(self)

        if not self.__ready:
            self.__meta_changed()
            self.__ready=True
            self.__nodeready()
        else:
            parts = self.__meta_changed()
            if parts:
                self.__nodechanged(parts)

        while self.__syncers is not None:
            syncers = self.__syncers
            self.__syncers = None

            for s in syncers:
                s.succeeded()

    def close_client(self):
        try:
            if self.__ready:
                self.__noderemoved()
                self.__ready=False
        except:
            traceback.print_stack()

        node.Client.close_client(self)

        if self.__syncers is not None:
            syncers = self.__syncers
            self.__syncers = None
            for s in syncers:
                s.failed()
