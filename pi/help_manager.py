
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

from pi import resource,utils,version
from xml.etree import cElementTree
import picross
import os
import urllib

doc_base = 'http://www.eigenlabs.com/wiki/doc'

class DocSection:
    def __init__(self,label,agent):
        self.label = label
        self.tooltip = ''
        self.helptext = ''
        self.children = {}
        self.agent = agent or self

    def get_tooltip(self):
        return self.tooltip

    def get_helptext(self):
        return self.helptext

def indent(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i


def split_ord(name):
    try:
        name_split = name.split(' ')
        ordinal = int(name_split[-1])
        return ' '.join(name_split[:-1]),ordinal
    except:
        pass

    return name,None


def get_node(node_list,node_name):
    doc_node = node_list.get(node_name)

    if doc_node:
        return doc_node

    node_type,ordinal = split_ord(node_name)

    if ordinal:
        doc_node = node_list.get(node_type)

    return doc_node


def get_port(doc_node,port_path):
    if not port_path:
        return doc_node

    child_node = get_node(doc_node.children,port_path[0])
    if not child_node:
        return None

    return get_port(child_node,port_path[1:])


class HelpManager:
    def __init__(self):
        self.load_documentation()

    def update(self):

        try:
            doc_file = resource.user_resource_file(resource.help_dir,'documentation.xml')
            ((major,minor,build),tag) = resource.split_version(version.version)
            doc_url = "%s/%d.%d" % (doc_base,major,minor)
            print 'loading documentation from',doc_url
            doc_conn = urllib.urlopen(doc_url)
            doc_text = doc_conn.read()
            doc_conn.close()
            doc_out = open(resource.WC(doc_file),'w')
            doc_out.write(doc_text)
            doc_out.close()
            print 'loaded documentation'
        except:
            print 'failed to update documentation'
            utils.log_exception()
            return

        self.load_documentation()

    def load_documentation(self):
        self.__agents = {}
        self.__cache = {}

        user_doc = resource.find_resource(resource.help_dir,'documentation.xml')
        release_doc = resource.find_release_resource(resource.help_dir,'documentation.xml')

        try:
            self.load_file(resource.WC(user_doc))
            return
        except:
            print user_doc,'invalid'
            pass

        self.load_file(release_doc)

    def load_file(self,path):
        print 'loading',path
        node = cElementTree.parse(path)

        assert node.getroot().tag == 'documentation'

        for a in node.findall('agent'):
            d = self.load_node(a,None)
            self.__agents[d.label] = d

    def load_node(self,element,agent):
        label = element.get('name').strip()
        docnode = DocSection(label,agent)

        docnode.helptext = element.findtext('help')
        docnode.tooltip = element.findtext('tip')

        for c in element.findall('port'):
            d = self.load_node(c,docnode.agent)
            docnode.children[d.label] = d

        return docnode

    def find_agent_help(self,agent,canonical_path):
        if not canonical_path:
            return agent

        cache_key = (agent.label,tuple(canonical_path))
        cache_ent = self.__cache.get(cache_key)

        if cache_ent:
            return cache_ent

        normalised_path = [ c.lower() for c in canonical_path ]
        doc_node = get_port(agent,normalised_path)
        self.__cache[cache_key] = doc_node
        return doc_node

    def find_help(self,canonical_path):
        if not canonical_path:
            return None

        canonical_agent = canonical_path[0].lower()

        agent = get_node(self.__agents,canonical_agent)

        if not agent:
            return None

        return self.find_agent_help(agent,canonical_path[1:])

