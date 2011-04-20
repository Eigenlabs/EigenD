
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

from pi import action, atom, bundles, const, domain

import piw

class Toggle(atom.Atom):
    def __init__(self,e,d,**kw):
        self.__enable = e
        atom.Atom.__init__(self,domain=domain.Bool(),protocols='set',policy=atom.default_policy(self.__change),**kw)
        self.add_verb2(1,'set([],~a,role(None,[instance(~self)]))', self.__verb_set)
        self.add_verb2(2,'set([un],~a,role(None,[instance(~self)]))', self.__verb_unset)
        self.add_verb2(3,'set([toggle],~a,role(None,[instance(~self)]))', callback=self.__verb_togset, status_action=self.__status)
        self[1]=bundles.Output(1,False,names='status output')
        self.light_output=bundles.Splitter(d,self[1])
        self.lights=piw.lightsource(piw.change_nb(),0,self.light_output.cookie())
        self.lights.set_size(1)
        self.__set_status(self.get_value())
        self.__state = False

    def __status(self,*a):
        return 'dsc(~(s)".1","1")'

    def __set_status(self,active):
        self.lights.set_status(1,const.status_active if active else const.status_inactive)

    def __change(self,e):
        self.__state = e
        self.__set_status(e)
        self.set_value(e)
        if self.__enable:
            self.__enable(e)
        return False

    def __verb_set(self,*a):
        self.__change(True)
        return action.nosync_return()

    def __verb_unset(self,*a):
        self.__change(False)
        return action.nosync_return()

    def __verb_togset(self,*a):
        self.__change(not self.__state)
        return action.nosync_return()

    def notify(self):
        self.__state = False
        self.__set_status(False)
        self.set_value(False)
