#!/usr/bin/env python
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


import atom,domain,const,policy
from pisession import test
from pi import paths
import unittest
import piw

def _s(s):
    return piw.makestring(s,0)

class AtomTest(test.SessionFixture):

    def test_slow2slow(self):
        slow1 = atom.Atom(domain=domain.BoundedInt(0,10))
        slow2 = atom.Atom(domain=domain.BoundedInt(0,100))

        self.run_server('slow1',slow1)
        self.run_server('slow2',slow2)

        proxy1 = yield self.atom_check('slow1')
        proxy2 = yield self.atom_check('slow2')

        self.assertEquals('slow1',slow1.id())
        self.assertEquals('slow2',slow2.id())
        yield self.bool_check('slow1#255.13',False)

        slow2.set_value(66)
        yield self.int_check('slow2#254',66)
        slow1.slave_connect(slow2.id())

        yield self.bool_check('slow1#255.13',True)
        yield self.int_check('slow1#254',6)

        slow2.set_value(33)
        yield self.int_check('slow2#254',33)
        yield self.int_check('slow1#254',3)

        slow2.set_value(66)
        yield self.int_check('slow2#254',66)
        yield self.int_check('slow1#254',6)

        slow1.slave_connect('')
        yield self.bool_check('slow1#255.13',False)
        yield self.int_check('slow1#254',6)

if __name__ == '__main__':
    unittest.main()
