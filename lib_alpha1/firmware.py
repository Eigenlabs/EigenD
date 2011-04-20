
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

import os.path
import picross

def find_release_resource(category,name):
    res_root = picross.release_resource_dir()
    reldir = os.path.join(res_root,category)

    if not os.path.exists(reldir):
        return None

    filename = os.path.join(reldir,name)

    if os.path.exists(filename):
        return filename

    return None

def firmware_location(vendor,product):
    return find_release_resource('firmware','alpha1.bin')
