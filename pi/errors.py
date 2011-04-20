
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

from pi import action
ENG=1

def render_message(db,args):
    s=args[0].split()

    c=1
    msg=[]
    if args[-1]==ENG:
        msg.append('*')
    for w in s:
        if w.startswith('%'):
            if w[1:]=='id':
                name=db.find_desc(args[c])
                for n in name.split():
                    msg.append(n)       
            if w[1:]=='s':
                msg.append(args[c]) 
            c=c+1 
        else:
           msg.append(w) 
    return msg


def cant_error(obj_id,verb):
    return action.error_return("%id cant %s",obj_id,verb)   

def invalid_value(value,verb):
    return action.error_return('%s cant %s',value,verb)

def out_of_range(range,verb):
    return action.error_return('Allowable range is %s',range,verb,ENG)

def integer_required(verb):
    return action.error_return('Integer value required','',verb,ENG)

def nothing_to_do(verb):
    return action.error_return('Nothing to do','',verb,ENG)

def doesnt_exist(thing,verb):
    return action.error_return('%s does not exist',thing,verb,ENG)

def already_exists(thing,verb):
    return action.error_return('%s already exists',thing,verb,ENG)

def cant_find(id,verb):
    return action.error_return("can't find %id",id,verb,ENG)

def state_error(obj_id,verb):
    return action.error_return("%id isnt %s",obj_id,verb)

def state_error1(name,verb):
    return action.error_return("%s isnt %s",name,verb)

def invalid_thing(thing,verb):
    return action.error_return("%s is not valid",thing,verb,ENG)

def none_applicable():
    return ("No verbs applicable",'','','',ENG)

def inappropriate_use(verb):
    return ("Inappropriate use of %s",verb,'','',ENG)

def nothing_uses_verb(verb):
    return ("No agents use the verb %s",verb,'','',ENG)

def inappropriate_arguments(verb):
    return ("Inappropriate arguments for the verb %s",verb,'','',ENG)

def no_current_voice(val,verb):
    return action.error_return("no current voice %s",val,verb,ENG)

def message(msg):
    return action.error_return(msg,'','',ENG)


   
