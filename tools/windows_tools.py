
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
import generic_tools
import shutil
import _winreg
import re

from os.path import join,isdir
from os import listdir
from SCons.Util import Split
from hashlib import md5

from xml.dom.minidom import *
from xml.sax.saxutils import escape

def dir_enumerator(factory,count,abs_path,node,node_path,directories,components):
    dirs = []

    for f in listdir(abs_path):
        af = join(abs_path,f)
        if isdir(af):
            dirs.append(f)
            continue

        count = count+1
        component_id = 'component%d' % count
        component = factory.createElement( 'Component' )
        component.attributes['Id'] = component_id
        file = factory.createElement( 'File')
        file.attributes['Id'] = 'file%d' % count
        file.attributes['DiskId'] = '1'
        file.attributes['Name'] = f
        file.attributes['Source'] = af
        component.childNodes.append(file)
        node.childNodes.append(component)
        components.append(component_id)

        if f.endswith('.py'):
            file = factory.createElement( 'RemoveFile')
            file.attributes['Id'] = 'removefile%d' % count
            file.attributes['Name'] = f+'c'
            file.attributes['On'] = 'uninstall'
            component.childNodes.append(file)

    for d in dirs:
        dir_path = node_path+(d,)
        if dir_path in directories:
            sub_directory = directories[dir_path]
            print 'directory hit for',dir_path
        else:
            count = count + 1
            sub_directory = factory.createElement( 'Directory' )
            sub_directory.attributes['Id'] = 'dir%d' % count
            sub_directory.attributes['Name'] = d
            node.childNodes.append(sub_directory)
            directories[dir_path] = sub_directory

        af = join(abs_path,d)
        count = dir_enumerator(factory,count,af,sub_directory,dir_path,directories,components)

    return count


def generate_guids(root):
    """ generates globally unique identifiers for parts of the xml which need
    them.

    Component tags have a special requirement. Their UUID is only allowed to
    change if the list of their contained resources has changed. This allows
    for clean removal and proper updates.

    To handle this requirement, the uuid is generated with an md5 hashing the
    whole subtree of a xml node.
    """

    # specify which tags need a guid and in which attribute this should be stored.
    needs_id = { 'Product'   : 'Id',
                 'Component' : 'Guid',
               }

    # find all XMl nodes matching the key, retrieve their attribute, hash their
    # subtree, convert hash to string and add as a attribute to the xml node.
    for (key,value) in needs_id.items():
        node_list = root.getElementsByTagName(key)
        attribute = value
        for node in node_list:
            if not node.hasAttribute(attribute):
                hash = md5(node.toxml()).hexdigest()
                hash_str = '%s-%s-%s-%s-%s' % ( hash[:8], hash[8:12], hash[12:16], hash[16:20], hash[20:] )
                node.attributes[attribute] = hash_str



class PiWindowsEnvironment(generic_tools.PiGenericEnvironment):
    def __init__(self):
        generic_tools.PiGenericEnvironment.__init__(self,'win32','EigenLabs','Belcanto',None,TARGET_ARCH='x86')

        self.Replace(IS_WINDOWS=True)
        self.Replace(PI_MODPREFIX='')
        self.Replace(PI_MODSUFFIX='.pyd')
        self.Replace(PI_MODLINKFLAGS='$SHLINKFLAGS')
        self.Replace(RELEASESTAGEROOTDIR=join('$STAGEDIR'))
        self.Replace(RELEASESTAGEDIR=join('$STAGEDIR','$PI_COLLECTION-$PI_RELEASE'))
        self.Append(CCFLAGS='/EHsc /w34355 /MD /O2 /fp:fast /arch:SSE2 /DWIN32')
        self.Replace(PI_PLATFORMTYPE='windows')
        self.Append(LINKFLAGS=Split('/MANIFEST /INCREMENTAL:NO /LARGEADDRESSAWARE'))
        self.Append(SHLINK=' $LIBMAPPER')
        self.Append(LINK=' $LIBMAPPER')
        self.Append(LIBS=Split('shell32'))

        self.Replace(LINKFLAGS_LOCKED=Split("/SECTION:.text,!P /SECTION:.data,!P /SECTION:.rdata,!P /SECTION:.bss,!P"))

        if os.environ.get('PI_DEBUGBUILD'):
            self.Append(CCFLAGS=Split('/Zi'))
            self.Append(LINKFLAGS=Split('/DEBUG'))

        self.shared.merge_modules = []
        self.shared.package_init = []
        self.shared.shortcuts = {}

    def sign(self,tgt):
        cert = self.get('PI_CERTFILE')
        if cert:
            pwd = self['PI_CERTPASS']
            self.AddPostAction(tgt,'signtool sign /q /t http://timestamp.verisign.com/scripts/timestamp.dll /f "%s" /p "%s" "${TARGET}"' % (cert,pwd))

    def add_manifest(self,tgt):
        self.AddPostAction(tgt, 'mt -nologo -manifest "${TARGET}.manifest" -outputresource:"$TARGET";2')

    def set_subsystem(self,tgt,subsys):
        self.AddPostAction(tgt,'editbin /nologo /subsystem:%s "${TARGET}"' % subsys)

    def cache_dir(self):
        return os.path.join(os.environ.get('TEMP'),'.sconscache')

    def doenv(self):
        pth_file = self.File('env.cmd',self.subst('#')).abspath

        pth_template=("set PATH=%(bindir)s;%%PATH%%\n")

        pp=self.Dir(self['BINRUNDIR']).abspath
        pth_node = self.baker(pth_file,pth_template,bindir=pp)
        self.Alias('target-default',pth_node)

    def make_wxs(self,name,filename,package):
        version = self.subst('$PI_RELEASE').split('-')[0]
        if re.match("^\\d+\\.\\d+$", version):
            version = version+".0"
        print 'wsx for collection',name,'containing',package
        meta = self.shared.package_descriptions[package]

        pkgid = meta.get('pkgid')

        doc  = Document()
        root = doc.createElement( 'Wix' )
        root.attributes['xmlns']='http://schemas.microsoft.com/wix/2006/wi'
        doc.appendChild( root )

        Product = doc.createElement( 'Product' )
        Package = doc.createElement( 'Package' )

        root.childNodes.append( Product )
        Product.childNodes.append( Package )

        Product.attributes['Name']         = escape( name )
        Product.attributes['Version']      = escape( version )
        Product.attributes['Manufacturer'] = escape( 'Eigenlabs Ltd' )
        Product.attributes['Language']     = escape( '1033' )
        Package.attributes['Description']  = escape( 'EigenD' )
        Package.attributes['Compressed']  = 'yes'
        Package.attributes['InstallerVersion']  = '300'
        Package.attributes['InstallPrivileges']  = 'elevated'
        Package.attributes['InstallScope']  = 'perMachine'

        Media = doc.createElement('Media')
        Media.attributes['Id']       = '1'
        Media.attributes['Cabinet']  = 'default.cab'
        Media.attributes['EmbedCab'] = 'yes'

        Product.childNodes.append( Media )

        root_directory = doc.createElement( 'Directory' )
        root_directory.attributes['Id'] = 'TARGETDIR'
        root_directory.attributes['Name'] = 'SourceDir'
        Product.childNodes.append(root_directory)

        pfiles_directory = doc.createElement( 'Directory' )
        pfiles_directory.attributes['Id'] = 'ProgramFilesFolder'
        pfiles_directory.attributes['Name'] = 'PFiles'
        root_directory.childNodes.append(pfiles_directory)

        eigen_directory = doc.createElement( 'Directory' )
        eigen_directory.attributes['Id'] = 'Eigenlabs'
        eigen_directory.attributes['Name'] = 'EigenLabs'
        pfiles_directory.childNodes.append(eigen_directory)

        directories = {}

        component_directory = doc.createElement( 'Directory' )
        component_directory.attributes['Id'] = 'Components'
        component_directory.attributes['Name'] = self.subst('$PI_COLLECTION-$PI_RELEASE')
        eigen_directory.childNodes.append(component_directory)

        directories[(self.subst('$PI_COLLECTION-$PI_RELEASE'),)] = component_directory

        seq = doc.createElement('InstallExecuteSequence')
        remove_node = doc.createElement('RemoveExistingProducts')
        remove_node.attributes['After'] = 'InstallInitialize'
        seq.childNodes.append(remove_node)

        Product.childNodes.append(seq)

        self.shared.package_init.sort(key=lambda e:e[3])

        for i,(p,v,u,o) in enumerate(self.shared.package_init):
            if p != package:
                continue

            action = doc.createElement('CustomAction')
            action.attributes['Id'] = 'action%d' % i
            action.attributes['Execute'] = 'deferred'
            action.attributes['Directory'] = 'Components'
            action.attributes['Impersonate'] = 'yes' if u else 'no'
            action.attributes['Return'] = 'ignore'
            action.attributes['ExeCommand'] = '[Components]bin\%s.exe [CURRENTDIRECTORY]' % v

            Product.childNodes.append(action)

            custom = doc.createElement('Custom')
            custom.attributes['Action'] = 'action%d' % i
            custom_text = doc.createTextNode('NOT Installed')
            custom.childNodes.append(custom_text)

            seq.childNodes.append(custom)

            if i==0:
                custom.attributes['After'] = 'InstallFiles'
            else:
                custom.attributes['After'] = 'action%d' % (i-1)


        for i,mf in enumerate(self.shared.merge_modules):
            merge = doc.createElement( 'Merge' )
            merge.attributes['Id'] = 'merge%d' % i
            merge.attributes['SourceFile'] = mf
            merge.attributes['DiskId'] = '1'
            merge.attributes['Language'] = '0'
            component_directory.childNodes.append(merge)

            feature = doc.createElement( 'Feature' )
            feature.attributes['Id']     = 'mfeature%d' % i
            feature.attributes['Level']  = '1'
            feature.attributes['Title']  = 'merge feature %d' % i
            feature.attributes['AllowAdvertise']  = 'no'
            feature.attributes['Display']  = 'hidden'
            Product.childNodes.append(feature)

            mergeref = doc.createElement('MergeRef')
            mergeref.attributes['Id'] = 'merge%d' % i
            feature.childNodes.append(mergeref)


        count = 0

        shortcuts = []

        abs_path = self.get_stage(package).abspath
        target_directory = eigen_directory

        feature = doc.createElement( 'Feature' )
        feature.attributes['Id']     = 'feature0'
        feature.attributes['Level']  = '1'
        feature.attributes['Title']  = meta['desc']
        Product.childNodes.append(feature)

        components = []

        count = dir_enumerator(doc,count,abs_path,target_directory,(),directories,components)

        for c in components:
            component_ref = doc.createElement( 'ComponentRef' )
            component_ref.attributes['Id'] = c
            feature.childNodes.append(component_ref)

        if package in self.shared.shortcuts:
            scomponent = doc.createElement('Component')
            scomponent.attributes['Id'] = 'scomponent0'
            shortcuts.append(scomponent)

            remove1 = doc.createElement('RemoveFolder')
            remove1.attributes['Id'] = 'sremove0'
            remove1.attributes['On'] = 'uninstall'
            remove1.attributes['Directory'] = 'ComponentsMenu'

            scomponent.childNodes.append(remove1)

            remove2 = doc.createElement('RemoveFolder')
            remove2.attributes['Id'] = 'xremove0'
            remove2.attributes['On'] = 'uninstall'
            remove2.attributes['Directory'] = 'EigenlabsMenu'

            scomponent.childNodes.append(remove2)

            reg_key = doc.createElement('RegistryKey')
            reg_key.attributes['Root'] = "HKCU"
            reg_key.attributes['Action'] = "createAndRemoveOnUninstall"
            reg_key.attributes['Key'] = "Software\\Eigenlabs\\Components\\" + self.subst('$PI_COLLECTION')+'-'+version+("\\%s"%package)

            reg_v1 = doc.createElement('RegistryValue')
            reg_v1.attributes['Type'] = "string"
            reg_v1.attributes['Name'] = 'CurrentRelease'
            reg_v1.attributes['Value'] = self.subst('$PI_RELEASE')
            reg_v1.attributes['KeyPath'] = 'yes'
            reg_key.childNodes.append(reg_v1)

            scomponent.childNodes.append(reg_key)

            for j,(n,e) in enumerate(self.shared.shortcuts[package]):
                shortcut = doc.createElement('Shortcut')
                shortcut.attributes['Id'] = 'shortcut0_%d' % j
                shortcut.attributes['Name'] = n
                shortcut.attributes['Description'] = n
                shortcut.attributes['Target'] = '[Components]%s' % e
                shortcut.attributes['WorkingDirectory'] = 'Components'
                scomponent.childNodes.append(shortcut)

            component_ref = doc.createElement( 'ComponentRef' )
            component_ref.attributes['Id'] = 'scomponent0'
            feature.childNodes.append(component_ref)

        if shortcuts:
            shortcuts_directory = doc.createElement( 'Directory' )
            shortcuts_directory.attributes['Id'] = 'ProgramMenuFolder'
            shortcuts_directory.attributes['Name'] = 'PMenus'
            root_directory.childNodes.append(shortcuts_directory)

            eigenshortcuts_directory = doc.createElement( 'Directory' )
            eigenshortcuts_directory.attributes['Id'] = 'EigenlabsMenu'
            eigenshortcuts_directory.attributes['Name'] = 'EigenLabs'
            shortcuts_directory.childNodes.append(eigenshortcuts_directory)

            shortcut_components_directory = doc.createElement( 'Directory' )
            shortcut_components_directory.attributes['Id'] = 'ComponentsMenu'
            shortcut_components_directory.attributes['Name'] = self.subst('$PI_COLLECTION-$PI_RELEASE')
            eigenshortcuts_directory.childNodes.append(shortcut_components_directory)
            shortcut_components_directory.childNodes.extend(shortcuts)


        if not pkgid:
            hash = md5(Product.toxml()).hexdigest()
            pkgid = '%s-%s-%s-%s-%s' % ( hash[:8], hash[8:12], hash[12:16], hash[16:20], hash[20:] )

        Product.attributes['UpgradeCode']  = pkgid
        upgrade_node = doc.createElement('Upgrade')
        upgrade_node.attributes['Id'] = pkgid
        upgrade_version = doc.createElement('UpgradeVersion')
        upgrade_version.attributes['Minimum'] = escape(version)
        upgrade_version.attributes['Maximum'] = escape(version)
        upgrade_version.attributes['IncludeMinimum'] = 'yes'
        upgrade_version.attributes['IncludeMaximum'] = 'yes'
        upgrade_version.attributes['OnlyDetect'] = 'no'
        upgrade_version.attributes['Property'] = 'OLDVERSIONBEINGUPGRADED'
        upgrade_node.childNodes.append(upgrade_version)
        Product.childNodes.append(upgrade_node)

        generate_guids(root)

        open(filename,'w').write( doc.toprettyxml() )

    def get_stage(self,package):
        env = self.Clone()
        env.Replace(PI_PACKAGENAME=package)
        return env.Dir('$STAGEDIR')

    def make_package(self,package_name):
        wixbin = os.path.join(os.environ.get('WIX'),'bin')

        if not os.path.exists(wixbin):
            return

        release = self.subst('$PI_RELEASE')

        wxsfile = self.File('%s.wxs' % (package_name), os.path.join(self['PKGDIR'],'tmp'))
        objfile = self.File('%s.wixobj' % (package_name), os.path.join(self['PKGDIR'],'tmp'))
        msifile = self.File('Eigenlabs-%s-%s.msi' % (package_name.capitalize(),release), self['PKGDIR'])

        stage = self.get_stage(package_name)
        name = "%s-%s" % (package_name,release)

        wxstgt = self.Command(wxsfile,stage,lambda env,target,source: self.make_wxs(name,target[0].abspath,package_name))
        objtgt = self.Command(objfile,wxstgt,'"%s\\candle.exe" -nologo -out $TARGET $SOURCE' % wixbin)
        msitgt = self.Command(msifile,objtgt,'"%s\\light.exe" -spdb -nologo -out $TARGET $SOURCE' % wixbin)

        self.sign(msitgt)

        self.Alias('target-pkg',msitgt)

        return msitgt[0]

    def make_setup_nsis(self,collection,packages,meta):
        nsisbin = "c:\\Program Files (x86)\\NSIS\\makensis.exe"

        release = self.subst('$PI_RELEASE')
        exefile = self.File('%s-%s.exe' % (collection,release), self['PKGDIR'])
        nsifile = self.File('%s.nsi' % (collection), os.path.join(self['PKGDIR'],'tmp'))
        bitmap = self.subst('$PI_BMPLOGO')
        icon = self.subst('$PI_ICOLOGO')
        license = self.subst('$PI_LICENSE')
        prereq = meta.get('prereq',[])

        def make_nsi(env,target,source):
            f = open(target[0].abspath,'w')
            f.write('!include "MUI2.nsh"\n')
            f.write('Name "%s-%s"\n' % (collection,release))
            f.write('OutFile "%s"\n' % exefile.abspath)
            f.write('BrandingText "http://www.eigenlabs.com"\n')
            f.write('!define MUI_HEADERIMAGE\n')

            if icon:
                f.write('!define MUI_ICON "%s"\n' % icon)
            if bitmap:
                f.write('!define MUI_HEADERIMAGE_BITMAP "%s"\n' % bitmap)

            if prereq:
                f.write('Function .onInit\n')

                for i,(d,m) in enumerate(prereq):
                    f.write('IfFileExists "$INSTDIR\\%s" PreReq%dok\n' % (d,i))
                    f.write('MessageBox MB_ICONEXCLAMATION "%s"\n' % m)
                    f.write('Abort\n')
                    f.write('PreReq%dok:\n' % i)


                f.write('FunctionEnd\n')

            if license:
                f.write('!insertmacro MUI_PAGE_LICENSE "%s"\n' % license)
                f.write('!insertmacro MUI_PAGE_INSTFILES\n')
                f.write('!insertmacro MUI_LANGUAGE "English"\n')

            f.write('InstallDir "$PROGRAMFILES\\Eigenlabs"\n')

            for i,(n,s) in enumerate(packages):
                b = os.path.basename(s.abspath)
                f.write('Section "%d"\n' %i)
                f.write('SetOutPath "$INSTDIR\\tmp"\n')
                f.write('SetDetailsPrint textonly\n')
                f.write('DetailPrint "Installing %s"\n' % n)
                f.write('SetDetailsPrint listonly\n')
                f.write('File "%s"\n' % s.abspath)
                f.write('SetOutPath "$EXEDIR"\n')
                f.write('ExecWait \'msiexec /q /i "$INSTDIR\\tmp\\%s"\'\n' % b)
                f.write('Delete "$INSTDIR\\tmp\\%s"\n' % b)
                f.write('SectionEnd\n')

        nsitgt = self.Command(nsifile,[p[1] for p in packages],make_nsi)
        exetgt = self.Command(exefile,nsitgt,'"%s" "$SOURCE"' % nsisbin)

        self.sign(exetgt)
        self.Alias('target-mpkg',exetgt)
        return exetgt[0]


    def Initialise(self):
        generic_tools.PiGenericEnvironment.Initialise(self)


    def Finalise(self):
        generic_tools.PiGenericEnvironment.Finalise(self)
        self.doenv()

        pkgs = dict([ (pkg,self.make_package(pkg)) for pkg in self.shared.packages ])

        for c in self.shared.collections:
            meta = self.shared.collections[c]
            cpkgs = []

            for (k,p) in meta['external'].items():
                cpkgs.append((k,self.File(p)))

            for p in self.get_packages(c):
                cpkgs.append((self.shared.package_descriptions[p]['desc'],pkgs[p]))

            setup = self.make_setup_nsis(c,cpkgs,meta)

    def libmapper(self,target,source,env,for_signature):
        libs = env.get('PILIBS')
        map = []

        for l in libs:
            ll=env.get_shlib(l)
            if not ll.libnode:
                map.append('"%s.lib"' % l)
            else:
                map.append('"%s"' % ll.libnode.abspath)

        return ' '.join(map)


    def PiPythonwWrapper(self,name,pypackage,module,main,appname=None,bg=False,di=False,private=False,usegil=False,package=None):
        bigname = appname or name[0:1].upper()+name[1:].lower()
        if package and not private:
            self.shared.shortcuts.setdefault(package,[]).append((bigname,'bin\\%s.exe' % name))

        return generic_tools.PiGenericEnvironment.PiPythonwWrapper(self,name,pypackage,module,main,appname=appname,bg=bg,di=di,private=private,usegil=usegil,package=package)

    def PiGuiProgram(self,target,sources,bg=False,di=False,package=None,appname=None,private=False,libraries=[],hidden=True):
        bigname = appname or target[0:1].upper()+target[1:].lower()
        if package and not private:
            self.shared.shortcuts.setdefault(package,[]).append((bigname,'bin\\%s.exe' % target))

        return self.PiProgram(target,sources,package=package,libraries=libraries,gui=True)

    def PiProgram(self,name,sources,libraries=[],package=None,gui=False,hidden=True):
        env = self.Clone()
        env.Append(PILIBS=libraries)
        env.Replace(PDB='%s.pdb' % name)

        ico = self.subst('$PI_ICOLOGO')
        srcs = env.Split(sources)

        if ico and gui:
            rc_source = env.File('%s.rc' % name);
            rc_source_node = env.baker(rc_source,'1 ICON "%s"\r\n' % ico.replace('\\','\\\\'))
            rc_res = env.Command('%s.res' % name, rc_source_node, 'rc /R /Fo "${TARGET}" "${SOURCE}"')
            env.Depends(rc_res,ico)
            srcs.extend(rc_res)

        bld_binary,bld_pdb=env.Program(name,srcs)
        bld_binary=(bld_binary,)

        env.add_manifest(bld_binary)
        env.add_runtime_user(bld_binary)
        env.addlibuser(bld_binary,libraries)

        rv = []

        run_binary=env.Install(env.subst('$BINRUNDIR'),bld_binary)
        run_pdb=env.Install(env.subst('$BINRUNDIR'),bld_pdb)

        rv.extend(run_binary)

        if gui:
            con_binary = env.InstallAs(env.File(join(env.subst('$BINRUNDIR'),'%s_con.exe' % name)),bld_binary)
            env.set_subsystem(run_binary,'WINDOWS')
            env.set_subsystem(con_binary,'CONSOLE')
            rv.extend(con_binary)
        else:
            env.set_subsystem(run_binary,'CONSOLE')

        inst_binary = []

        if package:
            env.set_package(package)
            inst_binary = env.Install(env.subst('$BINSTAGEDIR'),run_binary)
            env.sign(inst_binary)
            rv.extend(inst_binary)
            if gui:
                inst_con_binary = env.Install(env.subst('$BINSTAGEDIR'),con_binary)
                env.sign(inst_con_binary)
                rv.extend(inst_con_binary)

        return rv

    def PiRuntime(self,package):
        env = self.Clone()
        target = os.path.join(os.path.dirname(self['PI_PYTHON']),'python26.dll')
        f2 = env.File(target)

        run_file=env.Install(env.subst('$BINRUNDIR'),f2)
        env.add_runtime_files(run_file)

        if package:
            env.set_package(package)
            inst_file = env.Install(env.subst('$BINSTAGEDIR'),f2)

    def PiBinaryDLL(self,target,package=None):
        env = self.Clone()

        f1 = env.File(target+'.lib')
        f2 = env.File(target+'.dll')

        run_library1=env.Install(env.subst('$BINRUNDIR'),f1)
        run_library2=env.Install(env.subst('$BINRUNDIR'),f2)
        env.Depends(run_library1,run_library2)
        env.Alias('target-runtime',run_library1)

        if package:
            env.set_package(package)
            inst_library_1 = env.Install(env.subst('$BINSTAGEDIR'),f2)

        return env.addlibname(run_library1[0],target)


    def PiPipBinding(self,module,spec,locked=False,**kwds):
        env = self.Clone()

        if locked:
            env.Append(LINKFLAGS=env.subst('$LINKFLAGS_LOCKED'))
            
        return generic_tools.PiGenericEnvironment.PiPipBinding(env,module,spec,**kwds)

    def PiSharedLibrary(self,target,sources,libraries=[],package=None,hidden=True,deffile=None,per_agent=None,public=False,locked=False):
        env = self.Clone()

        env.Append(PILIBS=libraries)
        env.Append(CCFLAGS='-DBUILDING_%s' % target.upper())
        env.Replace(SHLIBNAME=target)
        env.Replace(PDB='%s.pdb' % target)

        if locked:
            env.Append(LINKFLAGS=env.subst('$LINKFLAGS_LOCKED'))

        env.set_agent_group(per_agent)

        objects = env.SharedObject(sources)

        self.Depends(objects,self.PiExports(target,package,public))

        if deffile:
            objects.append(deffile)

        bin_dll,bin_pdb,bin_lib,bin_exp = env.SharedLibrary(target,objects)

        env.add_manifest(bin_dll)

        run_dll = env.Install(env.subst('$BINRUNDIR'),bin_dll)
        run_lib = env.Install(env.subst('$BINRUNDIR'),bin_lib)
        run_pdb = env.Install(env.subst('$BINRUNDIR'),bin_pdb)

        env.Depends(run_lib,run_dll)

        inst_dll = []

        env.addlibuser(bin_dll,libraries)
        env.addlibname(run_lib[0],target,dependencies=libraries)

        if package:
            env.set_package(package)
            inst_dll = env.Install(env.subst('$BINSTAGEDIR'),run_dll)
            env.sign(inst_dll)
            if public:
                inst_lib = env.Install(env.subst('$BINSTAGEDIR'),run_lib)

        return inst_dll+run_lib

    def PiPackageInit(self,package,program,as_user=False,order=1):
        self.shared.package_init.append((package,program,as_user,order))

    def PiMergeModule(self,target):
        self.shared.merge_modules.append(self.File(target).srcnode().abspath)

    def PiReleaseFile(self,package,name,bigname):
        etc_env=self.Clone()
        etc_env.set_package(package)
        stagesource = etc_env.Install(etc_env.subst('$ETCSTAGEDIR'),name)
        self.shared.shortcuts.setdefault(package,[]).append((bigname,'etc\\%s\\%s' % (package,name)))

    def PiExternalRelease(self,version,compatible,organisation):
        if not self.PiRelease('contrib',compatible,compatible,organisation):
            return

        root = os.environ.get('ProgramFiles(x86)')
        if not root:
            root = os.environ.get('ProgramFiles')

        dist = os.path.join(root,'Eigenlabs','release-%s' % version)
        self.Append(LIBPATH=[os.path.join(dist,'bin')])
        self.Append(CPPPATH=[os.path.join(dist,'include')])
