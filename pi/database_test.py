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


import unittest
from pi import database,logic

class PartofTest(unittest.TestCase):
    
    def test1(self):
        c = database.RelationCache()
        self.failIf(c.relation(1,2))
        self.failIf(c.direct_relation(1,2))
        c.assert_relation(database.Relation('x',1,2))
        self.failUnless(c.relation(1,2))
        self.failUnless(c.direct_relation(1,2))
        c.assert_relation(database.Relation('x',1,2))
        self.failUnless(c.relation(1,2))
        self.failUnless(c.direct_relation(1,2))
        c.retract_relation(database.Relation('x',1,2))
        self.failUnless(c.relation(1,2))
        self.failUnless(c.direct_relation(1,2))
        c.retract_relation(database.Relation('x',1,2))
        self.failIf(c.relation(1,2))
        self.failIf(c.direct_relation(1,2))

    def test2(self):
        c = database.RelationCache()
        c.assert_relation(database.Relation('x',1,2))
        c.assert_relation(database.Relation('x',3,4))
        self.failUnless(c.relation(1,2))
        self.failUnless(c.relation(3,4))
        self.failUnless(c.direct_relation(1,2))
        self.failUnless(c.direct_relation(3,4))
        self.failIf(c.relation(2,3))
        self.failIf(c.relation(1,3))
        self.failIf(c.relation(1,4))
        self.failIf(c.direct_relation(2,3))
        self.failIf(c.direct_relation(1,3))
        self.failIf(c.direct_relation(1,4))

        self.assertEqual(set((2,)),c.direct_rights(1))
        self.assertEqual(set(),c.direct_rights(2))
        self.assertEqual(set((4,)),c.direct_rights(3))
        self.assertEqual(set(),c.direct_rights(4))
        self.assertEqual(set(),c.direct_lefts(1))
        self.assertEqual(set((1,)),c.direct_lefts(2))
        self.assertEqual(set(),c.direct_lefts(3))
        self.assertEqual(set((3,)),c.direct_lefts(4))

        self.assertEqual(set((2,)),c.rights(1))
        self.assertEqual(set(),c.rights(2))
        self.assertEqual(set((4,)),c.rights(3))
        self.assertEqual(set(),c.rights(4))
        self.assertEqual(set(),c.lefts(1))
        self.assertEqual(set((1,)),c.lefts(2))
        self.assertEqual(set(),c.lefts(3))
        self.assertEqual(set((3,)),c.lefts(4))

        c.assert_relation(database.Relation('x',2,3))

        self.assertEqual(set((2,3,4)),c.rights(1))
        self.assertEqual(set((3,4)),c.rights(2))
        self.assertEqual(set((4,)),c.rights(3))
        self.assertEqual(set(),c.rights(4))
        self.assertEqual(set(),c.lefts(1))
        self.assertEqual(set((1,)),c.lefts(2))
        self.assertEqual(set((1,2)),c.lefts(3))
        self.assertEqual(set((1,2,3)),c.lefts(4))

        self.assertEqual(set((2,)),c.direct_rights(1))
        self.assertEqual(set((3,)),c.direct_rights(2))
        self.assertEqual(set((4,)),c.direct_rights(3))
        self.assertEqual(set(),c.direct_rights(4))
        self.assertEqual(set(),c.direct_lefts(1))
        self.assertEqual(set((1,)),c.direct_lefts(2))
        self.assertEqual(set((2,)),c.direct_lefts(3))
        self.assertEqual(set((3,)),c.direct_lefts(4))

        c.retract_relation(database.Relation('x',2,3))

        self.assertEqual(set((2,)),c.direct_rights(1))
        self.assertEqual(set(),c.direct_rights(2))
        self.assertEqual(set((4,)),c.direct_rights(3))
        self.assertEqual(set(),c.direct_rights(4))
        self.assertEqual(set(),c.direct_lefts(1))
        self.assertEqual(set((1,)),c.direct_lefts(2))
        self.assertEqual(set(),c.direct_lefts(3))
        self.assertEqual(set((3,)),c.direct_lefts(4))

        self.assertEqual(set((2,)),c.rights(1))
        self.assertEqual(set(),c.rights(2))
        self.assertEqual(set((4,)),c.rights(3))
        self.assertEqual(set(),c.rights(4))
        self.assertEqual(set(),c.lefts(1))
        self.assertEqual(set((1,)),c.lefts(2))
        self.assertEqual(set(),c.lefts(3))
        self.assertEqual(set((3,)),c.lefts(4))

class CircleTest(logic.Fixture):

    def __init__(self,*a,**k):
        logic.Fixture.__init__(self,database.Database(),*a,**k)

    def setUp(self):
        r = (database.Relation('partof','wheel','car'),
             database.Relation('partof','hub','wheel'),
             database.Relation('partof','car','hub'))

        self.engine.assert_rules(r)

    def test1(self):
        self.checkVar('@partof(O,car)','O','wheel','hub','car')
        self.checkVar('@partof(O,hub)','O','wheel','hub','car')
        self.checkTrue('@partof(hub,car)')
        self.checkTrue('@partof(car,hub)')
        self.checkTrue('@partof(hub,hub)')
        self.checkTrue('@partof(car,car)')

    def test1d(self):
        self.checkVar('@partof_direct(O,car)','O','wheel')
        self.checkFalse('@partof_direct(hub,car)')
        self.checkTrue('@partof_direct(wheel,car)')

    def test1e(self):
        self.checkVar('@partof_extended(O,car)','O','wheel','hub','car')
        self.checkTrue('@partof_extended(hub,car)')
        self.checkTrue('@partof_extended(car,car)')
        self.checkTrue('@partof_extended(wheel,car)')
        self.checkTrue('@partof_extended(wheel,wheel)')
        self.checkTrue('@partof_extended(car,wheel)')

class RulesTest(logic.Fixture):

    def __init__(self,*a,**k):
        logic.Fixture.__init__(self,database.Database(),*a,**k)

    def setUp(self):
        r = (database.Relation('partof','wheel','car'),
             database.Relation('partof','hub','wheel'),
             database.Relation('partof','tyre','wheel'))

        self.engine.assert_rules(r)

    def test1(self):
        self.checkTrue('@partof(wheel,car)')
        self.checkTrue('@partof(hub,car)')
        self.checkTrue('@partof(tyre,car)')
        self.checkTrue('@partof(hub,wheel)')
        self.checkTrue('@partof(tyre,wheel)')

        self.checkFalse('@partof(car,wheel)')
        self.checkFalse('@partof(car,hub)')
        self.checkFalse('@partof(car,tyre)')
        self.checkFalse('@partof(wheel,hub)')
        self.checkFalse('@partof(wheel,tyre)')

        self.checkFalse('@partof(car,car)')
        self.checkFalse('@partof(wheel,wheel)')

    def test1e(self):
        self.checkTrue('@partof_extended(wheel,car)')
        self.checkTrue('@partof_extended(hub,car)')
        self.checkTrue('@partof_extended(tyre,car)')
        self.checkTrue('@partof_extended(hub,wheel)')
        self.checkTrue('@partof_extended(tyre,wheel)')

        self.checkFalse('@partof_extended(car,wheel)')
        self.checkFalse('@partof_extended(car,hub)')
        self.checkFalse('@partof_extended(car,tyre)')
        self.checkFalse('@partof_extended(wheel,hub)')
        self.checkFalse('@partof_extended(wheel,tyre)')

        self.checkTrue('@partof_extended(car,car)')
        self.checkTrue('@partof_extended(wheel,wheel)')

    def test1d(self):
        self.checkTrue('@partof_direct(wheel,car)')
        self.checkFalse('@partof_direct(hub,car)')
        self.checkFalse('@partof_direct(tyre,car)')
        self.checkTrue('@partof_direct(hub,wheel)')
        self.checkTrue('@partof_direct(tyre,wheel)')

        self.checkFalse('@partof_direct(car,wheel)')
        self.checkFalse('@partof_direct(car,hub)')
        self.checkFalse('@partof_direct(car,tyre)')
        self.checkFalse('@partof_direct(wheel,hub)')
        self.checkFalse('@partof_direct(wheel,tyre)')

        self.checkFalse('@partof_direct(car,car)')
        self.checkFalse('@partof_direct(wheel,wheel)')

    def test2(self):
        self.checkVar('@partof(C,car)','C','wheel','tyre','hub')
        self.checkVar('@partof(C,wheel)','C','tyre','hub')
        self.checkVar('@partof(wheel,C)','C','car')
        self.checkVar('@partof(tyre,C)','C','car','wheel')
        self.checkVar('@partof(hub,C)','C','car','wheel')
        self.checkResults('@partof(O,C)',[])

    def test2e(self):
        self.checkVar('@partof_extended(C,car)','C','wheel','tyre','hub','car')
        self.checkVar('@partof_extended(C,wheel)','C','tyre','hub','wheel')
        self.checkTrue('@partof_extended(wheel,wheel)')
        self.checkTrue('@partof_extended(wheel,car)')
        self.checkVar('@partof_extended(wheel,C)','C','car','wheel')
        self.checkVar('@partof_extended(tyre,C)','C','car','wheel','tyre')
        self.checkVar('@partof_extended(hub,C)','C','car','wheel','hub')
        self.checkResults('@partof_extended(O,C)',[])

    def test2d(self):
        self.checkVar('@partof_direct(C,car)','C','wheel')
        self.checkVar('@partof_direct(C,wheel)','C','tyre','hub')
        self.checkVar('@partof_direct(wheel,C)','C','car')
        self.checkVar('@partof_direct(tyre,C)','C','wheel')
        self.checkVar('@partof_direct(hub,C)','C','wheel')
        self.checkResults('@partof_direct(O,C)',[])

if __name__ == '__main__':
    unittest.main()
