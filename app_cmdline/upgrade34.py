
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


import sys
import shutil
import os
import optparse

def __mkdir(name):
    if not os.path.exists(name):
        os.mkdir(name)
    if not os.path.isdir(name):
        raise RuntimeError('%s is not a directory' % name)

def __copytree(src,dst):
    try:
        shutil.copytree(src,dst)
    except:
        pass

def cli():
    parser = optparse.OptionParser()
    parser.add_option('--version',action='store',dest='version',default=None,help='previous version')
    (opts,args) = parser.parse_args(sys.argv)

    if not opts.version:
        print 'No version specified.'
        sys.exit(1)

    libdir = os.path.join(os.environ.get('HOME'),'Library')
    olddir = os.path.join(libdir,'Belcanto')
    oldvdir = os.path.join(olddir,opts.version)
    if not os.path.exists(oldvdir):
        print 'Cannot find version directory %s.'%oldvdir
        sys.exit(1)

    newdir = os.path.join(libdir,'EigenD')
    if os.path.exists(newdir):
        print '%s already exists.'%newdir
        sys.exit(1)
    newvdir = os.path.join(newdir,'0.34.1')
    __mkdir(newdir)
    __mkdir(newvdir)

    print 'Copying loops...'
    __copytree(os.path.join(olddir,'loop'), os.path.join(newdir,'loop'))
    print 'Copying soundfonts...'
    __copytree(os.path.join(olddir,'soundfont'), os.path.join(newdir,'soundfont'))
    print 'Copying impulse responses...'
    __copytree(os.path.join(olddir,'impulseresponse'), os.path.join(newdir,'impulseresponse'))
    print 'Copying audiounit templates...'
    __copytree(os.path.join(olddir,'audiounit'), os.path.join(newdir,'audiounit'))
    print 'Copying keyboard data...'
    __copytree(os.path.join(olddir,'keyboard'), os.path.join(newdir,'keyboard'))


    print 'Copying setups from %s...' % opts.version
    __copytree(os.path.join(oldvdir,'state'), os.path.join(newvdir,'setups'))
    print 'Copying audiounit presets from %s...' % opts.version
    __copytree(os.path.join(oldvdir,'audiounit'), os.path.join(newvdir,'audiounit'))
    print 'Copying recordings from %s...' % opts.version
    __copytree(os.path.join(oldvdir,'recorder'), os.path.join(newvdir,'recorder'))
    print 'Copying instruments from %s...' % opts.version
    __copytree(os.path.join(oldvdir,'rig'), os.path.join(newvdir,'instruments'))
    print 'Copying global data from %s...' % opts.version
    __copytree(os.path.join(oldvdir,'global'), os.path.join(newvdir,'global'))
    os.rename(os.path.join(newvdir,'global','current_state'),os.path.join(newvdir,'global','current_setup'))
    os.rename(os.path.join(newvdir,'global','default_state'),os.path.join(newvdir,'global','default_setup'))
    print 'Done.'
