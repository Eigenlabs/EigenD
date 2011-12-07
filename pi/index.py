
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
from pi import const,async,utils,paths

class Monitor(piw.client):
    def __init__(self,index,name,real):
        piw.client.__init__(self,0)
        self.index=index
        self.real=real
        self.name=name

    def client_opened(self):
        piw.client.client_opened(self)
        if self.real is not None:
            self.clone(self.real)

    def add_sync(self):
        def insync():
            self.index.synced(self)
        r = self.real.add_sync()
        r.setCallback(insync).setErrback(insync)

    def close_client(self):
        if self.real is not None:
            try:
                self.real.close_client()
            except:
                pass
        self.index.member_died(self.servername_fq(),self.cookie(),self)
        piw.client.close_client(self)

class Index(piw.index):

    def __init__(self, factory = None, fast = False):
        piw.index.__init__(self)
        self.__factory=factory or self.index_create
        self.__members={}
        self.__fast=fast
        self.__callbacks=[]
        self.__outstanding=set()
        self.__user = None

    def sync(self, *names):
        for n in names:
            self.force(n)

        if len(self.__members) == 0:
            return async.success()

        if len(self.__callbacks) == 0:
            members = set(self.__members.values())
            self.__outstanding = members
            for c in members:
                c.add_sync()

        callback = async.Deferred()
        self.__callbacks.append(callback)
        return callback

    def members(self):
        for m in self.__members.values():
            if m.real is not None:
                yield m.real

    def synced(self,client):
        self.__outstanding.discard(client)

        if len(self.__outstanding):
            return

        callbacks = self.__callbacks

        if callbacks:
            self.__callbacks = []
            for cb in callbacks:
                cb.succeeded()

    def close_index(self):
        for m in self.__members.values():
            m.close_client()

    def index_create(self,name):
        pass

    def find(self,name):
        c =  self.__members.get(name)
        return c.real if c else None

    def member_died(self,name,cookie,client):
        sname = self.to_relative(name.as_string())

        piw.index.member_died(self,name,cookie)

        c=self.__members.get(sname)
        if c is client:
            self.synced(client)
            del self.__members[sname]

    def to_relative(self,a,scope=None):
        return paths.to_relative(a,scope or self.__user)

    def to_absolute(self,a,scope=None):
        return paths.to_absolute(a,scope or self.__user)

    def user(self):
        return self.__user

    def index_opened(self):
        piw.index.index_opened(self)

        a = self.index_name().as_string()
        cp = a.find(':')

        if cp < 0:
            self.__user = piw.tsd_user()
        else:
            self.__user = a[1:cp]

        self.scan()

    def index_changed(self):
        piw.index.index_changed(self)
        self.scan()

    def force(self,name):
        if name not in self.__members:
            c=Monitor(self,name,self.__factory(name))
            piw.tsd_client(name,c,self.__fast)
            self.__members[name]=c

    def scan(self):
        for n in utils.index_iter(self):
            c=self.__members.get(n)
            if c is None:
                c=Monitor(self,n,self.__factory(n))
                piw.tsd_client(self.to_absolute(n),c,self.__fast)
                self.__members[n]=c
