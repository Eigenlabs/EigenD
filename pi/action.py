
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

import random
import piw
import traceback
from pi import logic, utils, vocab, async, rpc
from pi.logic.shortcuts import *

def abstract_wordlist(arg):
    """ 
    Extract all the words of an abstract argument 
    as a list.  Should be used with the abstract
    singular constraint
    """

    return arg[0].args[0]
    return ()

def abstract_string(arg):
    """ 
    Extract all the words of an abstract argument 
    as a single string.  Should be used with the
    abstract singular constraint
    """
    return ' '.join(abstract_wordlist(arg))

def concrete_objects(arg):
    """ 
    Extract all the objects of a concrete argument 
    as a list.  With the singular constraint
    the single object will be returned as a list
    """
    return [o.args[0] for o in arg if logic.is_pred_arity(o,'cnc',1) ]

def concrete_object(arg):
    """ 
    Extract the first object of a concrete argument 
    as a list.  Should be used with the concrete
    singular constraint.
    """
    return concrete_objects(arg)[0]

def arg_objects(arg,filter=None):
    return map(filter,arg) if filter else arg

def is_ideal(obj):
    return logic.is_pred_arity(obj,'ideal',2)

def is_descriptor(obj):
    return logic.is_pred_arity(obj,'dsc',2)

def is_virtual(obj):
    return logic.is_pred_arity(obj,'virtual',1)

def is_concrete(obj):
    return logic.is_pred_arity(obj,'cnc',1)

def is_composite(obj):
    return logic.is_pred_arity(obj,'cmp',1)

def crack_descriptor(obj):
    assert is_descriptor(obj)
    return obj.args

def crack_virtual(obj):
    assert is_virtual(obj)
    return obj.args[0]

def crack_concrete(obj):
    assert is_concrete(obj)
    return obj.args[0]

def crack_composite(obj,filter=lambda x:x):
    assert is_composite(obj)
    return map(filter, obj.args[0])

def crack_ideal(obj):
    assert is_ideal(obj)
    return obj.args

def timespec_map(arg):
    import sys
    l=arg[0].args[0]
    print l
    m={}
    while l:
        clk=l[1]
        val=float(l[0])
        quan=l[2]
        if not quan: val=val-1
        m[clk]=(quan,val)
        l=l[3:]
    return m

def initialise_return(*ids):
    return T('initialise',tuple([T('cnc',n) for n in ids]))

def removed_return(*ids):
    return T('removed',tuple([T('cnc',n) for n in ids]))

def created_return(*ids):
    return T('created',tuple([T('cnc',n) for n in ids]))

def concrete_return(*ids):
    return T('concrete',tuple([T('cnc',n) for n in ids]))

def immediate_return(*words):
    return T('immediate',T('abstract',words))

def cancel_return(mode_id,mode_index,event_id):
    return T('cancel',mode_id,mode_index,event_id)

def error_return(msg,p1,verb,lang=''):
    return T('err',msg,p1,verb,lang)

#def failure_return(msg,p1,verb,lang=''):
#    return T('failure',msg,p1,verb,lang)

def message_return(msg):
    return T('msg',msg)

def nosync_return():
    return T('nosync')

def mass_quantity(arg):
    return float(arg[0].args[0][0])

def mass_unit(arg):
    return ' '.join(str(a) for a in arg[0].args[0][1:])

def mode_event(arg):
    return arg

def unmarshal(clause):
    return logic.parse_clause(clause)

def marshal(args):
    return logic.render_term(args)

def check_verb_schema(schema):
    clause = logic.parse_term(schema, dict(server='0',self='1',parent='2',grandparent='3',a='0',s='1',p='1',pp='1'))
    roles = [ a.args[0] for a in clause.args[2:] ]
    roles = [ r for r in roles if r != 'none' ]
    mod = clause.args[0]
    subject = clause.args[1]
    vocab.check_word('verb',clause.pred)
    vocab.check_words('role',roles,True)
    vocab.check_words('modifier',mod)

def check_mode_schema(schema):
    clause = logic.parse_term(schema, dict(server='0',self='1',parent='2',grandparent='3',a='0',s='1',p='1',pp='1'))
    roles = [ a.args[0] for a in clause.args[1:] ]
    roles = [ r for r in roles if r != 'none' ]
    vocab.check_words('role',roles)
