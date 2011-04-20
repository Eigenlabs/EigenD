
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
import domain
import unittest
import piw

class DomainTest(test.SessionFixture):

    def float_checker(self,want,got):
        self.assertAlmostEqual(want,got,places=6)

    def check_domain(self, signature, v,dv, canonical=None):
        dom = domain.traits(signature)
        self.assertEquals(canonical or signature, str(dom))

        if isinstance(v,float):
            checker=self.float_checker
        else:
            checker=self.assertEquals

        checker(v, dom.data2value(dv))
        self.assertEquals(dv, dom.value2data(v))

        n = dom.normalizer()
        d = dom.denormalizer()

        if n and d:
            nv = n(dv)
            dnv = d(nv)
            vdnv = dom.data2value(dnv)
            checker(v,vdnv)

    def test_numeric(self):
        f = 1.1
        i = 10
        f2 = -1.1
        i2 = -10
        b = True
        n = None

        df = piw.makefloat(f,0L)
        di = piw.makelong(i,0L)
        df2 = piw.makefloat(f2,0L)
        di2 = piw.makelong(i2,0L)
        db = piw.makebool(b,0L)
        dn = piw.data()

        self.check_domain('bint(1,10)',i,di)
        self.check_domain('bint(-10,-1)',i2,di2)
        self.check_domain('bfloat(1.1,10.1)',f,df)
        self.check_domain('bfloat(-10.1,-1.1)',f2,df2)
        self.check_domain('bool()',b,db)
        self.check_domain('null()',n,dn)

    def test_string(self):
        sdom = domain.traits('string()')
        self.assertEquals('string()',str(sdom))

        s = 'hello'
        ds = piw.makestring(s,0L)
        n = sdom.normalizer()(ds)
        xn = int(256.0*(n.as_norm()+1.0)/2.0)
        dn = sdom.denormalizer()(n)

        self.assertEquals(xn,ord(s[0]))
        self.assertEquals(str(n.as_norm()),dn.as_string())

    def check_updown(self,domain,val,up,down,min,max,bad=None):
        if isinstance(val,float):
            checker=self.float_checker
        else:
            checker=self.assertEquals

        checker(up,domain.up(val))
        checker(down,domain.down(val))
        checker(min,domain.down(min))
        checker(max,domain.up(max))

        if bad is not None:
            self.assertRaises(ValueError,domain.up,bad)
            self.assertRaises(ValueError,domain.down,bad)

    def test_updown(self):
        self.check_updown(domain.BoundedInt(3,8),5,6,4,3,8)
        self.check_updown(domain.BoundedInt(3,8,2),5,7,3,3,8)
        self.check_updown(domain.Enum(2,3,5,7,11),5,7,3,2,11,6)
        self.check_updown(domain.BoundedFloat(10,110),50.0,51.0,49.0,10.0,110.0)
        self.check_updown(domain.BoundedFloat(10,110,5),50.0,55.0,45.0,10.0,110.0)

        self.assertEquals(True,domain.Bool().up(False))
        self.assertEquals(False,domain.Bool().up(True))
        self.assertEquals(True,domain.Bool().down(False))
        self.assertEquals(False,domain.Bool().down(True))

if __name__ == '__main__':
    unittest.main()
