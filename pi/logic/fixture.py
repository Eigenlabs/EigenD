
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

import terms
import parse
import engine
import unittest

class Fixture(unittest.TestCase):

    def __init__(self,e,*a,**k):
        unittest.TestCase.__init__(self,*a,**k)
        self.engine = e or engine.Engine()
        self.engine.add_module(self)

    def checkFalse(self, query):
        query = parse.parse_termlist(query)
        for ans in self.engine.search(*query):
            self.fail('goal %s is true' % terms.render_termlist(query))

    def checkTrue(self, query):
        query = parse.parse_termlist(query)
        for ans in self.engine.search(*query):
            return

        self.fail('goal %s is false' % terms.render_termlist(query))

    def checkResults(self, query, want):
        query = parse.parse_termlist(query)

        for r in range(len(want)):
            for r2 in want[r]:
                want[r][r2] = parse.parse_clause(want[r][r2])

        unexpected=[]

        def find(result):
            for (i,r) in enumerate(want):
                if r == result:
                    del want[i]
                    return True
            return False

        for r in self.engine.search(*query):
            if not find(r):
                unexpected.append(r)

        if len(want) > 0 or len(unexpected) > 0:
            self.fail(
                terms.render_termlist(query)+
                " missing: "+" ".join([terms.render_result(r) for r in want])+
                " unexpected: "+" ".join([terms.render_result(r) for r in unexpected])
            )

    def checkResult(self, query, **kwds):
        self.checkResults(query,[kwds])

    def checkVar(self,query,var,*values):
        self.checkResults(query,[ {var: v} for v in values ])
