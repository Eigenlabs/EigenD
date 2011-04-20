
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

from pi import logic,upgrade,guid
from pi import version as pi_version
import piw

version=pi_version.version
cversion='1.0.2'
plugin='agentd'

def upgrade_plugins_v0(tools,address):
    n2 = tools.root(address).get_node(2)
    for n in n2.iter(extension=253):
        n3 = n.get_node(255,6)
        old_term = logic.parse_term(n3.get_string())
        (address,plugin,version,cversion) = old_term.args
        map = tools.mapping.get(address)
        if map:
            (nplugin,nversion,ncversion) = map[1].split(':')
            new_term = logic.make_term(old_term.pred,address,plugin,nversion,ncversion)
            n3.set_string(str(new_term))

    return True

def upgrade_plugins_v1(tools,address):
    n2 = tools.get_root(address).get_node(2)
    old_plugins = logic.parse_termlist(n2.get_data().as_dict_lookup('agents').as_string())
    new_plugins = []

    for old_term in old_plugins:
        (address,plugin,version,cversion,ordinal) = old_term.args
        ncversion = tools.newcversion(address)
        nversion = tools.newrversion(address)
        nplugin = tools.canonical_name(address)
        new_term = logic.make_term(old_term.pred,address,nplugin,nversion,ncversion,ordinal)
        new_plugins.append(new_term)

    n2.set_data(piw.dictset(piw.dictnull(0),'agents',piw.makestring(logic.render_termlist(new_plugins),0)))

class Upgrader(upgrade.Upgrader):
    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        n = root.get_node(2,253)
        if n:
            root.get_node(2,253).erase()
            root.ensure_node(2,253,1).copy(n)
        return True

    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)
        n = root.get_node(1)
        if n is not None:
            n.erase()
        return True

    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        plugins = []
        n2 = tools.get_root(address).get_node(2)
        for n in n2.iter(extension=253,exclude=[254,255]):
            n3 = n.get_node(255)
            old_term = logic.parse_term(n3.get_data().as_string())
            (address,plugin,version,cversion) = old_term.args
            ncversion = tools.newcversion(address)
            nversion = tools.newrversion(address)
            nplugin = tools.canonical_name(address)
            new_term = logic.make_term('a',address,nplugin,nversion,ncversion)
            plugins.append(new_term)

        n2.erase_children()
        n2.set_data(piw.dictset(piw.dictnull(0),'agents',piw.makestring(logic.render_termlist(plugins),0)))

    def upgrade_1_0_1_to_1_0_2(self,tools,address):
        # assign canonical ordinals to agents

        n2 = tools.get_root(address).get_node(2)
        old_plugins = logic.parse_termlist(n2.get_data().as_dict_lookup('agents').as_string())
        plugs_by_type = {}

        for old_term in old_plugins:
            (paddress,plugin,pversion,pcversion) = old_term.args
            pmeta = tools.get_root(paddress).get_data()
            pname = pmeta.as_dict_lookup('name')
            pordinal = pmeta.as_dict_lookup('ordinal')

            if not pname.is_string() or not pname.as_string():
                pname = plugin
            else:
                pname = pname.as_string()

            pname = pname.split()
            try: pname.remove('agent')
            except: pass
            pname = '_'.join(pname)

            if not pordinal.is_long() or not pordinal.as_long():
                pordinal = 0
            else:
                pordinal = pordinal.as_long()

            plugs_by_type.setdefault(plugin,[]).append([paddress,plugin,pversion,pcversion,pname,pordinal,0])

        plugins = []

        for (ptype,plist) in plugs_by_type.iteritems():
            ords_used = set()
            max_ord = 0

            for p in plist:
                if ptype == p[4] and p[5] and p[5] not in ords_used:
                    p[6] = p[5]
                    ords_used.add(p[5])
                    max_ord = max(max_ord,p[5])

                
            for p in plist:
                if not p[6]:
                    max_ord += 1
                    p[6] = max_ord

                new_term = logic.make_term('a',p[0],p[1],p[2],p[3],p[6])
                plugins.append(new_term)

        n2.set_data(piw.dictset(piw.dictnull(0),'agents',piw.makestring(logic.render_termlist(plugins),0)))

    def postupgrade(self,tools,address):
        print 'eigend postupgrade'

        if tools.major_version() != 1:
            return

        n2 = tools.get_root(address).get_node(2)
        old_plugins = logic.parse_termlist(n2.get_data().as_dict_lookup('agents').as_string())
        good_plugins = set()
        good_plugins.add(guid.split(address)[0])

        for old_term in old_plugins:
            good_address = old_term.args[0]
            good_plugins.add(guid.split(good_address)[0])

        for actual_address in tools.get_agents():
            actual_guid,guid_tail = guid.split(actual_address)
            if actual_guid not in good_plugins and guid_tail != 'rig':
                a = tools.get_agent(actual_address)
                if a and a.get_type() == 0:
                    tools.delete_agent(actual_address)
                    print 'pruned bad agent',actual_address

        return upgrade_plugins_v1(tools,address)


def upgrade(oldv,newv,tools,address,phase):
    print 'upgrade',address,oldv,newv,phase
    return Upgrader().upgrade(oldv,newv,tools,address,phase)
