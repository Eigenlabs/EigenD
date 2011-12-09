
#
# Copyright 2011 Eigenlabs Ltd.  http://www.eigenlabs.com
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


from pi import index,proxy,atom,domain,policy,bundles,async,logic,rpc
import piw

class FinderProxy(proxy.AtomProxy):
    def __init__(self,finder,name):
        self.name = name
        self.__finder = finder
        proxy.AtomProxy.__init__(self)

    def close_client(self):
        proxy.AtomProxy.close_client(self)
        self.__finder.removed(self)

class Finder:
    def __init__(self,name):
        self.__index = index.Index(self.__factory)
        self.__current = None
        piw.tsd_index(name,self.__index)

    def __factory(self,name):
        p = FinderProxy(self,name)
        if self.__current is None:
            self.__current = p
        return p

    def removed(self,p):
        if self.__current is p:
            self.__current is None

    def fetch(self):
        if self.__current:
            return self.__current.name

        m = [ i for i in self.__index.members() ]

        if m:
            self.__current = m
            return m.name

        return None


class TalkerFinder(Finder):
    def __init__(self):
        Finder.__init__(self,'<language>')

class Talker(atom.Atom):
    def __init__(self,finder,trigger,status_cookie,names='action',ordinal=None,protocols=None):
        p = protocols+' nostage hidden-connection' if protocols else 'nostage hidden-connection'
        atom.Atom.__init__(self,domain=domain.String(),policy=atom.default_policy(self.__change),names='action',protocols=p,ordinal=ordinal)
        self.__finder = finder
        self.__domain = piw.clockdomain_ctl()
        self.__domain.set_source(piw.makestring('*',0))
        self.__loading = False

        if status_cookie:
            self.status_input = bundles.VectorInput(status_cookie,self.__domain,signals=(1,))
            self[1] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.status_input.vector_policy(1,False,clocked=False),names='status input',protocols='nostage hidden-connection')
        self[2] = atom.Atom(domain=domain.Aniso(),policy=policy.FastReadOnlyPolicy(),names='trigger output', protocols='hidden-connection')
        self[2].get_policy().set_source(trigger)

        self.__running = []

    def __done(self,r,*a,**kw):
        self.__running.remove(r)
        print 'running:',self.__running

    def __change(self,v):
        if self.__loading:
            self.set_value(v)
            return

        r = self.set_phrase(v)
        self.__running.append(r)
        r.setCallback(self.__done,r).setErrback(self.__done,r)

    def load_state(self,state,delegate,phase):
        self.__loading = True
        atom.Atom.load_state(self,state,delegate,phase)
        self.__loading = False

    @async.coroutine('internal error')
    def builtin_set_value(self,v):
        result = self.set_phrase(v)
        yield result
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def clear_phrase(self):
        interp = self.get_property_string('interpreter')
        actions = self.get_property_string('actions')

        print 'clear phrase',interp,actions

        if actions and interp:
            r = rpc.invoke_rpc(interp,'cancel_action',actions)
            yield r

        self.del_property('interpreter')
        self.del_property('actions')
        self.del_property('help')
        self.set_value('')
        if self.has_key(1):
            self[1].clear_connections()

    def make_connection(self,index,dsc):
        return logic.make_term('conn',index,1,dsc.args[0],dsc.args[1],None)

    def trigger_id(self):
        return self[2].id()

    @async.coroutine('internal error')
    def set_phrase(self,v):
        interp = self.__finder.fetch()

        result = self.clear_phrase()

        yield result

        if not v:
            yield async.Coroutine.success()

        if not interp:
            yield async.Coroutine.failure('no interpreter')

        actions = logic.render_term(logic.make_term('phrase',self[2].id(),v))
        result = rpc.invoke_rpc(interp,'create_action',actions)

        yield result

        if not result.status():
            yield async.Coroutine.failure(*result.args(),**result.kwds())

        actions = result.args()[0]
        self.set_property_string('interpreter',interp)
        self.set_property_string('actions',actions)
        self.set_property_string('help',v)
        self.set_value(v)

        c = []
        for i,a in enumerate(logic.parse_termlist(actions)):
            if logic.is_pred_arity(a,'deferred_action',2):
                if a.args[1]:
                    c.append(self.make_connection(i,a.args[1]))

        if self.has_key(1):
            self[1].set_connections(logic.render_termlist(c))

        yield async.Coroutine.success()

