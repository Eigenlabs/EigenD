
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

import os
import glob

import time
import datetime
import picross
import shutil
import sys
import traceback

from pi import version,utils

if os.name != 'nt':
    import fcntl


def split_version(n):
    try:
        s1 = n.split('-',1)

        if len(s1)==1:
            v=s1[0]
            t=None
        else:
            (v,t)=s1

        (a,b,c) = v.split('.')
        return (int(a),int(b),int(c)),t
    except:
        return None,None

def is_macosx():
    return picross.is_macosx()

def is_linux():
    return picross.is_linux()
    
def is_windows():
    return picross.is_windows()

# convert to multi-byte string if needed
def MB(string):
    if is_windows() and string is not None and isinstance(string,unicode):
        return string.encode('utf_8')
    return string

# convert to wide character string if needed
def WC(string):
    if is_windows() and string is not None and isinstance(string,str):
        return string.decode('utf_8')
    return string

def safe_walk(rd):
    if rd and os_path_isdir(rd):
        for (a,b,c) in os.walk(WC(rd)):
            a = MB(a)
            b = map(MB,b)
            c = map(MB,c)
            yield (a,b,c)

def os_listdir(name):
    result = os.listdir(WC(name))
    return map(MB,result)

def os_mkdir(name):
    return os.mkdir(WC(name))

def os_makedirs(name):
    return os.makedirs(WC(name))

def os_unlink(name):
    return os.unlink(WC(name))

def os_rename(name1,name2):
    return os.rename(WC(name1),WC(name2))

def os_path_exists(path):
    return os.path.exists(WC(path))

def os_path_getsize(path):
    return os.path.getsize(WC(path))

def os_path_isdir(path):
    return os.path.isdir(WC(path))

def os_path_isfile(path):
    return os.path.isfile(WC(path))

def os_path_getmtime(path):
    return os.path.getmtime(WC(path))

def glob_glob(pattern):
    result = glob.glob(WC(pattern))
    return map(MB,result)

def os_remove(name):
    return os.remove(WC(name))

def file_open(name,mode='r',buff=-1):
    return open(WC(name),mode,buff)

def file_file(name,mode='r',buff=-1):
    return file(WC(name),mode,buff)

def shutil_copyfile(src,dst):
    shutil.copyfile(WC(src),WC(dst))

bugs_dir='Bugs'
global_dir='Global'
help_dir='Help'
impulseresponse_dir='ImpulseResponse'
instrument_dir='Instruments'
lock_dir='Lock'
log_dir='Log'
loop_dir='Loop'
plugins_dir='Plugins'
recordings_dir='Recordings'
recordingstmp_dir='Recordings-tmp'
scalemanager_dir='Scale Manager'
scripts_dir='Scripts'
setup_dir='Setups'
default_setup='default_setup'
current_setup='current_setup'
user_details='user_details'

def __mkdir(name):
    if not os_path_exists(name):
        os_mkdir(name)
    if not os_path_isdir(name):
        raise RuntimeError('%s is not a directory' % name)

def lock_file(name):
    userdir_g = picross.global_library_dir()
    __mkdir(userdir_g)
    lockdir_g = os.path.join(userdir_g,lock_dir)
    __mkdir(lockdir_g)

    return os.path.join(lockdir_g,name+'.lck')

def current_version():
    return picross.release()

def cache_dir():
    return os.path.join(get_home_dir(),'Cache')

def get_home_dir(version=None):
    release = current_version() if version is None else version
    userdir_g = picross.global_library_dir()
    __mkdir(userdir_g)

    if release is '':
        return userdir_g

    userdir_r = os.path.join(userdir_g,release)
    __mkdir(userdir_r)

    return userdir_r


def find_installed_versions(filter=None):
    userdir_g = picross.global_library_dir()

    if not os_path_isdir(userdir_g):
        return []

    ver = {}
     
    for r in os_listdir(WC(userdir_g)):
        r = MB(r)
        v,t = split_version(r)

        if v is None:
            continue

        if filter is not None and not filter(v,t,r):
            continue

        p = os.path.join(userdir_g,r)
        if os_path_isdir(p):
            ver[(v,t)]=r

    ver_keys = ver.keys()
    ver_keys.sort(reverse=True)
    vers = [ver[k] for k in ver_keys ]

    return vers

def get_release_dir(category):
    res_root = picross.release_resource_dir()
    res_dir = os.path.join(res_root,category)
    return res_dir if os_path_exists(res_dir) else None

def find_release_resource(category,name):
    reldir = get_release_dir(category)

    if not reldir:
        return None

    filename = os.path.join(reldir,name)

    if os_path_exists(filename):
        return filename

    return None

def find_resource(category,name,usercopy=False):
    userdir = user_resource_dir(category)
    user_filename = os.path.join(userdir,name)

    if os_path_exists(user_filename):
        return user_filename

    rel_filename = find_release_resource(category,name)

    if not rel_filename:
        return None

    if usercopy:
        shutil_copyfile(rel_filename,user_filename)
        return user_filename

    return rel_filename

def list_user_resources(category,pattern):
    userdir = user_resource_dir(category)
    files = []

    if userdir:
        files.extend(glob_glob(os.path.join(userdir,pattern)))

    return files

def list_resources(category,pattern):
    reldir = get_release_dir(category)
    userdir = user_resource_dir(category)
    files = []

    if userdir:
        files.extend(glob_glob(os.path.join(userdir,pattern)))

    files.extend(glob_glob(os.path.join(reldir,pattern)))
    return files

def user_resource_dir(category,version=None):
    userdir = get_home_dir(version=version)
    workdir = os.path.join(userdir,category)
    __mkdir(workdir)
    return workdir

def new_resource_file(category,name,version=None):
    userdir = user_resource_dir(category,version=version)
    filename = os.path.join(userdir,name)
    if os_path_exists(filename):
        os_unlink(filename)
    return filename

def user_resource_file(category,name,version=None):
    userdir = user_resource_dir(category,version=version)
    filename = os.path.join(userdir,name)
    return filename

def clean_current_setup():
    def_state_file = user_resource_file(global_dir,current_setup)
    for statefile in glob_glob(def_state_file+'*'):
        os_unlink(statefile)

class LockFile:
    def __init__(self,name):
        print 'init lock file'      
        
        filename = lock_file(name)
        self.__file = file_open(filename,'wb')
            
    def __term__(self):
        self.__file.close()

    def unlock(self):
        try:
            if is_windows():
                pass
                hfile = win32file._get_osfhandle(self.__file.fileno())
                overlapped = pywintypes.OVERLAPPED()
                win32file.UnlockFileEx(hfile, 0, -0x10000, overlapped)
            else: 
                fcntl.lockf(self.__file.fileno(),fcntl.LOCK_UN)
        except:
            pass

    def lock(self):
        try:
            if is_windows():
                return True
                hfile = win32file._get_osfhandle(self.__file.fileno())
                overlapped = pywintypes.OVERLAPPED()
                flags = win32con.LOCKFILE_EXCLUSIVE_LOCK|win32con.LOCKFILE_FAIL_IMMEDIATELY
                win32file.LockFileEx(hfile, flags, 0, -0x10000, overlapped)
            else: 
                fcntl.lockf(self.__file.fileno(),fcntl.LOCK_EX|fcntl.LOCK_NB)
        except:
            return False

        return True


def rotate_logfile(dir,target,logfile_max,suffix='log'):
    suffixes = range(0,logfile_max)
    suffixes.reverse()

    lf = os.path.join(dir,'%s.%d.%s' % (target,logfile_max,suffix))
    if os_path_exists(lf):
        os_unlink(lf)

    for s in suffixes:
        lfo = os.path.join(dir,'%s.%d.%s' % (target,s,suffix))
        lfn = os.path.join(dir,'%s.%d.%s' % (target,s+1,suffix))
        if os_path_exists(lfo):
            os_rename(lfo,lfn)


def get_logfile(target,logfile_max=7):
    dir = user_resource_dir(log_dir)
    rotate_logfile(dir,target,logfile_max)
    log = os.path.join(dir,'%s.0.log' % target)
    return log

def get_bugfile():
    dir = user_resource_dir(bugs_dir,version='')
    rotate_logfile(dir,'bug-report',logfile_max=30,suffix='zip')
    log = os.path.join(dir,'%s.0.zip' % 'bug-report')
    return log

class LogFile:
    def __init__(self,target,logfile_max=7):
        self.__file =  file_open(get_logfile(target,logfile_max=logfile_max),'w')
        self.__start = time.time()
        print >>self.__file,target,'startup on',str(datetime.datetime.today())

    def write(self,s):
        if s=='\n' or s==' ':
            line = s
        else:
            line = "%.2f: %s" % (time.time()-self.__start,s)
        self.__file.write(line)

    def flush(self):
        self.__file.flush()

def open_logfile(target,logfile_max=7):
    try:
        return LogFile(target,logfile_max)
    except:
        print >>sys.__stderr__,"Unexpected error while opening logfile target '%s'" % (target)
        traceback.print_exc()
        return None

