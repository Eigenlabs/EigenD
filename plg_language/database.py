
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

from pi import atom,database,utils,logic,container,node,action,const,async,rpc,paths
from pibelcanto import lexicon
from plg_language import noun,verb,macro,imperative

class Database(database.Database):

    def __init__(self):
        self.__primitives = {}

        database.Database.__init__(self)

        self.add_module(noun)
        self.add_module(verb)
        self.add_module(imperative)

        self.add_lexicon(lexicon.lexicon)

        # widget manager for updating widget names if they change
        self.__widget_manager = None

    def lookup_primitive(self,klass):
        return self.__primitives.get(klass)

    def add_module(self,module):
        logic.Engine.add_module(self,module)
        self.add_primitives(module)

    def add_primitives(self, module):
        a = dict([ (k[10:],getattr(module,k)) for k in dir(module) if k.startswith('primitive_') ])
        self.__primitives.update(a)

    def find_all_descendants(self, ids):
        s = frozenset()
        for id in ids:
            d = self.find_descendants(id)
            if len(d)==0:
                s = s.union(frozenset([id]))
            else:
                s = s.union(self.find_all_descendants(d))
        return s

    def object_changed(self,proxy,parts):
        # on agent or atom name change, updates the osc widgets and Stage tabs
        id=proxy.id()
        
        if 'name' in parts or 'ordinal' in parts:
            # build osc name
            name_str = '_'.join(proxy.names())
            ordinal = proxy.ordinal()
            if ordinal!=0:
                name_str += '_'+str(ordinal)
            
            # id change set is this object, plus any children

            # add subsystems that are not agents to prevent changes to rigs
            # from including the agents they contain
            agents = self.__propcache.get_idset('agent')
            changed_nodes = set(self.find_joined_slaves(id)).difference(agents)
            changed_nodes.add(id)
            changed_nodes_frozenset = self.find_all_descendants(frozenset(changed_nodes))
            
            #for changed_node in changed_nodes_frozenset:
            #    print changed_node, self.find_full_desc(changed_node)
            
            if self.__widget_manager is not None:
                self.__widget_manager.check_widget_name_updates(changed_nodes_frozenset)
            
       
    def set_widget_manager(self, widget_manager):
        self.__widget_manager = widget_manager


    def build_osc_name(self, id):
        # like find_full_desc but builds full osc path style name
        aid = paths.id2server(id)
        if aid==id:
            adesc = self.find_desc(id) or id
            sdesc = '-'
            pdesc = '-'
        else:
            adesc = self.find_desc(aid) or aid
            sdesc = '-'
            pdesc = self.find_desc(id) or id

        if 'agent' not in self.__propcache.get_valueset(aid):
            mid = self.find_joined_master(aid)
            if mid:
                sdesc = adesc
                adesc = self.build_osc_name(set(mid).pop())
            head = ''
        else:
            head = '/'

        return head+'/'.join((adesc,sdesc,pdesc)).replace('  ','_').replace(' ','_').replace('-/','')

        
