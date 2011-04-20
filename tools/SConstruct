
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

import glob
import os
import select_tools

from os.path import join,dirname

EnsureSConsVersion(0,96,91)

master_env = select_tools.select()
master_env.Initialise()

#CacheDir(master_env.cache_dir())
SConscriptChdir(0)
SConsignFile()

BuildDir(join(master_env.subst('$TMPDIR')),'.',duplicate=0)

Export(master_env=master_env)

for root,dirs,files in os.walk('.'):
    if 'SConscript.first' in files:
        env=master_env.Clone()
        SConscript(join(master_env.subst('$TMPDIR'),root,'SConscript.first'))

for root,dirs,files in os.walk('.'):
    if 'SConscript' in files:
        env=master_env.Clone()
        SConscript(join(master_env.subst('$TMPDIR'),root,'SConscript'), exports='env')

master_env.Finalise()

ctags=None
if master_env.Detect('ctags'): ctags='ctags'
if master_env.Detect('ctags-exuberant'): ctags='ctags-exuberant'
if master_env.Detect('exctags'): ctags='exctags'
if master_env.Detect('exuberant-ctags'): ctags='exuberant-ctags'

if ctags and not os.getenv('PI_NOTAGS'):
    srcfiles1 = map(dirname,glob.glob(join('*','SConscript')))
    srcfiles2 = map(dirname,glob.glob(join('*','*','SConscript')))
    tags = master_env.Command('#tags',[],ctags+' --langdef=pip --langmap=pip:.pip --extra=+fq --c-kinds=+pxvtmdc --recurse %s' % ' '.join(srcfiles1+srcfiles2))
    master_env.AlwaysBuild(tags)
    master_env.Alias('target-default',tags)

master_env.Default(master_env.Alias('target-default'))
