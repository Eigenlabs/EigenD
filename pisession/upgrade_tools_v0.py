
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

import upgrade
import registry
import piw
import picross
from pi import utils,const,paths,guid,logic
import os
import shutil
import traceback
import pisession.version as session_version
import hashlib
import upgrade_agentd

class Agent:
    def __init__(self,name,version,cversion,module=None,zip=None,instance=None,always=False):
        self.name = name
        self.version = version
        self.cversion = '99.0'
        self.module = module
        self.instance = instance
        self.zip = zip
        self.always = always

    def signature(self):
        return '%s:%s:%s' % (self.name,self.version,self.cversion)

class Registry(registry.Registry):
    def __init__(self):
        registry.Registry.__init__(self)
        self.__instances = {}
        zd=os.path.join(picross.release_root_dir(),'plugins')
        self.scan_path(zd, lambda n,v,c,z,m: Agent(n,v,c,module=m,zip=z))
        self.add_agentd()

    def add_module(self,name,version,cversion,module):
        return registry.Registry.add_module(self,name,version,'99.0',module)

    def add_agentd(self):
        self.add_module(upgrade_agentd.plugin,upgrade_agentd.version,upgrade_agentd.cversion,Agent(upgrade_agentd.plugin,upgrade_agentd.version,upgrade_agentd.cversion,instance=upgrade_agentd.upgrade,always=True))

    def get_instance(self,agent):
        if agent.instance:
            return agent.instance

        sig = agent.signature()
        if sig in self.__instances:
            return self.__instances[sig]

        m = __import__(agent.module,fromlist=['upgrade'])
        self.__instances[sig] = m.upgrade
        return m.upgrade

def find_upgrade(reg,sig):
    sig = sig.split(':')

    if len(sig)!=3:
        return None

    module = sig[0]
    version = sig[1]
    cversion = sig[2]
    compatible_module = None
    incompatible_module = None

    for(pv,cv,m) in reg.iter_versions(module):
        if registry.iscompatible(cv,cversion):
            if compatible_module is None:
                compatible_module = m
            else:
                if cv > compatible_module.cversion:
                    compatible_module = m
        else:
            if incompatible_module is None:
                incompatible_module = m
            else:
                if pv > incompatible_module.version:
                    incompatible_module = m

    if compatible_module:
        return compatible_module

    return incompatible_module

def build_mapping(snap,reg):
    mapping = {}

    for i in range(0,snap.agent_count()):
        agent = snap.get_agent_index(i)
        if agent.get_type()>1:
            continue

        address = agent.get_address()
        root = agent.get_root()
        old_signature = root.get_data()

        if not old_signature.is_string():
            continue

        old_signature = old_signature.as_string()
        if old_signature.endswith('*'):
            old_signature=old_signature[:-1]

        module = find_upgrade(reg,old_signature)

        if module is None or (not module.always and module.signature()==old_signature):
            continue

        mapping[address] = (old_signature,module)

    return mapping

class Node:
    def __init__(self,dbnode,server,path):
        self.dbnode = dbnode
        self.path = path
        self.server = server

    def id(self):
        return paths.makeid_list(self.server,*self.path)
    
    def __getitem__(self,n):
        return self.get_node(n)

    def set_data(self,data):
        self.dbnode.set_data(data)

    def get_data(self):
        return self.dbnode.get_data()

    def set_string(self,value,ts=0):
        self.dbnode.set_data(piw.makestring(value,ts))
        
    def set_long(self,value,ts=0):
        self.dbnode.set_data(piw.makelong(value,ts))

    def set_float(self,value,ts=0):
        self.dbnode.set_data(piw.makefloat(value,ts))

    def set_bool(self,value,ts=0):
        self.dbnode.set_data(piw.makebool(value,ts))

    def get_string_default(self,value):
        d = self.dbnode.get_data()
        if d.is_string():
            return d.as_string()
        self.dbnode.set_data(piw.makestring(value,0))
        return value

    def get_string(self):
        return self.dbnode.get_data().as_string()

    def iter_atom_tree(self):
        yield self
        for c in self.iter():
            for cc in c.iter_atom_tree():
                yield cc

    def __checknames(self,names):
        m = self.getmeta(const.meta_names)
        for n in names:
            if not n in m:
                return False
        return True

    def find_byname(self,*names):
        for c in self.iter_atom_tree():
            if c.__checknames(names):
                return c
        return None

    def get_node(self,*path):
        n = self.dbnode
        p = path
        while p:
            p1 = p[0]
            p = p[1:]
            if n.enum_children(p1-1)!=p1:
                return None
            n = n.get_child(p1)
        return Node(n,self.server,self.path+path)

    def ensure_node(self,*path):
        n = self.dbnode
        p = path
        while p:
            n = n.get_child(p[0])
            p = p[1:]
        return Node(n,self.server,self.path+path)

    def iter_all(self,extension=None):
        n = self.dbnode
        e = n.enum_children(0)
        while e!=0:
            node = Node(n.get_child(e),self.server,self.path+(e,))
            if e==extension:
                for n2 in node.iter_all(255):
                    yield n2
            else:
                yield node

            e=n.enum_children(e)

    def erase_all_children(self):
        n = self.dbnode
        e = n.enum_children(0)
        while e!=0:
            n.get_child(e).erase()
            e = n.enum_children(0)

    def erase_children(self):
        n = self.dbnode
        e = n.enum_children(0)
        while e!=0 and e<254:
            n.get_child(e).erase()
            e = n.enum_children(0)

    def iter(self,extension=None):
        n = self.dbnode
        e = n.enum_children(0)
        while e!=0:
            node = Node(n.get_child(e),self.server,self.path+(e,))
            if e==extension:
                for n2 in node.iter_all(255):
                    yield n2
            else:
                if e!=254 and e!=255:
                    yield node

            e=n.enum_children(e)

    def walk(self):
        yield self
        for c in self.iter():
            for cc in c.walk():
                yield cc
            

    def erase(self):
        self.dbnode.erase()
        self.dbnode = None
        self.path = None

    def remove(self,*path):
        n = self.get_node(*path)
        if n is not None:
            n.erase()

    def __copy(self,dst,src):
        dst.set_data(src.get_data())

        e = dst.enum_children(0)
        while e!=0:
            dst.erase_child(e)
            e=dst.enum_children(e)

        e = src.enum_children(0)
        while e!=0:
            self.__copy(dst.get_child(e),src.get_child(e))
            e=src.enum_children(e)

    def copy(self,src):
        self.__copy(self.dbnode,src.dbnode)

    def getname(self):
        t=[]
        t.extend(self.getmeta(const.meta_adjectives))
        t.extend(self.getmeta(const.meta_names))
        o = self.get_node(255,const.meta_ordinal)
        if o and o.get_data().is_long(): t.append(str(o.get_data().as_long()))
        return ' '.join(t)

    def getmeta(self,meta):
        meta = self.get_node(255,meta)
        return utils.strsplit(meta.get_string()) if meta else []
        
    def setmeta(self,meta,*bits):
        meta = self.ensure_node(255,meta)
        meta.set_string(' '.join(bits))

    def setmetalong(self,meta,value):
        meta = self.ensure_node(255,meta)
        meta.set_long(value)

    def delmeta(self,meta,*words):
        bits = self.getmeta(meta)
        for word in words:
            if word in bits:
                bits.remove(word)
        self.setmeta(meta,*bits)

    def addmeta(self,meta,*words):
        bits = self.getmeta(meta)
        for word in words:
            if word not in bits:
                bits.append(word)
        self.setmeta(meta,*bits)

    def setname(self,names='',ordinal=None):
        self.setmeta(const.meta_adjectives)
        self.setmeta(const.meta_names,*utils.strsplit(names))
        self.setmetalong(const.meta_ordinal,ordinal)

    def rename(self,names='',adjectives='',ordinal=None):
        self.delmeta(const.meta_adjectives,*utils.strsplit(names))
        self.addmeta(const.meta_names,*utils.strsplit(names))
        self.delmeta(const.meta_names,*utils.strsplit(adjectives))
        self.addmeta(const.meta_adjectives,*utils.strsplit(adjectives))

def do_upgrade(snap):
    tools = UpgradeTools(snap)
    return tools.do_upgrade()

class UpgradeTools:
    def major_version(self):
        return 0

    def __init__(self,snap):
        self.__snap = snap
        self.__scratch = {}
        self.__ccache = None
        self.__registry = Registry()
        self.__mapping = build_mapping(snap,self.__registry)

        self.mapping = {}
        for (k,(os,m)) in self.__mapping.iteritems():
            self.mapping[k] = (os,m.signature())

        self.roots = {}
        for i in range(0,snap.agent_count()):
            agent = snap.get_agent_index(i)
            address = agent.get_address()
            root = agent.get_root()

            if agent.get_type()>1 or not root.get_data().is_string():
                continue

            sig = root.get_data().as_string().split(':')
            if guid.isguid(address) and len(sig) == 3:
                root = guid.split(address)[0]
                self.roots[root] = address

        self.subsystems = {}
        for i in range(0,snap.agent_count()):
            agent = snap.get_agent_index(i)
            address = agent.get_address()
            root = agent.get_root()

            if agent.get_type()>1 or not root.get_data().is_string():
                continue

            sig = root.get_data().as_string().split(':')
            if guid.isguid(address) and len(sig)>3:
                root = guid.split(address)[0]
                parent = self.roots.get(root)
                if parent:
                    self.subsystems.setdefault(parent,[]).append(address)

    def call_phase1(self,module,tools,address):
        try:
            upgrader = self.__registry.get_instance(module)
            if upgrader:
                if not upgrader(tools.oldcversion(address),tools.newcversion(address),tools,address,1):
                    return False
            tools.update_signature(address)
            return True
        except:
            traceback.print_exc()
            return False

    def call_phase2(self,module,tools,address):
        try:
            upgrader = self.__registry.get_instance(module)
            if upgrader:
                if not upgrader(tools.oldcversion(address),tools.newcversion(address),tools,address,2):
                    return False
            return True
        except:
            traceback.print_exc()
            return False

    def do_upgrade(self):
        for (k,(oldsig,m)) in self.__mapping.iteritems():
            if not self.call_phase1(m,self,k):
                print 'upgrade v0 phase 1 failed',k
                return False

        for (k,(oldsig,m)) in self.__mapping.iteritems():
            if not self.call_phase2(m,self,k):
                print 'upgrade v0 phase 2 failed',k
                return False

        return True

    def __substitute_connection(self,tgt,old_id,new_id):
        (s,p) = paths.breakid_list(tgt)
        root = self.root(s)
        atom = root.get_node(*p)
        if atom is None: return
        conn = atom.get_node(255,2)
        if conn is None: return
        old_conn = logic.parse_clauselist(conn.get_string_default(''))
        new_conn = []

        for c in old_conn:
            if c.args[2]==old_id:
                new_conn.append(logic.make_term(c.pred,c.args[0],c.args[1],new_id,c.args[3]))
            else:
                new_conn.append(c)

        conn.set_string(logic.render_termlist(new_conn))

    def find_slaves(self,id):
        self.__build_ccache()
        return self.__ccache.get(id,[])

    def substitute_connection(self,old_id,new_id):
        self.__build_ccache()
        tgts = self.__ccache.get(old_id)
        if not tgts: return
        for tgt in tgts:
            self.__substitute_connection(tgt,old_id,new_id)

    def __build_ccache(self):
        if self.__ccache is not None:
            return

        self.__ccache = {}

        for i in range(0,self.__snap.agent_count()):
            agent = self.__snap.get_agent_index(i)
            address = agent.get_address()
            root = agent.get_root()

            if agent.get_type()>1 or not root.get_data().is_string():
                continue

            node = Node(root,address,())

            for a in node.walk():
                self.__build_ccache_atom(a)

    def __build_ccache_atom(self,atom):
        tgt = atom.id()
        conn = atom.get_node(255,2)
        if conn is None: return
        conn_term = logic.parse_clauselist(conn.get_string_default(''))
        for c in conn_term:
            src = c.args[2]
            self.__ccache.setdefault(src,[]).append(tgt)

    def root(self,agent):
        a = self.__snap.get_agent_address(0,agent,False)
        if a.isvalid(): return Node(a.get_root(),agent,())
        a = self.__snap.get_agent_address(1,agent,False)
        if a.isvalid(): return Node(a.get_root(),agent,())
        return None

    def scratchpad(self,agent):
        k='%s:%s' % (agent,self.newcversion(agent))
        s=self.__scratch.setdefault(k,{})
        return s

    def version(self):
        return session_version.version

    def agent_type(self,agent):
        return self.root(agent).get_data().as_string().split(':')[0]

    def newrversion(self,agent):
        return self.mapping[agent][1].split(':')[1]

    def oldrversion(self,agent):
        return self.mapping[agent][0].split(':')[1]

    def newcversion(self,agent):
        return self.mapping[agent][1].split(':')[2]

    def oldcversion(self,agent):
        return self.mapping[agent][0].split(':')[2]

    def update_signature(self,agent):
        sig = self.mapping[agent][1]
        self.root(agent).set_string(sig)

    def get_subsystems(self,address):
        ss = self.subsystems.get(address)
        if not ss: return []
        return ss

    def upgrade_trunk(self,oldstate,newstate,tweaker=None):
        return upgrade.upgrade_trunk(oldstate,newstate,tweaker)

    def link(self,src,dst):
        if hasattr(os,'link'):
            os.link(src,dst)
        else:
            shutil.copyfile(src,dst)


def __findroot(snap,base):
    for i in range(0,snap.agent_count()):
        agent = snap.get_agent_index(i)
        address = agent.get_address()
        root = agent.get_root()

        if agent.get_type()>1 or not root.get_data().is_string():
            continue

        sig = root.get_data().as_string().split(':')
        if guid.isguid(address) and len(sig) == 3:
            root = guid.split(address)[0]
            if root == base:
                return address

    return base

def __getfullname(snap,cache,address):
    (id,path) = paths.breakid_list(address)

    if id in cache:
        return paths.makeid_list(cache[id],*path)

    agent = snap.get_agent_address(0,id,False)
    if not agent.isvalid():
        agent = snap.get_agent_address(1,id,False)
        if not agent.isvalid():
            cache[id] = id
            return paths.makeid_list(id,*path)

    root = agent.get_root()
    sig = root.get_data().as_string().split(':')
    parent_name = None

    if guid.isguid(address) and len(sig)>3:
        parent = __findroot(snap,guid.split(address)[0])
        parent_name = __getfullname(snap,cache,parent)

    node = Node(root,id,())
    name = node.getname()

    if parent_name:
        name = parent_name+' '+name

    cache[id] = name
    return paths.makeid_list(name,*path)


def __getname(snap,cache,address):
    (id,path) = paths.breakid_list(address)

    if id in cache:
        return paths.makeid_list(cache[id],*path)

    agent = snap.get_agent_address(0,id,False)
    if not agent.isvalid():
        agent = snap.get_agent_address(1,id,False)
        if not agent.isvalid():
            cache[id] = id
            return paths.makeid_list(id,*path)

    root = agent.get_root()
    node = Node(root,id,())
    name = node.getname()

    cache[id] = name
    return paths.makeid_list(name,*path)



def __getsig(snap,cache,agent):
    address = agent.get_address()
    root = agent.get_root()

    if agent.get_type()!=0 or not root.get_data().is_string():
        return None

    node = Node(root,address,())
    srcs = set()
    agent_name = __getname(snap,cache,address)

    for atom in node.walk():
        tgt = atom.id()
        name = atom.get_node(255,8)
        pathstr = '.'.join([str(p) for p in atom.path])

        if not name:
            continue

        name = name.get_data()
        if not name.is_string() or not name.as_string():
            continue

        conn = atom.get_node(255,2)

        if not conn:
            continue

        if not conn.get_data().is_string():
            continue

        conn_term = logic.parse_clauselist(conn.get_string())
        for c in conn_term:
            src = c.args[2]
            srcs.add('%s: %s' % (pathstr,__getname(snap,cache,src)))

    l = list(srcs)
    l.sort()

    return agent_name+' '+' '.join(l)

def get_setup_signature(snap,text=False):
    agents = snap.agent_count()
    signature = []
    cache = {}

    for i in range(0,agents):
        agent = snap.get_agent_index(i)
        agent_signature = __getsig(snap,cache,agent)
        if agent_signature:
            signature.append(agent_signature)

    signature.sort()
    signature_text = '\n'.join(signature)

    if text:
        return signature_text
    else:
        return hashlib.md5(signature_text).hexdigest()


def __getportname(snap,address):
    (id,path) = paths.breakid_list(address)

    agent = snap.get_agent_address(0,id,False)
    if not agent.isvalid():
        agent = snap.get_agent_address(1,id,False)
        if not agent.isvalid():
            return id,'.'.join([str(p) for p in path])

    agent_node = Node(agent.get_root(),id,())
    port_node = agent_node.get_node(*path)
    port_name = port_node.get_node(255,8)

    if not port_name:
        return id,'.'.join([str(p) for p in path])

    return id,port_name.get_data().as_string()

def __getdot(snap,cache,agent):
    address = agent.get_address()
    root = agent.get_root()

    if agent.get_type()!=0 or not root.get_data().is_string():
        return []

    node = Node(root,address,())
    dot = []
    agent_name = __getfullname(snap,cache,address)

    dot_node = '"%s" [ label="%s" ]' % (address,agent_name)
    dot.append(dot_node)

    for atom in node.walk():
        tgt = atom.id()
        name = atom.get_node(255,8)
        pathstr = '.'.join([str(p) for p in atom.path])

        if not name:
            continue

        name = name.get_data()
        if not name.is_string() or not name.as_string():
            name = 'talker'

        conn = atom.get_node(255,2)

        if not conn:
            continue

        if not conn.get_data().is_string():
            continue

        conn_term = logic.parse_clauselist(conn.get_string())
        for c in conn_term:
            src = c.args[2]
            (src_addr,src_name) = __getportname(snap,src)
            edge = '"%s"  -> "%s" [label = "%s->%s"]' % (src_addr,address,src_name,name)
            dot.append(edge)

    return dot

def get_dot(snap):
    agents = snap.agent_count()
    signature = []
    cache = {}

    for i in range(0,agents):
        agent = snap.get_agent_index(i)
        agent_signature = __getdot(snap,cache,agent)
        signature.extend(agent_signature)

    signature_text = 'digraph eigend {'+'\n'.join(signature)+'\n}'

    return signature_text
