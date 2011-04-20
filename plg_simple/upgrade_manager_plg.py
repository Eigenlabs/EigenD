
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

from pi import agent,atom,action,logic,async,node,resource,rpc,paths
from plg_simple import upgrade_manager_version as version
import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='upgrade manager',container=1,ordinal=ordinal)

        self.__interp = node.Server(value=piw.makestring('',0), change=self.__setinterp)
        self.set_private(self.__interp)

        self.add_verb2(1,'upgrade([],None,role(None,[concrete,singular]),role(with,[abstract]))', self.__upgrade_sampler)
        self.add_verb2(2,'use([],None,role(None,[concrete,proto(langagent),singular]))', self.__use)

    @async.coroutine('internal error')
    def __runner(self,script,interp):
        print 'running script on',interp
        yield rpc.invoke_rpc(interp,'script',script)
        print 'ran script on',interp
        yield async.Coroutine.success()

    def __upgrade_sampler(self,subject,target,script):
        target = action.concrete_object(target)
        script = action.abstract_string(script)

        script = script.lower().replace(' ','_')
        filename = resource.find_resource('upgrade_manager','script_%s' % script)

        if not filename:
            return async.failure('upgrade script %s not defined' % script)

        interp = self.__interp.get_data().as_string()

        if not paths.valid_id(interp):
            return async.failure('no agent manager specified and no default')

        text = file(filename,'r').read().replace('%target%',target)

        print 'upgrade on',target,'with',script,'using',interp
        return self.__runner(text,interp)

    def __use(self,sub,arg):
        arg = action.concrete_object(arg)
        self.__interp.set_data(piw.makestring(arg,0))

    def __setinterp(self,value):
        if value.is_string():
            self.__interp.set_data(value)

agent.main(Agent)
