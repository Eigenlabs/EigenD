
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

"""
Module for talking to a standard language agent
"""

import piw
from pi import const,proxy,paths,utils,node,logic
import sys

def make_subc_path(*l):
    return ''.join([chr(c) for c in l])

class ContextConnector(node.Client):
    def __init__(self,handler):
        node.Client.__init__(self)
        self.__handler = handler

    def __decode(self,v):
        if not v.is_string():
            return None

        v = v.as_string().split(':',1)

        if len(v)!=2:
            return None

        name = v[0]
        (auto,li,lu) = logic.parse_clause(v[1])
        return (name,li,lu)

    def client_opened(self):
        node.Client.client_opened(self)
        print 'cmdline opened',self.__decode(self.get_data())
        v = self.get_data()
        if v.is_string():
            v = self.__decode(v)
            if v is not None:
                self.__handler.context_changed(*v)

    def client_data(self,v):
        node.Client.client_data(self,v)
        v = self.__decode(v)
        if v is not None:
            self.__handler.context_changed(*v)

    def close_client(self):
        node.Client.close_client(self)
        print 'cmdline closed'

    def get_context(self):
        if self.open():
            return self.__decode(self.get_data())
        return None

class MetaConnector(piw.subcanchor):
    def __init__(self,handler):
        piw.subcanchor.__init__(self)
        self.__context = ContextConnector(handler)
        self.set_client(self.__context)
        self.set_path(make_subc_path(3,1))

    def get_context(self):
        return __context.get_context()

class CmdlineConnector(node.Client):
    def __init__(self,handler):
        node.Client.__init__(self)
        self.__handler = utils.changify(handler)
        self.__unplumber = None

    def __plumb_fast(self):
        self.__fastdata = piw.fastdata_receiver(piw.slowchange(self.__handler))
        piw.tsd_fastdata(self.__fastdata)
        self.__fastdata.enable(True,True,False)
        self.__unplumber = self.__unplumb_fast

    def __unplumb_fast(self):
        print 'plumbing fast cmdline',self.get_data()
        self.__unplumber = None
        self.__fastdata.disable()
        self.__fastdata.close_fastdata()
        self.__fastdata = None
        self.set_sink(self.__fastdata)

    def __plumb_slow(self):
        print 'plumbing slow cmdline',self.get_data()
        self.set_change_handler(self.__handler)
        self.__unplumber = self.__unplumb_slow
        self.__handler(self.get_data())

    def __unplumb_slow(self):
        self.__unplumber = None
        self.clear_change_handler()

    def client_opened(self):
        node.Client.client_opened(self)
        print 'language connector opened',self.id()
        if (self.get_host_flags() & const.server_fast)!=0:
            self.__plumb_fast()
        else:
            self.__plumb_slow()

    def close_client(self):
        node.Client.close_client(self)
        print 'language connector closed'
        if self.__unplumber:
            self.__unplumber()

class HistoryProxy(proxy.AtomProxy):
    monitor = set(['protocols','timestamp'])

    def __init__(self,delegate):
        self.__tcrc = 0
        self.__dpath = chr(const.meta_node)
        self.__history = {}
        self.__length=0
        self.__min = 0
        self.__max = 0
        self.__cmdline = ['','']
        self.__delegate = delegate
        proxy.AtomProxy.__init__(self)
        self.__connector = CmdlineConnector(self.__cmdline_changed)
        self.set_meta_clone(self.__connector)

    def node_changed(self,parts):
        proxy.AtomProxy.node_changed(self,parts)

        tcrc = self.child_tcrc(self.__dpath,1)

        if tcrc != self.__tcrc:
            self.__tcrc = tcrc
            self.__history_changed()

    def flush(self):
        self.__delegate.cmdline_changed()
        self.__history_update()
        for k in range(self.__min,self.__max+1):
            self.__delegate.history_changed(k)

    def __cmdline_changed(self,v):
        #print 'cmdline:',v

        if not v.is_string():
            return

        self.__cmdline = v.as_string().split(':')
        self.__delegate.cmdline_changed()

    def __history_update(self):
        newhist = {}

        for c in utils.client_iter(self,self.__dpath):
            d = self.child_data_str(self.__dpath+chr(c),2)
            if not d.is_string(): continue
            h = d.as_string().split(':')
            if len(h) != 5: continue
            k = int(h[0])
            newhist[k] = h[1:]

        if newhist:
            self.__min = min(newhist.keys())
            self.__max = max(newhist.keys())
        else:
            self.__min = 0
            self.__max = 0

        self.__history = newhist
 
    def __history_changed(self):
        self.__length=len(self.__history)
        self.__history_update()
        #print '__history_changed',self.__history,'history length',len(self.__history),self.__length
        self.__delegate.history_changed(self.__max)

    def get_history(self,k):
        return self.__history.get(k)

    def get_cmdline(self):
        return self.__cmdline

class LanguageProxy(proxy.AtomProxy):

    monitor = set(['protocols'])

    def __init__(self, address, delegate = None):
        proxy.AtomProxy.__init__(self)
        self.__delegate = delegate or self
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)

        self[2] = proxy.AtomProxy()
        self[11] = HistoryProxy(self.__delegate)

        self.__anchor.set_address_str(address)

        try:
            self.__connector = MetaConnector(self.__delegate)
            self.set_meta_clone(self.__connector)
        except:
            utils.log_exception()

    def get_context(self):
        return __connector.get_context()

    def get_history(self,k):
        return self[11].get_history(k)

    def get_cmdline(self):
        return self[11].get_cmdline()

    def flush(self):
        self[11].flush()

    def yaw_id(self,key):
        assert self.is_ready()
        assert (key>0 and key<3) or key==6
        return ("%s#9.3" % self.id(), str(key+10))

    def roll_id(self,key):
        assert self.is_ready()
        assert (key>0 and key<3) or key==6
        return ("%s#9.2" % self.id(), str(key+10))
     
    def pressure_id(self,key):
        assert self.is_ready()
        assert key>0 and key<5
        return ("%s#9.4" % self.id(), str(key+10))

    def activation_id(self,key):
        assert self.is_ready()
        assert key>0 and key<7
        return ("%s#9.1" % self.id(), str(key+10))

    def node_ready(self):
        print 'proxy ready',self.id()
        if 'langagent' in self.protocols():
            self.__delegate.language_ready()

    def inject(self,msg):
        assert self.is_ready()
        self.invoke_rpc('inject',msg)

    def node_removed(self):
        self.__delegate.language_gone()

    def node_changed(self,parts):
        self.node_removed()
        self.node_ready()

    def language_ready(self):
        pass

    def language_gone(self):
        pass

    def history_changed(self,max):
        pass

    def cmdline_changed(self):
        pass

    def context_changed(self,name,listeners,lurkers):
        print 'context changed to',name,listeners,lurkers
