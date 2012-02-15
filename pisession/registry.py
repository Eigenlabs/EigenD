
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

import glob,zipfile,os
import sys
import picross
import imp
from pi import resource

def iscompatible(mod_version, state_version):
    mod_version = mod_version.split('.')
    state_version = state_version.split('.')

    if len(state_version)==2:
        state_version.insert(0,'0')

    if len(mod_version)==2:
        mod_version.insert(0,'0')

    if mod_version[0] != state_version[0]:
        return False

    if mod_version[1] < state_version[1]:
        return False

    if mod_version[1] > state_version[1]:
        return True

    if mod_version[2] < state_version[2]:
        return False

    return True

def import_module(full_path):
    mod_name = os.path.basename(full_path)
    mod_path = os.path.dirname(full_path)
    pkg_name = os.path.basename(mod_path)
    pkg_path = os.path.dirname(mod_path)

    if pkg_path not in sys.path:
        sys.path.insert(0,pkg_path)

    pkg = __import__('%s.%s' % (pkg_name,mod_name))

    return getattr(pkg,mod_name)

class Registry:
    def __init__(self,klass):
        self.__registry={}
        self.__alias={}
        self.__path = []
        self.__vocab = {}

        self.add_extra()
        self.add_path(resource.user_resource_dir('Plugins'))
        self.add_path(os.path.join(picross.release_root_dir(),'plugins'))
        self.add_path(os.path.join(picross.contrib_root_dir(),'plugins'))

        for p in self.__path:
            print 'Agent Path:',p
            self.scan_path(p,klass)

    def get_vocab(self):
        return self.__vocab

    def add_extra(self):
        extra_dir = os.path.join(picross.global_library_dir(),'Global')
        extra = os.path.join(extra_dir,'paths.txt')

        try: os.makedirs(extra_dir)
        except: pass

        if not os.path.exists(extra):
            f = open(extra,'w')
            f.write('# add plugin paths here\n')
            f.close()
            return

        paths = open(extra,'r').read()
        for p in paths.splitlines():
            p = p.strip()
            if p.startswith('#'): continue
            if os.path.exists(p):
                self.add_path(p)

    def add_vocab(self,e,m,c):
        self.__vocab[e] = (m,c)

    def add_path(self,path):
        if path not in self.__path:
            self.__path.append(path)

    def dump(self,dumper):
        for (mname,vlist) in self.__registry.iteritems():
            for (version,(cversion,module)) in vlist.iteritems():
                print 'plugin %s:%s:%s %s' % (mname,version,cversion,dumper(module))

        for (e,(m,c)) in self.__vocab.iteritems():
            print 'vocab %s %s %s' % (e,m,c)

    def modules(self):
        return self.__registry.keys()

    def get_alias(self,name):
        versions = self.__alias.get(name)
        if not versions:
            return None
        else:
            vkeys = versions.keys()
            vkeys.sort(reverse=True)
            return versions[vkeys[0]]

    def get_module(self,name):
        versions = self.__registry.get(name)

        if not versions:
            orig = self.get_alias(name)
            versions = self.__registry.get(orig)
            if not versions:
                return None

        vkeys = versions.keys()
        vkeys.sort(reverse=True)
        (cversion,module) = versions[vkeys[0]]
        return module

    def get_compatible_module(self,name,state_cversion):
        actual_module = None
        actual_cversion = None

        for (mod_version,mod_cversion,mod) in self.iter_versions(name):
            if iscompatible(mod_cversion,state_cversion):
                if actual_module is None or mod_cversion>actual_cversion:
                    actual_module = mod
                    actual_cversion = mod_cversion

        return actual_module

    def iter_versions(self,name):
        mlist = self.__registry.get(name)
        if not mlist:
            orig = self.get_alias(name)
            mlist = self.__registry.get(orig)

        if mlist:
            for (version,(cversion,module)) in mlist.iteritems():
                    yield (version,cversion,module)

    def add_module(self,name,version,cversion,module):
        r = self.__registry

        if name not in r:
            r[name] = {}

        if version not in r[name]:
            r[name][version] = (cversion,module)
            return

        print 'module %s:%s already defined' % (name,version)


    def add_alias(self,name,version,original):
        a = self.__alias

        if name not in a:
            a[name] = {}

        if version not in a[name]:
            a[name][version] = original
            return

        print 'alias %s:%s already defined' % (name,version)


    def __find_paths(self,path):
        p = set()

        try:
            for (root,dirs,files) in os.walk(path):
                if 'Manifest' in files:
                    p.add(root)
        except:
            pass

        return list(p)

    def scan_path(self,directory,klass):
        for p in self.__find_paths(directory):
            try:
                manifest = open(os.path.join(p,'Manifest'),'r').read()
                pkg = os.path.basename(p)
            except:
                continue

            for a in manifest.splitlines():
                a = a.strip()
                asplit = a.split()

                if len(asplit)==1:
                    asplit=['agent']+asplit

                if len(asplit)!=2:
                    continue

                if asplit[0] == 'agent':
                    a = asplit[1].strip().split(':')
                    (name,module,cversion,version) = a[0:4]
                    fullmodule = os.path.join(p,module)
                    self.add_module(name,version,cversion,klass(name,version,cversion,fullmodule))
                    for e in a[4:]:
                        self.add_alias(e,version,name)
                    continue

                if asplit[0] == 'vocab':
                    (e,m,c) = asplit[1].strip().split(':')
                    self.add_vocab(e,m,c)
