
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

import recorder_native
from pisession import session
from pi import paths
import sys
import os

def filedump(filename):
    rec = recorder_native.read(filename)
    base = os.path.basename(filename)

    print '%s s=%u w=%u l=%u' % (base,rec.signals(),rec.wires(),rec.events())

    rec.reset()
    while rec.isvalid():
        evt = rec.cur_event()
        evt.reset()
        print 'event t=%u b=%s %s' % (evt.evt_time(),evt.evt_beat(),evt.evt_id())
        while evt.isvalid():
            print ' t=%u b=%s s=%u %s' % (evt.cur_time(),evt.cur_beat(),evt.cur_signal(),evt.cur_value())
            evt.next()
        rec.next()

def main():
    def startup(dummy):
        filedump(sys.argv[1])
    session.run_session(startup,clock=False)
