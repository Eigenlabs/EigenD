
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

import post_install_native
import xmlrpclib

from pi import resource
from app_install import version

def get_latest_release():
    try:
        srv = xmlrpclib.Server('https://www.eigenlabs.com/xmlrpc/')
        (myversion,mytag) = resource.split_version(version.version)
        latest = srv.get_latest_tag('stable')
        latestv = resource.split_version(latest)[0]
        if latestv > myversion:
            return latest
    except:
        pass

    return ''


class Backend(post_install_native.c2p):
    def __init__(self):
        post_install_native.c2p.__init__(self)

    def get_new_version(self):
        return get_latest_release()

    def mediator(self):
        return self.token()

    def set_args(self,argv):
        pass

    def initialise(self,frontend,scaffold):
        self.scaffold = scaffold
        self.frontend = frontend

def main():
    return Backend()
