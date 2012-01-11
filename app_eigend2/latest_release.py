
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

from pi import resource
from app_eigend2 import version

import piw
import threading
import xmlrpclib

class HttpError(RuntimeError):
    pass

def get_latest_release(cookie,additional=""):
    try:
        srv = xmlrpclib.Server('https://www.eigenlabs.com/xmlrpc/')
        (myversion,mytag) = resource.split_version(version.version)
        if not mytag: mytag='stable'
        #latest = srv.get_latest_tag(mytag)
        latest = srv.get_latest_tag_extra(mytag,cookie,version.version,additional)
        latestv = resource.split_version(latest)[0]
        if latestv > myversion:
            return latest
    except:
        pass

    return ''

class LatestReleasePoller(threading.Thread):
    def __init__(self,backend):
        threading.Thread.__init__(self)
        self.__event = threading.Event()
        self.__context = piw.tsd_snapshot()
        self.__backend = backend
        self.__cookie = ""

    def start(self,cookie,info):
        self.__cookie = cookie
        self.__info = info
        threading.Thread.start(self)

    def run(self):
        self.__context.install()
        while True:
            print 'latest release poller running',self.__cookie
            self.__backend.set_latest_release(get_latest_release(self.__cookie,self.__info))
            print 'latest release poller sleeping'
            self.__event.wait(3600)
            self.__event.clear()

        print 'latest release poller exiting'
