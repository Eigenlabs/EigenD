
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

import generic_tools
import os
from SCons.Util import Split


class PiUnixEnvironment(generic_tools.PiGenericEnvironment):
    def __init__(self,platform,install_prefix,userdir_suffix,python = None):
        generic_tools.PiGenericEnvironment.__init__(self,platform,install_prefix,userdir_suffix,python)
        self.Append(LINKFLAGS=Split('-ggdb'))
        self.Append(SHLINKCOM=' $LIBMAPPER')
        self.Append(LINKCOM=' $LIBMAPPER')

    def Initialise(self):
        generic_tools.PiGenericEnvironment.Initialise(self)

    def Finalise(self):
        generic_tools.PiGenericEnvironment.Finalise(self)
        self.doenv()

    def doenv(self):
        pth_file = self.File('env.sh',self.subst('#')).abspath

        pth_template=("#!/bin/sh\n"
                      "#This script is auto generated - do not bother editing\n"
                      "#source this ...\n"
                      "export PATH=%(bindir)s:$PATH\n")

        pp=self.Dir(self['BINRUNDIR']).abspath
        pth_node = self.baker(pth_file,pth_template,bindir=pp)
        self.Alias('target-default','#env.sh')

