
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

from pi import const, node, domain,errors, utils, policy, vocab, async, logic, files, action, rpc, paths
from pi import domain as pidomain

import piw
import time
import os.path
import weakref
import copy

def default_policy(change):
    return policy.SlowPolicy(change)

def load_policy(change):
    return policy.LoadPolicy(change)

def null_policy():
    return policy.NullPolicy()

def readonly_policy():
    return policy.ReadOnlyPolicy()

def null_domain():
    return domain.Null()

standard_veto = set(['latency','frelation','modes','verbs','domain','ideals','protocols','timestamp','cname','cordinal'])

class ModeEntry:
    pass

class VerbEntry:
    pass

class Atom(node.Server):

    def __init__(self,init=None,domain=None,policy=None,transient=False,rtransient=False,names=None,protocols=None,ordinal=None,fuzzy=None,creator=None,wrecker=None,pronoun=None,icon=None,container=None,ideals=None,bignode=False,dynlist=False):

        self.__listeners = []

        ex = const.ext_node if bignode else None
        node.Server.__init__(self,creator=creator,wrecker=wrecker,rtransient=rtransient,extension=ex,dynlist=dynlist)
        node.Server.set_data(self,piw.dictnull(0))

        p = policy

        if p is not None:
            dom = domain
            if init is None:
                value = dom.default() if dom is not None else None
            else:
                value = init
        else:
            if not domain:
                p = null_policy()
                dom = null_domain()
            else:
                p = readonly_policy()
                dom = domain

            if init is None:
                value = dom.default()
            else:
                value = init

        self.__policy = p(self,dom,value,transient)

        self.set_readwrite()

        fullname = names or ''

        if fullname:
            self.set_property_string('cname',fullname,notify=False)

        if pronoun is not None:
            self.set_property_string('pronoun',' '.join(namelist),notify=False)

        if fuzzy is not None:
            self.set_property_string('fuzzy',fuzzy,notify=False)

        if ordinal is not None:
            self.set_property_long('cordinal',ordinal,notify=False)

        if icon is not None:
            ideal = files.get_ideal(logic.make_expansion('~self'),'pkg_res:%s'%icon,files.PkgResourceFile(icon),True)
            self.set_property_string('icon',ideal,notify=False)

        if ideals is not None:
            self.set_property_string('ideals',ideals,notify=False)

        user_p = protocols or ''
        policy_p = self.__policy.protocols

        if policy_p:
            if user_p:
                if fullname:
                    fullname = fullname+' '+user_p
                else:
                    fullname = user_p

            if 'input' not in fullname and 'output' not in fullname:
                user_p = user_p+' '+policy_p
                if 'explicit' not in fullname:
                    user_p = user_p+' explicit'
                    
            if 'nostage' in policy_p and 'nostage' not in user_p:
                user_p = user_p+' nostage'

        if user_p:
            self.set_property_string('protocols',user_p,notify=False)

        self.set_internal(const.data_node,self.__policy.data_node())

        self.__container = None

        if container:
            if isinstance(container,tuple):
                (n,l,c) = container
            else:
                (n,l,c) = (container,'agent',VerbContainer())

            c.add_atom(l,self.__create_action,self.__destroy_action,self.__isclocked,self.__status_action)

            self.__container = c
            self.__label = l
            self.__verblist = {}
            self.__modelist = {}

            if n is not None:
                self[n] = c

    def server_change(self,new_value):
        node.Server.server_change(self,new_value)

        if new_value.is_dict():
            old_value = node.Server.get_data(self)
            old_keys = set(utils.dict_keys(old_value))
            new_keys = set(utils.dict_keys(new_value))

            keys_del = old_keys.difference(new_keys)
            keys_create = new_keys.difference(old_keys)
            keys_common = old_keys.intersection(new_keys)

            for k in keys_del:
                self.set_property(k,None,allow_veto=True)

            for k in keys_create:
                v = new_value.as_dict_lookup(k)
                self.set_property(k,v,allow_veto=True)

            for k in keys_common:
                v = new_value.as_dict_lookup(k)
                if old_value.as_dict_lookup(k) != v:
                    self.set_property(k,v,allow_veto=True)

    def set_property_long(self,key,value,allow_veto=False,notify=True):
        self.set_property(key,piw.makelong(value,0),allow_veto,notify)

    def get_property_long(self,key,default = 0):
        v = self.get_property(key,None)
        return v.as_long() if v and v.is_long() else default

    def set_property_string(self,key,value,allow_veto=False,notify=True):
        self.set_property(key,piw.makestring(value,0),allow_veto,notify)

    def get_property_string(self,key,default = ''):
        v = self.get_property(key,None)
        return v.as_string() if v and v.is_string() else default

    def set_property_termlist(self,key,value,allow_veto=False,notify=True):
        self.set_property(key,piw.makestring(logic.render_termlist(value),0),allow_veto,notify)

    def get_property_termlist(self,key,default = []):
        v = self.get_property(key,None)
        if v and v.is_string(): return logic.parse_clauselist(v.as_string(),nosubst=True)
        return copy.copy(default)

    def add_property_termlist(self,key,term,allow_veto=False,notify=True):
        term = logic.parse_clause(term,nosubst=True)
        l = self.get_property_termlist(key)
        if term not in l: l.append(term)
        self.set_property_termlist(key,l,allow_veto,notify)

    def del_property_termlist(self,key,term,allow_veto=False,notify=True):
        term = logic.parse_clause(term,nosubst=True)
        l = self.get_property_termlist(key)
        if term in l:
            l.remove(term)
        self.set_property_termlist(key,l,allow_veto,notify)

    def del_property(self,key,allow_veto=False,notify=True):
        self.set_property(key,None,allow_veto,notify)

    def set_property(self,key,value,allow_veto=False,notify=True):
        if key == 'ordinal':
            if value == self.get_property('cordinal'):
                value = None

        if key == 'name':
            if value == self.get_property('cname'):
                value = None

        if allow_veto:
            if self.property_veto(key,value):
                return

            for lr in self.__listeners[:]:
                l = lr
                if l is not None:
                    if l(True,key,value):
                        return

        old_value = node.Server.get_data(self)

        if value is None:
            d = piw.dictdel(old_value,key)
        else:
            d = piw.dictset(old_value,key,value)

        node.Server.set_data(self,d)

        if notify:
            self.property_change(key,value)
            for lr in self.__listeners[:]:
                l = lr
                if l is not None:
                    l(False,key,value)

    def get_property(self,key,default=None):
        v = node.Server.get_data(self).as_dict_lookup(key)

        if not v.is_null():
            return v

        if key == 'ordinal':
            return self.get_property('cordinal',default)

        if key == 'name':
            return self.get_property('cname',default)

        return default

    def add_listener(self,listener):
        self.del_listener(listener)
        self.__listeners.append(listener)

    def del_listener(self,listener):
        for l in self.__listeners:
            if l == listener:
                self.__listeners.remove(l)
                break

    def property_change(self,key,value):
        pass

    def property_veto(self,key,value):
        return key in standard_veto

    def verb_container(self):
        return self.__container

    def builtin_up(self):
        self.change_value(self.get_domain().up(self.get_value()))

    def builtin_down(self):
        self.change_value(self.get_domain().down(self.get_value()))

    def builtin_up_by(self,inc):
        self.change_value(self.get_domain().up_by(self.get_value(),inc))

    def builtin_down_by(self,inc):
        self.change_value(self.get_domain().down_by(self.get_value(),inc))

    def builtin_toggle(self):
        v = self.get_value()
        self.builtin_set_value(not v)

    def builtin_set(self):
        self.builtin_set_value(True)

    def builtin_unset(self):
        self.builtin_set_value(False)

    def builtin_set_value(self,value):
        dom = self.get_domain()
        try:
            value = dom.data2value(dom.value2data(value))
        except:
            if dom.numeric:
                range='%s to %s' % (str(dom.min), str(dom.max))
                return (errors.out_of_range(range,'set'),)
            return (errors.invalid_value(value,'set'),)
        self.change_value(value)

    def add_mode2(self,label,schema,icallback,qcallback,ccallback,acallback=None):
        #action.check_mode_schema(schema)

        assert label not in self.__modelist

        s = ModeEntry()
        s.schema = 'v(%d,%s)' % (label,schema)
        s.icallback = utils.weaken(icallback)
        s.ccallback = utils.weaken(ccallback)
        s.qcallback = utils.weaken(qcallback)
        s.acallback = utils.weaken(acallback)

        self.__modelist[label]=s
        self.set_property_string('modes',','.join([s.schema for s in self.__modelist.itervalues()]))

    def add_verb2(self,label,schema,callback=None,create_action=None,destroy_action=None,clock=False,status_action=None):
        #action.check_verb_schema(schema)

        assert label not in self.__verblist

        s = VerbEntry()
        s.schema = 'v(%d,%s)' % (label,schema)
        s.callback = utils.weaken(callback)
        s.clock = clock
        s.create_action = utils.weaken(create_action)
        s.destroy_action = utils.weaken(destroy_action)
        s.status_action = utils.weaken(status_action)

        self.__verblist[label]=s
        self.set_property_string('verbs',','.join([s.schema for s in self.__verblist.itervalues()]))

    def rpc_vfind(self,arg):
        args = action.unmarshal(arg)
        index = int(args[0])
        args = args[1:]
        events = self.__container.verbcontainer_find(self.__label,index,args)
        return async.success(action.marshal(events))

    def rpc_vdefer(self,arg):
        args = action.unmarshal(arg)
        index = int(args[0])
        trigger = args[1]
        subject = args[2]
        args = args[3:]

        (id,status) = self.verb_defer(index,None,trigger,subject,*args)

        if id is None:
            return async.failure('deferred function setup failed')
        else:
            return async.success(action.marshal((id,status)))

    def verb_defer(self,index,ctx,trigger,subject,*args):
        print 'deferring',self.__label,index,args
        args = ((subject or self.id()),) + args
        return self.__container.verbcontainer_defer(self.__label,index,ctx,trigger,*args)

    def rpc_vcancel(self,arg):
        args = action.unmarshal(arg)
        index = args[0]
        id = args[1]

        if self.verb_cancel(id):
            return async.success(action.marshal(id))
        else:
            return async.success(action.marshal(None))

    def verb_cancel(self,id):
        return self.__container.verbcontainer_cancel(id)

    def verb_events(self,filt=None):
        for (l,i,id,ctx,data) in self.__container.events():
            if filt==None or filt(l,i):
                yield (id,ctx,data)

    def __fastinvoke(self,server,args):
        f = server.create_action(None,*args)

        if f is None:
            return async.failure()

        ff = piw.fastchange(utils.changify_nb(f[0]))
        t = piw.tsd_time()
        ff(piw.makefloat_bounded(1,0,0,1,t))
        ff(piw.makefloat_bounded(1,0,0,0,t+1))

        ret = (action.nosync_return(),)
        return async.success(action.marshal(ret))

    def __slowinvoke(self,server,interp,args):
        result = server.callback(*args)

        if not isinstance(result,async.Deferred):
            v = result if result else ()
            if not isinstance(v,tuple):
                if isinstance(v,list):
                    v = tuple(v)
                else:
                    v=(v,)
            return async.success(action.marshal(v))

        deferred = async.Deferred()

        def success(value = None):
            v = value if value else ()
            if not isinstance(v,tuple):
                if isinstance(v,list):
                    v=tuple(v)
                else:
                    v=(v,)
            deferred.succeeded(action.marshal(v))

        result.setCallback(success).setErrback(deferred.failed)
        return deferred
        
    def rpc_vinvoke(self,arg):
        print 'verb:',arg
        args = action.unmarshal(arg)
        interp = args[0]
        index = int(args[1])
        args = args[2:]
        server = self.__verblist[index]

        if server.callback:
            return self.__slowinvoke(server,interp,args)
        else:
            return self.__fastinvoke(server,args)

    def rpc_mattach(self,arg):
        args = action.unmarshal(arg)
        print 'attaching',args
        index = args[0]
        id = args[1]
        args = args[2:]
        server = self.__modelist[index]

        if server.acallback:
            if not server.acallback(id,*args):
                return async.success(action.marshal(None))

        return async.success(action.marshal(id))

    def rpc_mcancel(self,arg):
        args = action.unmarshal(arg)
        print 'canceling',args
        index = args[0]
        id = args[1]
        server = self.__modelist[index]

        if server.ccallback(id):
            return async.success(action.marshal(id))

        return async.success(action.marshal(None))

    def rpc_mfind(self,arg):
        args = action.unmarshal(arg)
        index = int(args[0])
        args = args[1:]
        server = self.__modelist[index]

        result = tuple(server.qcallback(*args))
        return async.success(action.marshal(result))

    def rpc_minvoke(self,arg):
        args = action.unmarshal(arg)
        index = int(args[0])
        args = args[1:]
        server = self.__modelist[index]

        result = server.icallback(*args)

        if not isinstance(result,async.Deferred):
            return async.success(action.marshal(result))

        deferred = async.Deferred()

        def success(value):
            deferred.succeeded(action.marshal(value))

        result.setCallback(success).setErrback(deferred.failed)
        return deferred

    def __status_action(self,index,ctx,*arg):
        s = self.__verblist[index]

        if not s.status_action:
            return None

        return s.status_action(*arg)

    def __create_action(self,index,ctx,*arg):
        s = self.__verblist[index]

        if s.create_action:
            return s.create_action(ctx,*arg)

        def func(value):
            if not value.is_null() and value.as_norm()!=0:
                s.callback(*arg)

        downshifter = piw.slowchange(utils.changify(func))
        return (downshifter,downshifter)

    def __isclocked(self,index):
        s = self.__verblist[index]
        return s.clock

    def __destroy_action(self,index,ctx,data):
        s = self.__verblist[index]

        if s.destroy_action:
            s.destroy_action(ctx,data)
            return

    def set_icon(self,icon):
        self.set_property_string(icon)

    def rpc_set_value(self,arg):
        return self.builtin_set_value(arg)

    def rpc_set_icon(self,arg):
        self.set_icon(arg)
    
    def rpc_notify_delete(self,arg):
        self.add_property_termlist('notify',arg)

    def close_server(self):
        if False:
            if self.open():
                id=self.id()
                for agent in self.get_property_termlist('notify'):
                    rpc.invoke_rpc(agent,'deleted',logic.render_term(id))

        self.__policy.close()
        node.Server.close_server(self)

    def notify_destroy(self):
        for slave in self.get_property_termlist('slave'):
            rpc.invoke_rpc(slave,'source_gone',self.id())

        for (k,v) in self.iteritems():
            v.notify_destroy()

    def rpc_download(self,arg):
        (cookie,start,length) = logic.parse_clause(arg)

        start=int(start)
        length=int(length)

        cache = getattr(self,'download_cache',None)
        if cache is None:
            cache = {}
            setattr(self,'download_cache',cache)

        file = cache.get(cookie)

        if file is None:
            file = self.resolve_file_cookie(cookie)

            if file is None:
                return async.failure('bad cookie %s' % cookie)

            cache[cookie] = file

        rsp = file.data(start,length)

        if start+len(rsp) >= file.size():
            del cache[cookie]
            if not cache:
                delattr(self,'download_cache')
        
        return rsp

    def resolve_file_cookie(self,cookie):
        print 'resolve_file_cookie',cookie
        if cookie.startswith('pkg_res:'):
            return files.PkgResourceFile(cookie[8:])
        elif cookie.startswith('file_sys:'):
            return files.FileSystemFile(cookie[9:],os.path.basename(cookie[9:]))
        else:
            return None

    def data_node(self):
        return self.__policy.data_node()

    def isinternal(self,k):
        if node.Server.isinternal(self,k): return True
        return k==const.meta_node or k==const.data_node

    def set_frelation(self,rel):
        self.set_property_string('frelation',rel)

    def add_frelation(self,rel):
        self.add_property_termlist('frelation',rel)

    def del_frelation(self,rel):
        self.del_property_termlist('frelation',rel)

    def clear_frelation(self):
        self.del_property('frelation')

    def set_relation(self,rel):
        self.set_property_string('relation',rel)

    def add_relation(self,rel):
        self.add_property_termlist('relation',rel)

    def del_relation(self,rel):
        self.del_property_termlist('relation',rel)

    def clear_relation(self):
        self.clr_property_termlist('relation')

    def rpc_add_relation(self,rel):
        self.add_relation(rel)

    def rpc_remove_relation(self,rel):
        self.del_relation(rel)

    def rpc_clear_relation(self,rel):
        self.clear_relation(rel)

    def rpc_set_relation(self,value):
        self.set_relation(value)

    def rpc_clear_ordinal(self,value):
        self.clear_ordinal()

    def rpc_set_ordinal(self,value):
        self.set_ordinal(int(value))

    def rpc_set_names(self,value):
        self.set_names(value)

    def set_names(self,value):
        self.set_property_string('name',value,allow_veto=True)

    def clear_ordinal(self):
        self.set_ordinal(0)

    def set_ordinal(self,ordinal):
        self.set_property_long('ordinal',ordinal,allow_veto=True)

    def set_latency(self,l):
        self.set_property_long('latency',l,allow_veto=True)

    def set_private(self,private_node):
        if private_node is None:
            self.del_internal(const.meta_node)
        else:
            self.set_internal(const.meta_node,private_node)

    def get_private(self):
        return self.get_internal(const.meta_node)

    def get_data(self):
        return self.__policy.get_data()

    def change_value(self,v,t=0,p=True):
        self.__policy.change_value(v,t,p)

    def set_value(self,v,t=0):
        self.__policy.set_value(v,t)

    def get_value(self):
        return self.__policy.get_value()

    def get_description(self):
        o = self.get_property_long('ordinal',None)
        n = self.get_property_string('name',None)
        if not n: return ''
        if not o: return n
        return "%s %d" % (n,o)

    def get_domain(self):
        return domain.traits(self.get_property_string('domain'))

    def add_connection(self,src):
        old = self.get_property_termlist('master')
        self.add_property_termlist('master',src)
        self.__update_listeners(old)

    def set_connections(self,srcs):
        old = self.get_property_termlist('master')
        self.set_property_string('master',srcs)
        self.__update_listeners(old)

    def clear_connections(self):
        old = self.get_property_termlist('master')
        self.del_property('master')
        self.__update_listeners(old)

    def remove_connection(self,src):
        old = self.get_property_termlist('master')
        self.del_property_termlist('master',src)
        self.__update_listeners(old)

    def rpc_connected(self,arg):
        self.add_property_termlist('slave',logic.render_term(arg))

    def rpc_disconnected(self,arg):
        self.del_property_termlist('slave',logic.render_term(arg))

    def __update_listeners(self,old):
        masterids = set([x.args[2] for x in self.get_property_termlist('master')])
        previous = set([x.args[2] for x in old])
        dead_listeners = previous.difference(masterids)
        new_listeners = masterids.difference(previous)
        print 'adding new listeners',new_listeners
        print 'removing old listeners',dead_listeners

        for id in dead_listeners:
            myrid = paths.to_relative(self.id(),scope=paths.id2scope(id))
            rpc.invoke_async_rpc(id,'disconnected',myrid)

        for id in new_listeners:
            myrid = paths.to_relative(self.id(),scope=paths.id2scope(id))
            rpc.invoke_async_rpc(id,'connected',myrid)

    def is_connected(self):
        return not not self.get_property_termlist('master')

    def rpc_connect(self,value):
        self.add_connection(value)

    def rpc_clrconnect(self,value):
        self.clear_connections()

    def rpc_disconnect(self,value):
        self.remove_connection(value)

    def rpc_source_gone(self,master):
        old = self.get_property_termlist('master')
        l = self.get_property_termlist('master')
        for x in l:
            if x.args[2] == master:
                l.remove(x)
        self.set_property_termlist('master',l)
        self.__update_listeners(old)

    def get_rpc(self,name):
        return getattr(self,'rpc_'+name,None)

    def get_policy(self):
        return self.__policy

class Null(Atom):
    def __init__(self,**kwd):
        Atom.__init__(self,**kwd)

class BoundedInt(Atom):
    """
    A bounded integer.  Constructed with a min, max, and initial values
    To respond to changes, supply a change callable.
    Notification will only be made if the value is in bounds
    """
    def __init__(self,min,max,init,**kwd):
        Atom.__init__(self,init=init,domain=domain.BoundedInt(min,max),**kwd)

class BoundedFloat(Atom):
    """
    A bounded float.  Constructed with a min, max, and initial values
    To respond to changes, supply a change callable.
    Notification will only be made if the value is in bounds
    """
    def __init__(self,min,max,init,**kwd):
        Atom.__init__(self,init=init,domain=domain.BoundedFloat(min,max),**kwd)

class String(Atom):
    """
    A string.  Constructed with a initial value
    To respond to changes, supply a change callable.
    """
    def __init__(self,init,**kwd):
        Atom.__init__(self,init=init,domain=domain.String(),**kwd)

class Bool(Atom):
    """
    A boolean.  Constructed with a initial value
    To respond to changes, supply a change callable.
    """
    def __init__(self,init,**kwd):
        Atom.__init__(self,init=init,domain=domain.Bool(),**kwd)

class FastEvent(Bool):
    def __init__(self,container):
        self.__func = piw.indirectchange()
        Bool.__init__(self,False,transient=True,protocols='deferred',policy=policy.FastPolicy(self.__func,policy.TriggerStreamPolicy(),clock=False))

        self.__private = node.Server(value=piw.makestring('',0), change=self.__change)
        self.set_private(self.__private)
        self.container=container
        self.index = 0
        self.label = None
        self.ctx = None
        self.args = None
        self.status = None

    def rpc_cancel(self,arg):
        self.clear()

        for (i,e) in self.container.iteritems():
            if e is self:
                def call(): del self.container[i]
                self.__deleter = async.deferred_call(call)
                break

        return async.success(action.marshal(None))

    def compare(self,label,index,args):
        if self.args is None:
            return False

        if self.label != label:
            return False

        if self.index != index:
            return False

        for(q,a) in zip(args,self.args):
            if q is not None and q!=a:
                return False

        return True

    @utils.nothrow
    def __change(self,arg):
        if arg.is_string() and arg.as_string():
            (label,index,ctx,args) = action.unmarshal(arg.as_string())
            self.setup(label,index,ctx,args)

    @utils.nothrow
    def setup(self,label,index,ctx,args):
        self.clear()

        act = self.container.create_action(label,index,ctx,*args)
        clk = self.container.isclocked(label,index)
        self.get_policy().set_clocked(clk)

        if clk:
            self.container.clock.add_upstream(self.get_policy().get_clock())

        if not act:
            return False

        (func,data) = act

        self.status = self.container.status_action(label,index,ctx,*args)
        self.index = index
        self.label = label
        self.data = data
        self.ctx = ctx
        self.args = args

        x = action.marshal((label,index,ctx,args))

        piw.indirectchange_set(self.__func,func)
        self.__private.set_data(piw.makestring(x,0))

        return True

    def clear(self):
        if self.index>0:

            data = self.data
            ctx = self.ctx
            self.data = None
            self.ctx = None
            args = self.args
            piw.indirectchange_clear(self.__func)

            if self.container.destroy_action:
                self.container.destroy_action(self.label,self.index,ctx,data)

            if hasattr(self.container,'clock'):
                self.container.clock.remove_upstream(self.get_policy().get_clock())

            self.index = 0
            self.label = None

class VerbContainer(Atom):

    def __init__(self,clock_domain=None):
        self.__atomlist = {}
        self.__upstream = None

        if clock_domain is not None:
            self.clock = piw.clocksink()
            clock_domain.sink(self.clock,'VerbContainer')

        Atom.__init__(self,protocols='verb',creator=self.__create,wrecker = self.__wreck, bignode=True)

    def load_state(self,state,delegate,phase):
        if phase == 1:
            delegate.set_deferred(self,state)
            return

        Atom.load_state(self,state,delegate,phase-1)

    def add_atom(self,label,c,d,k,s):
        self.__atomlist[label] = (utils.weaken(c),utils.weaken(d),utils.weaken(k),utils.weaken(s))

    def remove_atom(self,label):
        del self.__atomlist[label]

    def __create(self,i):
        return FastEvent(self)

    def __wreck(self,i,e):
        e.clear()

    def events(self):
        for e in self.values():
            if e.ctx:
                yield (e.label,e.index,e.id(),e.ctx,e.data)

    def status_action(self,label,index,ctx,*arg):
        (c,d,k,s) = self.__atomlist[label]
        return s(index,ctx,*arg)

    def create_action(self,label,index,ctx,*arg):
        (c,d,k,s) = self.__atomlist[label]
        return c(index,ctx,*arg)

    def destroy_action(self,label,index,ctx,data):
        (c,d,k,s) = self.__atomlist[label]
        d(index,ctx,data)

    def verbcontainer_defer(self,label,index,ctx,trigger,*args):
        event = FastEvent(self)
        
        if not event.setup(label,index,ctx,args):
            return None

        self.add(event)

        if trigger:
            c = logic.render_term(logic.make_term('conn',None,None,trigger,None,None))
            event.add_connection(c)

        return (event.id(),event.status)

    def verbcontainer_cancel(self,id):
        for (i,e) in self.iteritems():
            if e.id()==id:
                e.clear()
                del self[i]
                return True

        return False

    def verbcontainer_find(self,label,index,args):
        events = tuple( e.id() for e in self.itervalues() if e.compare(label,index,args) )
        return events

    def isclocked(self,label,index):
        (c,d,k,s) = self.__atomlist[label]
        return k(index)

