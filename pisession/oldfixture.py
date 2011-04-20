
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

import time
import unittest

import piagent
import picross
import piw

from pi import utils

class Fixture(unittest.TestCase):
    def __init__(self,tests,logger=None,**kwds):
        unittest.TestCase.__init__(self,tests)
        self.manager = piagent.scaffold_st()
        self.logger = logger

    def setUp(self):
        self.context = self.manager.context(utils.notify(None),utils.stringify(self.logger),'test')
        piw.setenv(self.context.getenv())

        if self.logger is not None:
            self.oldstd = sys.stdout,sys.stderr
            sys.stdout,sys.stderr = self.logger.Logger(),sys.stdout

    def tearDown(self):
        self.context.kill()
        self.context = None

    def once(self):
        self.manager.once()
