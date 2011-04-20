
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

from pisession import upgrade,session,version
from pi import resource,state
import optparse,os,sys,traceback,hashlib

def cli():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] db-file')
    parser.add_option('--text',action='store_true',dest='text',default=False,help='dump text version')

    (opts,args) = parser.parse_args(sys.argv)
    args=args[1:]

    if len(args)!=1:
        parser.error('wrong number of arguments')

    def session_main(manager):
        db = state.open_database(args[0],False)
        snap = db.get_trunk()
        sig = upgrade.get_setup_signature(snap,opts.text)
        print sig
        sys.exit(0)

    session.run_session(session_main,name='signature')
