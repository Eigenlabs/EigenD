
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
        self.__container.add_listener(self)
        self.__set_termlist()

    def __set_termlist(self):
        self.__container.set_property_string(self.__tag,logic.render_termlist(self.__nodes.keys()))

    def clear(self,destroy=False):
        while self.__nodes:
            (v,s) = self.__nodes.popitem()
            self.__retracted(v,s,destroy)

        self.__set_termlist()
        
    def property_veto(self,key,new_value):
        return False

    def property_change(self,key,new_value,delegate):
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
            self.__retracted(v,s,False)
            v = None
            s = None

        for v in new_terms:
            s = self.__asserted(v,delegate)
            nodes[v] = s

        self.__nodes = nodes

    def assert_state(self, value, state=None, delegate=None):
        if state is None:
            state=self.__asserted(value,delegate)
            if state is None:
                return None

        self.__nodes[value] = state
        self.__set_termlist()

        return state

    def state_error(self, bad_sigs):
        return None

    def state_asserted(self, value, delegate):
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

    def retract_state(self, test, destroy=False):
        for (v,s) in self.__nodes.iteritems():
            if test(v,s):
                del self.__nodes[v]
                self.__set_termlist()
                return self.__retracted(v,s,destroy)

        return None

    def state_retracted(self, value, state, destroy=False):
        pass
