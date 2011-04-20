
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

import picross
import sys
import piw
import os
import optparse

from pi import utils,resource,state
from pisession import session

def pathextend(path,i):
    if not path: return "%u"%i
    return "%s.%u" % (path,i)

def dump_tree(node,agent,path=''):
    print "%s#%s: %s" % (agent,path,str(node.get_data()))
    i=node.enum_children(0)
    while i!=0:
        dump_tree(node.get_child(i), agent, pathextend(path,i))
        i=node.enum_children(i)

def dump_agent(agent):
    root = agent.get_root()
    dump_tree(root,agent.get_address())

def dump_snapshot(snap):
    agents = snap.agent_count()
    print 'version:',snap.version(),'(',snap.previous(),'),',agents,'agents'
    for i in range(0,agents):
        agent = snap.get_agent_index(i)
        dump_agent(agent)

@utils.nothrow
def main(manager):
    parser = optparse.OptionParser()
    parser.add_option('--db',action='store',dest='db',default=None,help='db file')
    parser.add_option('--version',action='store',dest='version',default=None,help='version')
    parser.add_option('--target',action='store',dest='target',default='micro',help='target')

    (opts,args) = parser.parse_args(sys.argv)

    if opts.db:
        dbfile = opts.db
    else:
        dbfile = resource.user_resource_file(resource.setup_dir,opts.target)

    db = state.open_database(dbfile,False)

    if opts.version:
        snap = db.get_version(long(opts.version))
    else:
        snap = db.get_trunk()

    dump_snapshot(snap)

def cli():
    session.run_session(main,name='bstdump')
