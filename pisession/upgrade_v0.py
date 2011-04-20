
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

import piw
from pi import logic

def get_child(node,child):
    if node.enum_children(child-1)==child:
        return node.get_child(child)
    return None

def get_string(node,child,default=''):
    child_node = get_child(node,child)

    if child_node is not None:
        child_data = child_node.get_data()
        if child_data.is_string():
            return child_data.as_string()

    return default

def get_number(node,child,default=None):
    child_node = get_child(node,child)

    if child_node is not None:
        child_data = child_node.get_data()
        if child_data.is_long():
            return child_data.as_long()

    return default

def erase_children(node):
    child_names = map(ord,node.list_children())
    for c in child_names:
        node.erase_child(c)

def get_tree(node,index=0):
    value = node.get_data()
    child_names = map(ord,node.list_children())
    children = [get_tree(node.get_child(c),c) for c in child_names]
    return (index,value,children)

def set_tree(node,tree):
    (index,value,children) = tree
    node.set_data(value)
    erase_children(node)
    for c in children: set_tree(node.get_child(c[0]),c)

def convert_signature(signature):
    meta = {}

    if signature.endswith('*'):
        signature = signature[:-1]
        meta['volatile'] = True

    signature = signature.split(':')
    meta['plugin'] = signature[0]
    meta['version'] = signature[1]
    meta['cversion'] = '1.0.0'

    if len(signature)==4:
        meta['subsystem'] = signature[3]

    return meta

def convert_metadata(metanode):
    metadata = {}
    children = map(ord,metanode.list_children())

    adjectives = get_string(metanode,4)
    nouns = get_string(metanode,8)
    ordinal = get_number(metanode,7)
    fuzzy = get_string(metanode,12)
    relation = get_string(metanode,11)
    control = get_string(metanode,21)
    notify = get_string(metanode,22)
    master = get_string(metanode,2)
    pronoun = get_string(metanode,15)
    icon = get_string(metanode,16)

    words = adjectives.split()+nouns.split()
    notify = logic.render_termlist(logic.parse_clause(notify or '[]'))

    if words: metadata['name'] = ' '.join(words)
    if ordinal is not None: metadata['ordinal'] = ordinal
    if fuzzy: metadata['fuzzy'] = fuzzy
    if control: metadata['control'] = control
    if notify: metadata['notify'] = notify
    if master: metadata['master'] = master
    if pronoun: metadata['pronoun'] = pronoun
    if icon: metadata['icon'] = icon

    if metanode.enum_children(5)==6:
        private = get_tree(metanode.get_child(6))
        set_tree(metanode,private)
    else:
        metanode.erase()

    return metadata


def convert_list(root):
    children = map(ord,root.list_children())

    if 255 in children:
        children.remove(255)
        convert_list(root.get_child(255))

    for c in children:
        convert_atom(root.get_child(c))
    

def convert_atom(atom,extra={},ext=253):
    metadata = extra.copy()
    children = map(ord,atom.list_children())

    if 254 in children:
        children.remove(254)

    if 255 in children:
        children.remove(255)
        metanode = atom.get_child(255)
        metadata.update(convert_metadata(metanode))

    if 253 in children:
        children.remove(253)
        convert_list(atom.get_child(253))

    data = piw.dictnull(0)
    for (k,v) in metadata.iteritems():
        if isinstance(v,str):
            data = piw.dictset(data,k,piw.makestring(v,0))
        elif isinstance(v,int):
            data = piw.dictset(data,k,piw.makelong(v,0))
        elif isinstance(v,bool):
            data = piw.dictset(data,k,piw.makebool(v,0))

    atom.set_data(data)

    for c in children:
        convert_atom(atom.get_child(c))


def do_upgrade(snap):
    for agent_index in range(0,snap.agent_count()):
        agent = snap.get_agent_index(agent_index)

        if agent.get_type() > 1:
            continue

        address = agent.get_address()
        root = agent.get_root()
        if root.get_data().is_string():
            signature = root.get_data().as_string()
            agent_extra = convert_signature(signature)
            convert_atom(root,agent_extra)
