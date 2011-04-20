
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

from pi import node,utils,logic
import piw
import traceback

"""
servers to aid state management
"""

class PersistentNode(node.Server):
    __slots__=('state')
    def __init__(self,value):
        node.Server.__init__(self,value=piw.makestring(value,0))

class PersistentFactory(node.Server):
    """
    State as list of strings.  This server maintains a state which can be
    represented as a list of strings.  Each string is associated with some
    metadata.
    """
    def __init__(self, asserted=None, retracted=None, moved=None):
        node.Server.__init__(self)
        self.__asserted = asserted or self.state_asserted
        self.__retracted = retracted or self.state_retracted
        self.__moved = moved or self.state_moved

    def __change(self,index,node,value):
        if hasattr(node,'state'):
            self.__retracted(index,node.get_data().as_string(), node.state)
            delattr(node,'state')

        if value.is_string():
            v = value.as_string()
            if v:
                node.state = self.__asserted(index,v)
                node.set_data(value)

    def __create(self,index,value):
        n = PersistentNode(value)
        n.set_change_handler(lambda v: self.__change(index,n,v))
        return n

    def assert_state(self, value, state=None):
        i = self.find_hole()

        if state is None:
            state=self.__asserted(i,value)

        n = self.__create(i,value)
        n.state = state
        self[i]=n
        return state

    def state_asserted(self, k, value):
        return None

    def iterstate(self):
        for (k,v) in self.iteritems():
            if hasattr(v,'state'):
                yield v.state

    def find(self, test):
        for (k,v) in self.iteritems():
            if hasattr(v,'state') and test(v.get_data().as_string(),v.state):
                return v.state
        return None

    def retract_state(self, test):
        for (k,v) in self.iteritems():
            val = v.get_data().as_string()
            if hasattr(v,'state') and test(val,v.state):
                del self[k]
                self.__retracted(k,val,v.state)
                return True
        return False

    def move_state(self,dst,test):
        for (k,v) in self.iteritems():
            val = v.get_data().as_string()
            if hasattr(v,'state') and test(val,v.state):
                newval = str(dst)
                v.set_data(piw.makestring(newval,0))
                self.__moved(k,val,newval,v.state)
                return True
        return False

    def state_retracted(self, k, value, state):
        pass

    def state_moved(self,k,oval,nval,state):
        pass

    def dynamic_create(self,index):
        return self.__create(index,'')

    def dynamic_destroy(self,index,node):
        if hasattr(node,'state'):
            self.__retracted(index,node.get_data().as_string(), node.state)
            delattr(node,'state')

class PersistentMetaData:
    """
    State as list of strings.  This server maintains a state which can be
    represented as a list of strings.  Each string is associated with some
    metadata.
    """
    def __init__(self, container, tag, asserted=None, retracted=None):
        self.__container = container
        self.__tag = tag
        self.__asserted = utils.weaken(asserted or self.state_asserted)
        self.__retracted = utils.weaken(retracted or self.state_retracted)
        self.__nodes = {}
        self.__container.add_listener(self.__listener)
        self.__set_termlist()

    def __set_termlist(self):
        self.__container.set_property_string(self.__tag,logic.render_termlist(self.__nodes.keys()))

    def clear(self):
        while self.__nodes:
            (v,s) = self.__nodes.popitem()
            self.__retracted(v,s)

        self.__set_termlist()
        
    def __listener(self,veto,key,new_value):
        if veto:
            return False

        if key != self.__tag:
            return

        if new_value is None or not new_value.is_string():
            new_terms = set([])
        else:
            new_terms = set(logic.parse_clauselist(new_value.as_string()))

        discards = {}
        nodes = {}

        for (v,s) in self.__nodes.iteritems():
            if v in new_terms:
                new_terms.remove(v)
                nodes[v] = s
            else:
                discards[v] = s

        self.__nodes = None

        while discards:
            (v,s) = discards.popitem()
            self.__retracted(v,s)
            v = None
            s = None

        for v in new_terms:
            s = self.__asserted(v)
            nodes[v] = s

        self.__nodes = nodes

    def assert_state(self, value, state=None):
        if state is None:
            state=self.__asserted(value)
            if state is None:
                return None

        self.__nodes[value] = state
        self.__set_termlist()

        return state

    def state_asserted(self, value):
        return None

    def iterstate(self):
        return self.__nodes.itervalues()

    def visit(self, callback):
        for (v,s) in self.__nodes.iteritems():
            callback(v,s)

    def find(self, test):
        for (v,s) in self.__nodes.iteritems():
            if test(v,s):
                return v

        return None

    def retract_state(self, test):
        for (v,s) in self.__nodes.iteritems():
            if test(v,s):
                del self.__nodes[v]
                self.__set_termlist()
                return self.__retracted(v,s)

        return None

    def state_retracted(self, value, state):
        pass
