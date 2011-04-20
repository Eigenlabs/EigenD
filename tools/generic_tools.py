
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

    def PiSharedLibrary(self,target,sources,libraries={},package=None,hidden=True):
        fulltarget = '%s_%s' % (target,self.shared.release.replace('.','_').replace('-','_'))
        env = self.Clone()

        env.Append(PILIBS=libraries)
        env.Append(CCFLAGS='-DBUILDING_%s' % target.upper())
        env.Replace(SHLIBNAME=fulltarget)
        env.set_hidden(hidden)

        bin_library=env.SharedLibrary(fulltarget,sources)
        run_library=env.Install(env.subst('$BINRUNDIR'),bin_library)
        inst_library = []

        env.addlibuser(bin_library[0],libraries)
        env.addlibname(run_library[0],target,dependencies=libraries)

        if package:
            env.set_package(package)
            inst_library = env.Install(self['BINSTAGEDIR'],run_library)

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

    def set_tool(self,tool,exe):
        self.shared.buildtools[tool] = exe

    def find_tool(self,tool):
        exe = self.shared.buildtools.get(tool)
        if not exe:
            raise RuntimeError('No '+tool+' found')
        return exe

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

        for (k,v) in self.shared.buildtools.iteritems():
            print '%s: %s' % (k,v)

        if not self.shared.release:
            raise RuntimeError('PiRelease not called')

        print "building release",self.subst('$PI_RELEASE')

    def __getpython(self):
        status=os.popen('"%s" %s' % (self['PI_PYTHON'],join(os.path.dirname(__file__),'detect.py')),'r').read(1024)
        (exe,incpath,libpath,libs,linkextra,prefix) = [s.strip() for s in status.split(';')]

        self.Append(CPPPATH=incpath)
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
                raise RuntimeError('library %s not defined' % l)
            map.append(ll.libnode.abspath)

        return ' '.join(map)

    def __init__(self,platform,install_prefix,userdir_suffix,python = None,**kwds):
        SCons.Environment.Environment.__init__(self,ENV = os.environ,**kwds)

        self.py_template = generic_py_template

        self.shared = self.__Shared()
        self.shared.buildtools = {}
        self.shared.package_descriptions = {}
        self.shared.packages = []
        self.shared.shlibmap = {}
        self.shared.release = None
        self.shared.collections = {}
        self.shared.lib_pkgs = {}
        self.shared.py_pkgs = {}
        self.shared.runtime = []
        self.shared.runtime_user = []

        self.Replace(PI_PLATFORM=platform)
        self.Replace(PI_PYTHON=os.environ.get('PI_PYTHON',python) or sys.executable)

        self.__getpython()

        print "platform: %s python: %s" % (platform,self['PI_PYTHON'])

        #self.set_tool('pip',self.WhereIs('pip'))

        self.Replace(ROOTINSTALLDIR=os.path.join('/',install_prefix))

        self.Append(LINKFLAGS=Split('$LIBMAPPER'))
        self.Replace(LIBMAPPER=self.libmapper)

        self.Replace(PKGDIR=join('#tmp','pkg'))
        self.Replace(TMPDIR=join('#tmp','obj'))
        self.Replace(STAGEDIR=join('#tmp','stage','$PI_PACKAGENAME'))
        self.Replace(RELEASESTAGEROOTDIR=join('$STAGEDIR',install_prefix))
        self.Replace(RELEASESTAGEDIR=join('$RELEASESTAGEROOTDIR','$PI_COLLECTION-$PI_RELEASE'))
        self.Replace(BINSTAGEDIR=join('$RELEASESTAGEDIR','bin'))
        self.Replace(HDRSTAGEDIR=join('$RELEASESTAGEDIR','include'))
        self.Replace(MODSTAGEDIR=join('$RELEASESTAGEDIR','modules'))
        self.Replace(PYDSTAGEDIR=join('$RELEASESTAGEDIR','modules'))
        self.Replace(RESSTAGEDIR=join('$RELEASESTAGEDIR','resources'))
        self.Replace(PLGSTAGEDIR=join('$RELEASESTAGEDIR','plugins'))
        self.Replace(ETCSTAGEDIR=join('$RELEASESTAGEDIR','etc','$PI_PACKAGENAME'))

        if os.environ.get('PI_VERBOSE') is None:
            self.Replace(PRINT_CMD_LINE_FUNC=self.print_cmd)

        self.Replace(INSTALLROOTDIR=join(os.path.sep,install_prefix))
        self.Replace(INSTALLDIR=join('$INSTALLROOTDIR','$PI_COLLECTION-$PI_RELEASE'))
        self.Replace(HDRINSTALLDIR=join('$INSTALLDIR','include'))
        self.Replace(MODINSTALLDIR=join('$INSTALLDIR','modules'))
        self.Replace(PYDINSTALLDIR=join('$INSTALLDIR','modules'))
        self.Replace(BININSTALLDIR=join('$INSTALLDIR','bin'))
        self.Replace(RESINSTALLDIR=join('$INSTALLDIR','resources'))
        self.Replace(PLGINSTALLDIR=join('$INSTALLDIR','plugins'))
        self.Replace(ETCINSTALLDIR=join('$INSTALLDIR','etc','$PI_PACKAGENAME'))
        self.Replace(ETCROOTINSTALLDIR=join('$INSTALLDIR','etc'))

        self.Replace(PI_MODPREFIX='')
        self.Replace(PI_MODSUFFIX='.so')
        self.Replace(PI_MODLINKFLAGS='$SHLINKFLAGS')

        self.Replace(BINRUNDIR=join('#tmp','bin'))
        self.Replace(MODRUNDIR=join('#tmp','modules'))
        self.Replace(PYDRUNDIR=join('#tmp','modules'))
        self.Replace(RESRUNDIR=join('#tmp','resources'))
        self.Replace(PLGRUNDIR=join('#tmp','plugins'))
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

        self.Replace(PIPCMD=lambda target,source,env,for_signature: self.find_tool('pip'))
        self.Replace(PI_RELEASE=lambda target,source,env,for_signature: self.shared.release)
        self.Replace(PI_COLLECTION=lambda target,source,env,for_signature: self.shared.collection)

        self.Alias('target-default',join('#tmp','bin'))
        self.Alias('target-default',join('#tmp','modules'))
        self.Alias('target-default',join('#tmp','plugins'))
        self.Alias('target-default',join('#tmp','resources'))
        self.Alias('target-stage',join('#tmp','stage'))

        self.Replace(PI_COMPILER=os.path.join(os.path.dirname(__file__),'compile.py'))

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

    def __installpy(self,root1,root2,src,dest,subdirs):
        for f in os.listdir(src):
            if f=='.svn': continue
            if f=='.': continue
            if f=='..': continue

            if f.endswith('.pyc'): continue
            if f.endswith('.pyo'): continue

            fqs=join(src,f)
            fqd=join(dest,f)

            if os.path.isdir(fqs):
                if subdirs==None or self.__isinlist(fqs,subdirs):
                    self.__installpy(root1,root2,fqs,fqd,None)
                continue

            if f.endswith('_test.py') or not f.endswith('.py'):
                continue

            pyc_node=self.Command(join(root1,fqd+'c'),fqs,'"$PI_PYTHON" "$PI_COMPILER" $TARGET $SOURCE')
            if root2:
                self.InstallAs(self.File(join(root2,fqd+'c')),pyc_node)

    def PiDynamicPython(self,target,source,builder,package=None):
        env = self.Clone()
        tgt=env.File(target).srcnode().abspath
        src=[env.File(s) for s in env.Flatten((source,))]
        pkg=os.path.basename(env.Dir('.').abspath)

        prod1=env.Command(join(env['MODRUNDIR'],pkg,target),src,builder)
        prod2 = []

        if package:
            env.set_package(package)
            prod2 = env.Install(join(env['MODSTAGEDIR'],pkg),prod1)

        return prod1+prod2

    def PiDocumentation(self,file):
        pass

    def PiPythonPackage(self,cversion,package=None,subdirs=(),resources=(),natives=(),**deps):
        env = self.Clone()

        def build_version(target,source,env):
            output=file(target[0].abspath,'w')
            output.write(env.subst('version = "$PI_RELEASE"'))
            output.write("\n")
            output.close()

        node=env.PiDynamicPython('version.py',[],build_version,package=package)
        env.Depends(node,env.Value(self.shared.release))
        me=env.Dir('.').srcnode().abspath
        pypackage=os.path.basename(me)
        root1 = self['MODRUNDIR']
        root2 = None
        if package:
            env.set_package(package)
            root2 = self['MODSTAGEDIR']

        env.__installpy(root1,root2,me,pypackage,subdirs)

        res=[]
        for f in resources:
            for x in glob.glob(join(me,f)):
                env.Install(join(root1,pypackage),env.File(x))
                if root2:
                    env.Install(join(root2,pypackage),env.File(x))
        
    def PiResources(self,package,section,files):
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

    def PiRelease(self,collection,release):
        if self.shared.release:
            raise RuntimeError('PiRelease called twice')
        c = collection or 'release'
        self.shared.release=release
        self.shared.collection=c

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
               release=self.subst('$PI_RELEASE'),
               platform = self['PI_PLATFORM'],
               resrundir = escape(self.Dir(self['RESRUNDIR']).abspath),
               binrundir = escape(self.Dir(self['BINRUNDIR']).abspath),
               modrundir=escape(self.Dir(self['MODRUNDIR']).abspath),
               pydrundir=escape(self.Dir(self['PYDRUNDIR']).abspath),
               plgrundir=escape(self.Dir(self['PLGRUNDIR']).abspath),
               modinstalldir=escape(self.Dir(self['MODINSTALLDIR']).abspath),
               pydinstalldir=escape(self.Dir(self['PYDINSTALLDIR']).abspath),
               resinstalldir=escape(self.Dir(self['RESINSTALLDIR']).abspath),
               bininstalldir=escape(self.Dir(self['BININSTALLDIR']).abspath),
               plginstalldir=escape(self.Dir(self['PLGINSTALLDIR']).abspath),
               instrootdir=escape(self.Dir(self['ROOTINSTALLDIR']).abspath),
               python = escape(self['PI_PYTHON']),
               resrundir2 = escape2(self.Dir(self['RESRUNDIR']).abspath),
               binrundir2 = escape2(self.Dir(self['BINRUNDIR']).abspath),
               modrundir2=escape2(self.Dir(self['MODRUNDIR']).abspath),
               pydrundir2=escape2(self.Dir(self['PYDRUNDIR']).abspath),
               plgrundir2=escape2(self.Dir(self['PLGRUNDIR']).abspath),
               modinstalldir2=escape2(self.Dir(self['MODINSTALLDIR']).abspath),
               pydinstalldir2=escape2(self.Dir(self['PYDINSTALLDIR']).abspath),
               resinstalldir2=escape2(self.Dir(self['RESINSTALLDIR']).abspath),
               bininstalldir2=escape2(self.Dir(self['BININSTALLDIR']).abspath),
               plginstalldir2=escape2(self.Dir(self['PLGINSTALLDIR']).abspath),
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

    def PiPipBinding(self,module,spec,sources=[],libraries={},package=None):
        me=self.Dir('.').abspath

        inc=' '.join(map(lambda x: self.Dir(x).abspath,self['CPPPATH']))
        cppfile=self.File(module+'_python.cpp')
        cppnode=self.Command(cppfile,spec,'$PIPCMD '+module+' $SOURCES $TARGET '+inc)
        self.Depends(cppnode,self.Alias('build-tools'))

        allsources=[cppnode]
        allsources.extend(sources)

        bind_env=self.Clone()
        bind_env.Prepend(CPPPATH=[me])
        bind_env.Append(PILIBS=libraries)

        binding=bind_env.SharedLibrary(module,allsources,SHLIBPREFIX=bind_env['PI_MODPREFIX'],SHLIBSUFFIX=bind_env['PI_MODSUFFIX'],SHLINKFLAGS=bind_env['PI_MODLINKFLAGS'])
        bind_env.addlibuser(binding,libraries)

        binding_tgt1=bind_env.Install(bind_env['PYDRUNDIR'],binding[0])
        binding_tgt2=[]

        if package:
            bind_env.set_package(package)
            binding_tgt2=bind_env.Install(bind_env['PYDSTAGEDIR'],binding_tgt1)

        return binding_tgt1+binding_tgt2

    def PiEtc(self,package,source):
        etc_env=self.Clone()
        etc_env.set_package(package)
        stagesource = etc_env.Install(etc_env['ETCSTAGEDIR'],source)
        return stagesource

    def PiAgent(self,name,module,pypackage,cversion,subversion=None,package=None,extra=[]):
        (pypackage,pcversion) = pypackage.split(':')

        if subversion:
            subv = str(subversion).split('.')
        else:
            subv = []

        version = '.'.join(self.subst('$PI_RELEASE').split('.')+subv)

        env = self.Clone()
        def build_version(target,source,env):
            output=file(target[0].abspath,'w')
            output.write(env.subst('version = "$PI_RELEASE"'))
            output.write("\n")
            output.write(env.subst('cversion = "%s"' % cversion))
            output.write("\n")
            output.write(env.subst('plugin = "%s"' % name))
            output.write("\n")
            output.close()

        vnode=env.PiDynamicPython('%s_version.py'%name,[],build_version,package=package)

        text = "%s:%s:%s:%s" % (name,module,cversion,version)

        if extra:
            text = text + ':' + ':'.join(extra)

        def build_registry(target,source,env):
            t = target[0].abspath
            outp = file(t,"w")
            outp.write(source[0].value)
            outp.close()

        rnode = env.Command(join(self.subst("$PLGRUNDIR"),name),self.Value(text),build_registry)

        env.Depends(vnode,env.Value(cversion))
        env.Depends(vnode,env.Value(self.shared.release))
        env.Depends(rnode,env.Value(cversion))
        env.Depends(rnode,env.Value(self.shared.release))

        if package:
            env.set_package(package)
            env.Install(env.subst('$PLGSTAGEDIR'),rnode)

    def PiPythonBuildWrapper(self,name,package,module,main):
        return self.PiPythonWrapper(name,package,module,main)

    def PiPythonDebugWrapper(self,name,package,module,main):
        return self.PiPythonWrapper(name,package,module,main)

    def PiPythonWrapper(self,name,pypackage,module,main,package=None,gui=False):

        run_source = self.File('pystub_%s.cpp' % name);
        run_source_node = self.baker(run_source,self.py_template,main=main,package=pypackage,module=module)
        run_exe = self.PiProgram(name,run_source_node,package=package,libraries=['pic'],gui=gui)
        self.Depends(run_exe,self.Dir(pypackage,self.subst('$MODRUNDIR')))
        return run_exe

    def PiPythonwWrapper(self,name,pypackage,module,main,appname=None,bg=False,di=False,private=False,usegil=False,package=None):
        return self.PiPythonWrapper(name,pypackage,module,main,package=package,gui=True)
        
    def PiBuildTool(self,tool,exe):
        self.set_tool(tool,exe[0]);
        self.Alias('build-tools',exe[0])

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

#include <picross/pic_resources.h>

extern int main(int argc, char **argv)
{
  char *pyhome = strdup(pic::python_prefix_dir().c_str());
  Py_SetPythonHome(pyhome);
  Py_Initialize();
  PySys_SetArgv(argc, argv);

  {
      std::string root = pic::release_root_dir();
      char cmdbuffer[4096];
      char escbuffer[4096];

      char *q = escbuffer;
      const char *p = root.c_str();

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
