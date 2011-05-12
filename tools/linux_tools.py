
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

#
# This is all completely barking mad
#

import glob
import os
import os.path
import sys
import SCons.Environment
import unix_tools
import shutil

from os.path import join
from SCons.Util import Split

class PiLinuxEnvironment(unix_tools.PiUnixEnvironment):

    def __init__(self,platform):
        unix_tools.PiUnixEnvironment.__init__(self,platform,'usr/pi','.belcanto','/usr/bin/python')

        self.Append(LIBS=Split('dl m pthread rt'))
        self.Append(CCFLAGS=Split('-D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_REENTRANT -g -O0 -Wall -Werror -fmessage-length=0 -fno-strict-aliasing'))
        self.Append(LINKFLAGS=Split('-g -z origin -Wl,--rpath-link=tmp/bin -Wl,--rpath=\\$$ORIGIN/../bin'))
        self.Append(SHLINKFLAGS=Split('-g -z origin -Wl,--rpath-link=tmp/bin -Wl,--rpath=\\$$ORIGIN/../bin -Wl,-soname=lib${SHLIBNAME}.so'))
        self.Replace(PI_PLATFORMTYPE='linux')
        self.Replace(IS_LINUX=True)

    def __getarch(self):
        return os.popen('dpkg --print-architecture','r').read(1024)

    def set_hidden(self,hidden):
        if hidden:
            self.Append(CCFLAGS='-fvisibility=hidden')

    def PiBinaryDLL(self,target,package=None):
        env = self.Clone()

        f1 = env.File('lib'+target+'.so')

        run_library1=env.Install(env.subst('$BINRUNDIR'),f1)
        env.Alias('target-runtime',run_library1)

        if package:
            env.set_package(package)
            inst_library_1 = env.Install(env['BINSTAGEDIR'],f1)

        return env.addlibname(run_library1[0],target)


    def Initialise(self):
        unix_tools.PiUnixEnvironment.Initialise(self)

    def Finalise(self):
        unix_tools.PiUnixEnvironment.Finalise(self)
        pkgs = [ self.make_package(pkg) for pkg in self.shared.packages ]
        idx = self.File(join('$PKGDIR','Packages.gz'))
        pkgdir = self.Dir('$PKGDIR').abspath

        def make_index(target,source,env):
            cmd=env.subst('cd %s && dpkg-scanpackages . /dev/null | gzip -f >%s' % (pkgdir,target[0].abspath))
            os.system(cmd)

        idxtgt = self.Command(idx,pkgs,make_index)
        self.Alias('target-pkg',idxtgt)

    def debname(self,pkg):
        if pkg in self.shared.packages:
           return 'pi-%s' % pkg.replace('_','-')
        else:
           return pkg

    def make_package(self,pkg):
        meta = self.shared.package_descriptions.get(pkg,None)
        debname = self.debname(pkg)
        arch = self.__getarch().strip()

        if meta is None:
            raise RuntimeError(pkg+' has no metadata')

        assert 'desc' in meta

        env = self.Clone()
        env.Replace(PI_PACKAGENAME=pkg)
        v = meta['version'] or env.subst('$PI_RELEASE')

        template =["Package: %s" % debname,
                   "Version: %s" % v,
                   "Maintainer: support@performance-instruments.com",
                   "Architecture: %s" % arch,
                   "Section: sound",
                   "Priority: optional",
                   "Description: %s" % meta['desc']
                  ]

        if 'longdesc' in meta:
            for l in meta['longdesc'].splitlines():
                l = l.strip()
                if l:
                    template.append(" %s" % l)
                else:
                    template.append(" .")

        if 'depends' in meta and meta['depends']:
            template.append(
                "Depends: %s" % ','.join([self.debname(d) for d in meta['depends']])
            )

        template = "\n".join(template)+"\n"

        ctl = env.File(join('$STAGEDIR','DEBIAN','control'))
        deb = env.File(join('$PKGDIR','%s_%s.deb' % (debname,v)))
	
        env.baker(ctl,template)

        pkgtgt = env.Command(deb,env['STAGEDIR'],'fakeroot sh -c " chown -R 0:0 $SOURCE && chmod -R go-w $SOURCE && chmod -R a+rX $SOURCE && dpkg-deb -b $SOURCE $TARGET "')[0]
        env.Alias('target-pkg',pkgtgt)
        return pkgtgt
