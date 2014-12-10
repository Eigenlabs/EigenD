#
# Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com
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

from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node,utils,async,resource
from . import clip_manager_version as version

import piw
import conductor_native

class ClipManagerWidget(atom.Atom):
    def __init__(self, manager):
        atom.Atom.__init__(self,domain=domain.String(),names='clip manager',transient=True,policy=atom.default_policy(self.__recv_change),protocols="widget-clipmanager")
        self.__widget = conductor_native.widget(utils.changify(self.__send_change));
        self.set_value("");
        manager.initialise_widget(self.__widget)

    def widget_rpc(self,method,arg):
        result = async.Deferred()

        def completed(rv):
            if not rv.is_string():
                result.failed()
            else:   
                result.succeeded(rv.as_string())

        darg = piw.makestring(arg,0);
        if not self.__widget.invoke_rpc(method,darg,utils.changify(completed)):
            result.failed()

        return result

    def rpc_change_tags_for_clip(self,arg): return self.widget_rpc(2,arg)
    def rpc_get_column_categories(self,arg): return self.widget_rpc(3,arg)
    def rpc_get_all_tags(self,arg): return self.widget_rpc(4,arg)
    def rpc_get_selected_clips(self,arg): return self.widget_rpc(5,arg)
    def rpc_add_to_clip_pool(self,arg): return self.widget_rpc(6,arg)
    def rpc_add_tag(self,arg): return self.widget_rpc(7,arg)
    def rpc_change_tag(self,arg): return self.widget_rpc(8,arg)
    def rpc_remove_tag(self,arg): return self.widget_rpc(9,arg)
    def rpc_add_category(self,arg):return self.widget_rpc(10,arg)

    def __send_change(self,value):
        self.set_value(value.as_string())

    def __recv_change(self,value):
        self.__widget.change_value(piw.makestring(value,0))

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='clip manager', ordinal=ordinal)
        self.clip_manager = conductor_native.clip_manager(resource.user_resource_dir(resource.conductor_dir,version=''))
        self[1] = ClipManagerWidget(self.clip_manager);

agent.main(Agent)

