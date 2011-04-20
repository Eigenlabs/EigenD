
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

from pi import logic,async

class StackObj:
    def interpret(self,interp,klass,word):
        return async.success(False)
    def stack_copy(self):
        return self
    def close(self,interp):
        pass

class Referent(StackObj):
    def __init__(self,words=None,objects=None):
        self.set_referent(words=words,objects=objects)

    def reinterpret(self,interp,scope):
        return async.success(self)

    def set_referent(self,words=None,objects=None):
        self.__objects = tuple(objects or ())
        self.__words = tuple(words or ())

    def stack_copy(self):
        return Referent(words=self.__words,objects=self.__objects)

    def __repr__(self):
        return str(self.to_prolog())

    def __str__(self):
        return str(self.to_prolog())

    def generic_objects(self):
        return self.__objects

    def concrete_ids(self):
        return tuple([o.args[0] for o in self.__objects if logic.is_pred_arity(o,'cnc',1) ])

    def words(self):
        return self.__words

    @staticmethod
    def from_prolog(clause):
        if not logic.is_list(clause):
            raise ValueError('Invalid Referent: %s' % clause)
        return Referent(objects=clause)

    def to_prolog(self):
        return self.__objects

