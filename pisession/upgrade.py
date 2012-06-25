
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

from pisession import session,upgrade_tools_v1
from pi import utils,paths,resource,guid,state
import os,sys,traceback,shutil
import pisession_native
import piw,picross
import glob

def get_format(snap):
    a = snap.get_agent_address(255,'meta',False)

    if not a.isvalid():
        return False

    r = a.get_root()
    c = map(ord,r.list_children())
    u = r.get_child(6).get_data().as_long() if 6 in c else 0
    return u

def get_upgrade(snap):
    a = snap.get_agent_address(255,'meta',False)

    if not a.isvalid():
        return False

    r = a.get_root()
    c = map(ord,r.list_children())
    u = r.get_child(4).get_data().as_bool() if 4 in c else False
    return u

def get_description(snap):
    a = snap.get_agent_address(255,'meta',False)

    if not a.isvalid():
        return ''

    r = a.get_root()
    c = map(ord,r.list_children())
    u = r.get_child(5).get_data().as_string() if 5 in c else ''
    return u

def get_version(snap):
    a = snap.get_agent_address(255,'meta',False)

    if not a.isvalid():
        return None

    r = a.get_root()
    c = map(ord,r.list_children())
    v = r.get_child(3).get_data().as_string() if 3 in c else None

    return v

def set_format(snap,fmt):
    a = snap.get_agent_address(255,'meta',True)
    a.get_root().get_child(6).set_data(piw.makelong(fmt,0))

def set_version(snap,version):
    a = snap.get_agent_address(255,'meta',True)
    a.get_root().get_child(3).set_data(piw.makestring(version,0))

def set_description(snap,desc=''):
    a = snap.get_agent_address(255,'meta',True)
    a.get_root().get_child(5).set_data(piw.makestring(desc,0))

def set_upgrade(snap,upg=True):
    a = snap.get_agent_address(255,'meta',True)
    a.get_root().get_child(4).set_data(piw.makebool(upg,0))

def find_tag(db,tag):
    snap = db.get_trunk()

    while True:
        if snap.tag() == tag:
            return snap
        p = snap.previous()
        if not p: break
        snap = db.get_version(p)

    raise RuntimeError('cant find tag: %s' % tag)

def copy_node(src,dst):
    dst.set_data(src.get_data())
    e = src.enum_children(0)
    while e!=0:
        sc = src.get_child(e)
        dc = dst.get_child(e)
        copy_node(sc,dc)
        e=src.enum_children(e)

def erase_snap(dst):
    while dst.agent_count()>0:
        dst_agent = dst.get_agent_index(0)
        dst_address = dst_agent.get_address()
        dst.erase_agent(dst_agent)

def copy_snap(src,dst):
    erase_snap(dst)

    for i in range(0,src.agent_count()):
        src_agent = src.get_agent_index(i)
        address = src_agent.get_address()
        type = src_agent.get_type()
        dst_agent = dst.get_agent_address(type,address,True)
        copy_node(src_agent.get_root(),dst_agent.get_root())

def do_upgrade(snap):
    upgrade_tools_v1.do_upgrade(snap)
    return True
 
def upgrade_trunk(src,dst,tweaker=None):
    src_db = state.open_database(src,False)
    src_snap = src_db.get_trunk()
    dst_db = state.open_database(dst,True)
    dst_snap = dst_db.get_trunk()
    copy_snap(src_snap,dst_snap)

    if do_upgrade(dst_snap):
        if tweaker is not None:
            tweaker(dst_snap,src_snap)
        else:
            dst_snap.save(0,'')
        dst_db.flush()
        return True

    try: os.unlink(dst)
    except: pass

    return False

def randomise_data(d,mapping):
    if not d.is_string():
        return d

    t = d.time()
    v = d.as_string()

    for (fn,tn) in mapping.iteritems():
        v=v.replace(fn,tn)

    if v == d.as_string():
        return d

    return piw.makestring(v,t)

def map_node(fn,tn,mapping):
    tn.set_data(randomise_data(fn.get_data(),mapping))

    e=fn.enum_children(0)
    while e!=0:
        fc = fn.get_child(e)
        tc = tn.get_child(e)
        map_node(fc,tc,mapping)
        e=fn.enum_children(e)

def map_snap(srcsnap,srcuid,dstsnap,dstuid):
    erase_snap(dstsnap)
    mapping = { srcuid: dstuid }

    for i in range(srcsnap.agent_count()):
        sa = srcsnap.get_agent_index(i)

        if sa.get_type() > 1:
            continue

        saddr = sa.get_address()

        if saddr == srcuid:
            taddr = dstuid
        else:
            taddr = saddr

        ta = dstsnap.get_agent_address(0,taddr,True)
        ta.set_type(sa.get_type())
        map_node(sa.get_root(),ta.get_root(),mapping)

def isint(s):
    try:
        return int(s)
    except:
        return None

def backup(file):
    dir=os.path.dirname(file)
    name=os.path.basename(file)

    spl = name.split()

    if len(spl)>1:
        num=isint(spl[-1])
        if num is not None:
            name=' '.join(spl[:-1])
        else:
            num=1
    else:
        num=1

    for x in xrange(num,100000):
        nn = os.path.join(dir,'%s %d'%(name,x))
        if not os.path.exists(nn):
            shutil.move(file,nn)
            return
            
    os.unlink(file)

def __copy_trunk(srcfile,dstfile):
    srcdb = state.open_database(srcfile,False)
    srcsnap = srcdb.get_trunk()

    if os.path.exists(resource.WC(dstfile)):
        os.unlink(resource.WC(dstfile))

    dstdb = state.open_database(dstfile,True)
    dstsnap = dstdb.get_trunk()

    copy_snap(srcsnap,dstsnap)
    dstsnap.save(0,'')
    dstdb.flush()

def get_description_from_file(dbfile):
    try:
        db = state.open_database(dbfile,False)
        snap = db.get_trunk()
        desc = get_description(snap)
        return desc
    except:
        utils.log_exception()
        return ''
   
def get_tmp_setup():
    i = 0
    while True:
        dbfile = resource.user_resource_file(resource.setup_dir,'tmpsetup%d'%i)
        if not os.path.exists(resource.WC(dbfile)):
            return dbfile
        i=i+1

def clr_tmp_setup():
    dbdir = resource.user_resource_dir(resource.setup_dir)
    for g in glob.glob(resource.WC(os.path.join(dbdir,'tmpsetup*'))):
        try:
            os.unlink(g)
            print 'delete',resource.MB(g)
        except:
            pass

def prepare_file(srcfile,version):
    dbfile = get_tmp_setup()

    if dbfile != srcfile:
        __copy_trunk(srcfile,dbfile)

    db = state.open_database(dbfile,True)
    snap = db.get_trunk()

    print srcfile,'version',snap.version()

    srcversion = get_version(snap)

    print 'prepare:',srcversion,'for',version,'to',dbfile

    if srcversion != version:
        if not do_upgrade(snap):
            raise RuntimeError('cant upgrade %s' % srcfile)
        set_version(snap,version)
        snap.save(0,'upgraded to %s' % version)
        db.flush()

    return snap

def copy_snap2file(srcsnap,dstfile,tweaker=None):
    try: os.unlink(dstfile)
    except: pass
    dstdb = state.open_database(dstfile,True)
    dstsnap = dstdb.get_trunk()
    copy_snap(srcsnap,dstsnap)
    if tweaker is not None:
        tweaker(dstsnap,srcsnap)
    dstsnap.save(0,'')
    dstdb.flush()
    return dstdb.get_trunk()

def get_setup_signature(snap,text=False):
    return upgrade_tools_v1.get_setup_signature(snap,text)

def split_setup(s):
    split1 = s.split('~',1)

    if len(split1) == 1:
        return '',s.strip()

    return split1[1].strip(),split1[0].strip()

