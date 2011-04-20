
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

from pi import logic,resource,timeout,async

def writelib(kbd,i,libfile):
    print 'writelib',i,libfile
    for line in open(libfile).readlines():
        p,r,y = map(int,line.split())
        kbd.testmsg_write_lib(i,p,r,y)
    kbd.testmsg_finish_lib()

def writeseq(kbd,seq):
    print 'writeseq',seq
    for l,k,t in seq:
        kbd.testmsg_write_seq(l,k,t)
    kbd.testmsg_finish_seq()

def runtest(kbd,clause):
    term = logic.parse_clause(clause)
    duration = int(term.args[0])
    keypresses = term.args[1:]

    libs = {}
    for i,k in enumerate(set([k.pred for k in keypresses])):
        libs[k] = i
        print 'sending lib message',i,k
        libfile = resource.find_resource('keyboard','testlib_%s' % k)
        if not libfile:
            print k,'keypress not found'
            return

        writelib(kbd,i,libfile)

    seq = []
    for t in keypresses:
        assert t.args[0]>0
        key = int(t.args[0]-1)
        time = int(t.args[1])
        seq.append((libs[t.pred],key,time))

    print 'sending sequence len',len(seq)
    writeseq(kbd,seq)

    print 'running test'
    kbd.start_test(duration)
    print 'waiting',duration,'ms for completion'
    return timeout.Timer(duration)

def arm_recording(kbd,clause):
    term = logic.parse_clause(clause)
    duration = int(term.args[0])
    kbd.arm_recording(duration)
