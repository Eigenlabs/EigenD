
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

import os
import sys

def get_resource(name,level=1):
    try:
        raise RuntimeError
    except RuntimeError:
        e,b,t = sys.exc_info()
        f = t.tb_frame
        while level>0:
            f = f.f_back
            level = level-1
        caller_globals = f.f_globals

    if '__loader__' not in caller_globals:
        f = caller_globals['__file__']
        f2 = os.path.join(os.path.dirname(f),name)
        return open(f2).read()

    n = caller_globals['__name__'].replace('.','/')
    n2 = os.path.join(os.path.dirname(n),name)

    return caller_globals['__loader__'].get_data(n2)


def get_pkg_resource(pkg,name):
    p = __import__(pkg)

    if not hasattr(p,'__loader__'):
        f = getattr(p,'__file__')
        f2 = os.path.join(os.path.dirname(f),name)
        return open(f2,'rb').read()

    n = getattr(p,'__file__')
    n2 = os.path.join(os.path.dirname(n),name)
    return getattr(p,'__loader__').get_data(n2)
