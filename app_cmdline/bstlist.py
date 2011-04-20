
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

from pi import utils,paths,resource,state
from pisession import session
import picross

import time
import sys
import optparse
import os

def dump_node(db,snap,address,agent,path):
    node = agent.get_root()
    for e in path:
        if node.enum_children(e-1)!=e:
            print 'No such path',address
            return
        node = node.get_child(e)

    print "Snapshot: ",snap.version(),'Agent:',address,'Type:',agent.get_type(),'Value:',node.get_data()
    e=node.enum_children(0)
    while e!=0:
        print "%4u:%s" % (e,node.get_child(e).get_data())
        e=node.enum_children(e)

def dump_agent(db,snap,address):
    (name,path) = paths.breakid_list(address)
    agents = snap.agent_count()
    for i in range(0,agents):
        agent = snap.get_agent_index(i)
        if agent.get_address() == name:
            dump_node(db,snap,address,agent,path)
            return

    print 'No agent',name

def dump_version(db,version,address):
    snap = db.get_trunk()

    if version != 'trunk':
        while True:
            if snap.tag() == version or str(snap.version()) == version: break
            p = snap.previous()
            if not p:
                snap = None
                break
            snap = db.get_version(p)

    if snap is None:
        print 'No version',version
        return

    if address:
        dump_agent(db,snap,address[0])
        return

    print "Snapshot: ",snap.version()
    print "Agent                               Type Version Signature"
    print "-----                               ---- ------- ---------"

    agents = snap.agent_count()
    for i in range(0,agents):
        agent = snap.get_agent_index(i)
        print "%-35s %-4d %10u %s" % (agent.get_address(),agent.get_type(),agent.get_checkpoint(),agent.get_root().get_data())

def dump_versions(db):
    snap = db.get_trunk()

    print "Version     Timestamp                  Tag"
    print "-------     ---------                  ---"

    while True:
        p = snap.previous()
        t = time.ctime(snap.timestamp()/1000000.0)
        v = snap.version()
        l = snap.tag() or '-'
        print "%-10u  %-24s   %s" % (v,t,l)
        if not p: break
        snap = db.get_version(p)

@utils.nothrow
def main(manager):

    parser = optparse.OptionParser()
    parser.add_option('--db',action='store',dest='db',default=None,help='db file')
    parser.add_option('--version',action='store',dest='version',default=None,help='version')
    parser.add_option('--target',action='store',dest='target',default='micro',help='target')

    (opts,args) = parser.parse_args(sys.argv)
    args=args[1:]

    if opts.db:
        dbfile = opts.db
    else:
        dbfile = resource.user_resource_file(resource.setup_dir,opts.target)

    db = state.open_database(dbfile,False)

    if opts.version:
        dump_version(db,opts.version,args)
    else:
        dump_versions(db)

    picross.exit(0)

def cli():
    session.run_session(main,name='bstlist')
