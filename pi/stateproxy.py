
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
Module for talking to a standard state manager
"""

import piw,const,proxy,node

class StateProxy(proxy.AtomProxy):

    monitor = set(['protocols'])

    def __init__(self, delegate = None):
        proxy.AtomProxy.__init__(self)
        self.__delegate = delegate or self
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)

    def set_address(self,address):
        self.__anchor.set_address_str(address)

    def get_address(self):
        return self.__anchor.get_address()

    def node_ready(self):
        self.__delegate.statemanager_ready()

    def node_removed(self):
        self.__delegate.statemanager_gone()

    def get_state(self):
        assert(self.is_ready())
        return self.invoke_rpc('get','')

    def load_state(self,state):
        assert(self.is_ready())
        return self.invoke_rpc('load',state)

    def rewind_state(self,state):
        assert(self.is_ready())
        return self.invoke_rpc('rewind',state)

    def statemanager_ready(self):
        pass

    def statemanager_gone(self):
        pass


