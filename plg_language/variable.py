
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

from pi import atom,bundles,node,action,logic,utils,errors,async
import piw
from . import interpreter

class Variable(bundles.Output):
    def __init__(self,manager,index,name="",value=""):
        self.__manager = manager
        self.__index = index
        self.name = ""
        self.value = ""
        bundles.Output.__init__(self,1,False)
        self.__output = bundles.Splitter(self.__manager.agent.domain,self)
        self.__lights = piw.lightsource(piw.change_nb(),0,self.__output.cookie())
        self.__private = node.Server(change=self.__change)
        self.set_private(self.__private)
        self.__alternatives = []

        if name:
            self.set_name(name)
            self.set_value(value)

    def __change(self,value):
        if value.is_string():
            (name,value) = value.as_string().split(':',1)
            self.set_name(name)
            self.set_value(value)

    def __update(self):
        d = "%s:%s" % (self.name,self.value)
        self.__private.set_data(piw.makestring(d,0))
        self.__update_lights()

    def __update_lights(self):
        mv = self.value
        for (i,(r,v)) in enumerate(self.__alternatives):
            if r==0:
                c = 0
            else:
                c = (1 if v==mv else 2)
            self.__lights.set_status(i+1,c)

    def set_name(self,name):
        if name != self.name:
            self.name = name
            self.__manager.update_cache()
            self.__update()

    def set_value(self,value):
        self.value = value
        self.__update()

    def find_alternative(self,value):
        for (i,(r,v)) in enumerate(self.__alternatives):
            if r>0 and v==value:
                return 'dsc(~(s)".%d","%d")' % (self.__index,i+1)
        return None

    def create_alternative(self,value):
        for (i,(r,v)) in enumerate(self.__alternatives):
            if r==0:
                self.__alternatives[i][1] = value
                self.__alternatives[i][0] += 1
                self.__update_lights()
                return i

        i = len(self.__alternatives)
        self.__alternatives.append([1,value])
        self.__lights.set_size(i+1)
        self.__update_lights()
        return i

    def delete_alternative(self,ind):
        if ind in self.__alternatives:
            self.__alternatives[ind][0] -= 1
            self.__update_lights()

class SubDelegate(interpreter.Delegate):
    def __init__(self,agent):
        self.__agent = agent

    def error_message(self,err):
        return self.__agent.error_message(err)

    def buffer_done(self,*a,**kw):
        return 

class VariableManager(atom.Atom):
    def __init__(self,agent):
        self.agent = agent
        self.variables = {}
        atom.Atom.__init__(self,creator = self.__create_variable, wrecker = self.__wreck_variable, container=(None,'variable',agent.verb_container()))

        self.add_verb2(70,'define([],global_name,role(None,[]),role(as,[abstract]))',callback=self.__define_slow,create_action=self.__define_fast,status_action=self.__status)
        self.add_verb2(71,'run([],global_run,role(None,[abstract]),option(with,[]))',self.__run)

    @async.coroutine('internal error')
    def __run(self,subject,script,arg):
        name = action.abstract_string(script)
        words = self.get_var(name)

        if not words:
            yield async.Coroutine.success(errors.doesnt_exist('%s does not exist'%name,'run'))

        interp = interpreter.Interpreter(self.agent,self.agent.database, SubDelegate(self.agent))

        print 'doing',words
        self.agent.register_interpreter(interp)

        try:
            yield interp.process_block(words.split())
        finally:
            self.agent.unregister_interpreter(interp)

        print 'done',words

    def __create_variable(self,index):
        return Variable(self,index)

    def __wreck_variable(self,index,v):
        pass

    def update_cache(self):
        self.variables = {}
        for v in self.values():
            if v.name:
                self.variables[v.name] = v

    def create_alternative(self,name,value):
        v = self.variables.get(name)
        if v is None:
            i = self.find_hole()
            v = Variable(self,i,name,value)
            self[i] = v
        return (v,v.create_alternative(value))

    def find_alternative(self,name,value):
        v = self.variables.get(name)
        if v is None:
            return None
        return v.find_alternative(value)

    def set_var(self,name,value):
        print '*** SET VARIABLE',name,value
        v = self.variables.get(name)
        if v is None:
            i = self.find_hole()
            v = Variable(self,i,name,value)
            self[i] = v
        else:
            v.set_value(value)

    def get_var(self,name):
        v = self.variables.get(name)
        if v is None:
            return None
        return v.value

    def __define_slow(self,subject,value,name):
        name = action.abstract_string(name)
        value = logic.render_term(value)
        self.set_var(name,value)

    def __status(self,subject,value,name):
        n = action.abstract_string(name)
        v = logic.render_term(value)
        return self.find_alternative(n,v)

    def __define_fast(self,ctx,subject,value,name):
        n = action.abstract_string(name)
        v = logic.render_term(value)
        var,ind = self.create_alternative(n,v)

        def changer(dummy):
            if not dummy.is_null() and dummy.as_norm()!=0.0:
                self.set_var(n,v)

        return (piw.slowchange(utils.changify(changer)),(var,ind))

    def __undefine_fast(self,ctx,data):
        (var,ind) = data
        var.delete_alternative(ind)
