
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

from pi.logic.shortcuts import *
from pi import action,logic,async,domain,agent,rpc,plumber
from plg_language import interpreter, interpreter_version as version

class Plumber(agent.Agent):

    def __init__(self,master_agent,database,ordinal):
        self.database = database
        self.master_agent = master_agent
        agent.Agent.__init__(self,names='plumber',subsystem='plumber',signature=version,container=1,protocols='is_subsys',ordinal=ordinal)

        self.add_verb2(20,'connect([],None,role(None,[or([concrete],[composite([descriptor])])]),role(to,[concrete,singular]),option(using,[numeric,singular]))',self.verb_20_connect)
        self.add_verb2(18,'connect([un],None,role(None,[concrete]))',self.verb_18_unconnect)
        self.add_verb2(19,'connect([un],None,role(None,[concrete]),role(from,[concrete,singular]))',self.verb_19_unconnect_from)

    @async.coroutine('internal error')
    def __unconnect_from(self,t,tproxy,f):
        objs = self.database.search_any_key('W',T('unconnect_from_list',t,f,V('W')))

        for (s,m) in objs:
            sproxy = self.database.find_item(s)
            sconn = find_conn(sproxy,m)
            for c in sconn:
                print 'disconnect',c,'from',s
                yield interpreter.RpcAdapter(sproxy.invoke_rpc('disconnect',c))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __unconnect(self,t,tproxy):
        print '__unconnect',t
        objs = self.database.search_any_key('W',T('input_list',t,V('W')))
        print '__unconnect',t,objs

        for (s,m) in objs:
            sproxy = self.database.find_item(s)
            yield interpreter.RpcAdapter(sproxy.invoke_rpc('clrconnect',''))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __unconnect_inputlist_from(self,t,tproxy,f):
        print 'deleting',f,'inputs from',t

        inputs = yield interpreter.RpcAdapter(tproxy.invoke_rpc('lstinput',''))
        inputs = action.unmarshal(inputs)
        print 'candidate inputs are:',inputs

        for input in inputs:
            print 'unconnecting',input
            iproxy = self.database.find_item(input)
            objs = self.database.search_any_key('W',T('unconnect_from_list',input,f,V('W')))

            if objs:
                yield self.__unconnect(input,iproxy)
                print 'deleting input',input
                yield interpreter.RpcAdapter(tproxy.invoke_rpc('delinput',input))

        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __unconnect_inputlist(self,t,tproxy):
        print 'deleting all inputs from',t

        inputs = yield interpreter.RpcAdapter(tproxy.invoke_rpc('lstinput',''))
        inputs = action.unmarshal(inputs)
        print 'candidate inputs are:',inputs

        for input in inputs:
            print 'unconnecting',input
            iproxy = self.database.find_item(input)
            yield self.__unconnect(input,iproxy)
            print 'deleting input',input
            yield interpreter.RpcAdapter(tproxy.invoke_rpc('delinput',input))

        yield async.Coroutine.success()

    def verb_18_unconnect(self,subject,t):
        print 'un connect',t
        t = action.concrete_object(t)
        tproxy = self.database.find_item(t)

        if 'inputlist' in tproxy.protocols():
            return self.__unconnect_inputlist(t,tproxy)
        else:
            return self.__unconnect(t,tproxy)

    def verb_19_unconnect_from(self,subject,t,f):
        f = action.concrete_object(f)
        t = action.concrete_object(t)
        print 'un connect',t,'from',f
        tproxy = self.database.find_item(t)

        if 'inputlist' in tproxy.protocols():
            return self.__unconnect_inputlist_from(t,tproxy,f)
        else:
            return self.__unconnect_from(t,tproxy,f)


    @async.coroutine('internal error')
    def verb_20_connect(self,subject,f,t,u):
        if u is not None:
            u=int(action.abstract_string(u))

        to_descriptor = (action.concrete_object(t),None)
        created = []

        for ff in action.arg_objects(f):
            if action.is_concrete(ff):
                from_descriptors = [(action.crack_concrete(ff),None)]
            else:
                from_descriptors = action.crack_composite(ff,action.crack_descriptor)

            r = plumber.plumber(self.database,to_descriptor,from_descriptors,u)
            yield r
            if not r.status():
                yield async.Coroutine.failure(*r.args(),**r.kwds())

            created.extend(r.args()[0])

        yield async.Coroutine.success(action.initialise_return(*created))
        


@async.coroutine('internal error')
def unconnect_set(db,objs):
    cnx=[]

    for o in objs:
        cn = db.get_subsys_masters(o)
        for (ss,cl) in cn.items():
            print o,ss,cl
            if ss not in objs:
                for c in cl:
                    cnx.append(c)

    for (m,s) in cnx:
        print m,'->',s
        sproxy = db.find_item(s)
        sconn = find_conn(sproxy,m)
        for c in sconn:
            print 'disconnect',c,'from',s
            yield interpreter.RpcAdapter(sproxy.invoke_rpc('disconnect',c))
