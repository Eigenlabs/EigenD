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

from pi import agent,atom,domain,policy,bundles,resource,collection,async,errors,action
from . import conductor_version as version

import piw
import conductor_native

class ClipPoolWidget(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,domain=domain.Blob(),names='Clip Pool',transient=True,policy=atom.default_policy(self.__changed),protocols="widget-clippool")

    def __changed(self,value):
        print 'received',value

class ArrangeViewWidget(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,domain=domain.Blob(),names='Arrangement View',transient=True,policy=atom.default_policy(self.__changed),protocols="widget-arrangeview")

    def __changed(self,value):
        print 'received',value

class SceneViewWidget(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,domain=domain.Blob(),names='Scene View',transient=True,policy=atom.default_policy(self.__changed),protocols="widget-sceneview")

    def __changed(self,value):
        print 'received',value

class ChannelList(collection.Collection):
    def __init__(self, agent, names):
        self.agent = agent
        self.__timestamp = piw.tsd_time()

        collection.Collection.__init__(self,names=names,creator=self.__create_channel,wrecker=self.__wreck_channel,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def new_channel(self, index):
        return None
    
    def channels_changed(self):
        pass
    
    def __create_channel(self, index):
        return self.new_channel(index)
    
    def __wreck_channel(self, index, node):
        node.disconnect()
        self.channels_changed()

    def create_channel(self, ordinal=None):
        o = ordinal or self.find_hole()
        o = int(o)
        if o < 1:
            return errors.out_of_range(str(1),'create')
        if o in self:
            return errors.already_exists(str(o),'create')
        e = self.new_channel(o)
        self[o] = e
        self.channels_changed()
        self.agent.update()
        return e 

    def get_channel(self, index):
        return self.get(index)

    def del_channel(self, index):
        v = self[index]
        del self[index]
        v.disconnect()
        self.channels_changed()
        self.agent.update()
    
    @async.coroutine('internal error')
    def __create_inst(self, ordinal=None):
        e = self.create_channel(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self, key, inst, ordinal):
        inst.disconnect()
        self.channels_changed()
        yield async.Coroutine.success()

class InputChannel(atom.Atom):
    def __init__(self, agent, index):
        self.agent = agent
        self.index = index

        atom.Atom.__init__(self, names=self.get_names(), ordinal=index, protocols='remove nostage')

        self.__input = self.get_vector_input()
        if self.__input:
            self[1] = atom.Atom(domain=domain.Aniso(), policy=self.__input.vector_policy(1,False), names='audio input', protocols='nostage')
            self[2] = atom.Atom(domain=domain.Aniso(), policy=self.__input.merge_nodefault_policy(2,False,callback=self.channel_changed), names='controller input')

    def channel_changed(self, arg):
        if arg is None:
            self.agent.conductor.clear_controller_input(self.index)

    def get_vector_input(self):
        return None

    def get_names(self):
        return 'channel'

    def disconnect(self):
        pass

class AudioInputChannel(InputChannel):
    def get_vector_input(self):
        return bundles.VectorInput(self.agent.conductor.audio_input_cookie(self.index), self.agent.domain, signals=(1,2))

    def get_names(self):
        return 'audio channel'

    def disconnect(self):
        self.agent.conductor.remove_audio_input(self.index)

class AudioInputChannelList(ChannelList):
    def __init__(self, agent):
        ChannelList.__init__(self, agent, 'audio inputs')

    def new_channel(self,index):
        return AudioInputChannel(self.agent, index)

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='conductor', ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()

        self.__timestamp = 0

        self[2] = atom.Atom(names='outputs')
        self[2][1] = bundles.Output(1, True, names='audio output 1')
        self.output = bundles.Splitter(self.domain, self[2][1])

        self.conductor = conductor_native.conductor(self.output.cookie(), self.domain, resource.user_resource_dir(resource.conductor_dir,version=''))

        self[1] = AudioInputChannelList(self)

        self.input_clock = bundles.ScalarInput(self.conductor.metronome_cookie(), self.domain, signals=(1,2,3,4))
        self[3] = atom.Atom(names='metronome inputs')
        self[3][1] = atom.Atom(domain=domain.BoundedFloat(0,10000000), policy=self.input_clock.nodefault_policy(1,False), names='song beat input')
        self[3][2] = atom.Atom(domain=domain.BoundedFloat(0,100), policy=self.input_clock.nodefault_policy(2,False), names='bar beat input')
        self[3][3] = atom.Atom(domain=domain.Bool(), init=False, policy=self.input_clock.nodefault_policy(3,False), names='running input')
        self[3][4] = atom.Atom(domain=domain.BoundedFloat(0,100000), init=120, policy=self.input_clock.nodefault_policy(4,False), names="tempo input")

        self.add_verb2(1,'create([],None,role(None,[mass([audio,input])]))', self.__create_audio_input)
        self.add_verb2(2,'create([un],None,role(None,[mass([audio,input])]))', self.__uncreate_audio_input)
        self.add_verb2(3,'record([],None)',self.__record)
        self.add_verb2(4,'stop([],None)',self.__stop)

        self[10] = atom.Atom("stage");
        self[10][1]=ClipPoolWidget();
        self[10][2]=ArrangeViewWidget();
        self[10][3]=SceneViewWidget();

    def __create_audio_input(self,subj,mass):
        id = int(action.mass_quantity(mass))
        e = self[1].create_channel(id)
        if not isinstance(e,AudioInputChannel):
            return e

    def __uncreate_audio_input(self,subj,mass):
        id = int(action.mass_quantity(mass))
        channel = self[1].get_channel(id)

        if channel is None:       
            thing='input %s' %str(id)
            return async.success(errors.invalid_thing(thing,'un create'))

        self[1].del_channel(id)

    def __record(self,subj):
        self.conductor.start_recording()

    def __stop(self,subj):
        self.conductor.stop_recording()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))
        if 1 in self: self[1].update()
        if 2 in self: self[2].update()

    def close_server(self):
        self.conductor.shutdown()
        agent.Agent.close_server(self)

    def on_quit(self):
        self.conductor.shutdown()

agent.main(Agent)

