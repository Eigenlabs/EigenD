
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

bugs_dir='Bugs'
global_dir='Global'
help_dir='Help'
impulseresponse_dir='ImpulseResponse'
lock_dir='Lock'
log_dir='Log'
loop_dir='Loop'
plugins_dir='Plugins'
recordings_dir='Recordings'
recordingstmp_dir='Recordings-tmp'
scripts_dir='Scripts'
setup_dir='Setups'
default_setup='default_setup'
current_setup='current_setup'
user_details='user_details'
instrument_dir='instruments'

def __mkdir(name):
    if not os.path.exists(name):
        os.mkdir(name)
    if not os.path.isdir(name):
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

    if not os.path.isdir(userdir_g):
        return []

    ver = {}

    for r in os.listdir(userdir_g):
        v,t = split_version(r)

        if v is None:
            continue

        if filter is not None and not filter(v,t,r):
            continue

        p = os.path.join(userdir_g,r)
        if os.path.isdir(p):
            ver[(v,t)]=r

    ver_keys = ver.keys()
    ver_keys.sort(reverse=True)
    vers = [ver[k] for k in ver_keys ]

    return vers

def get_release_dir(category):
    res_root = picross.release_resource_dir()
    res_dir = os.path.join(res_root,category)
    return res_dir if os.path.exists(res_dir) else None

def find_release_resource(category,name):
    reldir = get_release_dir(category)

    if not reldir:
        return None

    filename = os.path.join(reldir,name)

    if os.path.exists(filename):
        return filename

    return None

def find_resource(category,name,usercopy=False):
    userdir = user_resource_dir(category)
    user_filename = os.path.join(userdir,name)

    if os.path.exists(user_filename):
        return user_filename

    rel_filename = find_release_resource(category,name)

    if not rel_filename:
        return None

    if usercopy:
        shutil.copyfile(rel_filename,user_filename)
        return user_filename

    return rel_filename

def list_user_resources(category,pattern):
    userdir = user_resource_dir(category)
    files = []

    if userdir:
        files.extend(glob.glob(os.path.join(userdir,pattern)))

    return files

def list_resources(category,pattern):
    reldir = get_release_dir(category)
    userdir = user_resource_dir(category)
    files = []

    if userdir:
        files.extend(glob.glob(os.path.join(userdir,pattern)))

    files.extend(glob.glob(os.path.join(reldir,pattern)))
    return files

def user_resource_dir(category,version=None):
    userdir = get_home_dir(version=version)
    workdir = os.path.join(userdir,category)
    __mkdir(workdir)
    return workdir

def new_resource_file(category,name,version=None):
    userdir = user_resource_dir(category,version=version)
    filename = os.path.join(userdir,name)
    if os.path.exists(filename):
        os.unlink(filename)
    return filename

def user_resource_file(category,name,version=None):
    userdir = user_resource_dir(category,version=version)
    filename = os.path.join(userdir,name)
    return filename

def clean_current_setup():
    def_state_file = user_resource_file(global_dir,current_setup)
    for statefile in glob.glob(def_state_file+'*'):
        os.unlink(statefile)

class LockFile:
    def __init__(self,name):
        print 'init lock file'      
        
        filename = lock_file(name)
        self.__file = open(filename,'wb')
            
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
    if os.path.exists(lf):
        os.unlink(lf)

    for s in suffixes:
        lfo = os.path.join(dir,'%s.%d.%s' % (target,s,suffix))
        lfn = os.path.join(dir,'%s.%d.%s' % (target,s+1,suffix))
        if os.path.exists(lfo):
            os.rename(lfo,lfn)


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
        self.__file =  open(get_logfile(target,logfile_max=logfile_max),'w')
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

