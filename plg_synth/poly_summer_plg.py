
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

from pi import agent,atom,domain,errors,action,utils,bundles,async,paths,collection
from plg_synth import summer_version as version

import piw
import synth_native

MAX_CHANNEL = 24

class AudioInput(atom.Atom):
    
    def __init__(self, agent, index):
        
        self.__agent = agent
        self.__index = index

        atom.Atom.__init__(self,domain=domain.BoundedFloat(-1,1),names='audio input',policy=agent.input.vector_policy(index,True),protocols='remove nostage',ordinal=index)

    def disconnect(self):
        pass
        
class AudioInputList(collection.Collection):

    def __init__(self,agent):
        self.__agent = agent
        self.__timestamp = piw.tsd_time()

        collection.Collection.__init__(self,names='audio channels',creator=self.__create_input,wrecker=self.__wreck_input,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __create_input(self,index):
        return AudioInput(self.__agent, index)
    
    def __wreck_input(self,index,node):
        node.disconnect()

    def create_input(self,ordinal=None):
        o = ordinal or self.find_hole()
        o = int(o)
        if o < 1 or o > MAX_CHANNEL:
            return errors.out_of_range(str(MAX_CHANNEL),'create')
        e = AudioInput(self.__agent,o)
        self[o] = e
        self.__agent.update()
        return e 

    def get_input(self,index):
        return self.get(index)

    def del_input(self,index):
        v = self[index]
        del self[index]
        v.disconnect()
    
    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        e = self.create_input(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,key,inst,ordinal):
        inst.disconnect()
        yield async.Coroutine.success()

class Agent(agent.Agent):
    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='poly summer',ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()

        self.__timestamp = 0

        self[2] = bundles.Output(1, True, names='audio output')

        self.output = bundles.Splitter(self.domain, self[2])
        self.summer = synth_native.polysummer(self.output.cookie(), self.domain)
        self.input = bundles.VectorInput(self.summer.cookie(), self.domain, signals=(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24))

        self[1] = AudioInputList(self)

        self.add_verb2(1,'create([],None,role(None,[matches([channel])]))', self.__create)
        self.add_verb2(2,'create([un],None,role(None,[numeric]))', self.__uncreate)

    def __create(self,subj,channel):
        e = self[1].create_input()
        if not isinstance(e,AudioInput):
            return e

    def __uncreate(self,subj,channel):
        id = int(action.abstract_string(channel))
        channel = self[1].get_input(id)

        if channel is None:       
            thing='channel %s' %str(id)
            return async.success(errors.invalid_thing(thing,'un create'))

        self[1].del_input(id)

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))
        self[1].update()

agent.main(Agent)

