
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
from pi import atom,domain,utils

class ScrollerDelegate(piw.scrolldelegate):
    def __init__(self,scrollfunc,tapfunc):
        self.__scrollfunc = scrollfunc
        self.__tapfunc=tapfunc
        piw.scrolldelegate.__init__(self)

    def scroll(self,h,v):
        if self.__scrollfunc:
            self.__scrollfunc(h,v)

    def tap(self):
        if self.__tapfunc:
            self.__tapfunc()

class Scroller(atom.Atom):
    def __init__(self,func1,func2,*args,**kwds):
        self.__delegate = ScrollerDelegate(func1,func2)
        self.__scroller = piw.scroller(self.__delegate,0.5,0.5,100)
        atom.Atom.__init__(self,*args,**kwds)

        self[1] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='column', init=None, policy=atom.default_policy(self.__change_column))
        self[2] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='row', init=None, policy=atom.default_policy(self.__change_row))
        self[3] = atom.Atom(domain=domain.Bool(),names='column end relative',init=False,policy=atom.default_policy(self.__change_column_endrel))
        self[4] = atom.Atom(domain=domain.Bool(),names='row end relative',init=False,policy=atom.default_policy(self.__change_row_endrel))
        self.__key_changed()

    def __change_column(self,val):
        self[1].set_value(val)
        self.__key_changed()
        
    def __change_row(self,val):
        self[2].set_value(val)
        self.__key_changed()

    def __change_column_endrel(self,val):
        self[3].set_value(val)
        self.__key_changed()
        
    def __change_row_endrel(self,val):
        self[4].set_value(val)
        self.__key_changed()

    def __make_key_coordinate(self):
        return piw.coordinate(self[1].get_value(),self[2].get_value(),self[3].get_value(),self[4].get_value())

    def __key_changed(self):
        self.__scroller.set_key(self.__make_key_coordinate())

    def cookie(self):
        return self.__scroller.cookie()

    def enable(self):
        self.__scroller.enable()

    def reset(self,x,y):
        self.__scroller.reset(x,y)


class Scroller2(atom.Atom):
    def __init__(self,callback,*args,**kwds):
        self.__scroller = piw.scroller2(utils.changify(callback))
        atom.Atom.__init__(self,*args,**kwds)

        self[1] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='column', init=None, policy=atom.default_policy(self.__change_column))
        self[2] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='row', init=None, policy=atom.default_policy(self.__change_row))
        self[3] = atom.Atom(domain=domain.Bool(),names='column end relative',init=False,policy=atom.default_policy(self.__change_column_endrel))
        self[4] = atom.Atom(domain=domain.Bool(),names='row end relative',init=False,policy=atom.default_policy(self.__change_row_endrel))
        self.__key_changed()

    def __change_column(self,val):
        self[1].set_value(val)
        self.__key_changed()
        
    def __change_row(self,val):
        self[2].set_value(val)
        self.__key_changed()

    def __change_column_endrel(self,val):
        self[3].set_value(val)
        self.__key_changed()
        
    def __change_row_endrel(self,val):
        self[4].set_value(val)
        self.__key_changed()

    def __make_key_coordinate(self):
        return piw.coordinate(self[1].get_value(),self[2].get_value(),self[3].get_value(),self[4].get_value())

    def __key_changed(self):
        self.__scroller.set_key(self.__make_key_coordinate())

    def cookie(self):
        return self.__scroller.cookie()

    def enable(self):
        self.__scroller.enable()
