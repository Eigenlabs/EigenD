
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

def posix_Linux_ppc64():
    import linux_tools
    env = linux_tools.PiLinuxEnvironment('linux-ppc32')
    env.Replace(IS_LINUX_PPC32=True)
    env.Replace(IS_BIGENDIAN=True)
    return env

def posix_Linux_ppc():
    import linux_tools
    env = linux_tools.PiLinuxEnvironment('linux-ppc32')
    env.Replace(IS_LINUX_PPC32=True)
    env.Replace(IS_BIGENDIAN=True)
    return env

def posix_Linux_x86_64():
    import linux_tools
    env = linux_tools.PiLinuxEnvironment('linux-x86-64')
    env.Replace(IS_LINUX_8664=True)
    return env

def posix_Linux_i686():
    import linux_tools
    env = linux_tools.PiLinuxEnvironment('linux-86')
    env.Replace(IS_LINUX_86=True)
    return env

def posix_Darwin_x86_64():
    import darwin_tools
    env = darwin_tools.PiDarwinEnvironment('macosx-i386')
    env.Replace(IS_MACOSX_86=True)
    return env

def posix_Darwin_i386():
    import darwin_tools
    env = darwin_tools.PiDarwinEnvironment('macosx-i386')
    env.Replace(IS_MACOSX_86=True)
    return env

def posix_Darwin_Power_Macintosh():
    import darwin_tools
    env = darwin_tools.PiDarwinEnvironment('macosx-ppc')
    env.Replace(IS_BIGENDIAN=True)
    env.Replace(IS_MACOSX_PPC32=True)
    return env

def posix():
    from posix import uname
    p = ('posix_'+uname()[0]+'_'+uname()[4]).replace(' ','_')

    if globals().has_key(p):
        return globals()[p]()

    raise RuntimeError("unsupported posix platform %s" % p)

def win32():
    import windows_tools
    env = windows_tools.PiWindowsEnvironment()
    return env

darwin=posix
linux2=posix

def select():
    if globals().has_key(sys.platform):
        return globals()[sys.platform]()

    raise RuntimeError("unsupported platform %s" % sys.platform)
