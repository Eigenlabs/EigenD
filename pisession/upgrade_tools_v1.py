
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

from pi import paths,logic,guid
from pisession import registry,upgrade_agentd

import piw
import os
import traceback
import hashlib
import picross

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
    name = node.get_name()

    cache[id] = name
    return paths.makeid_list(name,*path)


def __getsig(snap,cache,agent):
    address = agent.get_address()
    root = agent.get_root()

    if agent.get_type()!=0 or not root.get_data().is_dict():
        return None

    node = Node(root,address,())
    srcs = set()
    agent_name = __getname(snap,cache,address)

    for atom in node.iter_tree():
        meta = atom.get_data()
        name = atom.get_name()

        if not meta.is_dict() or not name:
            continue

        conn = meta.as_dict_lookup('master')
        if not conn.is_string():
            continue

        pathstr = '.'.join([str(p) for p in atom.path])
        conn_term = logic.parse_clauselist(conn.as_string())
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


class Node:
    def __init__(self,dbnode,server,path,tools=None):
        self.dbnode = dbnode
        self.path = path
        self.server = server
        self.tools = tools

    def id(self):
        return paths.makeid_list(self.server,*self.path)
    
    def __getitem__(self,n):
        return self.get_node(n)

    def set_data(self,data):
        self.dbnode.set_data(data)

    def get_data(self):
        return self.dbnode.get_data()

    def set_name(self,name):
        name = name.split()
        ordinal = None

        try:
            if len(name)>0:
                ordinal=int(name[-1])
                name = name[:-1]
        except:
            pass

        name = ' '.join(name)

        d = self.get_data()

        if d.is_null():
            d = piw.dictnull(0)

        if d.is_dict():
            if name:
                d = piw.dictset(d,'name',piw.makestring(name,0))
            else:
                d = piw.dictdel(d,'name')
            if ordinal:
                d = piw.dictset(d,'ordinal',piw.makelong(ordinal,0))
            else:
                d = piw.dictdel(d,'ordinal')

        self.set_data(d)

    def set_meta_long(self,key,value):
        self.set_meta(key,piw.makelong(value,0))

    def set_meta_string(self,key,value):
        self.set_meta(key,piw.makestring(value,0))

    def set_meta(self,key,value):
        m = self.get_data()

        if m.is_null():
            m = piw.dictnull(0)

        if m.is_dict():
            m = piw.dictset(m,key,value)
            self.set_data(m)

        if key == 'master' and self.tools:
            self.tools.invalidate_connections()

    def get_meta_string(self,key,default=''):
        m = self.get_meta(key)
        if m is not None and m.is_string():
            return m.as_string()
        return default

    def get_meta_long(self,key,default=0):
        m = self.get_meta(key)
        if m is not None and m.is_long():
            return m.as_long()
        return default

    def get_meta(self,key):
        d = self.get_data()
        if d.is_dict():
            n = d.as_dict_lookup(key)
            if not n.is_null():
                return n
        return None

    def get_master(self):
        conn = self.get_meta('master')
        if conn and conn.is_string() and conn.as_string():
            try:
                conn_parsed = logic.parse_termlist(conn.as_string())
                return [c.args[2] for c in conn_parsed]
            except:
                pass

        return []

    def move_connection(self,old_id,new_id):
        print self.id(),'replacing',old_id,'with',new_id
        conn = self.get_meta('master')
        if conn and conn.is_string() and conn.as_string():
            conn_parsed = logic.parse_termlist(conn.as_string())
            conn_replaced = []
            for c in conn_parsed:
                if c.args[2] == old_id:
                    conn_replaced.append(logic.make_term(c.pred,c.args[0],c.args[1],new_id,c.args[3]))
                else:
                    conn_replaced.append(c)
            conn_new = logic.render_termlist(conn_replaced)
            self.set_meta_string('master',conn_new)
            print self.id(),'replaced',conn,'with',conn_new

    def get_name(self):
        d = self.get_data()
        if d.is_dict():
            n = d.as_dict_lookup('name')
            if n.is_string():
                n = n.as_string()
                if n:
                    o = d.as_dict_lookup('ordinal')
                    if o.is_long():
                        n=n+' '+str(o.as_long())
                    return n
        return ''

    def get_tree(self,index=0):
        node = self.dbnode
        value = node.get_data()
        child_names = map(ord,node.list_children())
        children = [self.get_node(c).get_tree(c) for c in child_names]
        return (index,value,children)

    def set_tree(self,tree):
        node = self.dbnode
        (index,value,children) = tree
        node.set_data(value)
        self.erase_children()
        for c in children: self.ensure_node(c[0]).set_tree(c)

    def copy(self,node):
        self.set_tree(node.get_tree())

    def erase_child(self,c):
        n = self.dbnode
        e = n.enum_children(c-1)
        if e!=0:
            n.get_child(e).erase()

    def erase_children(self):
        n = self.dbnode
        e = n.enum_children(0)
        while e!=0:
            n.get_child(e).erase()
            e = n.enum_children(0)

    def get_node(self,*path):
        n = self.dbnode
        p = path
        while p:
            p1 = p[0]
            p = p[1:]
            if n.enum_children(p1-1)!=p1:
                return None
            n = n.get_child(p1)
        return Node(n,self.server,self.path+path,self.tools)

    def ensure_node(self,*path):
        n = self.dbnode
        p = path
        while p:
            n = n.get_child(p[0])
            p = p[1:]
        return Node(n,self.server,self.path+path,self.tools)

    def iter(self,extension=None,exclude=[]):
        n = self.dbnode
        e = n.enum_children(0)
        while e!=0:
            node = Node(n.get_child(e),self.server,self.path+(e,),self.tools)
            if e==extension:
                for n2 in node.iter(255):
                    yield n2
            else:
                if e not in exclude:
                    yield node

            e=n.enum_children(e)

    def iter_tree(self,extension=None):
        yield self
        for c in self.iter():
            for cc in c.iter_tree():
                yield cc

    def mimic_connections(self,fr,to,name,argstocopy=(0,1,3)):
        from_input = self.get_node(*fr)
        if from_input:
            input_name = from_input.get_name()

            input_connections = from_input.get_meta('master')
            if input_connections and input_connections.is_string() and input_connections.as_string():
                input_connections_parsed = logic.parse_termlist(input_connections.as_string())
                output_connections = []

                if input_connections_parsed:
                    for input_conn in input_connections_parsed:
                        upstream_addr,upstream_path = paths.breakid_list(input_conn.args[2])
                        upstream_root = self.tools.get_root(upstream_addr)
                        if upstream_root:
                            upstream_node = upstream_root
                            for n in upstream_path[:-1]:
                                c = upstream_node.get_node(n)
                                if c:
                                    upstream_node = c
                            for c in upstream_node.iter():
                                if name == c.get_name():
                                    conn = logic.make_term('conn',input_conn.args[0] if 0 in argstocopy else None,input_conn.args[1] if 1 in argstocopy else None,c.id(),input_conn.args[3] if 3 in argstocopy else None)
                                    output_connections.append(conn)

                    if len(output_connections):
                        conn_txt = logic.render_termlist(output_connections)
                        c = self.ensure_node(*to)
                        c.set_meta_string('master', conn_txt)
                        print 'connected',name,'for',c.id(),'from',conn_txt
            

class Agent:
    def __init__(self,name,version,cversion,module,instance=None):
        self.name = name
        self.version = version
        self.cversion = cversion
        self.module = module
        self.instance = instance

    def signature(self):
        return '%s:%s:%s' % (self.name,self.version,self.cversion)


class Registry(registry.Registry):
    def __init__(self):
        registry.Registry.__init__(self,Agent)
        self.__instances = {}
        self.add_agentd()

    def add_agentd(self):
        self.add_module(upgrade_agentd.plugin,upgrade_agentd.version,upgrade_agentd.cversion,Agent(upgrade_agentd.plugin,upgrade_agentd.version,upgrade_agentd.cversion,None,instance=upgrade_agentd.upgrade))

    def get_instance(self,agent):
        if agent.instance:
            return agent.instance

        sig = agent.signature()
        if sig in self.__instances:
            return self.__instances[sig]

        m = registry.import_module(agent.module)
        self.__instances[sig] = m.upgrade
        return m.upgrade



class UpgradeTools:
    def __init__(self,snap):
        self.__snap = snap
        self.__registry = Registry()
        self.__build_mapping()
        self.__connections = None

    def major_version(self):
        return 1

    def __build_mapping(self):
        self.__mapping = {}
        self.__subsys = set()

        for i in range(0,self.__snap.agent_count()):
            agent = self.__snap.get_agent_index(i)
            if agent.get_type()>1:
                continue

            address = agent.get_address()
            root = agent.get_root()
            old_signature = root.get_data()

            if not old_signature.is_dict():
                continue

            plugin = old_signature.as_dict_lookup('plugin').as_string()
            version = old_signature.as_dict_lookup('version').as_string()
            cversion = old_signature.as_dict_lookup('cversion').as_string()
            module = self.__registry.get_compatible_module(plugin,cversion)

            if module is not None:
                self.__mapping[address] = (old_signature,module)

            if not old_signature.as_dict_lookup("subsystem").is_null():
                self.__subsys.add(address)

    def invalidate_connections(self):
        self.__connections = None

    def move_connections(self,old_id,new_id):
        connections = self.get_connections()

        old_server,old_path = paths.breakid_list(old_id)
        old_path = tuple(old_path)
        old_path_len = len(old_path)
        old_connections = connections.get(old_server,{})
        new_server,new_path = paths.breakid_list(new_id)
        new_path = tuple(new_path)

        for src_path,tgt_id_set in old_connections.items():
            if src_path[:old_path_len] == old_path:
                old_src_id = paths.makeid_list(old_server,*src_path)
                new_src_id = paths.makeid_list(new_server,*(new_path+src_path[old_path_len:]))
                for tgt_id in tgt_id_set:
                    tgt_server,tgt_path = paths.breakid_list(tgt_id)
                    tgt_node = self.get_root(tgt_server).get_node(*tgt_path)
                    if tgt_node:
                        tgt_node.move_connection(old_src_id,new_src_id)
            

    def get_connections(self):
        if self.__connections is not None:
            return self.__connections

        connections = {}

        for i in range(0,self.__snap.agent_count()):
            agent = self.__snap.get_agent_index(i)
            address = agent.get_address()
            root = agent.get_root()
            signature = root.get_data()

            if agent.get_type()>1 or not signature.is_dict():
                continue

            node = Node(root,address,(),self)

            for a in node.iter_tree():
                tgt = a.id()
                for src in a.get_master():
                    src_server,src_path = paths.breakid_list(src)
                    connections.setdefault(src_server,{}).setdefault(tuple(src_path),set()).add(tgt)

        self.__connections = connections
        return connections


    def get_agent(self,address):
        a = self.__snap.get_agent_address(0,address,False)
        if not a.isvalid():
            a = self.__snap.get_agent_address(1,address,False)
            if not a.isvalid():
                return None

        return a

    def get_agents(self):
        return self.__mapping.keys()

    def get_subsystems(self,agent):
        (id1,id2) = guid.split(agent)
        ss = []

        for a in self.__mapping.keys():
            if a == agent:
                continue

            (aid1,aid2) = guid.split(a)

            if aid1==id1:
                ss.append(a)

        return ss

    def get_root(self,agent):
        a = self.__snap.get_agent_address(0,agent,False)
        if a.isvalid(): return Node(a.get_root(),agent,(),self)
        a = self.__snap.get_agent_address(1,agent,False)
        if a.isvalid(): return Node(a.get_root(),agent,(),self)
        return None

    def delete_agent(self,agent):
        a = self.__snap.get_agent_address(0,agent,False)
        if not a.isvalid():
            a = self.__snap.get_agent_address(1,agent,False)
            if not a.isvalid():
                return

        self.__snap.erase_agent(a)
        self.invalidate_connections()

    def newrversion(self,agent):
        return self.__mapping[agent][1].version

    def newcversion(self,agent):
        return self.__mapping[agent][1].cversion

    def oldrversion(self,agent):
        return self.__mapping[agent][0].as_dict_lookup('version').as_string()

    def oldcversion(self,agent):
        return self.__mapping[agent][0].as_dict_lookup('cversion').as_string()

    def canonical_name(self,agent):
        return self.__mapping[agent][1].name

    def call_phase(self,module,address,phase):
        try:
            upgrader = self.__registry.get_instance(module)
            if upgrader:
                if upgrader(self.oldcversion(address),self.newcversion(address),self,address,phase) == False:
                    return False
            return True
        except:
            traceback.print_exc()
            return False

    def do_upgrade(self):
        for (k,(oldsig,m)) in self.__mapping.iteritems():
            if self.get_root(k):
                if oldsig.as_dict_lookup("subsystem").is_null():
                    if not self.call_phase(m,k,0):
                        print 'upgrade v1 phase 0 failed',k
                        return False

        for (k,(oldsig,m)) in self.__mapping.iteritems():
            if self.get_root(k):
                if oldsig.as_dict_lookup("subsystem").is_null():
                    if not self.call_phase(m,k,1):
                        print 'upgrade v1 phase 1 failed',k
                        return False

        for (k,(oldsig,m)) in self.__mapping.iteritems():
            if self.get_root(k):
                if oldsig.as_dict_lookup("subsystem").is_null():
                    if not self.call_phase(m,k,2):
                        print 'upgrade v1 phase 2 failed',k
                        return False

        for (k,(oldsig,m)) in self.__mapping.iteritems():
            if self.get_root(k):
                if oldsig.as_dict_lookup("subsystem").is_null():
                    if not self.call_phase(m,k,3):
                        print 'upgrade v1 phase 3 failed',k
                        return False

            root = self.get_root(k)
            if root:
                oldsig = piw.dictset(oldsig,'version',piw.makestring(m.version,0))
                oldsig = piw.dictset(oldsig,'cversion',piw.makestring(m.cversion,0))
                oldsig = piw.dictset(oldsig,'plugin',piw.makestring(m.name,0))
                root.set_data(oldsig)

        return True


def do_upgrade(snap):
    tools = UpgradeTools(snap)
    return tools.do_upgrade()
