
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

from pi import atom,agent,action,errors,node,utils,async,index,guid,logic,files,resource,state,rpc,async,timeout,version,container,database
from pi.logic.shortcuts import *
from pi.logic.terms import *
from pisession import registry,upgrade,upgrade_agentd,session,workspace
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
import gc

blacklisted_versions = ()
first_upgradeable_version = '2.0.32'

def filter_valid_setup(s):
    if s.startswith('tmpsetup') or '.' in s or '#' in s or s.startswith('~'):
        return False

    s2 = upgrade.split_setup(s)

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

def slotcmp(a, b):
    return cmp(natsort_key(a), natsort_key(b))

class ProbeInstrument:
    def __init__(self):
        self.__mm_bases = None
        self.__em_bases = None
        self.__psus = None
        self.__legacy_alphas = None
        self.__instrument = None

    def has_alpha(self):
        if self.__instrument == 2:
            return False
        return self.__mm_bases != 0 or self.__em_bases or self.__psus != 0 or self.__legacy_alphas != 0

    def has_tau(self):
        return self.__instrument == 2

    def detect(self):
        self.__init__()

        f = picross.make_string_functor(self.__detect_instrument)

        self.__mm_bases = picross.enumerate(0x2139,0x0104,f)
        self.__em_bases = picross.enumerate(0x2139,0x0002,f)
        self.__psus = picross.enumerate(0x2139,0x0003,f)
        self.__legacy_alphas = picross.enumerate(0xbeca,0x0102,picross.f_string())

    def __detect_instrument(self,usbname):
        if not self.__instrument and usbname:
            device = picross.usbdevice(usbname,0)
            instcfg = device.control_in(0x40|0x80,0xc6,0,0,64)
            self.__instrument = ord(instcfg[0])
            device.close()


def get_detected_setup():
    pico_setup = ''
    alpha_setup = ''
    tau_setup = ''

    rd = resource.get_release_dir('state')
    if rd is not None:
        rs = [os.path.basename(x) for x in resource.glob_glob(os.path.join(rd,'*'))]
        fs = filter(filter_valid_setup,rs)
        fs.sort(natcmp,reverse=False)

        for s in fs:
            if s.startswith('pico') and not pico_setup: pico_setup=os.path.join(rd,s)
            if s.startswith('alpha') and not alpha_setup: alpha_setup=os.path.join(rd,s)
            if s.startswith('tau') and not tau_setup: tau_setup=os.path.join(rd,s)

    probe = ProbeInstrument()
    probe.detect()

    if probe.has_alpha():
        return alpha_setup

    if probe.has_tau():
        return tau_setup

    return pico_setup


def get_default_setup():
    filename = resource.user_resource_file(resource.global_dir,resource.default_setup)

    if resource.os_path_exists(filename):
        setup = resource.file_open(filename,'r').read()
        if ''==setup:
            return ''
        if resource.os_path_exists(setup):
            return setup

    return None


def find_setup(srcname):
    if os.path.isabs(srcname):
        return srcname

    user_dir = resource.user_resource_dir(resource.setup_dir)
    release_dir = resource.get_release_dir('state')

    user_setups  = [os.path.basename(x) for x in resource.glob_glob(os.path.join(user_dir,'*'))]
    factory_setups  = [os.path.basename(x) for x in resource.glob_glob(os.path.join(release_dir,'*'))]

    for s in user_setups:
        if s == srcname:
            return os.path.join(user_dir,s)
        s2 = upgrade.split_setup(s)
        if s2[1] == srcname:
            return os.path.join(user_dir,s)

    for s in factory_setups:
        if s == srcname:
            return os.path.join(release_dir,s)
        s2 = upgrade.split_setup(s)
        if s2[1] == srcname:
            return os.path.join(release_dir,s)

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
    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in sn:
            s3 = upgrade.split_setup(s)
            if s3[1] == slot:
                resource.os_unlink(os.path.join(rd,s))


def get_setup_slot(slot):
    slot = slot.strip()
    rd = resource.user_resource_dir(resource.setup_dir)

    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = upgrade.split_setup(s)
            if s3[1]==slot:
                # unencode url encoded illegal chars to display them properly
                return urllib.unquote(s3[0]) or 'none'

    return ''

def find_user_setups_flat():
    rd = resource.user_resource_dir(resource.setup_dir)

    t = piw.term("tree",0)
    t.add_arg(-1,piw.term(piw.makestring('user setups',0)))

    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in filter(filter_valid_setup,sn):
            t3 = piw.term('leaf',0)
            s3 = upgrade.split_setup(s)
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
        names.sort(slotcmp)

        for n in names:
            m = self.get_submenu(n.split())
            m.leaf = self.setups[n]

        children = piw.term(0)

        for c in self.children:
            children.add_arg(-1,c.term())

        k = self.children2.keys()
        k.sort(slotcmp)

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

    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = upgrade.split_setup(s)
            m.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),False,True)

    return m


def find_example_setups():
    rd = resource.get_release_dir('example')
    m = Menu('Example Setups')

    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = upgrade.split_setup(s)
            m.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),False,False)

    return m


def find_experimental_setups():
    rd = resource.get_release_dir('experimental')
    m = Menu('Experimental Setups')

    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = upgrade.split_setup(s)
            m.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),False,False)

    return m


def find_factory_setups():
    rd = resource.get_release_dir('state')
    m = Menu('Factory Setups')

    for (sp,sd,sn) in resource.safe_walk(rd):
        for s in filter(filter_valid_setup,sn):
            s3 = upgrade.split_setup(s)
            m.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),False,False)

    return m


def find_old_setups():
    m = Menu("Previous Versions")

    for v in resource.find_installed_versions(filter_upgradeable_version):
        rd = resource.user_resource_dir(resource.setup_dir,version=v)
        for (sp,sd,sn) in resource.safe_walk(rd):
            sd=()
            sv = filter(filter_valid_setup,sn)
            if sv:
                m2 = Menu(v)
                for s in sv:
                    s3 = upgrade.split_setup(s)
                    m2.add_setup(urllib.unquote(s3[0]),s3[1],os.path.join(rd,s),True,False)
                m.add_child(m2)

    return m


def find_all_setups():
    try:
        m = Menu('Setups')
        m.add_child(find_user_setups())
        m.add_child(find_factory_setups())
        m.add_child(find_example_setups())
        m.add_child(find_experimental_setups())
        m.add_child(find_old_setups())
        return m.term()
    except:
        utils.log_exception()
        return Menu('Setups')

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


def all_setup_files(dst_file,src_file):
    sdir = os.path.dirname(src_file)
    sbase = os.path.basename(src_file)
    ddir = os.path.dirname(dst_file)

    for sn in resource.os_listdir(sdir):
        if sn.startswith(sbase):
            yield os.path.join(ddir,sn),os.path.join(sdir,sn)

def upgradeable_old_setups():
    md = resource.user_resource_dir(resource.setup_dir)

    slots = [ upgrade.split_setup(os.path.basename(s))[1] for s in resource.glob_glob(os.path.join(md,'*')) ]

    releases={}

    for v in resource.find_installed_versions(filter_upgradeable_version):
        rd = resource.user_resource_dir(resource.setup_dir,version=v)
        (vs,ts) = resource.split_version(v)
        for (sp,sd,sn) in resource.safe_walk(rd):
            sd=()
            for s in filter(filter_valid_setup,sn):
                slot = upgrade.split_setup(s)[1]

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


def copy_old_setup(src,dst):
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

    try: resource.os_unlink(dst)
    except: pass


class Agent(agent.Agent):
    def __init__(self,backend,ordinal):
        self.__backend = backend
        self.__registry = workspace.get_registry()

        self.__foreground = piw.tsd_snapshot()

        agent.Agent.__init__(self,signature=upgrade_agentd,names='eigend', protocols='agentfactory setupmanager set', container = 3, ordinal = ordinal, vocab = self.__registry.get_vocab())

        self.ordinal = ordinal
        self.uid = '<eigend%d>' % ordinal

        self.__workspace = workspace.Workspace(piw.tsd_scope(),self.__backend,self.__registry)

        self[2] = self.__workspace

        constraint = 'or([%s])' % ','.join(['[matches([%s],%s)]' % (m.replace('_',','),m) for m in self.__registry.modules()])
        self.add_verb2(1,'create([un],None,role(None,[concrete,issubject(create,[role(by,[instance(~self)])])]))',callback=self.__uncreateverb)
        self.add_verb2(2,'create([],None,role(None,[abstract,%s]))' % constraint, callback=self.__createverb)
        self.add_verb2(3,'save([],None,role(None,[abstract]))', self.__saveverb)
        self.add_verb2(4,'load([],None,role(None,[abstract]))', self.__loadverb)
        self.add_verb2(5,'set([],None,role(None,[abstract,matches([startup])]),role(to,[abstract]))', self.__set_startup)
        self.add_verb2(6,'create([],None,role(None,[abstract,%s]),role(in,[proto(rigouter)]))' % constraint, callback=self.__rigcreateverb)
        self.add_verb2(7,'create([un],None,role(None,[concrete,issubjectextended(create,by,[role(by,[proto(rigouter)])])]))',callback=self.__riguncreateverb)

        piw.tsd_server(self.uid,self)

    def rpc_listmodules(self,arg):
        return self.__workspace.listmodules_rpc(arg)

    def rpc_addmodule(self,arg):
        return self.__workspace.addmodule_rpc(arg)

    def __parse_return(self,rv):
        if not rv: return []
        try: return list(logic.parse_clause(rv))
        except: return []

    def rpc_destroy(self,arg):
        a=logic.parse_clause(arg)
        if not self.__workspace.unload(a,True):
            return async.failure('no such agent')
        return async.success(a)

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.advertise('<main>')

    def rpc_create(self,plugin_def):
        plugin_def = plugin_def.split()

        if len(plugin_def)==2:
            address = guid.toguid(plugin_def[1])
        else:
            address = None

        plugin_sig = plugin_def[0].split(':')

        if len(plugin_sig)==2:
            plugin_cver = plugin_sig[2]
            plugin_name = plugin_sig[0]
            plugin = '_'.join(plugin_name.split())
            factory = self.__registry.get_compatible_module(plugin_name,plugin_cver)
        else:
            plugin_name = plugin_sig[0]
            plugin = '_'.join(plugin_name.split())
            factory = self.__registry.get_module(plugin_name)

        if not factory:
            return async.failure('no such agent')

        class DummyDelegate():
            def __init__(self):
                self.errors = []
            def add_error(self,msg):
                self.errors.append(msg)

        delegate = DummyDelegate()
        address = self.__workspace.create(factory,delegate,address=address)

        if not address:
            return async.failure(','.join(delegate.errors))

        return async.success(address)

    @async.coroutine('internal error')
    def __rigcreateverb(self,subject,plugin,rig):
        rig = action.concrete_object(rig)
        plugin = action.abstract_string(plugin)
        result = rpc.invoke_rpc(rig,"createagent",plugin);
        yield result
        if result.status():
            yield async.Coroutine.success(logic.parse_clause(result.args()[0]))
        yield async.Coroutine.failure(*result.args())


    @async.coroutine('internal error')
    def __riguncreateverb(self,subject,agents):
        results = []

        for a in action.arg_objects(agents):
            print 'a',a
            o = action.crack_composite(a,action.crack_concrete)
            plugin = o[0]
            rig = o[1]
            print 'un creating',plugin,'in',rig
            result = rpc.invoke_rpc(rig,'uncreateagent',logic.render_term((plugin,)))
            yield result
            if result.status():
                results.extend(logic.parse_clause(result.args()[0]))

        yield async.Coroutine.success(results)

    def __createverb(self,subject,plugin):
        plugin = action.abstract_string(plugin)
        plugin = '_'.join(plugin.split())
        factory = self.__registry.get_module(plugin)

        if not factory:
            return async.failure('no such agent')

        class DummyDelegate():
            def __init__(self):
                self.errors = []
            def add_error(self,msg):
                self.errors.append(msg)

        delegate = DummyDelegate()
        address = self.__workspace.create(factory,delegate)

        if not address:
            return [action.error_return(m,plugin,'create',ENG) for m in delegate.errors]

        return action.concrete_return(address)

    def __uncreateverb(self,subject,agents):
        r = []
        for a in action.concrete_objects(agents):
            if not self.__workspace.unload(a,True):
                r.append(errors.doesnt_exist('agent','un create'))

        return async.success(r)

    @async.coroutine('internal error')
    def save_file(self,path,desc=''):
        r = self.__workspace.save_file(path,desc)
        print 'calling workspace save'
        yield r
        print 'called workspace save',r.status()
        if not r.status():
            yield async.Coroutine.failure(*r.args(),**r.kwds())

        self.__backend.setups_changed(path)
        print 'finished agentd save_file'

    @async.coroutine('internal error')
    def __saveverb(self,subject,tag):
        tag = self.__process_tag(action.abstract_string(tag))
        delete_user_slot(tag)
        filename = user_setup_file(tag,'')

        r = self.__workspace.save_file(filename,'')
        yield r
        if not r.status():
            yield async.Coroutine.failure('failed to save setup')

        self.__backend.setups_changed(filename)

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
            self.__backend.setups_changed(path)
            self.__workspace.load_file(path)

        self.__thing = piw.thing()
        piw.tsd_thing(self.__thing)
        self.__thing.set_slow_timer_handler(utils.notify(deferred_load))
        self.__thing.timer_slow(500)
        return async.success()


    def load_file(self,filename,upgrade_flag = False):

        path = find_setup(filename)

        if not path:
            raise RuntimeError('Cannot locate state file %s' % filename)

        self.__backend.setups_changed(path)
        return self.__workspace.load_file(path,upgrade_flag=upgrade_flag)


    def __set_startup(self,subject,dummy,tag):
        tag = self.__process_tag(action.abstract_string(tag))
        print '__set_startup',tag
        self.__backend.set_default_setup(tag)

    def __process_tag(self,tag):
        tag=tag.replace('!','')
        return tag

    def quit(self):
        self.__workspace.on_quit()


def set_default_setup(path):
    try:
        def_state_file = resource.user_resource_file(resource.global_dir,resource.default_setup)
        print 'default file:',def_state_file,path
        fd = resource.file_open(def_state_file,'w').write(path)
        fd.close()
    except:
        pass


def upgrade_default_setup():
    filename = resource.user_resource_file(resource.global_dir,resource.default_setup)

    if not resource.os_path_exists(filename):
        for v in resource.find_installed_versions(filter_upgradeable_version):
            old_filename = resource.user_resource_file(resource.global_dir,resource.default_setup,version=v)
            if resource.os_path_exists(old_filename):
                setup = resource.file_open(old_filename,'r').read()
                if ''==setup:
                    set_default_setup('')
                else:
                    set_default_setup(setup.replace(v,resource.current_version()))
                return

def upgrade_old_setups():
    if 0 == find_user_setups().number_of_setups():
        for (dst,src,src_ver) in upgradeable_old_setups():
            for (dst_file,src_file) in all_setup_files(dst,src):
                copy_old_setup(src_file,dst_file)
