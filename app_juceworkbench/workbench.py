
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

import workbench_native
import piw
import zlib
import sys
from pi import database,agent,logic,node, async,rpc,index,plumber,resource,paths, help_manager,utils
from pisession import session
#from pibelcanto import lexicon
from app_juceworkbench import upgrade

class EigenOpts:
    def __init__(self):
        self.stdout = False
        self.noauto = False
        self.noredirect = False

class Database(database.SimpleDatabase):

    def __init__(self):
        database.SimpleDatabase.__init__(self)
        self.__props = self.get_propcache('props')
        self.__addedList=[]
        self.__changedDict={}
        self.__agentCount=0
        self.__mainIndex=Index()
        self.__mainIndex.addListener(self)
        self.__listener=None

    def start(self,index_name):
        database.SimpleDatabase.start(self,index_name)
        self.set_monitor_listener(self.value_changed)
        piw.tsd_index(index_name,self.__mainIndex)

    def stop(self):
        self.clear_monitor_listener()
        database.SimpleDatabase.stop(self)
        self.__mainIndex.close_index()

    def value_changed(self,pid,value):
        print 'id',pid,'value changed to',value
        self.__listener.value_changed(pid)

    def object_added(self,proxy):
        database.SimpleDatabase.object_added(self,proxy)
        id=proxy.database_id()
        if 'notagent' in proxy.protocols():
            print id,'is not agent'
        #print "object_added",id,'(',proxy.id(),')'
        s=id.split('#')
        name_part=s[0]
        if not(name_part=="<workbench>" and len(s)>1):
            if not name_part in self.__addedList:
                self.__addedList.append(name_part)

        if not '#' in id:
            self.__agentCount=self.__agentCount+1
            print 'agentCount=',self.__agentCount, id, 'added'
            self.update_progress()

    def update_progress(self):
        target=self.__mainIndex.member_count()
        delta=abs(self.__agentCount-target)
        p=float(target-delta)/float(target)
        #print 'agentCount=',self.__agentCount,self.__mainIndex.member_count(),p
        if self.__listener:
            self.__listener.loaded(p)

    def stop_progress(self):
        if self.__listener:
            self.__listener.loaded(0.999)

    def check_progress(self):
        print 'check_progress', self.__agentCount, self.__mainIndex.member_count()
        if(self.__agentCount==self.__mainIndex.member_count()):
            if self.__listener:
                self.__listener.loaded(0.999)

    def object_removed(self,proxy):
        database.SimpleDatabase.object_removed(self,proxy)
        id=proxy.database_id()

        if self.__listener:
            if not '#' in id:
                print "Top level object removed", id
                self.__listener.agentRemoved(id)
                self.__agentCount=self.__agentCount-1;
                print 'agentCount=',self.__agentCount, id, 'removed'
                self.update_progress()

            else:

                 i=id.rfind('.')
                 if i>-1:
                    pid=id[:i]
                    p=self.find_item(pid)
                    if p:
                        if "create" in p.protocols():
                            self.__listener.instanceRemoved(p.database_id())
#                        else:
#                            self.__listener.portRemoved(p.database_id())
                            

    def object_changed(self,proxy,parts):
        id=proxy.database_id()
        database.SimpleDatabase.object_changed(self,proxy,parts)
        #print "object_changed",id,parts
        name_part=id.split('#')[0]

        if not name_part in self.__changedDict:
            self.__changedDict[name_part]=parts
        else:
            for p in parts:
                self.__changedDict[name_part].add(p)

        if ('name' in parts) or ('ordinal' in parts):
            if self.__listener:
                self.__listener.nameChanged(id)



    def subsys_sync(self,proxy):
        id =proxy.database_id()
        print "subsys_sync",id
        changed_parts=None
        added=False
        if id in self.__changedDict:
            changed_parts=self.__changedDict[id]
            print 'database:subsystem sync',id,'changed parts=',changed_parts

        if id in self.__addedList:
            added=True
            print 'database:subsystem sync',id,'added'

        if added:
            if self.__listener:
                if self.isTopLevel(id):
                    self.__listener.agentAdded(id)
                self.__addedList.remove(id)

        if changed_parts:
            if self.__listener:
                if ('master' in changed_parts) or ('frelation' in changed_parts) or ('relation' in changed_parts):
                    self.__listener.agentChanged(id)
                
                del self.__changedDict[id]

    def addListener(self, listener):
        self.__listener=listener

    def isTopLevel(self,id):
        return 'agent' in self.__props.get_valueset(id)



class WorkbenchStateNode(node.server):
    def __init__(self,**kw):
        node.server.__init__(self,change=lambda d: self.set_data(d),**kw)


class WorkbenchState(node.server):
    preset_chunk_size = 8000

    def chunker(self,s):
        z = zlib.compress(s)
        zl = len(z)
        chunk = 1
        index = 0
        remain = zl
        while remain>0:
            l = min(self.preset_chunk_size,remain)
            yield chunk,piw.makeblob2(z[index:index+l],0)
            chunk += 1
            index += l
            remain -= l

    def __init__(self):
        node.server.__init__(self,creator=self.__create,extension=255)

    def __create(self,k):
        return WorkbenchStateNode()

    def set_state(self,state):
        self.clear()
        for i,c in self.chunker(state):
            self[i] = self.__create(i)
            self[i].set_data(c)

    def get_state(self):
        z = ''.join([ n.get_data().as_blob2() for n in self.itervalues() ])
        return zlib.decompress(z) if z else ''

class Index(piw.index):
    def __init__(self):
        piw.index.__init__(self)

    def index_opened(self):
        print 'Index: index_opened'

    @utils.nothrow
    def index_changed(self):
        print 'Index: index_changed', self.member_count()
        if self.__listener:
            self.__listener.check_progress()

    def index_closed(self):
        print 'Index: index_closed'

    def addListener(self, listener):
        self.__listener=listener


class Agent(agent.Agent):
    def __init__(self,backend):
        self.__backend = backend
        self.__database = backend.database()
        agent.Agent.__init__(self,signature=upgrade.Signature(),names="workbench",volatile=True)
        self.__state = WorkbenchState()
        self.set_private(self.__state)
        self.__current_state = ''

    def rpc_addmon(self,arg):
        print 'start monitoring',arg
        self.__database.start_monitor(arg)

    def rpc_delmon(self,arg):
        print 'stop monitoring',arg
        self.__database.stop_monitor(arg)

    def start(self):
        piw.tsd_server(self.__backend.agent_name,self)
        self.advertise(self.__backend.index_name)

    def set_state(self,state):
        self.__current_state = state
        print 'saving setup'
#        print state
        self.__state.set_state(self.__current_state)

    @async.coroutine('internal error')
    def load_state(self,state,delegate,phase):
        yield node.server.load_state(self,state,delegate,phase)
        self.__current_state = self.__state.get_state()
        self.__backend.state_changed(self.__current_state)

    def rpc_dump(self,arg):
        self.__database.dump_connections()

    def rpc_dumpid(self,arg):
        self.__database.dump_connections(arg)


class Backend0(workbench_native.c2p0):
    def __init__(self):
        workbench_native.c2p0.__init__(self)

    def set_args(self,argv):
        args = argv.split()

        self.opts = EigenOpts()

        if '--stdout' in args:
            args.remove('--stdout')
            self.opts.stdout = True

        if '--noauto' in args:
            args.remove('--noauto')
            self.opts.noauto = True

        if '--noredirect' in args:
            args.remove('--noredirect')
            self.opts.stdout = True
            self.opts.noredirect = True

        self.args = args

        self.stdio=(sys.stdout,sys.stderr)

        if not self.opts.noredirect:
            sys.stdout=session.Logger()
            sys.stderr=sys.stdout

    def get_logfile(self):
        return resource.get_logfile('workbench') if not self.opts.stdout else ''

    def mediator(self):
        return self.token()


class Backend(workbench_native.c2p):
    def __init__(self):
        workbench_native.c2p.__init__(self)
        self.__database = Database()
        self.__agent = Agent(self)
        self.__help_manager=help_manager.HelpManager()

    def stop_progress(self):
        self.__database.stop_progress()

    def set_state(self,state):
        self.__agent.set_state(state)

    def state_changed(self,state):
        self.__frontend.stateChanged(state)

    def get_test_string(self,s):
        test=''
        i=0
        while i<s:
            test=test+'a'
            i=i+1

        print "backend get_test_string",s,test
        self.__frontend.test_string(test);

    def database(self):
        return self.__database

    def initialise(self,frontend,scope):
        self.__frontend = frontend
        self.__scope = piw.tsd_scope()+'.'+scope if scope else piw.tsd_scope()
        self.index_name = '<%s:main>' % self.__scope
        self.agent_name = '<%s:workbench>' % self.__scope
        print >>sys.__stdout__,'workspace scope=',self.__scope,'agent=',self.agent_name,'index=',self.index_name
        self.__database.addListener(self.__frontend)
        self.__database.start(self.index_name)
        self.__agent.start()

    def quit(self):
        self.__agent.close_server()
        self.__database.stop()
        pass

    def get_tooltip(self,agent):
        help=self.__help_manager.find_help([agent])
        if help is not None:
            return help.get_tooltip()
        return ''

    def get_tooltip_by_id(self,id):
        canonical_name = self.__database.find_full_canonical(id,False)
        help=self.__help_manager.find_help(canonical_name)
        if help is not None:
            return help.get_tooltip()
        return '';

    def get_helptext(self,agent):
        help=self.__help_manager.find_help([agent])
        if help is not None:
            return help.get_helptext()
        return ''

    def get_helptext_by_id(self,id):
        canonical_name = self.__database.find_full_canonical(id,False)
        help=self.__help_manager.find_help(canonical_name)
        if help is not None:
            return help.get_helptext()
        return ''


    def get_name(self,id):
        return self.__database.find_desc(id)

    def get_desc(self,id):
        return self.__database.find_desc(id)

    def get_fulldesc(self,id):
        return self.__database.find_full_display_desc(id)

    @async.coroutine('internal error')
    def get_sourcekeys(self, pid, id):
        name=self.get_desc(id);
        if name=="musical map" or name=="physical map":
            r=rpc.invoke_rpc(pid,'fetch_sourcekeys',name); 
            yield r
            if not r.status():
                yield async.Coroutine.failure('fetch_sourcekeys failed') 
            result=r.args()[0]
            if result!='None':
                print "fetch_sourcekeys: result=",result
                self.__frontend.sourcekeys_updated(id,result)
            yield async.Coroutine.success();

    def get_inputs(self,sid,mid):
        #print "get_inputs", sid,mid,self.__database.get_inputs(sid,mid)
        inputs=self.__database.get_inputs(sid,mid);
        for i in inputs:
            if i:
                return str(i)
        return ""
#        return self.__database.get_inputs(sid,mid);

    def get_children(self,id):
        children=self.__database.find_children(id).union(self.__database.find_joined_slaves(id))
        named=set()
        for c in children:
            if (self.isNamed(c) or self.hasNamedDescendants(c)):
                 named.add(c)
        return named

    def get_child_props(self,id,showNames):
        children=self.__database.find_children(id).union(self.__database.find_joined_slaves(id))
        named=set()
        for c in children:
            if (self.isNamed(c) or self.hasNamedDescendants(c)):
                 if(showNames or self.hasEditableValue(c) or self.hasEditableValueDescendants(c)):
                     named.add(c)
        return named
    

    def get_child(self,id,name):
        children=self.__database.find_children(id).union(self.__database.find_joined_slaves(id))
        for c in children:
           if self.get_desc(c)==name:
               return c
        return ''

    def is_input(self,id):
        if(self.child_count(id)==0):
            name=self.get_desc(id)
            return self.has_protocol(id,"input") 
        else:
            for c in self.get_descendants(id):
                if self.get_desc(c)!=0:
                    if (self.has_protocol(c,"input") ):
                        return True
        return False


    def is_output(self,id):
        if(self.child_count(id)==0):
            return (self.has_protocol(id,"output"))
        else:
            for c in self.get_descendants(id):
                if self.get_desc(c)!=0:
                    if (self.has_protocol(c,"output")):
                        return True
        return False

    def child_count(self,id):
        return len(self.get_children(id))

    def isNamed(self,id):
        return self.get_desc(id)!=""
 
    def hasNamedDescendants(self,id):
        for c in self.get_descendants(id):
           if self.isNamed(c):
                 return True
        return False

    def hasEditableValueDescendants(self,id):
        for c in self.get_descendants(id):
           if self.hasEditableValue(c):
                 return True
        return False


    def get_descendants(self,id):
        descendants=self.__database.find_descendants(id)
        js=self.__database.find_joined_slaves(id)
        for c in descendants:
            js=js.union(self.__database.find_joined_slaves(c))

        for s in js:
            jsdescendants=js.union(self.__database.find_descendants(s))
            descendants=descendants.union(jsdescendants) 

        return descendants

    def get_connections(self,id):
        connections=set()
        for d in self.get_descendants(id):

            masters=self.__database.find_masters(d)
            for m in masters:   
                connections.add(m + ":" + d)

        return connections

    def is_slave_of(self, sid, otherid):
        for d in self.get_descendants(sid):

            masters=self.__database.find_masters(d)
            for m in masters:   
                if otherid in m:
                    return True;
        return False;

    def get_numInputs(self,id):
        print "get_numInputs: masters=", self.__database.find_masters(id);
        return len(self.__database.find_masters(id))

    def get_using_inputs(self,id):
        using=set()
#        for d in self.get_descendants(id):

        masters=self.__database.find_masters(id)
        for m in masters:   
            u=self.get_inputs(id,m)

            print "workbench.py get_using_inputs: id=",id, "from=",m,"using",u
            if u!="":
                using.add(u)

        return using

    def get_parent(self,id):
#        proxy=self.__database.find_item(id)
#        if(proxy==None):
#            print 'backend: getparent called for',id , 'but not in database'
          
        parents= self.__database.find_parents(id).union(self.__database.find_joined_master(id))
        return parents

    def make_path(self, spath):
        path=[]
        if spath=='/root':
            return path
        else:
            for p in spath.split('/')[2:]:
                path.append(p.replace('\\','/'))
                
            return path

    @async.coroutine('internal error')
    def activate(self,id,spath,uid):
        print 'activate',id,spath,uid
        path=self.make_path(spath)   
        args=logic.render_term((tuple(path),uid))
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'activated',args)
        yield r
        if not r.status():
            yield async.Coroutine.failure('rpc error')
        print 'activate success'
        self.__frontend.activated(id)
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def enumerate(self,id,spath):
        path=self.make_path(spath)
        a=logic.render_term(tuple(path))
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'enumerate',a)
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        nf,nc=logic.parse_clause(r.args()[0])
        print 'enumerate:path=', path,' nf=',nf, 'nc=',nc
        self.__frontend.enumerate_updated(id,spath,nf,nc);
        yield async.Coroutine.success(nf,nc)

    @async.coroutine('internal error')
    def cinfo(self,id,spath,start,ncolls):
        path=self.make_path(spath)
        a=logic.render_term(tuple(path))
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'enumerate',a)
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        nf,nc=logic.parse_clause(r.args()[0])
        finish=start+ncolls
        if finish>nc:finish=nc

        colls=[]
        while start<finish:
            a=logic.render_term((tuple(path),start))
            r=rpc.invoke_rpc(self.__database.to_usable_id(id),'cinfo',a)
            yield r

            if not r.status():
                yield async.Coroutine.failure('rpc error')
       
            clist=logic.parse_clause(r.args()[0])

            if not clist:
                break

            colls.extend(clist)
            start=start+len(clist)

        colls=colls[:ncolls]
#        print 'cinfo=',colls

        self.__frontend.cinfo_updated(id,spath,set(colls));

        yield async.Coroutine.success(colls,nc)

    @async.coroutine('internal error')
    def finfo(self,id,spath):
        path=self.make_path(spath)
        a=logic.render_term(tuple(path))
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'enumerate',a)
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        nf,nc=logic.parse_clause(r.args()[0])
        start=0
        finish=nf
        current=start

        files=[]
        while current < finish:
            a=logic.render_term((tuple(path),current))
            r=(yield rpc.invoke_rpc(self.__database.to_usable_id(id),'finfo',a))

            if not r.status():
                print 'rpc error, finfo',r.args()
                yield async.Coroutine.failure('rpc error')

            flist = logic.parse_clause(r.args()[0])

            if not flist:
                break

            files.extend(flist)
            current=current+len(flist)

        files=files[:nf]
        s=set([])

        for f in files:
           s.add(str(f[0])+"&&"+str(f[1]))
        self.__frontend.finfo_updated(id,spath,s)
        yield async.Coroutine.success(files,nf)

    @async.coroutine('internal error')
    def current(self,id):
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'current','')
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        c=logic.parse_clause(r.args()[0],paths.make_subst(id))
        print  'r.args()[0]=',r.args()[0]
        curr=''
        if c:
            
            print  'backend current for',id, '=',c
            (curr,cat)=c[0]
            print 'current=',curr, 'current_cat=',cat
        self.__frontend.current(id, str(curr))

        yield async.Coroutine.success(c)
 
    @async.coroutine('internal error')
    def get_agents(self):
        print 'Backend - get_agents'    
        names=set([])
        r=rpc.invoke_rpc(self.__database.to_usable_id('<eigend1>'),'listmodules','')
        yield r
        if not r.status():
            yield async.Coroutine.failure('get_agents failed') 
        result=r.args()[0]
        if result!='None':

    #        print "get_agents: result=",result
            terms=logic.parse_termlist(result)
            for t in terms:
                r=t.args[0]
                r=r+","
                ords=t.args[2]
                for o in ords:
                    r=r+str(o)
                    r=r+','
                names.add(r)
            self.__frontend.agents_updated(names)
        yield async.Coroutine.success(names) 

    @async.coroutine('internal error')
    def get_instanceName(self,id):
        print 'Backend - getInstanceName: id=',id
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'instancename','')
        yield r
        if not r.status():
            yield async.Coroutine.failure('get_instanceName failed')
        result=r.args()[0]
        self.__frontend.instanceName(id,result)
        yield async.Coroutine.success()


    @async.coroutine('internal error')
    def get_instances(self,id):
        print 'Backend - get_instances: id=',id
        ords=set([])
        r=rpc.invoke_rpc(self.__database.to_usable_id(id),'listinstances','')
        yield r
        if not r.status():
            yield async.Coroutine.failure('get_instances failed')
        result=r.args()[0]
        if result!='None':
            ordinals=logic.parse_clauselist(result)
            for o in ordinals:
                ords.add(str(o))
            self.__frontend.instances_updated(ords)
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def get_ordinals_used(self, agentType):
        print 'Backend - get_ordinals_used'    

        r=rpc.invoke_rpc(self.__database.to_usable_id('<eigend1>'),'listmodules','')
        yield r
        if not r.status():
            yield async.Coroutine.failure('get_ordinals_used failed') 
        result=r.args()[0]
        if result!='None':

            terms=logic.parse_termlist(result)
            for t in terms:
                if t.args[0]==agentType:
                    print "ordinals used=",t.args[2]
        yield async.Coroutine.success() 

    def create_instance(self,id,ord):
        print 'backend create instance',id,ord
        rpc.invoke_rpc(self.__database.to_usable_id(id),'createinstance',logic.render_term(ord))

    def delete_instance(self,id,cid):
        print 'backend delete instance',id,cid
        rpc.invoke_rpc(self.__database.to_usable_id(id),'delinstance',cid)

    def delete_agent(self, id):
        print 'backend delete agent',id
        rpc.invoke_rpc(self.__database.to_usable_id('<eigend1>'),'destroy',logic.render_term(id))

    @async.coroutine('internal error')
    def create_agent(self,agent,ordinal):
        print 'backend create agent',agent,ordinal
        plugin_def=logic.make_term('module',agent,ordinal)
        r=rpc.invoke_rpc(self.__database.to_usable_id('<eigend1>'),'addmodule',logic.render_term(plugin_def))

        yield(r)
        if not r.status():

            print 'addmodule failed',agent,ordinal,r.args()[0]

            self.__frontend.report_error('eigenD could not create agent '+ agent + ' ' + str(ordinal),r.args()[0])
            yield async.Coroutine.failure('rpc_addmodule failed') 

        yield async.Coroutine.success() 


    def hasEditableValue(self,id):
        proxy=self.__database.find_item(id)
        if (not proxy==None) and proxy.domain():
            validPropertyDomains=['bool','bfloat','bint','string','enum']
            for d in validPropertyDomains:
                if proxy.domain().canonical().startswith(d):
                    return True
            if self.has_protocol(id,'browse'):
                return True
        return False

    def has_master(self,id):
        m=self.__database.find_masters(id)
        if m:
            return True

        return False;    

    def has_non_controller_master(self,id):
        val=False
        for m in self.__database.find_masters(id):
           print m, m.split('#')[0]

           if (('interpreter'not in m) and (not self.has_protocol(m.split('#')[0],'controller'))):
               val=True

        return val

    def get_domain(self,id):
        proxy=self.__database.find_item(id)
        if (not proxy==None):
            return proxy.domain().canonical()
        return ''

    def get_ordinal(self,id):
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            return proxy.ordinal()
        print 'proxy for',id,'not found'
        return 0

    def get_master_filter(self,id,mid):
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            master=proxy.get_master()
            terms=logic.parse_termlist(master or '')
            for t in terms:
                if logic.is_pred_arity(t,'conn',5,5):
#                    print '******',t.args[3]
                    if t.args[2]==mid:
                        #print 'filter for',id,'=',t.args[3]
                        if (not t.args[3]==None):
                            return t.args[3]
        return ''

    def get_master_control(self,id,mid):
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            master=proxy.get_master()
            terms=logic.parse_termlist(master or '')
            for t in terms:
                if logic.is_pred_arity(t,'conn',5,5):
#                    print '******',t.args[3]
                    if t.args[2]==mid:
                        #print 'ctrl for',id,'=',t.args[4]
                        if (not t.args[4]==None):
                            return t.args[4]
        return ''


    def get_value(self,id):
        val=''
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            val=proxy.get_value()
        return str(val)

    def set_boolvalue(self,id,val):
        print 'backend set_boolvalue',id,val
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            if val:
                proxy.invoke_rpc('set_value',logic.render_term(val))
            else:
                proxy.invoke_rpc('set_value','')
#
    def set_stringvalue(self,id,val):
        print 'backend set_stringvalue',id,val
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            proxy.invoke_rpc('set_value',val)

    def set_intvalue(self,id,val):
        print 'backend set_intvalue',id,val
        self.__set_value(id,val)

    def set_floatvalue(self,id,val):
        print 'backend set_floatvalue',id,val
        self.__set_value(id,val);

    def __set_value(self,id,val):
        proxy=self.__database.find_item(id)
        if(not proxy==None):
            proxy.invoke_rpc('set_value',logic.render_term(val))

    def setName(self,id,name):
        print 'Backend set name',name
        proxy=self.__database.find_item(id)
        if (not proxy==None):
            names=name.split(' ')

            if names[-1].isdigit():
                ord=names[-1]
#                print 'call set_ordinal',ord
                proxy.invoke_rpc('set_ordinal',ord)
                name=' '.join(names[:-1])
            else:
#                print 'call clear_ordinal'
                proxy.invoke_rpc('clear_ordinal','')

#            print 'call set_names',name
            valid=True
            errors=''
            for w in name.split():
                #e=lexicon.lexicon.get(w)
                e=self.__database.get_lexicon().is_valid_belcanto(w)
                if e is None:
                    valid=False
                    errors=errors + ' ' + w

            if valid:
                proxy.invoke_rpc('set_names',name)
            else:
                self.__frontend.report_error('Invalid Belcanto name',errors + ' not in Belcanto lexicon')

    @async.coroutine('internal error')
    def connect_check(self, srcid,dstid):
        r=plumber.plumber(self.__database,(dstid,None),[(srcid,None)],check_only=True)
        yield(r)
        if not r.status():
            print 'connect_check: connection not possible', srcid,dstid
            self.__frontend.connectionPossible(srcid,dstid, False)
            yield async.Coroutine.failure('connection not possible') 
        print 'connect_check: connection is possible',srcid,dstid
        self.__frontend.connectionPossible(srcid,dstid, True)
        yield async.Coroutine.success()


    @async.coroutine('internal error')
    def connect(self, srcid,dstid):
        r=plumber.plumber(self.__database,(dstid,None),[(srcid,None)])
        yield(r)
        if not r.status():
            print 'connect failed', srcid,dstid
            yield async.Coroutine.failure('plumber connect failed') 
        print 'Backend connect',srcid,dstid
        yield async.Coroutine.success()


    @async.coroutine('internal eror')
    def connect_test(self,srcid,dstid,u,f,c):
        print "connect_test",srcid,dstid,u,f,c
        proxy=self.__database.find_item(dstid)
        if proxy:
            if u==0:
                u=None
            if f=='':
                f=None
            if c=='':
                c=None
            r=proxy.invoke_rpc('connect',logic.render_term(logic.make_term('conn',u,None,srcid,f,c)))
            yield(r)
            if not r.status():
                print 'connect failed', srcid,dstid,u,f,c
                self.__frontend.report_error('eigenD could not make connection '+ srcid + ':' + dstid,r.args()[0])
                yield async.Coroutine.failure('rpc_connect failed') 

            print 'Backend connect_test',srcid,dstid,u,f
            yield async.Coroutine.success()
        else:
            print 'proxy not found'

    @async.coroutine('internal eror')
    def disconnect(self,srcid,dstid,u,f,c):
        proxy=self.__database.find_item(dstid)

        if proxy:
            if u==0:
                u=None
            if f=='':
                f=None
            if c=='':
                c=None
            r=proxy.invoke_rpc('disconnect',logic.render_term(logic.make_term('conn',u,None,srcid,f,c)))
            yield(r)
            if not r.status():
                print 'disconnect failed', srcid,dstid,u,f,c
                self.__frontend.report_error('eigenD could not disconnect wire '+ srcid + ':' + dstid,r.args()[0])
                yield async.Coroutine.failure('rpc_disconnect failed') 

            yield async.Coroutine.success()
        else:
            print 'proxy not found'


    def mediator(self):
        return self.token()

    def has_protocol(self, id, prot):
        return prot in self.__database.get_propcache('protocol').get_valueset(id)

    def get_scope(self,id):
        proxy=self.__database.find_item(id)
        if proxy:
            return str(proxy.get_property('rig',''))
        return ''

    def get_absoluteID(self,id):
        return self.__database.to_absolute_id(id)
 
    def is_rig(self,id):
        proxy=self.__database.find_item(id)
        if proxy:
            if proxy.get_property('rig',None) is None:
                return  False
            else:
                return True
        return False

    def has_property(self,id,p):
        proxy=self.__database.find_item(id)
        if proxy:
            if proxy.get_property(p,None) is None:
                return  False
            else:
                return True
        return False

    def get_property(self,id,p):
        proxy=self.__database.find_item(id)
        if proxy:
            prop=proxy.get_property(p,None);
            print "getProperty",prop 
            if prop is None:
                return ''
            else:
                return str(prop);
        return ''

    def invoke(self,id,name,val):
        print "Workbench backend invoke",name,val,"on",id
        proxy=self.__database.find_item(id)
        if proxy:
            print "proxy found"
            proxy.invoke_rpc(name,logic.render_term(val))

    def monitor_on(self,id):
        self.__database.start_monitor(id)

    def monitor_off(self,id):
        self.__database.stop_monitor(id)

def main0():
    return Backend0()

def main():
    return Backend()
