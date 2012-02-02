
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

# ------------------------------------------------------------------------------------------------------------------
# MIDI output plugin
#
# Agent to allow output of multiple MIDI streams to a MIDI output port
# ------------------------------------------------------------------------------------------------------------------

import os
import picross
import piw
from pi import atom,bundles,domain,agent,logic,utils,node,action,async,upgrade
from . import midi_output_version as version,midi_native


# ------------------------------------------------------------------------------------------------------------------
# Output MIDI delegate
#
# Manages the list of available MIDI outputs in the system
# ------------------------------------------------------------------------------------------------------------------

# actually implements the native midi_delegate_t struct so that the native
# MIDI output can update the list of MIDI outputs
# TODO: define a general MIDI delegate in plg_midi, move from macosx_native
class OutputMidiDelegate(midi_native.midi_output):
    def __init__(self,notify):
        midi_native.midi_output.__init__(self)
        self.sinks = []
        self.__notify = notify

    def sink_added(self,id,name):
        xid = '%x'%id
        for i,(u,n) in enumerate(self.sinks):
            if u==xid:
                print 'midi sink changed',xid,name
                self.sinks[i] = (xid,name)
                self.__notify()
                return

        print 'midi sink added',xid,name
        self.sinks.append((xid,name))
        self.__notify()

    def sink_removed(self,id):
        xid = '%x'%id
        for i,(u,n) in enumerate(self.sinks):
            if u==xid:
                print 'midi sink removed',xid,n
                del self.sinks[i]
                self.__notify()
                return


# ------------------------------------------------------------------------------------------------------------------
# Output MIDI port
#
# Allows an output midi port to be chosen using the browser
# ------------------------------------------------------------------------------------------------------------------

class OutputMidiPort(atom.Atom):
    def __init__(self,clk_domain):
        self.__timestamp = piw.tsd_time()

        # output MIDI port
        self.__midi_port = OutputMidiDelegate(self.__update)
        self.__midi_functor = self.__midi_port.get_midi_output_functor()

        # merge MIDI channels to output
        self.__midi_merge = midi_native.midi_merge_output(self.__midi_functor,clk_domain)

        atom.Atom.__init__(self,domain=domain.String(),names='midi port',policy=atom.default_policy(self.set_port),protocols='virtual browse')

        self.__midi_port.set_port(0)
        self.__midi_port.set_source('')
        self.__selected=None
        self.__index = 1
        self.__update()

    def rpc_displayname(self,arg):
        return 'MIDI output ports'

    def cookie(self):
        return self.__midi_merge.cookie()

    def set_index(self,index):
        self.__index = index
        if self.open():
            self.__midi_port.set_source('Eigenlabs %d' % self.__index)

    def server_opened(self):
        atom.Atom.server_opened(self)
        self.__midi_port.set_source('Eigenlabs %d' % self.__index)
        self.set_port(self.get_value())
        self.__midi_port.run()

    def close_server(self):
        atom.Atom.close_server(self)
        self.__midi_port.set_source('')
        self.__midi_port.set_port(0)
        self.__midi_port.stop()
        

    # set_port: set the chosen midi port
    def set_port(self,port):
        if self.open():
            if port:
                self.__midi_port.set_port(int(port,16))
            else:
                self.__midi_port.set_port(0)
            print 'OutputMidiPort: set port to',port
        self.set_value(port)
        self.__update()

    # ---------------------------------------------------------------------------------------------
    # functions to support selection of port by browsing
    # ---------------------------------------------------------------------------------------------

    # update: update timestamp to refresh the browser
    # the MIDI delegate will refresh the browser when system changes
    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'OutputMidiPort:setselected',selected    
        self.__selected=selected
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'OutputMidiPort:activated',selected    
        self.set_port(selected)
        return logic.render_term(('',''))
    
    def rpc_current(self,arg):
        current = self.__midi_port.get_port()
        if current==0:
            return '[]'
        return '[["%x",[]]]' % current

    def rpc_enumerate(self,a):
        return logic.render_term((len(self.__midi_port.sinks),0))

    def rpc_cinfo(self,a):
        return '[]'

    def rpc_finfo(self,a):
        (dlist,cnum) = logic.parse_clause(a)
        map = tuple([(uid,dsc,None) for (uid,dsc) in self.__midi_port.sinks[cnum:]])
        return logic.render_term(map)

    # ---------------------------------------------------------------------------------------------
    # functions to support selection of port by belcanto
    # ---------------------------------------------------------------------------------------------

    def __ideal(self,uid):
        return '[ideal([~server,midiport],%s)]' % logic.render_term(uid)

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        for id,n in self.__midi_port.sinks:
            if id==cookie:
                return 'ideal([~server,midiport],%s)' % logic.render_term(cookie)
        return async.failure('invalid cookie')

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if a or not o:
            return '[]'
        return self.resolve_name(o)

    def resolve_name(self,name):
        if name=='OutputMidiPort: selection':
            o=self.__selected
            return self.__ideal(self.__selected)
        else:
            try:
                o = int(name)
            except:
                return '[]'

        if o>0 and o<len(self.__midi_port.sinks)+1:
            return self.__ideal(self.__midi_port.sinks[o-1][0])

        return '[]'




# ------------------------------------------------------------------------------------------------------------------
# Main Output MIDI agent
#
# contains a single MIDI port, which can be assigned to a system MIDI output by browsing
# ------------------------------------------------------------------------------------------------------------------

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,names='midi output',signature=version,container=3,ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self[1] = OutputMidiPort(self.domain)

        # midi input vector, merge down multiple MIDI input streams
        self.input = bundles.VectorInput(self[1].cookie(), self.domain, signals=(1,))
        self[2] = atom.Atom(domain=domain.Aniso(), names="midi input", policy=self.input.vector_policy(1,False))

        # self[3] is verb container

        # choose verb for choosing an output MIDI port
        self.add_verb2(1,'choose([],None,role(None,[ideal([~server,midiport]),singular]))',self.__chooseport)

        self.set_ordinal(ordinal)

    def property_change(self,key,value,delegate):
        if key == 'ordinal':
            self[1].set_index(self.get_property_long('ordinal',1))
            
    # resolve: resolve an MIDI port name into a MIDI port cookie
    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)
        print 'resolving',arg

        if type=='midiport':
            return self[1].resolve_name(' '.join(arg))

        return action.marshal(())

    # choose: implement choosing a port with the choose verb
    def __chooseport(self,subj,arg):
        print 'choose port',arg
        print action.arg_objects(arg)[0]
        (type,thing) = action.crack_ideal(action.arg_objects(arg)[0])
        print type,thing
        self[1].set_port(thing)

agent.main(Agent,gui=True)
