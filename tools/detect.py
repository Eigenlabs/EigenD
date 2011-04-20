
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
import distutils.sysconfig
import os.path

cv = distutils.sysconfig.get_config_vars()

def do_win32():
    executable = sys.executable
    incpath = distutils.sysconfig.get_python_inc(False)
    libpath = os.path.join(distutils.sysconfig.PREFIX,'libs')
    return (executable,incpath,libpath,'Python26','',sys.prefix)

def do_nonframework():
    executable = sys.executable
    version = 'python'+cv['VERSION']
    incpath = distutils.sysconfig.get_python_inc(False)
    library = version
    libpath = os.path.join(distutils.sysconfig.PREFIX,'lib',version,'config')
    return (executable,incpath,libpath,library,'',sys.prefix)

def do_framework():
    executable = sys.executable
    framework = os.path.join(cv['PYTHONFRAMEWORKPREFIX'],cv['PYTHONFRAMEWORKDIR'],'Versions',cv['VERSION'])
    incpath = distutils.sysconfig.get_python_inc(False)
    library = ''
    libpath = ''
    link = os.path.join(framework,'Python')
    wexecutable = os.path.join(framework,'Resources','Python.app','Contents','MacOS','Python')
    return (executable,incpath,libpath,library,link,sys.prefix)

def do_detect():
    if sys.platform == 'win32':
        return do_win32()

    if cv.has_key('PYTHONFRAMEWORK'):
        framework = cv['PYTHONFRAMEWORK']
        if framework:
            return do_framework()
    return do_nonframework()

print ';'.join(do_detect())

