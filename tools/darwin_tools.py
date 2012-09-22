
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

import os
import os.path
import sys
import SCons.Environment
import unix_tools
import shutil
import glob
from posix import uname

from os.path import join,basename
from SCons.Util import Split
from SCons.Defaults import Copy

def svn_filter(src,names):
    if '.svn' in names:
        return ['.svn']
    return []

def mycopytree(src, dst, symlinks=False, ignore=None):
    """Recursively copy a directory tree using copy2().

    The destination directory must not already exist.
    If exception(s) occur, an Error is raised with a list of reasons.

    If the optional symlinks flag is true, symbolic links in the
    source tree result in symbolic links in the destination tree; if
    it is false, the contents of the files pointed to by symbolic
    links are copied.

    The optional ignore argument is a callable. If given, it
    is called with the `src` parameter, which is the directory
    being visited by copytree(), and `names` which is the list of
    `src` contents, as returned by os.listdir():

        callable(src, names) -> ignored_names

    Since copytree() is called recursively, the callable will be
    called once for each directory that is copied. It returns a
    list of names relative to the `src` directory that should
    not be copied.

    XXX Consider this example code rather than the ultimate tool.

    """
    names = os.listdir(src)
    if ignore is not None:
        ignored_names = ignore(src, names)
    else:
        ignored_names = set()

    os.makedirs(dst)
    errors = []
    for name in names:
        if name in ignored_names:
            continue
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                mycopytree(srcname, dstname, symlinks, ignore)
            else:
                os.link(srcname, dstname)
            # XXX What about devices, sockets etc.?
        except (IOError, os.error), why:
            errors.append((srcname, dstname, str(why)))
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error, err:
            errors.extend(err.args[0])
    try:
        shutil.copystat(src, dst)
    except OSError, why:
        if WindowsError is not None and isinstance(why, WindowsError):
            # Copying file access times may fail on Windows
            pass
        else:
            errors.extend((src, dst, str(why)))
    if errors:
        raise shutil.Error, errors

class PiDarwinEnvironment(unix_tools.PiUnixEnvironment):

    def __init__(self,platform):
        unix_tools.PiUnixEnvironment.__init__(self,platform,'usr/pi','Library/Eigenlabs',python='/usr/pi/bin/python')
        os_major=uname()[2].split('.')[0]
        self.Append(LIBS=Split('dl m pthread'))
        self.Append(CCFLAGS=Split('-arch i386 -DDEBUG_DATA_ATOMICITY_DISABLED -DPI_PREFIX=\\"$PI_PREFIX\\" -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk'))
        self.Append(LINKFLAGS=Split('-arch i386 -framework Accelerate -Wl,-rpath,@executable_path/ -no_compact_linkedit -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk'))
        self.Replace(CXX='g++-4.2')
        self.Replace(CC='gcc-4.2')

        self.Append(CCFLAGS=Split('-ggdb -Werror -Wall -Wno-deprecated-declarations -Wno-format -O4 -fmessage-length=0 -falign-loops=16 -msse3'))

        self.Replace(PI_DLLENVNAME='DYLD_LIBRARY_PATH')
        self.Replace(IS_MACOSX=os_major)
        self.Replace(PI_MODLINKFLAGS=['$LINKFLAGS','-bundle','-Wl,-rpath,@loader_path/'])
        self.Append(SHLINKFLAGS=['-Wl,-install_name,@rpath/${SHLIBPREFIX}${SHLIBNAME}${SHLIBSUFFIX}','-Wl,-rpath,@loader_path/'])
        self.Replace(PI_PLATFORMTYPE='macosx')

        self.Replace(APPRUNDIR=join('#tmp','app'))
        self.Replace(APPSTAGEDIR=join('$STAGEDIR','Applications','Eigenlabs','$PI_RELEASE'))
        self.Replace(APPINSTALLDIR=join('/','Applications','Eigenlabs','$PI_RELEASE'))
        self.Replace(APPSTAGEDIR_PRIV=join('$RELEASESTAGEDIR','Applications'))
        self.Replace(APPINSTALLDIR_PRIV=join('/','$INSTALLDIR','Applications'))

        self.Alias('target-default','#tmp/app')

    def set_hidden(self,hidden):
        if hidden:
            self.Append(CCFLAGS='-fvisibility=hidden')

    def Initialise(self):
        unix_tools.PiUnixEnvironment.Initialise(self)
        self.safe_mkdir(self.Dir(self.subst('$APPRUNDIR')).abspath)

    def Finalise(self):
        unix_tools.PiUnixEnvironment.Finalise(self)

        pkgs = dict([ (pkg,self.make_package(pkg)) for pkg in self.shared.packages ])

        for c in self.shared.collections:
            mpkg = self.make_mpkg(c,pkgs)
            dmg = self.make_dmg(c,mpkg)

    def GetLockMarker(self,tag,locked=False):
        if not locked:
            return []

        marker_name = "%s_fastmark.c" % tag

        def action(target,source,env):
            t = target[0].abspath
            outp = file(t,"w")
            outp.write(source[0].value)
            outp.close()

        marker_node = self.Command(marker_name,self.Value(fastmark_template),action)
        return marker_node

    def PiSharedLibrary(self,target,sources,locked=False,**kwds):
        sources.extend(self.GetLockMarker(target,locked))
        return unix_tools.PiUnixEnvironment.PiSharedLibrary(self,target,sources=sources,**kwds)
        

    def PiPipBinding(self,module,spec,sources=[],locked=False,**kwds):
        sources.extend(self.GetLockMarker(module,locked))
        return unix_tools.PiUnixEnvironment.PiPipBinding(self,module,spec,sources=sources,**kwds)


    def PiPythonwWrapper(self,name,pypackage,module,main,appname=None,bg=False,private=False,usegil=False,di=False,package=None):
        env = self.Clone()
        program = env.PiPythonWrapper(name,pypackage,module,main,package=package)
        bigname = appname or name[0:1].upper()+name[1:].lower()
        runapp = env.make_app_bundle4(env.subst('$APPRUNDIR'),bigname,program[0].abspath,di=di,bg=bg)

        if package:
            env.set_package(package)
            progname = join(env.subst('$BININSTALLDIR'),name)

            if not private:
                instapp = env.make_app_bundle4(env.subst('$APPSTAGEDIR'),bigname,progname,di=di,bg=bg)
            else:
                instapp = env.make_app_bundle4(env.subst('$APPSTAGEDIR_PRIV'),bigname,progname,di=di,bg=bg)

        return runapp

    def PiGuiProgram(self,target,sources,bg=False,di=False,package=None,appname=None,private=False,libraries=[],hidden=True):
        return self.PiCocoaProgram(target,sources,bg=bg,di=di,package=package,appname=appname,private=private,libraries=libraries)

    def PiCocoaProgram(self,target,sources,resources=(),bg=False,di=False,package=None,appname=None,private=False,libraries=[],hidden=True):
        env = self.Clone()
        program = env.PiProgram(target,sources,package=package,libraries=libraries,hidden=hidden)
        bigname = appname or target[0:1].upper()+target[1:].lower()
        runapp = env.make_app_bundle4(env.subst('$APPRUNDIR'),bigname,program[0].abspath,di=di,bg=bg,resources=resources)

        if package:
            env.set_package(package)
            progname = join(env.subst('$BININSTALLDIR'),target)

            if not private:
                instapp = env.make_app_bundle4(env.subst('$APPSTAGEDIR'),bigname,progname,di=di,bg=bg,resources=resources)
            else:
                instapp = env.make_app_bundle4(env.subst('$APPSTAGEDIR_PRIV'),bigname,progname,di=di,bg=bg,resources=resources)
        return runapp

    def PiBinaryDLL(self,target,package=None):
        env = self.Clone()

        f1 = env.File('lib'+target+'.dylib')

        run_library1=env.Install(env.subst('$BINRUNDIR'),f1)
        env.Alias('target-runtime',run_library1)

        if package:
            env.set_package(package)
            inst_library_1 = env.Install(env.subst('$BINSTAGEDIR'),f1)

        return env.addlibname(run_library1[0],target)


    def make_app_bundle4(self,dir,name,program,di=False,bg=False,resources=()):
        app=name+'.app'
        me=self.Dir('.').srcnode().abspath

        def make_app(target,source,env):
            d = target[0].abspath

            stub = name.replace(' ','_')+'__'
            env.safe_mkdir(join(d,'Contents','MacOS'))
            env.safe_mkdir(join(d,'Contents','Resources'))

            f=open(join(d,'Contents','PkgInfo'),'w')
            f.write("APPL????\n")
            f.close()

            pname=join(d,'Contents','MacOS',name)
            try: os.unlink(pname)
            except: pass
            os.symlink(program,pname)
            
            bgs = "<false/>"
            di_active = "<false/>"
            
            if di:
                di_active = "<true/>"
           
            if bg:
                bgs = "<true/>"

            f=open(join(d,'Contents','Info.plist'),'w')
            f.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
            f.write("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n")
            f.write("<plist version=\"1.0\">\n")
            f.write("<dict>\n")
            f.write("    <key>CFBundleDevelopmentRegion</key><string>English</string>\n")
            f.write("    <key>CFBundleExecutable</key><string>%s</string>\n" % name)
            f.write("    <key>CFBundleIconFile</key><string>%s.icns</string>\n" % name)
            f.write("    <key>CFBundleIdentifier</key><string>%s</string>\n" % app)
            f.write("    <key>CFBundleName</key><string>%s</string>\n" % name)
            f.write("    <key>CFBundlePackageType</key><string>APPL</string>\n")
            f.write("    <key>CFBundleSignature</key><string>????</string>\n")
            f.write("    <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>\n")
            f.write("    <key>CFBundleVersion</key><string>%s</string>\n" % env.subst('$PI_RELEASE') )
            f.write("    <key>LSBackgroundOnly</key>%s\n" % bgs)
            f.write("    <key>LSUIElement</key>%s\n" % di_active )
            f.write("</dict>\n")
            f.write("</plist>\n")
            f.close()

            ico = self.subst('$PI_ICNLOGO')
            if ico:
                f=open(join(d,'Contents','Resources',name+'.icns'),'wb')
                i=open(env.File(ico).abspath,'rb')
                f.write(i.read())
                f.close()
                i.close()

            res=[]
            for r in resources:
                res.extend(glob.glob(join(me,r)))

            res_dir = join(d,'Contents','Resources')
            for r in res:
                if os.path.isfile(r):
                    print 'link',r,'to',res_dir
                    os.link(r,join(res_dir,basename(r)))
                else:
                    print 'copy',r,'to',res_dir
                    mycopytree(r,join(res_dir,basename(r)))

        return self.Command(self.Dir(app,self.Dir(dir)),[],make_app)

    def make_package(self,name):

        env = self.Clone()
        env.Replace(PI_PACKAGENAME=name)

        meta = env.shared.package_descriptions[name]
        v = meta['version'] or env.subst('$PI_RELEASE')
        pkg='Eigenlabs-%s-%s.pkg' % (name.capitalize(),v)
        required = meta['required']

        def make_pkg(target,source,env):

            s = source[0].abspath
            d = target[0].abspath

            env.safe_mkdir(join(d,'Contents','Resources'))

            f=open(join(d,'Contents','PkgInfo'),'w')
            f.write("sjmdsjmd")
            f.close()
            
            f=open(join(d,'Contents','Info.plist'),'w')
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            f.write('<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
            f.write('<plist version="1.0">\n')
            f.write('<dict>\n')
            f.write('	<key>CFBundleGetInfoString</key>\n')
            f.write('	<string>Copyright (C) Eigenlabs Ltd</string>\n')
            f.write('	<key>CFBundleIdentifier</key>\n')
            f.write('	<string>%s</string>\n' % pkg)
            f.write('	<key>CFBundleShortVersionString</key>\n')
            f.write('	<string>%s</string>' % v)
            f.write('	<key>IFPkgFlagAllowBackRev</key>\n')
            f.write('	<false/>\n')
            f.write('	<key>IFPkgFlagAuthorizationAction</key>\n')
            f.write('	<string>AdminAuthorization</string>\n')
            f.write('	<key>IFPkgFlagBackgroundAlignment</key>\n')
            f.write('	<string>center</string>\n')
            f.write('	<key>IFPkgFlagBackgroundScaling</key>\n')
            f.write('	<string>tofit</string>\n')
            f.write('	<key>IFPkgFlagDefaultLocation</key>\n')
            f.write('	<string>/</string>\n')
            f.write('	<key>IFPkgFlagFollowLinks</key>\n')
            f.write('	<true/>\n')
            f.write('	<key>IFPkgFlagInstallFat</key>\n')
            f.write('	<false/>\n')
            f.write('	<key>IFPkgFlagIsRequired</key>\n')
            f.write('	<true/>\n' if required else '	<false/>\n')
            f.write('	<key>IFPkgFlagOverwritePermissions</key>\n')
            f.write('	<false/>\n')
            f.write('	<key>IFPkgFlagRelocatable</key>\n')
            f.write('	<false/>\n')
            f.write('	<key>IFPkgFlagRestartAction</key>\n')
            f.write('	<string>NoRestart</string>\n')
            f.write('	<key>IFPkgFlagRootVolumeOnly</key>\n')
            f.write('	<true/>\n')
            f.write('	<key>IFPkgFlagUpdateInstalledLanguages</key>\n')
            f.write('	<false/>\n')
            f.write('	<key>IFPkgFormatVersion</key>\n')
            f.write('	<real>0.10000000149011612</real>\n')
            f.write('</dict>\n')
            f.write('</plist>\n')
            f.close()

            f=open(join(d,'Contents','Resources','BundleVersions.plist'),'w')
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            f.write('<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
            f.write('<plist version="1.0">\n')
            f.write('<dict/>\n')
            f.write('</plist>\n')
            f.close()

            f=open(join(d,'Contents','Resources','Description.plist'),'w')
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            f.write('<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
            f.write('<plist version="1.0">\n')
            f.write('<dict>\n')
            f.write('	<key>IFPkgDescriptionDescription</key>\n')
            f.write('	<string>%(longdesc)s</string>\n' % meta)
            f.write('	<key>IFPkgDescriptionTitle</key>\n')
            f.write('	<string>%(desc)s</string>\n' % meta)
            f.write('</dict>\n')
            f.write('</plist>\n')
            f.close()

            os.system('mkbom %s %s' % (s,join(d,'Contents','Archive.bom')))
            #os.system('pax -x ustar -w -f %s -s ,%s/,, %s/*' % (join(d,'Contents','Archive.pax'),s,s))
            #print 'pax -x ustar -w -f %s -s ,%s/,, %s/*' % (join(d,'Contents','Archive.pax'),s,s)
            os.system('ditto -c -z %s %s' % (s,join(d,'Contents','Archive.pax.gz')))
            print 'ditto -c -z %s %s' % (s,join(d,'Contents','Archive.pax.gz'))

            etc = env.subst('$ETCINSTALLDIR')
            for script in ['preflight','postflight','preremove','postremove']:
                path=join(d,'Contents','Resources',script)
                f=open(path,'w')
                f.write('#!/bin/sh\n')
                f.write('export PI_PREFIX=%s\n' % env.subst('$INSTALLDIR'))
                f.write('export PI_ROOT=%s\n' % env.subst('$INSTALLROOTDIR'))
                f.write('export PI_RELEASE=%s\n' % env.subst('$PI_RELEASE'))
                f.write('export USER=`basename $HOME`\n')
                f.write('for script in %s/%s-*\n' % (etc,script))
                f.write('do\n')
                f.write('  if test -x "$script"; then sh -c "$script"; fi\n')
                f.write('done\n')
                f.write('exit 0\n')
                f.close()
                os.chmod(path,0755)

            f=open(join(d,'Contents','Resources','Welcome.txt'),'w')
            f.write('%(longdesc)s\n' % meta)
            f.close()

            # Not sure how to do the sizes.  It seems
            # to work OK without them anyway (??)

        return env.Command(env.Dir(pkg,env.Dir(env['PKGDIR'])),env.Dir('$STAGEDIR'),make_pkg)[0]


    def old_make_mpkg(self,name,pkgs):
        included_pkgnames = self.get_packages(name)
        included_pkgfiles = {}
        included_pkgnodes = {}
        included_pkgvers = {}
        pkg_path = []

        meta = self.shared.collections[name]
        v = self.subst('$PI_RELEASE')
        prereq = meta.get('prereq',[])

        for p in included_pkgnames:
            included_pkgvers[p] = v
            included_pkgnodes[p] = pkgs[p]
            included_pkgfiles[p] = os.path.basename(pkgs[p].abspath)
            d = os.path.dirname(pkgs[p].abspath)
            if d not in pkg_path:
                pkg_path.append(d)

        resdir = self.Dir(name,self.Dir('resources',self.Dir('$PKGDIR')))
        self.safe_mkdir(resdir.abspath)

        background = self.subst('$PI_BACKGROUND')
        if background and os.path.exists(background):
            shutil.copy(background,resdir.abspath)
        else:
            background = None

        license = self.subst('$PI_LICENSE')
        if license and os.path.exists(license):
            shutil.copy(license,resdir.abspath)
        else:
            license = None

        def pkg_info(env,target,source):
            d = target[0].abspath
            f=open(d+'','w')
            f.write('<?xml version="1.0" encoding="utf-8"?>\n')
            f.write('<installer-gui-script minSpecVersion="1">\n')
            f.write('<title>%s %s</title>\n' % (name,v))

            if license:
                f.write('<license file="%s"></license>\n' % os.path.basename(license))

            if background:
                f.write('<background file="%s"></background>\n' % os.path.basename(background))

            for pkg in included_pkgnames:
                f.write('  <pkg-ref id="%s"/>\n' % pkg.capitalize())

            f.write('  <options customize="never" allow-external-scripts="true" require-scripts="false" rootVolumeOnly="true"/>\n')

            if prereq:
                f.write('  <volume-check script="pm_volume_check();"/>\n')
                f.write('  <script>function pm_volume_check() {\n')

                for tf,m in prereq:
                    f.write("   if(!(system.files.fileExistsAtPath(my.target.mountpoint + '%s') == true)) { my.result.title = 'Failure'; my.result.message = '%s'; my.result.type = 'Fatal'; return false; }\n" % (tf,m))

                f.write('  return true;\n')
                f.write('  }\n')
                f.write('  </script>\n')


            f.write('  <choices-outline>\n')
            f.write('    <line choice="default">\n')

            for pkg in included_pkgnames:
                f.write('      <line choice="%s"/>\n' % pkg.capitalize())

            f.write('    </line>\n')
            f.write('  </choices-outline>\n')
            f.write('  <choice id="default"/>')

            for pkg in included_pkgnames:
                f.write('  <choice id="%s" visible="false">\n' % pkg.capitalize())
                f.write('    <pkg-ref id="%s"/>\n' % pkg.capitalize())
                f.write('  </choice>\n')
                f.write('  <pkg-ref id="%s" version="%s" onConclusion="none">%s</pkg-ref>\n' % (pkg.capitalize(),included_pkgvers[pkg],included_pkgfiles[pkg]))

            f.write('</installer-gui-script>\n')
            f.close()

        infoname = '%s-%s.pkgd' % (name,v)
        infonode = self.Command(self.File(infoname,self.Dir('info',self.subst('$PKGDIR'))),included_pkgnodes.values(),pkg_info)

        def pkg_file(env,target,source):
            d = target[0].abspath
            s = source[0].abspath
            pp = ''.join([' --package-path %s' % p for p in pkg_path])
            cmd = 'productbuild --resources %s --distribution %s %s %s' % (resdir.abspath,infonode[0].abspath,pp,d)
            print cmd
            os.system(cmd)

        mpkgname = '%s-%s.pkg' % (name,v)
        mpkgnode = self.Command(self.File(mpkgname,self.subst('$PKGDIR')),infonode+included_pkgnodes.values(),pkg_file)
        self.Alias('target-mpkg',mpkgnode)
        return mpkgnode

    def PiExternalRelease(self,version,compatible,organisation):
        if not self.PiRelease('contrib',compatible,compatible,organisation):
            return

        root = '/usr/pi'
        dist = os.path.join(root,'release-%s' % version)
        self.Append(LIBPATH=[os.path.join(dist,'bin')])
        self.Append(CPPPATH=[os.path.join(dist,'include')])

    def make_mpkg(self,name,pkgs):

        included_pkgnames = self.get_packages(name)
        included_pkgfiles = {}
        included_pkgnodes = {}
        included_pkgvers = {}

        meta = self.shared.collections[name]
        v = self.subst('$PI_RELEASE')
        prereq = meta.get('prereq',[])

        for p in included_pkgnames:
            included_pkgvers[p] = v
            included_pkgnodes[p] = pkgs[p]
            included_pkgfiles[p] = os.path.basename(pkgs[p].abspath)

        def mpkg(env,target,source):
            d = target[0].abspath
            env.safe_mkdir(join(d,'Contents','Packages'))
            f=open(join(d,'Contents','PkgInfo'),'w')
            f.write("sjmdsjmd")
            f.close()

            env.safe_mkdir(join(d,'Contents','Resources'))

            f=open(join(d,'Contents','Resources','package_version'),'w')
            f.write('major: 1\nminor: 0\n')
            f.close()

            bg = env.subst('$PI_BACKGROUND')
            if bg and os.path.exists(bg):
                shutil.copy(bg,join(d,'Contents','Resources','background.tiff'))

            env.safe_mkdir(join(d,'Contents','Resources','English.lproj'))

            license = env.subst('$PI_LICENSE')
            if license:
                shutil.copy(license,join(d,'Contents','Resources','English.lproj','License.rtf'))
            
            f=open(join(d,'Contents','Resources','English.lproj','%s.info'%name),'w')
            f.write('Title %s\n' % name)
            f.write('Version %s\n' % env.subst('$PI_RELEASE'))
            f.write('Description The world of PI.\n')
            f.write('DefaultLocation \n')
            f.write('DeleteWarning \n')
            f.write('### Package Flags\n')
            f.write('NeedsAuthorization NO\n')
            f.write('Required NO\n')
            f.write('Relocatable NO\n')
            f.write('RequiresReboot NO\n')
            f.write('UseUserMask NO\n')
            f.write('OverwritePermissions NO\n')
            f.write('InstallFat NO\n')
            f.write('RootVolumeOnly YES\n')
            f.write('OnlyUpdateInstalledLanguages NO\n')
            f.close()

            f=open(join(d,'Contents','Resources','English.lproj','Description.plist'),'w')
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            f.write('<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
            f.write('<plist version="1.0">\n')
            f.write('<dict>\n')
            f.write('	<key>IFPkgDescriptionDescription</key>\n')
            f.write('	<string>The world of PI.</string>\n')
            f.write('	<key>IFPkgDescriptionTitle</key>\n')
            f.write('	<string>%s</string>\n' % name)
            f.write('</dict>\n')
            f.write('</plist>\n')
            f.close()

            f=open(join(d,'Contents','Info.plist'),'w')
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            f.write('<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
            f.write('<plist version="1.0">\n')
            f.write('<dict>\n')
            f.write('	<key>IFPkgFlagBackgroundAlignment</key>\n')
            f.write('	<string>topleft</string>\n')
            f.write('	<key>IFPkgFlagBackgroundScaling</key>\n')
            f.write('	<string>none</string>\n')
            f.write('	<key>IFPkgFlagComponentDirectory</key>\n')
            f.write('	<string>Contents/Packages</string>\n')
            f.write('	<key>IFPkgFlagPackageList</key>\n')
            f.write('	<array>\n')

            for pkg in source:
                pn=os.path.basename(pkg.abspath)
                mycopytree(pkg.abspath,join(d,'Contents','Packages',pn),ignore=svn_filter)
                f.write('		<dict>\n')
                f.write('            <key>IFPkgFlagPackageLocation</key>\n')
                f.write('            <string>%s</string>\n' % pn)
                f.write('            <key>IFPkgFlagPackageSelection</key>\n')
                f.write('            <string>selected</string>\n')
                f.write('		</dict>\n')

            f.write('	</array>\n')
            f.write('	<key>IFPkgFormatVersion</key>\n')
            f.write('	<real>0.10000000149011612</real>\n')
            f.write('</dict>\n')
            f.write('</plist>\n')
            f.close()

            etc = env.subst('$ETCROOTINSTALLDIR')
            for script in ['preflight','postflight','preremove','postremove']:
                path=join(d,'Contents','Resources',script)
                f=open(path,'w')
                f.write('#!/bin/sh\n')
                f.write('export PI_PREFIX=/%s\n' % env.subst('$INSTALLDIR'))
                f.write('export PI_RELEASE=%s\n' % env.subst('$PI_RELEASE'))
                f.write('export USER=`basename $HOME`\n')
                f.write('if [ `echo %s/*/global-%s` != "%s/*/global-%s" ]; then\n' % (etc,script,etc,script))
                f.write('  for script in %s/*/global-%s\n' % (etc,script))
                f.write('  do\n')
                f.write('    sh -c "$script"\n')
                f.write('  done\n')
                f.write('fi\n')
                f.write('exit 0\n')
                f.close()
                os.chmod(path,0755)

        mpkgname = '%s-%s.mpkg' % (name,self.subst('$PI_RELEASE'))
        mpkgdname = 'mpkg-%s-%s' % (name,self.subst('$PI_RELEASE'))
        tgt = self.Command(self.Dir(mpkgname,self.Dir(mpkgdname,self.Dir('mpkg',self['PKGDIR']))),included_pkgnodes.values(),mpkg)
        self.Alias('target-pkg',tgt)
        self.Alias('target-mpkg',tgt)
        return tgt

    def make_dmg(env,name,mpkg):
        
        def dmg(env,target,source):
            img = target[0].abspath
            src = source[0].abspath
            print 'dmg src is',src
            os.system('hdiutil create -format UDBZ -volname %s-%s -srcfolder "%s" %s' % (name,env.subst('$PI_RELEASE'),src,img))

        dmgfile = env.File('%s-%s.dmg' % (name,env.subst('$PI_RELEASE')), env['PKGDIR'])
        tgt = env.Command(dmgfile,mpkg,dmg)
        env.Alias('target-pkg',tgt)
        env.Alias('target-mpkg',tgt)
        return tgt
fastmark_template = """
static const __attribute((section("__DATA,__fastdata"))) __attribute__((used)) unsigned fastmark__ = 0;
"""
