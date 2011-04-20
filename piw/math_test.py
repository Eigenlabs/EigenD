
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

from pisession import test
import picross
import math

class MathTest(test.SessionFixture):
    def test_ftable_1(self):
        t = picross.ftable(2)

        t.set_bounds(0.0,10.0)
        t.set_entry(0,0.0)
        t.set_entry(1,1.0)

        self.failUnlessAlmostEqual(0.0,t.interp(0.0))
        self.failUnlessAlmostEqual(0.5,t.interp(5.0))
        self.failUnlessAlmostEqual(1.0,t.interp(10.0))

    def test_ftable_2(self):
        t = picross.ftable(101)

        t.set_bounds(0.0,10.0)

        for i in xrange(0,100):
            v=math.sin(float(i)*2*math.pi/100.0)
            t.set_entry(i,v)

        self.failUnlessAlmostEqual(0.0,t.interp(0.0), places=3)
        self.failUnlessAlmostEqual(1.0,t.interp(2.5), places=3)
        self.failUnlessAlmostEqual(0.0,t.interp(5.0), places=3)
        self.failUnlessAlmostEqual(-1.0,t.interp(7.5), places=3)
        self.failUnlessAlmostEqual(0.0,t.interp(10.0), places=3)
