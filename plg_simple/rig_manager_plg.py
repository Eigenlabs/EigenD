
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

from pi import agent,domain,container,files,action,node,logic,atom,index,guid,rpc,async,state,resource,upgrade,paths,utils
from plg_simple import rig_manager_version as version

import piw
import os
import glob
import time
import sys

def getssid(i,a):
    x=a.split('/')
    ssid=x[0]+'/'+str(i)+x[1]
    if len(ssid)>28:
        ssid=ssid[:27]+'>'
    return ssid

def isrig(id):
    s=id.split('/')
    if len(s)>1:
        if s[1]=='rig>':
            return True
    return False


class Controller(state.Manager):
    def __init__(self,address):
        state.Manager.__init__(self,state.Node())
        self.__anchor = piw.canchor()
        self.__sync = []
        self.__address = address
        self.__anchor.set_client(self)
        self.__anchor.set_address_str(address)

    def client_closed(self):
        state.Manager.client_closed(self)
        #self.__anchor = None

    def client_sync(self):
        state.Manager.client_sync(self)

        sl = self.__sync
        self.__sync = []
        for s in sl: s.succeeded()

    def add_sync(self):
        s = async.Deferred()
        if not self.__sync and self.open():
            self.sync()
        self.__sync.append(s)
        return s

    @async.coroutine('internal error')
    def doload(self,node,map):
        yield self.add_sync()
        diff = self.get_diff(node,map).render()

        spl = []
        while diff:
            s = diff[:10240]
            diff = diff[len(s):]
            spl.append(s)

        for (i,s) in enumerate(spl):
            r = rpc.invoke_rpc(self.__address,'loadstate','%d:%d:%s' % (i,len(spl),s))
            yield r

        yield async.Coroutine.success()

    def address(self):
        return self.__address

class Instance(agent.Agent):
    def __init__(self,manager,address, ordinal):
        agent.Agent.__init__(self,signature=version,subsystem='rig',names='rig agent',container=1,protocols='using',ordinal=ordinal)

        self.set_frelation('create(cnc(~self),role(by,[instance("%s")]))' % manager.id())

        self.add_verb2(1,'add([],None,role(None,[concrete,singular,notproto(is_subsys)]))', self.__add)
        self.add_verb2(2,'add([un],None,role(None,[concrete,singular,notproto(is_subsys)]))', self.__unadd)

        self.__state = container.PersistentFactory(asserted=self.__bit_asserted,retracted=self.__bit_retracted)
        self.__manager = manager
        self.__address = address

        self.set_private(self.__state)

        piw.tsd_server(self.__address,self)
        self.advertise('<main>')

    def iteragents(self):
        return self.__state.iterstate()
    
    def iterall(self):
        for a in self.iteragents():
            yield a

    def rpc_agents(self,arg):
        print '__getagents'
        ag=tuple(self.iteragents())
        if ag:
            return logic.render_term(ag)
        return None

    def __bit_asserted(self,index,id):
        rel = 'join(cnc("%s"),role(to,[instance(~self)]))' % id
        self.add_frelation(rel)
        return id

    def __bit_retracted(self,index,value,id):
        rel = 'join(cnc("%s"),role(to,[instance(~self)]))' % id
        self.del_frelation(rel)

    def __add(self,subject,agent):
        id = action.concrete_object(agent)
        if self.__state.find(lambda v,s: s==id):
            print id,'already in',self.id()
            return
        print 'adding',id, 'to',self.id()
        rpc.invoke_rpc(id,'notify_delete',logic.render_term(self.id()))
        self.__state.assert_state(id)

    def rpc_deleted(self,arg):
        id=logic.parse_clause(arg)
        self.__do_unadd(id)

    def __unadd(self,subject,agent):
        id = action.concrete_object(agent)
        self.__do_unadd(id)

    def __do_unadd(self, id):
        print '__do_unadd',id
        self.__state.retract_state(lambda v,s: s==id)

    @async.coroutine('internal error')
    def __get_rig_agents(self,id):
        all=[]
        rr=rpc.invoke_rpc(id,'agents',self.id())
        yield rr
        if not rr.status():
            yield async.Coroutine.failure('rpc error')
            
        if rr.args()[0]:
            agnts=logic.parse_clause(rr.args()[0])
            all.extend(agnts)
            for aa in agnts:
                if isrig(aa):
                    r=self.__get_rig_agents(aa)
                    yield r
                    if not r.status():
                        yield async.Coroutine.failure('error getting agents in rig')
                    all.extend(r.args()[0])
                                
        yield async.Coroutine.success(all)

    @async.coroutine('internal error')
    def save(self,name):
        dir = resource.user_resource_dir(resource.instrument_dir)
        file = os.path.join(dir,name)

        if os.path.exists(file):
            os.unlink(file)

        agg = async.Aggregate()
        map = state.create_mapping(True)
        ctl = []
        ssctl=[]
        id =self.id()
        all=[id]
        for a in self.iterall():
            all.append(a)

            if isrig(a):
                rr=self.__get_rig_agents(a)
                yield rr
                if not rr.status():
                    yield async.Coroutine.failure('error getting agents in rig')
                rigall=rr.args()[0]
                all.extend(rigall)
        subs={}       

        for a in all:
            r=rpc.invoke_rpc(a,'subsystems',logic.render_term(self.id()))
            yield r
            if not r.status():
                yield async.Coroutine.failure('rpc error')
            if r.args()[0]:
                ss=logic.parse_clause(r.args()[0])
                print 'subsystems of %s' %a,ss
                subs[a]=ss
 
        for i,a in enumerate(all):
            c = Controller(a)
            ctl.append(c)
            agg.add(c,c.add_sync())
            map.add(a,'|rig-%d-rig|' % i)
            print 'adding to map',a,'|rig-%d-rig|' % i
            if a in subs:
                for s in subs[a]:
                    ss=getssid(s,a)
                    c = Controller(ss)
                    #ctl.append(c)
                    agg.add(c,c.add_sync())
                    ssname=self.__getssname(ss)
                    ssctl.append((c,str(i) +'/'+ ssname))
                    map.add(ss,'|rig-%d/%s-rig|' % (i,ssname))
                    print 'adding to map',ss,'|rig-%d/%s-rig|' % (i,ssname)
                    
        agg.enable()

        yield agg

        db = state.open_database(file,True)
        snap = db.get_trunk()

        for i,c in enumerate(ctl):
            a = snap.get_agent_address(0,str(i),True)
            c.save_template(a.get_root(),map)
            a.set_checkpoint()

        for c,name in ssctl:
            a = snap.get_agent_address(0,name,True)
            c.save_template(a.get_root(),map)
            a.set_checkpoint()

        snap.save(0,'')
        db.flush()
        db.close()

        yield async.Coroutine.success(action.nosync_return())

    def __getssname(self,id):
        s=id.split('/')
        s=s[1].split('>')
        return s[0]

class Manager(agent.Agent):
    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='rig manager',container=1,ordinal=ordinal)

        self.__instances = container.PersistentFactory(asserted=self.__inst_asserted,retracted=self.__inst_retracted)
        self.__agentd = node.Server(value=piw.makestring('',0), change=self.__setagentd)
        self.__state = node.Server()
        self.__state[1] = self.__agentd
        self.__state[2] = self.__instances
        self.set_private(self.__state)

        self.add_verb2(1,'create([],None,role(None,[abstract,matches([rig])]))', self.__create_inst)
        self.add_verb2(2,'create([un],None,role(None,[concrete,issubject(create,[role(by,[cnc(~self)])])]))',callback=self.__uncreate_inst)
        self.add_verb2(3,'load([],None,role(None,[abstract]),option(with,[concrete,proto(agentfactory),singular]))', self.__create_tmpl)
        self.add_verb2(4,'use([],None,role(None,[concrete,proto(agentfactory),singular]))', self.__use)
#        self.add_verb2(5,'set([],None,role(None,[abstract,matches([icon])]),role(of,[concrete,singular,issubject(create,[role(by,[cnc(~server)])])]),role(to,[ideal([~server,icon]),singular])   )',self.__do_icon)
        self.add_verb2(6,'save([],None,role(None,[concrete,singular,issubject(create,[role(by,[cnc(~server)])])]),role(as,[abstract]))', self.__save_inst)

        self.__bucket = []

    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)
        print 'resolving',type,arg

        if type=='icon':
            return self[2].resolve_name(' '.join(arg))

        return action.marshal(())

    def __use(self,sub,arg):
        arg = action.concrete_object(arg)
        self.__agentd.set_data(piw.makestring(arg,0))

    def __setagentd(self,value):
        if value.is_string():
            self.__agentd.set_data(value)

    def __save_inst(self,subject,arg,name):
        id = action.concrete_object(arg)
        name = action.abstract_string(name)
        rig = self.__instances.find(lambda v,i: v==id)

        if rig is None:
            return async.failure('not a rig')

        return rig.save(name)

    @async.coroutine('internal error')
    def __uncreate_inst(self,subject,arg):
        id = action.concrete_object(arg)
        agentd = self.__agentd.get_data().as_string()
        rig = self.__instances.find(lambda v,i: v==id)
        rv = [id]

        if rig is None:
            yield async.Coroutine.failure('not a rig')

        f = async.Aggregate()
        for a in rig.iteragents():
            f.add(a,rpc.invoke_rpc(agentd,'destroy',a))
            rv.append(a)

        f.enable()
        yield f

        self.__instances.retract_state(lambda v,i: v==id)
        yield async.Coroutine.success(action.removed_return(*rv))

    def __create_inst(self,subject,arg):
        address = guid.address('rig')
        self.__instances.assert_state(address)
        return action.created_return(address)

    def __inst_asserted(self, index, value):
        return Instance(self,value,index)

    def __inst_retracted(self, index, value, state):
        state.close_server()

    @async.coroutine('internal error')
    def __create_agent(self,signature,agentd):
        r = rpc.invoke_rpc(agentd,'create',signature)
        yield r
        if not r.status():
            yield async.Coroutine.failure(*r.args())

        yield async.Coroutine.success(r.args()[0])

    @async.coroutine('internal error')
    def __create_tmpl(self,subject,name,arg):

        name = action.abstract_string(name)

        agentd = self.__agentd.get_data().as_string()
        if arg is not None:
            agentd = action.concrete_object(arg)

        if not paths.valid_id(agentd):
            yield async.Coroutine.failure('no agent manager specified and no default')

        print 'loading',name,'onto',agentd

        file = resource.find_resource(resource.instrument_dir,name)
        if not file or not os.path.exists(file):
            yield async.Coroutine.failure('no rig %s' % name)

        db = state.open_database(file,False)
        agnts = []
        map = state.create_mapping(True)
        ret = []
#        sscount=0
        ssid=''
        ssaddress=''
        
        for s,n,a in self.__agentlist(db.get_trunk()):
            x = a.get_address()

            if self.__issubsystem(s):
                #xa=getssid(sscount,ssid)  
                #sscount=sscount+1
                xa=self.__getssid(x,ssid)
                ssaddress='/'+(str(x).split('/')[1])
            elif self.__isrig(s):
                xa = guid.address('rig')
                self.__instances.assert_state(xa)
                ssid=xa
                ssaddress=''
#                sscount=0
            else:
                c = self.__create_agent(s,agentd)
                yield c

                if not c.status():
                    yield async.Coroutine.failure(*c.args())

                xa = c.args()[0]
                ssid=xa
                ssaddress=''
#                sscount=0

            map.add('|rig-%s%s-rig|'%(x,ssaddress),xa)
            print 'map','|rig-%s%s-rig|'%(x,ssaddress),xa

            if x == '0':
                ret.append(action.created_return(xa))
            elif not self.__issubsystem(s):
                ret.append(action.initialise_return(xa))
            
            print x,'->',s,xa

            agnts.append((xa,n))

        start = time.time()
        agg = async.Aggregate(progress=lambda agg,a,b: self.__status(start,agg,a,b))

        for i,(a,n) in enumerate(agnts):
            c = Controller(a)
            r = c.doload(n,map)
            agg.add(c,r)

        print 'loading'
        agg.enable()
        self.__bucket.append(agg)
        yield agg
        print 'loaded',agg
        self.__bucket.remove(agg)

        yield async.Coroutine.success(tuple(ret))

    def __getssid(self,address,id):
        a=id.split('/')[0]
        ad=address.split('/')[1]
        ssid=a+'/'+ad+'>'
        print ssid, 'created from', address,id
        return ssid

    def __agentlist(self,snap):
        createlist=[]
        sslist=[]
        tmplist=[]
        ac = snap.agent_count()

        for i in range(0,ac):
            a = snap.get_agent_index(i)
            n = a.get_root()
            s = n.get_data().as_string()

            if self.__issubsystem(s):
                sslist.append((s,n,a))
            else:
                createlist.append((s,n,a))

        for g in createlist:
            tmplist.append(g)
            ca=g[2].get_address()
            for h in sslist:
#                ss1=h[0].split(':')
#                ss2=g[0].split(':')
#                if ss1[0]==ss2[0]:
                if h[2].get_address().split('/')[0]==ca:    
                    tmplist.append(h)
        return tmplist


    def __isrig(self,sig):
        ss = sig.split(':')
        return (ss[0]=='rig_manager' and len(ss)>3 and ss[3]=='rig' and ss[2]==version.cversion)

    def __issubsystem(self,sig):
        ss = sig.split(':')
        return (len(ss)>3 and ss[3]!='rig')

    def __status(self,start,agg,done,total):
        elapsed = time.time()-start

        for o in agg.get_outstanding():
            print ' still to load:',o.address()


class Upgrader(upgrade.Upgrader):

    def __dedup(self,tools,address):
        print 'upgrading',address,'for duplicates'
        root = tools.root(address)

        member_list = root.ensure_node(255,6)
        members = [ n for n in member_list.iter() ]
        check = set()

        for m in members:
            mn = m.get_data().as_string()
            if mn in check:
                print 'erasing dup',mn,m.path
                m.erase()
            else:
                check.add(mn)

        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        print 'upgrade removing rig_manager icon node'
        root = tools.root(address)
        root.remove(2)
        return True

    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)

        rig_list = root.ensure_node(255,6,2)
        rigs = [ n.get_data().as_string() for n in rig_list.iter() ]

        for ss in rigs:
            self.__dedup(tools,ss)

        return True

    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)

        rig_list = root.ensure_node(255,6,2)
        rigs = [ n.get_data().as_string() for n in rig_list.iter() ]

        for ss in rigs:
            print 'upgrading',ss,'for auto'
            ssr = tools.root(ss)
            ssr.ensure_node(255,3).set_string('using')

        return True

    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(1).erase_children()

        rig_list = root.ensure_node(255,6,2)
        rigs = [ n.get_data().as_string() for n in rig_list.iter() ]

        for ss in rigs:
            print 'upgrading',ss
            ssr = tools.root(ss)
            ssr.ensure_node(1).erase_children()

        return True

    def postupgrade(self,tools,agent):
        return self.upgrade_rigs(tools,tools.oldrversion(agent))

    def upgrade_rigs(self,tools,v):
        od = resource.user_resource_dir(resource.instrument_dir,version=v)
        nd = resource.user_resource_dir(resource.instrument_dir)
        for r in os.listdir(od):
            nr = os.path.join(nd,r)
            if not os.path.exists(nr):
                print 'upgrading rig',r,'from',od,'to',nd
                if not tools.upgrade_trunk(os.path.join(od,r),nr):
                    print 'upgrading rig',r,'failed'
                    return False
        return True


agent.main(Manager,Upgrader)

