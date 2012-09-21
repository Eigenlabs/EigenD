
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
import shutil
import zipfile

from os.path import join
from SCons.Util import Split

class PiGenericEnvironment(SCons.Environment.Environment):

    class __Shared:
        pass

    class __Library:
        def __init__(self,name):
            self.name = name
            self.users = []
            self.libnode = None
            self.deps = []

        def set_library(self,libnode, dependencies):
            assert self.libnode is None
            self.libnode = libnode
            self.deps = dependencies

        def add_user(self,user_node):
            self.users.append(user_node)

        def fixup(self,env):
            if self.libnode is None:
                #raise RuntimeError('library %s used but not defined' % self.name)
                return
            for u in self.users:
                env.Depends(u,self.libnode)

    def cache_dir(self):
        return os.path.join(os.environ.get('HOME'),'.sconscache')

    def get_shlib(self,libname):
        if libname not in self.shared.shlibmap:
            self.shared.shlibmap[libname] = self.__Library(libname)
        return self.shared.shlibmap[libname]

    def addlibname(self,libobj,libname,dependencies=[]):
        l = self.get_shlib(libname)
        l.set_library(libobj,dependencies)
        return libobj

    def addlibuser(self,target,libnames):
        for libname in libnames:
            l = self.get_shlib(libname)
            l.add_user(target)

    def __libs_fixup(self):
        for l in self.shared.shlibmap.itervalues():
            l.fixup(self)

    def __runtime_fixup(self):
        r = self.shared.runtime
        for u in self.shared.runtime_user:
            self.Depends(u,r)

    def add_runtime_files(self,files):
        self.shared.runtime.extend(files)

    def add_runtime_user(self,files):
        self.shared.runtime_user.extend(files)

    def set_hidden(self,hidden):
        pass

    def PiExports(self,token,package,public):
        text = exports_template % dict(libtoken=token.upper())
        target = '%s_exports.h' % token

        def action(target,source,env):
            t = target[0].abspath
            outp = file(t,"w")
            outp.write(source[0].value)
            outp.close()
            os.chmod(t,0755)

        run_exp = self.Command(join(self['EXPDIR'],target),self.Value(text),action)

        if package and public:
            e2 = self.Clone()
            e2.set_package(package)
            root1 = e2.subst('$HDRSTAGEDIR')
            e2.Install(root1,run_exp)

    def PiSharedLibrary(self,target,sources,libraries={},package=None,hidden=True,deffile=None,per_agent=None,public=False,locked=False):
        env = self.Clone()
        env.Append(PILIBS=libraries)
        env.Append(CCFLAGS='-DBUILDING_%s' % target.upper())
        env.Replace(SHLIBNAME=target)
        env.set_hidden(hidden)

        env.set_agent_group(per_agent)

        objects = env.SharedObject(sources)

        self.Depends(objects,self.PiExports(target,package,public))

        bin_library=env.SharedLibrary(target,objects)
        run_library=env.Install(env.subst('$BINRUNDIR'),bin_library)

        inst_library = []

        env.addlibuser(bin_library[0],libraries)
        env.addlibname(run_library[0],target,dependencies=libraries)

        if package:
            env.set_package(package)
            inst_library = env.Install(env.subst('$BINSTAGEDIR'),run_library)

        return inst_library+run_library

    @staticmethod
    def safe_mkdir(d):
        try:
            os.makedirs(d)
        except:
            pass

    def Clone(self,*args,**kwds):
        g = self.shared
        c = SCons.Environment.Environment.Clone(self,*args,**kwds)
        c.shared = g
        return c

    def clear_dir(self,var):
        path = self.Dir(self[var]).abspath
        print 'clearing',path
        shutil.rmtree(path, ignore_errors=True)
        self.safe_mkdir(path)

    def Initialise(self):
        self.clear_dir('STAGEDIR')
        self.clear_dir('PKGDIR')

    def Finalise(self):
        self.__libs_fixup()
        self.__runtime_fixup()

        if not self.shared.release:
            raise RuntimeError('PiRelease not called')

        print "building release",self.subst('$PI_RELEASE'),'compatible with',self.subst('$PI_COMPATIBLE')

        for (k,v) in self.shared.agent_groups.items():
            self.__build_manifest(k,v[0],v[1],v[2])


    def __getpython(self):
        status=os.popen('"%s" %s' % (self['PI_PYTHON'],join(os.path.dirname(__file__),'detect.py')),'r').read(1024)
        (exe,incpath,libpath,libs,linkextra,prefix) = [s.strip() for s in status.split(';')]

        self.Append(CPPPATH=[incpath])
        self.Append(LIBS=libs)
        self.Append(LIBPATH=libpath)
        self.Append(LINKFLAGS=linkextra)
        self.Replace(PI_PREFIX=prefix or None)

    def libmapper(self,target,source,env,for_signature):
        libs = env['PILIBS']
        map = []

        for l in libs:
            ll=env.get_shlib(l)
            if not ll.libnode:
                map.append('-l%s' % l)
            else:
                map.append('"%s"' % ll.libnode.abspath)

        return ' '.join(map)

    def get_pyd_run_dir(self,target,source,env,for_signature):
        if 'PI_AGENTGROUP' in env:
            return env.subst('$PYDRUNDIR_PLUGIN')
        return env.subst('$PYDRUNDIR_GLOBAL')

    def get_pyd_stage_dir(self,target,source,env,for_signature):
        if 'PI_AGENTGROUP' in env:
            return env.subst('$PYDSTAGEDIR_PLUGIN')
        return env.subst('$PYDSTAGEDIR_GLOBAL')

    def get_mod_run_dir(self,target,source,env,for_signature):
        if 'PI_AGENTGROUP' in env:
            return env.subst('$MODRUNDIR_PLUGIN')
        return env.subst('$MODRUNDIR_GLOBAL')

    def get_mod_stage_dir(self,target,source,env,for_signature):
        if 'PI_AGENTGROUP' in env:
            return env.subst('$MODSTAGEDIR_PLUGIN')
        return env.subst('$MODSTAGEDIR_GLOBAL')

    def get_bin_run_dir(self,target,source,env,for_signature):
        if 'PI_AGENTGROUP' in env:
            return env.subst('$BINRUNDIR_PLUGIN')
        return env.subst('$BINRUNDIR_GLOBAL')

    def get_bin_stage_dir(self,target,source,env,for_signature):
        if 'PI_AGENTGROUP' in env:
            return env.subst('$BINSTAGEDIR_PLUGIN')
        return env.subst('$BINSTAGEDIR_GLOBAL')

    def __init__(self,platform,install_prefix,userdir_suffix,python = None,**kwds):
        SCons.Environment.Environment.__init__(self,ENV = os.environ,**kwds)

        self.py_template = generic_py_template

        self.shared = self.__Shared()
        self.shared.package_descriptions = {}
        self.shared.packages = []
        self.shared.agent_groups = {}
        self.shared.shlibmap = {}
        self.shared.release = None
        self.shared.collections = {}
        self.shared.lib_pkgs = {}
        self.shared.py_pkgs = {}
        self.shared.runtime = []
        self.shared.runtime_user = []
        self.shared.organisation = 'Eigenlabs'

        self.Replace(PI_PLATFORM=platform)
        self.Replace(PI_PYTHON=os.environ.get('PI_PYTHON',python) or sys.executable)

        self.__getpython()

        print "platform: %s python: %s" % (platform,self['PI_PYTHON'])

        #self.set_tool('pip',self.WhereIs('pip'))

        self.Replace(ROOTINSTALLDIR=os.path.join('/',install_prefix))

        self.Replace(LIBMAPPER=self.libmapper)

        self.Replace(EXPDIR=join('#tmp','exp'))
        self.Replace(PKGDIR=join('#tmp','pkg'))
        self.Replace(TMPDIR=join('#tmp','obj'))
        self.Replace(STAGEDIR=join('#tmp','stage','$PI_PACKAGENAME'))
        self.Replace(RELEASESTAGEROOTDIR=join('$STAGEDIR',install_prefix))
        self.Replace(RELEASESTAGEDIR=join('$RELEASESTAGEROOTDIR','$PI_COLLECTION-$PI_RELEASE'))

        self.Replace(MODSTAGEDIR=self.get_mod_stage_dir)
        self.Replace(MODRUNDIR=self.get_mod_run_dir)
        self.Replace(PYDSTAGEDIR=self.get_pyd_stage_dir)
        self.Replace(PYDRUNDIR=self.get_pyd_run_dir)
        self.Replace(BINSTAGEDIR=self.get_bin_stage_dir)
        self.Replace(BINRUNDIR=self.get_bin_run_dir)

        self.Replace(MODSTAGEDIR_GLOBAL=join('$RELEASESTAGEDIR','modules','$PI_PYTHONPKG'))
        self.Replace(MODRUNDIR_GLOBAL=join('#tmp','modules','$PI_PYTHONPKG'))
        self.Replace(MODSTAGEDIR_PLUGIN=join('$RELEASESTAGEDIR','plugins','$PI_ORGANISATION','$PI_AGENTGROUP'))
        self.Replace(MODRUNDIR_PLUGIN=join('#tmp','plugins','$PI_ORGANISATION','$PI_AGENTGROUP'))

        self.Replace(BINSTAGEDIR_GLOBAL=join('$RELEASESTAGEDIR','bin'))
        self.Replace(BINRUNDIR_GLOBAL=join('#tmp','bin'))
        self.Replace(BINSTAGEDIR_PLUGIN=join('$RELEASESTAGEDIR','plugins','$PI_ORGANISATION','$PI_AGENTGROUP'))
        self.Replace(BINRUNDIR_PLUGIN=join('#tmp','plugins','$PI_ORGANISATION','$PI_AGENTGROUP'))

        self.Replace(HDRSTAGEDIR=join('$RELEASESTAGEDIR','include'))

        self.Replace(PYDSTAGEDIR_GLOBAL=join('$RELEASESTAGEDIR','modules'))
        self.Replace(PYDRUNDIR_GLOBAL=join('#tmp','modules'))
        self.Replace(PYDSTAGEDIR_PLUGIN=join('$RELEASESTAGEDIR','plugins','$PI_ORGANISATION','$PI_AGENTGROUP'))
        self.Replace(PYDRUNDIR_PLUGIN=join('#tmp','plugins','$PI_ORGANISATION','$PI_AGENTGROUP'))

        self.Replace(RESSTAGEDIR=join('$RELEASESTAGEDIR','resources'))
        self.Replace(RESRUNDIR=join('#tmp','resources'))

        self.Replace(ETCSTAGEDIR=join('$RELEASESTAGEDIR','etc','$PI_PACKAGENAME'))

        self.Append(CPPPATH='$EXPDIR')

        if os.environ.get('PI_VERBOSE') is None:
            self.Replace(PRINT_CMD_LINE_FUNC=self.print_cmd)

        self.Replace(INSTALLROOTDIR=join(os.path.sep,install_prefix))
        self.Replace(INSTALLDIR=join('$INSTALLROOTDIR','$PI_COLLECTION-$PI_RELEASE'))
        self.Replace(HDRINSTALLDIR=join('$INSTALLDIR','include'))
        self.Replace(MODINSTALLDIR=join('$INSTALLDIR','modules'))
        self.Replace(PYDINSTALLDIR=join('$INSTALLDIR','modules'))
        self.Replace(BININSTALLDIR=join('$INSTALLDIR','bin'))
        self.Replace(RESINSTALLDIR=join('$INSTALLDIR','resources'))
        self.Replace(ETCINSTALLDIR=join('$INSTALLDIR','etc','$PI_PACKAGENAME'))
        self.Replace(ETCROOTINSTALLDIR=join('$INSTALLDIR','etc'))

        self.Replace(PI_MODPREFIX='')
        self.Replace(PI_MODSUFFIX='.so')
        self.Replace(PI_MODLINKFLAGS='$SHLINKFLAGS')

        self.Replace(PILIBS=[])

        self.Replace(IS_LINUX_PPC32=False)
        self.Replace(IS_LINUX_PPC64=False)
        self.Replace(IS_LINUX_86=False)
        self.Replace(IS_LINUX_8664=False)
        self.Replace(IS_LINUX=False)
        self.Replace(IS_MACOSX=False)
        self.Replace(IS_MACOSX_PPC32=False)
        self.Replace(IS_MACOSX_PPC64=False)
        self.Replace(IS_MACOSX_86=False)
        self.Replace(IS_WINDOWS=False)
        self.Replace(IS_BIGENDIAN=False)

        self.Replace(PI_RELEASE=lambda target,source,env,for_signature: self.shared.release)
        self.Replace(PI_COMPATIBLE=lambda target,source,env,for_signature: self.shared.compatible)
        self.Replace(PI_COLLECTION=lambda target,source,env,for_signature: self.shared.collection)
        self.Replace(PI_ORGANISATION=lambda target,source,env,for_signature: self.shared.organisation)

        self.Alias('target-exports',join('#tmp','exp'))
        self.Alias('target-default',join('#tmp','plugins'))
        self.Alias('target-default',join('#tmp','bin'))
        self.Alias('target-default',join('#tmp','modules'))
        self.Alias('target-default',join('#tmp','plugins'))
        self.Alias('target-default',join('#tmp','resources'))
        self.Alias('target-stage',join('#tmp','stage'))

        self.Replace(PI_COMPILER=os.path.join(os.path.dirname(__file__),'compile.py'))
        self.Replace(PI_PIPCMD=os.path.join(os.path.dirname(__file__),'pip_cmd','pip.py'))

    @staticmethod
    def print_cmd(line,targets,source,env):
        targets=" ".join([target.path for target in targets])
        cmds=line.split()
        if cmds[0]==env['PI_PYTHON']:
            cmds=cmds[1:]
            while cmds[0].startswith('-'):
                cmds=cmds[1:]
        print "%s (%s)" % (targets,cmds[0])

    @staticmethod
    def __isinlist(filename,filter):
        for f in filter:
            if filename.endswith(f):
                return True
        return False

    def __installdir(self,root,src,dest=None):
        for f in os.listdir(src):
            if f=='.svn': continue
            if f=='.git': continue
            if f=='.': continue
            if f=='..': continue

            if f.endswith('.pyc'): continue
            if f.endswith('.pyo'): continue

            fqs=join(src,f)
            fqd=join(dest,f) if dest else f

            if os.path.isdir(fqs):
                self.__installdir(root,fqs,fqd)
                continue

            self.InstallAs(join(root,fqd),fqs)

    def __installpy(self,root1,root2,src,subdirs,dest=None):
        for f in os.listdir(src):
            if f=='.svn': continue
            if f=='.': continue
            if f=='..': continue

            if f.endswith('.pyc'): continue
            if f.endswith('.pyo'): continue

            fqs=join(src,f)
            fqd=join(dest,f) if dest else f

            if os.path.isdir(fqs):
                if subdirs==None or self.__isinlist(fqs,subdirs):
                    self.__installpy(root1,root2,fqs,None,fqd)
                continue

            if f.endswith('_test.py') or not f.endswith('.py'):
                continue

            pyc_node=self.Command(join(root1,fqd+'c'),fqs,'"$PI_PYTHON" "$PI_COMPILER" $TARGET $SOURCE')
            if root2:
                self.InstallAs(self.File(join(root2,fqd+'c')),pyc_node)

    @staticmethod
    def read_lexicon(source,e2m):
        m2e=dict()
        input=file(source)
        lineno=0

        for line in input:
            lineno=lineno+1

            fields = line.split(':')

            if len(fields) != 4:
                continue

            music = ''.join(fields[0].split())
            classification = fields[2].strip().lower()
            english = fields[1].strip().lower()

            if english == '' or english == 'unused' or classification == 'unused' or classification == '':
                continue

            if music:
                if not music.isdigit():
                    return 'line %d invalid music word: %s' % (lineno,music)

                if music in m2e:
                    return 'line %d duplicate music word: %s' % (lineno,music)
            else:
                music = None

            if english in e2m:
                return 'line %d duplicate english word: %s' % (lineno,english)

            e2m[english]=(music,classification)

            if music:
                m2e[music]=(english,classification)

        return None


    @staticmethod
    def build_lexicon(target,source,env):
        e2m=dict()

        err = PiGenericEnvironment.read_lexicon(source[0].abspath,e2m)
        if err:
            raise SCons.Errors.BuildError(node=source,errstr=err)

        output=file(target[0].abspath,'w')

        output.write('lexicon={\n')
        for (e,(m,c)) in e2m.iteritems():
            e = e.replace("'","\\'")
            if m is not None:
                output.write("    '%s': ('%s','%s'),\n" % (e,m,c))
            else:
                output.write("    '%s': (None,'%s'),\n" % (e,c))
        output.write("}\n");

        output.write('reverse_lexicon={\n')
        for (e,(m,c)) in e2m.iteritems():
            e = e.replace("'","\\'")
            if m is not None:
                output.write("    '%s': ('%s','%s'),\n" % (m,e,c))
        output.write("}\n");

        output.close()


    def PiLexicon(self,target,source,package=None,per_agent=None):
        return self.PiDynamicPython(target,source,PiGenericEnvironment.build_lexicon,package=package,per_agent=per_agent)

    def PiDynamicPython(self,target,source,builder,package=None,per_agent=None):
        env = self.Clone()

        tgt=env.File(target).srcnode().abspath
        src=[env.File(s) for s in env.Flatten((source,))]
        pypackage=os.path.basename(env.Dir('.').srcnode().abspath)

        if per_agent:
            pypackage = per_agent
            env.set_agent_group(pypackage)

        env.set_python_pkg(pypackage)

        prod1=env.Command(join(env.subst('$MODRUNDIR'),target),src,builder)
        prod2 = []

        if package:
            env.set_package(package)
            prod2 = env.Install(env.subst('$MODSTAGEDIR'),prod1)

        return prod1+prod2

    def PiDocumentation(self,file):
        pass

    def PiIncludeFiles(self,package=None):
        if not package:
            return

        env = self.Clone()
        env.set_package(package)
        me=env.Dir('.').srcnode().abspath
        incname=os.path.basename(me)
        root1 = os.path.join(env.subst('$HDRSTAGEDIR'),incname)

        for hdr in glob.glob(join(me,'*.h')):
            env.Install(root1,env.File(hdr))

        for hdr in glob.glob(join(me,'*.pip')):
            env.Install(root1,env.File(hdr))

    def PiBuildSystem(self,package=None):
        env = self.Clone()
        env.set_package(package)
        dst = os.path.join(env.subst('$RELEASESTAGEDIR'),'tools')
        src = os.path.join(os.path.dirname(__file__))
        env.__installdir(dst,src)

    def PiPythonPackage(self,package=None,per_agent=None,subdirs=(),resources=()):
        env = self.Clone()

        def build_version(target,source,env):
            output=file(target[0].abspath,'w')
            output.write(env.subst('version = "$PI_RELEASE"'))
            output.write("\n")
            output.close()

        node=env.PiDynamicPython('version.py',[],build_version,package=package,per_agent=per_agent)

        env.Depends(node,env.Value(self.shared.release))
        me=env.Dir('.').srcnode().abspath
        pypackage=os.path.basename(me)

        if per_agent:
            pypackage = per_agent
            env.set_agent_group(pypackage)

        env.set_python_pkg(pypackage)

        root1 = env.subst('$MODRUNDIR')
        root2 = None

        if package:
            env.set_package(package)
            root2 = env.subst('$MODSTAGEDIR')
            if per_agent:
                self.shared.agent_groups[pypackage][0] = package

        env.__installpy(root1,root2,me,subdirs)

        res=[]
        for f in resources:
            for x in glob.glob(join(me,f)):
                env.Install(root1,env.File(x))
                if root2:
                    env.Install(root2,env.File(x))
        
    def PiResources(self,section,files,package=None):
        env = self.Clone()

        me=env.Dir('.').srcnode().abspath
        res=[]

        res.extend(glob.glob(join(me,files)))

        if package:
            env.set_package(package)
            stageroot = join(env['RESSTAGEDIR'],section)
            env.Install(stageroot,res)

        runroot = join(env['RESRUNDIR'],section)
        env.Install(runroot,res)

    def PiRelease(self,collection,release,compatible,organisation=None):
        if self.shared.release:
            return False
        c = collection or 'release'
        self.shared.release=release
        self.shared.compatible=compatible
        self.shared.collection=c
        self.shared.organisation=organisation or 'Eigenlabs'
        return True

    def set_agent_group(self,ag):
        if ag:
            self.Replace(PI_AGENTGROUP=ag)
            if not ag in self.shared.agent_groups:
                self.shared.agent_groups[ag] = [None,[],{}]

    def set_python_pkg(self,pkg):
        self.Replace(PI_PYTHONPKG=pkg)

    def set_package(self,pkg):
        self.Replace(PI_PACKAGENAME=pkg)
        if not pkg in self.shared.packages:
            self.shared.packages.append(pkg)

    def PiPackageDescription(env,pkg,desc=None,longdesc=None,version=None,groups=None,depends=None,required=True,pkgid=None):
        meta = {}
        meta['desc'] = desc or pkg
        meta['longdesc'] = longdesc or pkg
        meta['version'] = version
        meta['groups'] = groups or ()
        meta['depends'] = depends or ()
        meta['required'] = required
        meta['pkgid'] = pkgid

        env.shared.package_descriptions[pkg] = meta

        if not pkg in env.shared.packages:
            env.shared.packages.append(pkg)

        return meta

    def PiPackageCollection(self,name,**meta):
        if 'external' in meta:
            for e in meta['external'].values():
                if not os.path.exists(e):
                    print e,'missing, skipping collection',name
                    return
        else:
            meta['external'] = {}
            
        self.shared.collections[name] = meta

    def get_packages(self,collection):
        p = []
        g = self.shared.collections[collection]['groups']
        for (n,m) in self.shared.package_descriptions.items():
            if not m['groups'] or set(m['groups']).intersection(g):
                p.append(n)
        return p

    def Pipfile(env,file):
        me=os.path.dirname(env.Dir('.').abspath)
        return env.File(file,me)

    def PiBranding(self,bmp=None,ico=None,icn=None,cert=None,passwd=None,bg=None,license=None):
        if bmp and not self.get('PI_BMPLOGO'): self.Replace(PI_BMPLOGO=self.File(bmp).srcnode().abspath)
        if ico and not self.get('PI_ICOLOGO'): self.Replace(PI_ICOLOGO=self.File(ico).srcnode().abspath)
        if icn and not self.get('PI_ICNLOGO'): self.Replace(PI_ICNLOGO=self.File(icn).srcnode().abspath)
        if cert and not self.get('PI_CERTFILE'): self.Replace(PI_CERTFILE=self.File(cert).srcnode().abspath)
        if bg and not self.get('PI_BACKGROUND'): self.Replace(PI_BACKGROUND=self.File(bg).srcnode().abspath)
        if license and not self.get('PI_LICENSE'): self.Replace(PI_LICENSE=self.File(license).srcnode().abspath)
        if passwd and not self.get('PI_CERTPASS'): self.Replace(PI_CERTPASS=passwd)

    def baker_subst(self,kwds):
        def escape(s):
            return repr(s)[1:-1]

        def escape2(s):
            return repr(repr(s)[1:-1])[1:-1]

        subst=dict(
               release = self.subst('$PI_RELEASE'),
               compatible = self.subst('$PI_COMPATIBLE'),
               platform = self['PI_PLATFORM'],
               resrundir = escape(self.Dir(self['RESRUNDIR']).abspath),
               binrundir = escape(self.Dir(self['BINRUNDIR']).abspath),
               modrundir=escape(self.Dir(self['MODRUNDIR']).abspath),
               pydrundir=escape(self.Dir(self['PYDRUNDIR']).abspath),
               modinstalldir=escape(self.Dir(self['MODINSTALLDIR']).abspath),
               pydinstalldir=escape(self.Dir(self['PYDINSTALLDIR']).abspath),
               resinstalldir=escape(self.Dir(self['RESINSTALLDIR']).abspath),
               bininstalldir=escape(self.Dir(self['BININSTALLDIR']).abspath),
               instrootdir=escape(self.Dir(self['ROOTINSTALLDIR']).abspath),
               python = escape(self['PI_PYTHON']),
               resrundir2 = escape2(self.Dir(self['RESRUNDIR']).abspath),
               binrundir2 = escape2(self.Dir(self['BINRUNDIR']).abspath),
               modrundir2=escape2(self.Dir(self['MODRUNDIR']).abspath),
               pydrundir2=escape2(self.Dir(self['PYDRUNDIR']).abspath),
               modinstalldir2=escape2(self.Dir(self['MODINSTALLDIR']).abspath),
               pydinstalldir2=escape2(self.Dir(self['PYDINSTALLDIR']).abspath),
               resinstalldir2=escape2(self.Dir(self['RESINSTALLDIR']).abspath),
               bininstalldir2=escape2(self.Dir(self['BININSTALLDIR']).abspath),
               instrootdir2=escape2(self.Dir(self['ROOTINSTALLDIR']).abspath),
               python2 = escape2(self['PI_PYTHON']),
               prefix = escape(self['PI_PREFIX']),
               prefix2 = escape2(self['PI_PREFIX'])
             )

        subst.update(kwds)
        return subst

    def baker(self,target,template,**kwds):

        text = template % self.baker_subst(kwds)

        def action(target,source,env):
            t = target[0].abspath
            outp = file(t,"w")
            outp.write(source[0].value)
            outp.close()
            os.chmod(t,0755)

        return self.Command(target,self.Value(text),action)

    def PiPipBinding(self,module,spec,sources=[],libraries={},package=None,hidden=True,per_agent=None,locked=True):
        me=self.Dir('.').abspath

        inc=' '.join(map(lambda x: '"%s"'% self.Dir(x).abspath,self['CPPPATH']))
        cppfile=self.File(module+'_python.cpp')

        cppnode=self.Command(cppfile,spec,'"$PI_PYTHON" "$PI_PIPCMD" "'+module+'" $SOURCES $TARGET '+inc)

        self.Depends(cppnode,self.Alias('build-tools'))

        allsources=[cppnode]
        allsources.extend(sources)

        bind_env=self.Clone()
        bind_env.Prepend(CPPPATH=[me])
        bind_env.Append(PILIBS=libraries)
        bind_env.set_hidden(hidden)

        bind_env.set_agent_group(per_agent)

        binding=bind_env.SharedLibrary(module,allsources,SHLIBPREFIX=bind_env['PI_MODPREFIX'],SHLIBSUFFIX=bind_env['PI_MODSUFFIX'],SHLINKFLAGS=bind_env['PI_MODLINKFLAGS'])
        bind_env.addlibuser(binding,libraries)

        binding_tgt1=bind_env.Install(bind_env.subst('$PYDRUNDIR'),binding[0])
        binding_tgt2=[]

        if package:
            bind_env.set_package(package)
            binding_tgt2=bind_env.Install(bind_env.subst('$PYDSTAGEDIR'),binding_tgt1)

        return binding_tgt1+binding_tgt2

    def PiEtc(self,package,source):
        etc_env=self.Clone()
        etc_env.set_package(package)
        stagesource = etc_env.Install(etc_env['ETCSTAGEDIR'],source)
        return stagesource

    def PiAgent(self,name,pypackage,pymodule,cversion,extra=[],lexicon=None):
        version = '.'.join(self.subst('$PI_RELEASE').split('.'))
        env = self.Clone()
        meta = (name,pymodule,cversion,version,extra)
        env.set_agent_group(pypackage)
        env.shared.agent_groups[pypackage][1].append(meta)

        if lexicon:
            lexfile = env.File(lexicon).srcnode().abspath
            e2m = dict()
            err = env.read_lexicon(lexfile,e2m)
            env.shared.agent_groups[pypackage][2].update(e2m)

    def __build_manifest(self,module,package,agent_list,manifest):
        env = self.Clone()

        env.set_agent_group(module)

        if package:
            env.set_package(package)

        def build_version(target,source,env):
            output=file(target[0].abspath,'w')
            output.write(env.subst('version = "$PI_RELEASE"'))
            output.write("\n")
            output.write(env.subst('cversion = "%s"' % source[1].value))
            output.write("\n")
            output.write(env.subst('plugin = "%s"' % source[0].value))
            output.write("\n")
            output.close()


        textnodes = []

        for a in agent_list:
            extra = ':'.join(a[4])
            if extra: extra=':'+extra
            textnodes.append(env.Value('agent %s:%s:%s:%s%s' % (a[0],a[1],a[2],a[3],extra)))
            vnode = env.Command(join(env.subst("$MODRUNDIR_PLUGIN"),'%s_version.py' % a[0]),[env.Value(a[0]),env.Value(a[2])],build_version)
            if package:
                env.Install(env.subst("$MODSTAGEDIR_PLUGIN"),vnode)

        for (e,(m,c)) in manifest.items():
            textnodes.append(env.Value('vocab %s:%s:%s' % (e,m,c)))


        def build_manifest(target,source,env):
            t = target[0].abspath
            outp = file(t,"w")

            for a in source:
                outp.write("%s\n" % a.value.lower())

            outp.close()

        mnode = env.Command(join(env.subst("$MODRUNDIR_PLUGIN"),'Manifest'),textnodes,build_manifest)

        if package:
            env.Install(env.subst("$MODSTAGEDIR_PLUGIN"),mnode)

    def PiPythonBuildWrapper(self,name,package,module,main):
        return self.PiPythonWrapper(name,package,module,main)

    def PiPythonDebugWrapper(self,name,package,module,main):
        return self.PiPythonWrapper(name,package,module,main)

    def PiPythonWrapper(self,name,pypackage,module,main,package=None,gui=False):

        run_source = self.File('pystub_%s.cpp' % name);
        run_source_node = self.baker(run_source,self.py_template,main=main,package=pypackage,module=module)
        run_exe = self.PiProgram(name,run_source_node,package=package,gui=gui)
        self.Depends(run_exe,self.Dir(pypackage,self.subst('$MODRUNDIR')))
        return run_exe

    def PiPythonwWrapper(self,name,pypackage,module,main,appname=None,bg=False,di=False,private=False,usegil=False,package=None):
        return self.PiPythonWrapper(name,pypackage,module,main,package=package,gui=True)
        
    def Pi3rdParty(self,s,package,release):
        assert package in self.shared.package_descriptions

        env = self.Clone()
        env.set_package(package)

        def extract_unzip(target=None,source=None,env=None):
            target = target[0].abspath
            source = source[0].srcnode().abspath
            env.safe_mkdir(target)
            os.system('unzip -q %s -d %s' % (source,target))

        def extract_python(target=None,source=None,env=None):
            target = target[0].abspath
            source = source[0].srcnode().abspath
            env.safe_mkdir(target)
            zip = zipfile.ZipFile(source, 'r')
            zip.extractall(target)

        extract = extract_unzip if not self['IS_WINDOWS'] else extract_python

        if release:
            return env.Command(env.Dir(env['RELEASESTAGEDIR']),s,extract)
        else:
            return env.Command(env.Dir(env['RELEASESTAGEROOTDIR']),s,extract)

    def PiGuiProgram(self,target,sources,bg=False,di=False,package=None,appname=None,private=False,libraries=[],hidden=True):
        return self.PiProgram(target,sources,libraries=libraries,package=package,hidden=hidden)

    def PiProgram(self,name,sources,libraries=[],package=None,gui=False,hidden=True):
        env = self.Clone()
        env.Append(PILIBS=libraries)

        env.set_hidden(hidden)

        bld_binary=env.Program(name,env.Split(sources))
        run_binary=env.Install(env.subst('$BINRUNDIR'),bld_binary)

        env.add_runtime_user(run_binary)
        env.addlibuser(bld_binary,libraries)

        inst_binary = []

        if package:
            env.set_package(package)
            inst_binary=env.Install(env.subst('$BINSTAGEDIR'),bld_binary)

        return run_binary+inst_binary


    def PiBinaryDLL(self,target,package=None):
        pass

generic_py_template = """
#ifdef _WIN32  
#ifdef _DEBUG
#define __REDEF_DEBUG__
#undef _DEBUG
#endif
#endif

#include <Python.h>

#ifdef __REDEF_DEBUG__
#define _DEBUG
#undef __REDEF_DEBUG__
#endif

#include <string.h>

#ifdef _WIN32
#define UNICODE 1
#define _UNICODE 1
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef _WIN32
#define UNICODE 1
#define _UNICODE 1
#include <windows.h>
#include <shlobj.h>

static void get_exe(char *buffer)
{
    WCHAR dest [MAX_PATH+1];
    HINSTANCE moduleHandle = GetModuleHandle(0);
    GetModuleFileName (moduleHandle, dest, MAX_PATH+1);
    WideCharToMultiByte(CP_UTF8,0,dest,-1,buffer,MAX_PATH+1,0,0);
}

static void dirname(char *buffer)
{
    char *p = strrchr(buffer,'\\\\');

    if(p)
    {
        *p = 0;
    }
    else
    {
        buffer[0] = '\\\\';
        buffer[1] = 0;
    }
}

static void init_pyhome(char *buffer)
{
  WCHAR dest [MAX_PATH+1];
  SHGetSpecialFolderPath (0, dest, CSIDL_PROGRAM_FILES, 0);
  WideCharToMultiByte(CP_UTF8,0,dest,-1,buffer,MAX_PATH+1,0,0);
  strcat(buffer,"\\\\Eigenlabs\\\\runtime-1.0.0\\\\Python26");
}

static void init_release_root(char *buffer)
{
    get_exe(buffer);
    dirname(buffer);
    dirname(buffer);
}

static void init_path()
{
   char buffer[4096];
   get_exe(buffer);
   dirname(buffer);
   SetDllDirectoryA(buffer);
}

#endif

#ifdef __APPLE__

#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>

static void get_exe(char *buffer)
{
    Dl_info exeInfo;
    dladdr ((const void*) get_exe, &exeInfo);
    realpath(exeInfo.dli_fname,buffer);
}

static void dirname(char *buffer)
{
    char *p = strrchr(buffer,'/');

    if(p)
    {
        *p = 0;
    }
    else
    {
        buffer[0] = '/';
        buffer[1] = 0;
    }
}
static void init_pyhome(char *buffer)
{
    strcpy(buffer,PI_PREFIX);
}

static void init_release_root(char *buffer)
{
    get_exe(buffer);
    dirname(buffer);
    dirname(buffer);
}

static void init_path()
{
}

#endif

#ifdef __linux__

#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>

static void dirname(char *buffer)
{
    char *p = strrchr(buffer,'/');

    if(p)
    {
        *p = 0;
    }
    else
    {
        buffer[0] = '/';
        buffer[1] = 0;
    }
}

static void get_exe(char *buffer)
{
    Dl_info exeInfo;
    dladdr ((const void*) get_exe, &exeInfo);
    realpath(exeInfo.dli_fname,buffer);
}

static void init_pyhome(char *buffer)
{
    strcpy(buffer,"/usr");
}

static void init_release_root(char *buffer)
{
    get_exe(buffer);
    dirname(buffer);
    dirname(buffer);
}

static void init_path()
{
}

#endif

extern int main(int argc, char **argv)
{
  char pyhome[4096];

  init_pyhome(pyhome);
  init_path();

  Py_SetPythonHome(pyhome);
  Py_Initialize();
  PySys_SetArgv(argc, argv);

  {
      char cmdbuffer[4096];
      char escbuffer[4096];

      init_release_root(cmdbuffer);

      char *q = escbuffer;
      const char *p = cmdbuffer;

      while(*p)
      {
        if(*p=='\\\\')
        {
            *q++ = '\\\\';
        }

        *q++ = *p++;
      }

      *q = 0;

      sprintf(cmdbuffer,
         "import sys,os\\n"
         "if '' in sys.path: sys.path.remove('')\\n"
         "if os.getcwd() in sys.path: sys.path.remove(os.getcwd())\\n"
         "sys.path.insert(0,os.path.join('%%s','modules'))\\n"
         "sys.path.insert(0,os.path.join('%%s','bin'))\\n"
         "from %(package)s import %(module)s\\n",
            escbuffer,escbuffer
      );

      PyRun_SimpleString(cmdbuffer);
  }

  PyRun_SimpleString("%(module)s.%(main)s()");
  Py_Finalize();
  return 0;
}
"""

exports_template = """
#ifdef _WIN32
  #ifdef BUILDING_%(libtoken)s
    #define %(libtoken)s_DECLSPEC_FUNC(rt) rt __declspec(dllexport)
    #define %(libtoken)s_DECLSPEC_CLASS __declspec(dllexport)
    #define %(libtoken)s_DECLSPEC_INLINE_FUNC(rt)
    #define %(libtoken)s_DECLSPEC_INLINE_CLASS
  #else
    #define %(libtoken)s_DECLSPEC_FUNC(rt) rt __declspec(dllimport)
    #define %(libtoken)s_DECLSPEC_CLASS __declspec(dllimport)
    #define %(libtoken)s_DECLSPEC_INLINE_FUNC(rt)
    #define %(libtoken)s_DECLSPEC_INLINE_CLASS
  #endif
#else
  #if __GNUC__ >= 4
    #ifdef BUILDING_%(libtoken)s
      #define %(libtoken)s_DECLSPEC_FUNC(rt) rt __attribute__ ((visibility("default")))
      #define %(libtoken)s_DECLSPEC_CLASS __attribute__ ((visibility("default")))
      #define %(libtoken)s_DECLSPEC_INLINE_FUNC(rt) rt __attribute__ ((visibility("default")))
      #define %(libtoken)s_DECLSPEC_INLINE_CLASS __attribute__ ((visibility("default")))
    #else
      #define %(libtoken)s_DECLSPEC_FUNC(rt) rt  __attribute__ ((visibility("default")))
      #define %(libtoken)s_DECLSPEC_CLASS  __attribute__ ((visibility("default")))
      #define %(libtoken)s_DECLSPEC_INLINE_FUNC(rt) rt  __attribute__ ((visibility("default")))
      #define %(libtoken)s_DECLSPEC_INLINE_CLASS  __attribute__ ((visibility("default")))
    #endif
  #else
    #define %(libtoken)s_DECLSPEC_FUNC(rt) rt
    #define %(libtoken)s_DECLSPEC_CLASS
    #define %(libtoken)s_DECLSPEC_INLINE_FUNC(rt) rt
    #define %(libtoken)s_DECLSPEC_INLINE_CLASS
  #endif
#endif
"""
