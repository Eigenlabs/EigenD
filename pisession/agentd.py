
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

from pi import atom,agent,action,errors,node,utils,async,index,guid,logic,files,resource,state,rpc,async,timeout,version,container
from pi.logic.shortcuts import *
from pisession import registry,upgrade,upgrade_agentd,session
from pibelcanto import translate

import picross
import piw
import sys
import random
import time
import os
import pisession_native
import re
import glob
import urllib
import binascii
import traceback

blacklisted_versions = ()
first_upgradeable_version = '1.0.0'
rpc_chunksize = 1200

def split_setup(s):
    split1 = s.split('~',1)

    if len(split1) == 1:
        return '',s.strip()

    return split1[1].strip(),split1[0].strip()

def filter_valid_setup(s):
    if (s.startswith('tmpsetup') or '.' in s or s.endswith('~')):
        return False

    s2 = split_setup(s)

    if translate.words_to_notes(s2[1]):
        return True

    return False


def try_int(s):
    try: return int(s)
    except: return s

def natsort_key(s):
    return map(try_int, re.findall(r'(\d+|\D+)', s))

def natcmp(a, b):
    return cmp(natsort_key(a[0]), natsort_key(b[0]))

def probe_alpha():
    f = picross.f_string()
    mm_bases = picross.enumerate(0x2139,0x0104,f)
    em_bases = picross.enumerate(0x2139,0x0002,f)
    legacy_alphas = picross.enumerate(0xbeca,0x0102,f)
    return mm_bases!=0 or em_bases!=0 or legacy_alphas!=0


def get_default_setup():
    filename = resource.user_resource_file('global',resource.default_setup)

    if os.path.exists(filename):
        setup = open(filename,'r').read()
        if ''==setup:
            return ''
        if os.path.exists(setup):
            return setup

    rd = resource.get_release_dir('state')
    rs = [os.path.basename(x) for x in glob.glob(os.path.join(rd,'*'))]
    fs = filter(filter_valid_setup,rs)
    fs.sort(natcmp,reverse=False)

    pico_setup = ''
    alpha_setup = ''

    for s in fs:
        if s.startswith('pico') and not pico_setup: pico_setup=os.path.join(rd,s)
        if s.startswith('alpha') and not alpha_setup: alpha_setup=os.path.join(rd,s)

    if probe_alpha():
        return alpha_setup

    return pico_setup


def set_default_setup(path):
    try:
        def_state_file = resource.user_resource_file('global',resource.default_setup)
        print 'default file:',def_state_file,path
        fd = open(def_state_file,'w').write(path)
        fd.close()
    except:
        pass


def find_setup(srcname):
    if os.path.isabs(srcname):
        return srcname

    user_dir = resource.user_resource_dir(resource.setup_dir)
    release_dir = resource.get_release_dir('state')

    user_setups  = [os.path.basename(x) for x in glob.glob(os.path.join(user_dir,'*'))]
    factory_setups  = [os.path.basename(x) for x in glob.glob(os.path.join(release_dir,'*'))]

    for s in user_setups:
        if s == srcname:
            return os.path.join(user_dir,s)
        s2 = split_setup(s)
        if s2[1] == srcname:
            return os.path.join(user_dir,s)

    for s in factory_setups:
        if s == srcname:
            return os.path.join(factory_setups,s)
        s2 = split_setup(s)
        if s2[1] == srcname:
            return os.path.join(factory_setups,s)

        return None


def user_setup_file(slot,tag):
    if tag:
        # url encode any illegal chars except ' ' to stop them ending up in the filename
        tag = urllib.quote(tag, ' ')
        # must encode . also as urllib doesn't do this
        tag = str.replace(tag, '.', '%'+binascii.b2a_hex('.')) 
        base = '%s ~ %s' % (slot,tag)
    else:
        base = slot
    name = resource.user_resource_file(resource.setup_dir,base)
    return name

def delete_user_slot(slot):
    slot = slot.strip()
    rd = resource.user_resource_dir(resource.setup_dir)
    for (sp,sd,sn) in os.walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = split_setup(s)
            if s3[1]==slot:
                os.unlink(os.path.join(rd,s))


def get_setup_slot(slot):
    slot = slot.strip()
    rd = resource.user_resource_dir(resource.setup_dir)

    for (sp,sd,sn) in os.walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = split_setup(s)
            if s3[1]==slot:
                # unencode url encoded illegal chars to display them properly
                return urllib.unquote(s3[0]) or 'none'

    return ''

def find_user_setups_flat():
    rd = resource.user_resource_dir(resource.setup_dir)

    t = piw.term("tree",0)
    t.add_arg(-1,piw.term(piw.makestring('user setups',0)))

    for (sp,sd,sn) in os.walk(rd):
        for s in filter(filter_valid_setup,sn):
            t3 = piw.term('leaf',0)
            s3 = split_setup(s)
            # unencode url encoded illegal chars to display them properly
            name = piw.term(piw.makestring(urllib.unquote(s3[0]),0)) if s3[0] else piw.term()
            slot = piw.term(piw.makestring(s3[1],0))
            t3.add_arg(-1,name)
            t3.add_arg(-1,slot)
            t3.add_arg(-1,piw.term(piw.makestring(os.path.join(rd,s),0)))
            t3.add_arg(-1,piw.term(piw.makebool(False,0)))
            t3.add_arg(-1,piw.term(piw.makebool(True,0)))
            t.add_arg(-1,t3)

    return t

class Menu:
    def __init__(self,label):
        self.label = label
        self.children = []
        self.children2 = {}
        self.setups = {}
        self.leaf = None

    def number_of_setups(self):
        return len(self.setups);

    def add_setup(self,name,slot,file,upg,user):
        self.setups[slot] = (name,slot,file,upg,user)

    def add_child(self,menu):
        self.children.append(menu)

    def get_submenu(self,n):
        if not n:
            return self

        n0 = n[0]
        n = n[1:]

        if n0 not in self.children2:
            self.children2[n0] = Menu(n0)

        return self.children2[n0].get_submenu(n)

    def term(self):
        names = self.setups.keys()
        names.sort()

        for n in names:
            m = self.get_submenu(n.split())
            m.leaf = self.setups[n]

        children = piw.term(0)

        for c in self.children:
            children.add_arg(-1,c.term())

        k = self.children2.keys()
        k.sort()

        for c in k:
            children.add_arg(-1,self.children2[c].term())

        l = self.leaf

        if l:
            t = piw.term('n',7)
            t.set_arg(0,piw.term(piw.makestring(self.label,0)))
            t.set_arg(1,children)
            t.set_arg(2,piw.term(piw.makestring(l[0],0)))
            t.set_arg(3,piw.term(piw.makestring(l[1],0)))
            t.set_arg(4,piw.term(piw.makestring(l[2],0)))
            t.set_arg(5,piw.term(piw.makebool(l[3],0)))
            t.set_arg(6,piw.term(piw.makebool(l[4],0)))
        else:
            t = piw.term('n',2)
            t.set_arg(0,piw.term(piw.makestring(self.label,0)))
            t.set_arg(1,children)

        return t

def find_user_setups():
    rd = resource.user_resource_dir(resource.setup_dir)
    m = Menu('User Setups')

    for (sp,sd,sn) in os.walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = split_setup(s)
            m.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),False,True)

    return m


def find_factory_setups():
    rd = resource.get_release_dir('state')
    m = Menu('Factory Setups')

    for (sp,sd,sn) in os.walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = split_setup(s)
            m.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),False,False)

    return m


def find_old_setups():
    m = Menu("Previous Versions")

    for v in resource.find_installed_versions(filter_upgradeable_version):
        rd = resource.user_resource_dir(resource.setup_dir,version=v)
        for (sp,sd,sn) in os.walk(rd):
            sd=()
            sv = filter(filter_valid_setup,sn)
            if sv:
                m2 = Menu(v)
                for s in sv:
                    s3 = split_setup(s)
                    m2.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),True,False)
                m.add_child(m2)

    return m


def find_all_setups():
    m = Menu('Setups')
    m.add_child(find_user_setups())
    m.add_child(find_factory_setups())
    m.add_child(find_old_setups())
    return m.term()

def annotate(file,text):
    database = state.open_database(file,True)
    trunk = database.get_trunk()
    upgrade.set_description(trunk,text)
    trunk.save(piw.tsd_time(),'')
    database.flush()


def filter_upgradeable_version(v,t,r):
    if r in blacklisted_versions: return False

    first_v,first_t = resource.split_version(first_upgradeable_version)
    my_v,my_t = resource.split_version(version.version)

    if t and t.startswith('unstable'): return False
    if t and t.startswith('experimental'): return False

    if v<first_v: return False
    if v>=my_v: return False

    return True


def upgradeable_old_setups():
    md = resource.user_resource_dir(resource.setup_dir)

    slots = [ split_setup(os.path.basename(s))[1] for s in glob.glob(os.path.join(md,'*')) ]

    releases={}

    for v in resource.find_installed_versions(filter_upgradeable_version):
        rd = resource.user_resource_dir(resource.setup_dir,version=v)
        (vs,ts) = resource.split_version(v)
        for (sp,sd,sn) in os.walk(rd):
            sd=()
            for s in filter(filter_valid_setup,sn):
                slot = split_setup(s)[1]

                if slot in slots:
                    continue

                vf = os.path.join(rd,s)
                mf = os.path.join(md,s)

                if not s in releases or vs > releases[s][0]:
                    if s not in releases:
                        releases[s] = (vs,vf,v)
                        slots.append(slot)

    mapping = []
    for (n,(vs,vf,v)) in releases.items():
        mapping.append((os.path.join(md,n),vf,v))

    return mapping


def copy_old_setup(src,dst,src_ver):
    def tweaker(snap,src_snap):
        old_desc = upgrade.get_description(src_snap)
        old_version = upgrade.get_version(src_snap)
        if old_desc and not old_desc.endswith('\n'):
            old_desc=old_desc+'\n'
        old_desc = old_desc + 'upgraded from '+old_version+'\n'
        upgrade.set_version(snap,version.version)
        upgrade.set_upgrade(snap,True)
        upgrade.set_description(snap,old_desc)
        snap.save(0,'upgraded to %s' % version.version)

    try:
        if upgrade.upgrade_trunk(src,dst,tweaker=tweaker):
            return
    except:
        traceback.print_exc()

    try: os.unlink(dst)
    except: pass


def merge_snapshot(trunk,snapshot):
    mapping = state.Mapping()
    trunk.copy(snapshot,mapping,True)
    

class Controller(state.Manager):

    def add_sync(self):
        r = async.Deferred()
        if not self.__syncers and self.open():
            self.sync()
        self.__syncers.append(r)
        return r

    def __init__(self, glue, address):
        self.address = address
        self.__glue = glue
        self.__agent = glue.create_sink(address)
        self.__volatile = 1
        self.__syncers = []
        self.__first_sync = True
        self.__saving = False

        sink = self.__agent.get_root()
        state.Manager.__init__(self,sink)

    def enable_save(self,e=True):
        self.__saving = e

    def client_sync(self):
        if self.get_data().is_dict() and not self.get_data().as_dict_lookup('volatile').is_null():
            self.__volatile = 1
        else:
            self.__volatile = 0

        state.Manager.client_sync(self)

        while self.__syncers:
            syncers = self.__syncers
            self.__syncers = []

            for s in syncers:
                s.succeeded()

        if self.__first_sync:
            self.__first_sync = False
            self.__glue.agent_connected(self)

    def client_opened(self):
        state.Manager.client_opened(self)

    def close_client(self):
        if not self.__first_sync:
            self.__glue.agent_disconnected(self)

        # turn off saving, and checkpoint the setup
        # as of right now to avoid tracking the
        # tear down of the agent

        if self.__saving:
            self.__saving = False
            self.__agent.set_checkpoint()
            checkpoint = self.__agent.checkpoint()
            checkpoint.set_type(self.__volatile)
            self.__glue.trunk.set_agent(checkpoint)
            self.__glue.flush()

        state.Manager.close_client(self)

    def manager_checkpoint(self):
        if self.__saving:
            if self.__agent.isdirty():
                self.__agent.set_type(self.__volatile)
                self.__agent.set_checkpoint()
                self.__glue.trunk.set_agent(self.__agent)
                self.__glue.flush()

    @async.coroutine('internal error')
    def reload(self,snap):
        t = time.time()
        yield self.add_sync()

        version=snap.get_checkpoint()

        if not version:
            print self.address,'nothing to load'
            yield async.Coroutine.success([])

        diff = self.get_diff(snap.get_root(),state.Mapping()).render()

        spl = []
        while diff:
            s = diff[:rpc_chunksize]
            diff = diff[len(s):]
            spl.append(s)

        rve = []

        for (i,s) in enumerate(spl):
            r = rpc.invoke_rpc(self.address,'loadstate','%d:%d:%s' % (i,len(spl),s))
            yield r
            if r.status() and len(r.args())>0:
                v = logic.parse_clause(r.args()[0])
                rve.extend(v)

        yield async.Coroutine.success(rve)

class AgentLoader:
    def __init__(self,module):
        self.agent = None
        self.context = None
        self.module = __import__(module,fromlist=['main','unload','isgui'])

    def __run(self,func,*args,**kwds):
        current_context = piw.tsd_snapshot()

        try:
            piw.setenv(self.context.getenv())
            piw.tsd_lock()
            try:
                return func(*args,**kwds)
            finally:
                piw.tsd_unlock()
        finally:
            current_context.install()

    def is_gui(self):
        return self.module.isgui

    def __load(self,name,ordinal):
        self.agent = self.module.main(self.context.getenv(),name,ordinal)

    def __quit(self):
        if self.agent:
            self.module.on_quit(self.agent)

    def __unload(self):
        if self.agent:
            ss = self.module.unload(self.context.getenv(),self.agent)
            self.context.kill()
            self.context = None
            self.agent = None
            return ss

    def load(self,context,name,ordinal):
        if self.context is not None and self.agent is not None:
            self.unload()
        self.context = context
        self.__run(self.__load,name,ordinal)

    def unload(self):
        if self.context is not None and self.agent is not None:
            return self.__run(self.__unload)

    def on_quit(self):
        if self.context is not None and self.agent is not None:
            self.__run(self.__quit)

class AgentFactory:
    def __init__(self,name,version,cversion,zip,module):
        self.zip=zip
        self.name=name
        self.version=version
        self.cversion=cversion
        self.module=module

    def dump(self):
        return self.module

class DynamicPluginList(atom.Atom):
    def __init__(self, agent):
        atom.Atom.__init__(self)
        self.__agent = agent
        self.__meta = container.PersistentMetaData(self,'agents',asserted=self.__asserted,retracted=self.__retracted)

    @staticmethod
    def __relation(address):
        return 'create(cnc("%s"),role(by,[instance(~a)]))' % address

    def check_ordinal(self,name,ordinal):
        found = [False]

        def visitor(v,s):
            if v.args[1]==name and v.args[4]==ordinal:
                found[0] = True

        self.__meta.visit(visitor)
        return found[0]

    def find_all_ordinals(self,name):
        ordinals = []

        def visitor(v,s):
            if v.args[1]==name:
                ordinals.append(v.args[4])

        self.__meta.visit(visitor)
        return ordinals

    def find_new_ordinal(self,name):
        ordinal = [0]

        def visitor(v,s):
            if v.args[1]==name:
                ordinal[0] = max(v.args[4],ordinal[0])

        self.__meta.visit(visitor)
        return ordinal[0]+1

    def __asserted(self,signature):
        (address,plugin,version,cversion,ordinal) = signature.args
        factory = self.__agent.registry.get_compatible_module(plugin,cversion)
        self.add_frelation(self.__relation(address))
        return self.__agent.load_agent(factory,address,ordinal)

    def __retracted(self,signature,plugin):
        self.del_frelation(self.__relation(signature.args[0]))
        return plugin.unload()

    def unload(self,address):
        # unload address and return ss or None
        ss = self.__meta.retract_state(lambda v,s: v.args[0]==address)
        return ss

    def on_quit(self):
        # call on_quit for all plugins
        self.__meta.visit(lambda v,s: s.on_quit())

    def create(self,factory,address,ordinal=0):
        if ordinal:
            if self.check_ordinal(factory.name,ordinal):
                return None
        else:
            ordinal = self.find_new_ordinal(factory.name)

        print 'assigned ordinal',ordinal,'to',factory.name
        signature = logic.make_term('a',address,factory.name,factory.version,factory.cversion,ordinal)
        plugin = self.__meta.assert_state(signature)
        return plugin


class Agent(agent.Agent):
    def __init__(self,ordinal,path,icon=None):
        self.__foreground = piw.tsd_snapshot()
        agent.Agent.__init__(self,signature=upgrade_agentd,names='eigend', protocols='agentfactory setupmanager set', icon = icon, container = 3, ordinal = ordinal)

        self.ordinal = ordinal
        self.uid = '<eigend%d>' % ordinal

        self.dynamic = DynamicPluginList(self)
        self.node = random.randrange(0, 1<<48L) | 0x010000000000L

        self[2] = self.dynamic

        self.registry = registry.Registry()

        for p in path:
            self.registry.scan_path(p,AgentFactory)

        self.registry.dump(lambda m: m.dump())

        constraint = 'or([%s])' % ','.join(['[matches([%s],%s)]' % (m.replace('_',','),m) for m in self.registry.modules()])
        self.add_verb2(1,'create([un],None,role(None,[concrete,issubject(create,[role(by,[cnc(~self)])])]))',callback=self.__destroy)
        self.add_verb2(2,'create([],None,role(None,[abstract,%s]))' % constraint, callback=self.__create)
        self.add_verb2(3,'save([],None,role(None,[abstract]))', self.__saveverb)
        self.add_verb2(4,'load([],None,role(None,[abstract]))', self.__loadverb)
        self.add_verb2(5,'set([],None,role(None,[abstract,matches([startup])]),role(to,[abstract]))', self.__set_startup)

        dbfile = resource.user_resource_file('global',resource.current_setup)
        if os.path.exists(dbfile):
            os.remove(dbfile)

        self.database = state.open_database(dbfile,True)
        self.trunk = self.database.get_trunk()
        upgrade.set_version(self.trunk,version.version)
        self.flush()

        self.index = index.Index(self.factory,False)

        self.__load_queue = []
        self.__load_result = None
        self.__plugin_count = 0
        self.__load_errors = None

    def rpc_listmodules(self,arg):
        modules = []

        for mslug in self.registry.modules():
            mname = ' '.join([s.capitalize() for s in mslug.split('_')])
            mordinals = tuple(self.dynamic.find_all_ordinals(mslug))
            modules.append(logic.make_term('module',mname,'%s Agent' % mname,mordinals))

        return logic.render_termlist(tuple(modules))


    def rpc_addmodule(self,plugin_def):
        plugin_def = logic.parse_term(plugin_def)

        assert(logic.is_pred(plugin_def,'module') and plugin_def.arity>0)

        plugin_name = plugin_def.args[0]
        plugin_slug = '_'.join([w.lower() for w in plugin_name.split()])
        factory = self.registry.get_module(plugin_slug)

        if not factory:
            return async.failure('no such agent')

        if plugin_def.arity>1:
            plugin_ordinal = plugin_def.args[1]
            if self.dynamic.check_ordinal(plugin_slug,plugin_ordinal):
                return async.failure('ordinal in use')
        else:
            plugin_ordinal = 0

        plugin_addr = guid.address(plugin_slug)
        print 'creating',plugin_addr,'as',plugin_slug
        self.dynamic.create(factory,plugin_addr,plugin_ordinal)
        return async.success(plugin_addr)

    def __make_unloader(self,name):
        def u(status):
            self.__plugin_count -= 1
            print 'unloaded',name,'plg count:',self.__plugin_count
        return u

    def create_sink(self,address):
        for i in range(0,self.trunk.agent_count()):
            a = self.trunk.get_agent_index(i)

            if a.get_address() == address:
                return a

        return self.trunk.get_agent_address(0,address,True)


    def load_agent(self,factory,name,ordinal):
        ctx = None
        agent = None

        agent = AgentLoader(factory.module)
        self.__plugin_count += 1
        ctx = self.create_context(name,self.__make_unloader(name),agent.is_gui())
        agent.load(ctx,name,ordinal)

        return agent


    def close_server(self):
        self.index.close_index()
        agent.Agent.close_server(self)

    def __parse_return(self,rv):
        if not rv: return []
        try: return list(logic.parse_clause(rv))
        except: return []

    def rpc_destroy(self,a):
        ss = self.dynamic.unload(a)
        if ss is None:
            return async.failure('no such agent')
        rvt = self.__parse_return(ss)
        rvt.append(a)

        return async.success(logic.render_term(tuple(rvt)))

    def __destroy(self,subject,agents):
        r = []
        for a in action.concrete_objects(agents):
            ss = self.dynamic.unload(a)
            if ss is None:
                r.append(errors.doesnt_exist('agent','un create'))
                continue

            rvt = self.__parse_return(ss)
            r.append(action.removed_return(a,*rvt))

        return async.success(r)

    def server_opened(self):
        agent.Agent.server_opened(self)
        piw.tsd_index('<main>',self.index)
        self.advertise('<main>')

    def add_agent(self,address,plugin_name):
        factory = self.registry.get_module(plugin_name)

        if not factory:
            print 'no factory for',plugin_name
            return False

        self.dynamic.create(factory,address)
        return True

    def rpc_create(self,plugin_def):
        plugin_def = plugin_def.split()

        if len(plugin_def)==2:
            name = guid.toguid(plugin_def[1])
        else:
            name = None

        plugin_sig = plugin_def[0].split(':')

        if len(plugin_sig)==2:
            plugin_cver = plugin_sig[2]
            plugin_name = plugin_sig[0]
            plugin = '_'.join(plugin_name.split())
            factory = self.registry.get_compatible_module(plugin_name,plugin_cver)
        else:
            plugin_name = plugin_sig[0]
            plugin = '_'.join(plugin_name.split())
            factory = self.registry.get_module(plugin_name)

        if not factory:
            return async.failure('no such agent')

        if not name:
            name = guid.address(plugin_name)

        self.dynamic.create(factory,name)
        return async.success(name)

    def __create(self,subject,plugin):
        plugin = action.abstract_string(plugin)
        plugin = '_'.join(plugin.split())
        factory = self.registry.get_module(plugin)

        if not factory:
            return async.failure('no such agent')

        name = guid.address(plugin)
        self.dynamic.create(factory,name)
        return action.created_return(name)


    @async.coroutine('internal error')
    def rpc_get(self,arg):
        yield self.index.sync()
        cp = self.trunk.save(piw.tsd_time(),'')
        yield async.Coroutine.success(str(cp))

    @async.coroutine('internal error')
    def save_file(self,path,desc=''):

        def save_tweaker(snap,src_snap):
            upgrade.set_upgrade(snap,False)
            upgrade.set_description(snap,desc)

        tag = os.path.basename(path)
        yield self.index.sync()

        agents = set(self.all_agents(self.trunk))

        for m in self.index.members():
            ma = m.address
            if ma in agents:
                yield rpc.invoke_rpc(ma,'presave',action.marshal((tag,)))

        yield self.index.sync()

        m = [ c.address for c in self.index.members() ]

        for i in range(0,self.trunk.agent_count()):
            agent = self.trunk.get_agent_index(i)
            address = agent.get_address()

            if address in m or agent.get_type()!=0:
                continue
            
            print 'parking',address
            self.trunk.erase_agent(agent)
            checkpoint = agent.checkpoint()
            checkpoint.set_type(1)
            self.trunk.set_agent(checkpoint)

        self.flush('saved')
        upgrade.copy_snap2file(self.trunk,path,tweaker=save_tweaker)
        self.setups_changed(path)

    def edit_file(self,orig,path,desc=''):
        if orig!=path:
            os.rename(orig,path)

        database = state.open_database(path,True)
        trunk = database.get_trunk()
        upgrade.set_description(trunk,desc)
        trunk.save(piw.tsd_time(),'')
        database.flush()

        self.setups_changed(path)

    def __saveverb(self,subject,tag):
        tag = self.__process_tag(action.abstract_string(tag))
        delete_user_slot(tag)
        filename = user_setup_file(tag,'')
        path=self.save_file(filename)
        return path

    def flush(self,tag=''):
        cp=self.trunk.save(piw.tsd_time(),tag)
        self.database.flush()
        return cp


    def __loadverb(self,subject,t):

        tag = self.__process_tag(action.abstract_string(t))
        path = find_setup(tag)

        if not path:
            print 'no such state',tag
            thing= 'setup %s' %str(tag)
            return async.success(errors.doesnt_exist(thing,'load'))


        def deferred_load():
            self.__thing.cancel_timer_slow()
            self.__thing = None
            snap = self.prepare_file(path,self.uid,version.version)

            if not snap:
                print 'no such state'
                return async.failure('no such state %s' % tag)

            print 'loading from version',snap.version(),'in',tag
            self.setups_changed(path)
            self.__load(snap,tag)

        self.__thing = piw.thing()
        piw.tsd_thing(self.__thing)
        self.__thing.set_slow_timer_handler(utils.notify(deferred_load))
        self.__thing.timer_slow(500)
        return async.success()


    def rpc_load(self,arg):
        snap = self.__findversion(long(arg))
        if not snap:
            return async.failure('no such version %s' % arg)

        agents = snap.agent_count()
        print 'version:',arg,'version=',snap.version(),agents,'agents'
        self.__load(snap,'')


    def rpc_loadfile(self,state_file):
        self.load_file(state_file)


    def prepare_file(self,file,uid,version):
        return self.run_in_gui(upgrade.prepare_file,file,uid,version)

    def load_file(self,file,upgrade_flag = False):

        path = find_setup(file)
        tag = split_setup(os.path.basename(file))[1]

        if not path:
            raise RuntimeError('Cannot locate state file %s' % file)

        self.load_started(tag)
        self.load_status('Preparing',0)
        snap = self.prepare_file(path,self.uid,version.version)
        print 'loading from version',snap.version(),'in',path,'sig',upgrade.get_setup_signature(snap)
        self.setups_changed(path)
        return path,self.__load(snap,tag,upgrade_flag)


    def __set_startup(self,subject,dummy,tag):
        tag = self.__process_tag(action.abstract_string(tag))
        print '__set_startup',tag
        self.set_default_setup(tag)

    def __process_tag(self,tag):
        tag=tag.replace('!','')
        return tag

    def agent_disconnected(self,controller):
        if controller in self.__load_queue:
            self.__load_queue.remove(controller)

    def agent_connected(self,controller):
        if controller not in self.__load_queue:
            self.__load_queue.append(controller)
            if len(self.__load_queue)==1:
                self.__doload()

    def all_agents(self,snap):
        agents = []

        for i in range(0,snap.agent_count()):
            a = snap.get_agent_index(i)
            if a.get_checkpoint():
                agents.append(a.get_address())

        return agents

    def find_agent(self,address):
        for i in range(0,self.trunk.agent_count()):
            a = self.trunk.get_agent_index(i)

            if a.get_address() == address and a.get_checkpoint():
                return a.checkpoint()

        return None

    def __doload(self):
        while self.__load_queue:
            f = self.__load_queue[0]
            s = self.find_agent(f.address)

            if not s:
                print 'skipped',f.address
                self.__load_queue = self.__load_queue[1:]
                f.enable_save()
                continue

            def ok(*args,**kwds):
                if self.__load_queue and self.__load_queue[0]==f:
                    self.__load_queue = self.__load_queue[1:]

                if self.__load_result:
                    self.__load_result(True,n,f.address)

                if self.__load_errors is not None:
                    self.__load_errors.extend(args[0])

                f.enable_save()
                self.__doload()

            def not_ok(*args,**kwds):
                if self.__load_queue and self.__load_queue[0]==f:
                    self.__load_queue = self.__load_queue[1:]

                if self.__load_result:
                    self.__load_result(False,n,f.address)

                f.enable_save()
                self.__doload()

            n = s.get_name()
            r = f.reload(s)
            r.setCallback(ok).setErrback(not_ok)

            if self.__load_result:
                self.__load_result(None,n)

            break

    @async.coroutine('internal error')
    def __load(self,snapshot,label,upgrade_flag = False):
        self.load_started(label)
        self.stop_gc()

        if upgrade.get_upgrade(snapshot):
            upgrade_flag = True

        if upgrade_flag:
            setup_signature = upgrade.get_setup_signature(snapshot)

        agents = set(self.all_agents(snapshot))

        for m in self.index.members():
            ma = m.address
            m.enable_save(False)
            if ma in agents:
                yield rpc.invoke_rpc(ma,'preload','')

        r = self.__load1(snapshot,label)
        yield r
        yield self.index.sync()
        e = r.args()[0]


        if upgrade_flag and r.status():
            self.load_status('Upgrading',100)
            r = rpc.invoke_rpc('<interpreter>','upgrade',setup_signature)
            yield r

        self.load_status('Cleaning up',100)
        yield timeout.Timer(1000)
        self.start_gc()

        if e:
            self.load_complete(e)
        else:
            self.load_complete()

        yield async.Coroutine.completion(r.status(),e)

    def __load1(self,snapshot,label):
        merge_snapshot(self.trunk,snapshot)

        self.flush(label)

        self.__load_queue = []
        self.__load_errors = []
        pending = set()
        parked = set()
        total = 0

        r = async.Deferred()
        r2 = async.Deferred()
        w = timeout.Watchdog(r2,False,'load timeout')

        for i in range(0,self.trunk.agent_count()):
            a = self.trunk.get_agent_index(i)
            addr = a.get_address()
            if a.get_type()==0:
                pending.add(a.get_address())
            if a.get_type()==1:
                parked.add(a.get_address())

        total = len(pending)
        start = time.time()

        def progress(status,n,*args,**kwds):
            if status is not None:
                try: pending.discard(args[0])
                except: pass
                w.enable(60000)
            else:
                w.disable()

            p = len(pending)
            print 'loaded:',total-p,total,'in',time.time()-start,'s',n
            if not p:
                r2.succeeded()
            else:
                self.load_status(n,100*(total-p)/total)

        def watchdog(status,*args,**kwds):
            self.__load_result = None

            if status:
                r.succeeded(self.__load_errors)
            else:
                print 'watchdog fired; load failed'
                print pending
                r.failed(self.__load_errors)

            self.__load_errors = None

        w.setCallback(watchdog,True).setErrback(watchdog,False)
        self.__load_result = progress

        for c in self.index.members():
            if c.address in pending or c.address in parked:
                self.__load_queue.append(c)

        self.__doload()
        return r

    def factory(self, address):
        return Controller(self, address)

    def __findversion(self,ver):
        snap = self.database.get_trunk()

        while True:
            p = snap.previous()
            v = snap.version()
            if v==ver: return snap
            if not p: break
            snap = self.database.get_version(p)

        return None

    def quit(self):
        self.dynamic.on_quit()

def upgrade_default_setup():
    filename = resource.user_resource_file('global',resource.default_setup)

    if not os.path.exists(filename):
        for v in resource.find_installed_versions(filter_upgradeable_version):
            old_filename = resource.user_resource_file('global',resource.default_setup,version=v)
            if os.path.exists(old_filename):
                setup = open(old_filename,'r').read()
                if ''==setup:
                    set_default_setup('')
                else:
                    set_default_setup(setup.replace(v,resource.current_version()))
                return

def upgrade_old_setups():
    if 0 == find_user_setups().number_of_setups():
        for (dst,src,src_ver) in upgradeable_old_setups():
            copy_old_setup(src,dst,src_ver)
