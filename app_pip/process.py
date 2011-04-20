
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

import parse
import os.path
from error import PipError

#
# In this module we fix up the parse tree.  This includes setting the root
# attribute of each class (the ultimate base class) and copying handlers 
# up the hierarchy from class to derived class.  Handlers are copied because
# the class we derived from the wrapped class does not itself derive from
# the class wrapped around the base class.  Virtal (but not pure) methods
# are copied up so that there is a method to invoke the correct derived
# method;  the method that would be called via python inheritance will be
# incorrect.
#
# Imported modules are read here, and imported classes lifted out.  All 
# classes from the imported module are lifted into our module, and marked
# as imported.
#

def find_spec(path, source_spec, spec):

    file = os.path.normpath(os.path.join(os.path.dirname(source_spec),spec))

    if os.path.exists(file):
        return file

    for dir in path:
        file = os.path.join(dir,spec)
        if os.path.exists(file):
            return file

    raise PipError('cannot find specification file %s' % spec)

def process_inheritance(klasses, name):

    for k in klasses:
        if k['name'] == name:
            if not k.has_key('processed'):
                k['processed']=1

                if 'gc' in k['classflags']:
                    k['isgc']=1

                if not k['base']:
                    k['root']=k['name']
                    return k

                b = process_inheritance(klasses, k['base'])

                if 'isgc' in b:
                    k['isgc']=1

                k['root']=b['root']

                kh={}
                for h in k['handlers']:
                    kh[h['name']]=h

                for h in b['handlers']:
                    if not kh.has_key(h['name']):
                        kh[h['name']]=h

                k['handlers']=kh.values()

                km={}
                for m in k['methods']:
                    km[m['name']]=m

                for m in b['methods']:
                    if m['isvirtual'] and not km.has_key(m['name']):
                        km[m['name']]=m

                k['methods']=km.values()

            return k

    raise PipError('class %s used but not defined' % name)

def process_imports(path,source_spec,tree):
    
    for i in tree['imports']:
        
        included_module = i['name']
        included_spec = i['spec']
        included_classes = i['classes']
        included_tree = process(path,included_module,find_spec(path,source_spec,included_spec))

        list=()

        for k in included_classes:
            list+=(k['name'],)

        for k in included_tree['classes']:
            if k['name'] in list:
                k['imported']=1
                tree['classes']+=(k,)
            

def process(path,module,specfile):

    input = file(specfile,"rU")

    spec=""
    for line in input:
        try: spec += line
        except StopIteration: break

    input.close();

    tree = parse.parse(module,spec)
    process_imports(path,specfile,tree)
    klasses = tree['classes']

    for k in klasses:
        process_inheritance(klasses,k['name'])

    for k in klasses:
        kn='const '+k['name']+'_type_ &'

        for c in k['ctors']:
            ca=c['args']
            if len(ca)==1:
                if ca[0]['rtype']==kn:
                    k['iscopyable']=1

        for h in k['handlers']:
            if h.has_key('ispure'):
                k['isabstract']=1

    return tree
