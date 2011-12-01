
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
import loop_native
from pi import agent,atom,toggle,domain,bundles,action,upgrade,policy,utils,timeout,const,node,logic
from pi.logic.shortcuts import T
from plg_loop import metronome_version as version

def sc(f):
    return piw.slowchange(utils.changify(f))

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='metronome',container=4,protocols='browse',ordinal=ordinal)

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,False,names='bar beat output')
        self[1][2] = bundles.Output(2,False,names='song beat output')
        self[1][3] = bundles.Output(3,False,names='running output')
        self[1][4] = bundles.Output(4,False,names='bar output')
        self[1][5] = bundles.Output(5,False,names='tempo output')
        self[14]=bundles.Output(1,False,names='status output')

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self.clk_output = bundles.Splitter(self.domain,self[1][1],self[1][2],self[1][3],self[1][4])
        self.tempo_output = bundles.Splitter(self.domain,self[1][5])
        self.status_output = bundles.Splitter(self.domain,self[14])

        self.pinger = loop_native.pinger(self.clk_output.cookie(), self.tempo_output.cookie(), self.status_output.cookie(), self.domain, sc(self.__tempo_set), sc(self.__beats_set), sc(self.__preroll_set))
        self.aggregator = piw.aggregator(self.pinger.cookie(),self.domain)

        self.tap_input = bundles.ScalarInput(self.aggregator.get_output(1),self.domain, signals=(1,2))
        self.midi_clock_input = bundles.ScalarInput(self.aggregator.get_output(2),self.domain,signals=(1,))

        self[2] = atom.Atom(domain=domain.BoundedFloat(0,500,hints=(T('inc',1),T('biginc',10),T('control','updown'))), init=120, names='tempo input', policy=atom.default_policy(self.__set_tempo))
        self[3] = atom.Atom(domain=domain.BoundedFloat(0,100,rest=4,hints=(T('inc',1),)), names='beat input', policy=atom.default_policy(self.__set_beats))
        # self[4] is the verb container
        self[5] = atom.Atom(domain=domain.BoundedFloat(0,1,rest=0,hints=(T('control','trigger'),)),policy=self.tap_input.nodefault_policy(1,policy.ImpulseStreamPolicy()),names='beat trigger',transient=True)
        self[6] = atom.Atom(domain=domain.BoundedFloat(0,1,rest=0,hints=(T('control','trigger'),)),policy=self.tap_input.nodefault_policy(2,policy.ImpulseStreamPolicy()),names='bar trigger',transient=True)
        self[7] = atom.Atom(domain=domain.BoundedFloat(1,500), init=30, names='tempo minimum', policy=atom.default_policy(self.__set_tempo_lbound))
        self[8] = atom.Atom(domain=domain.BoundedFloat(1,500), init=500, names='tempo maximum', policy=atom.default_policy(self.__set_tempo_ubound))
        self[9] = atom.Atom(domain=domain.Bool(hints=(T('control','toggle'),)),policy=atom.default_policy(self.__preroll),names='preroll trigger',transient=True)
        self[10] = atom.Atom(domain=domain.BoundedInt(1,32), init=4, names='preroll', policy=atom.default_policy(self.__set_preroll_count))

        self[15] = atom.Atom(domain=domain.Aniso(),policy=self.midi_clock_input.nodefault_policy(1,False),names='midi clock input')
        self[16] = toggle.Toggle(self.__set_midi_clock_enable,self.domain,container=(None,'midi clock enable',self.verb_container()),names='midi clock enable')
        self[17] = atom.Atom(domain=domain.BoundedFloat(-1000,1000,hints=(T('inc',1),T('control','updown'))),init=0,names='midi clock latency', policy=atom.default_policy(self.__midi_clock_set_latency))
        self[18] = atom.Atom(domain=domain.BoundedIntOrNull(0,2000),init=0,names='beat flash persistence',policy=atom.default_policy(self.__set_beat_flash_persistence))

        self.add_verb2(1,'start([],None)',self.__start,status_action=self.__status)
        self.add_verb2(2,'stop([],None)',self.__stop,status_action=self.__status)
        self.add_verb2(3,'start([toggle],None)',self.__toggle,status_action=self.__status)

        print 'init tempo=',self[2].get_value()
        self.pinger.set_tempo(self[2].get_value())
        self.pinger.set_beats(self[3].get_value())
        self.pinger.set_range(self[8].get_value(),self[7].get_value())
        
        self.__playing=False

        self.__timestamp = piw.tsd_time()
        self.__selected=None
        self.update()

    def update(self):
        #print 'update'
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_enumerate(self,arg):
        print 'metronome enumerate'
        #return logic.render_term((1,0))
        return logic.render_term((0,0))

    def rpc_cinfo(self,arg):
        print 'metronome __cinfo'
        return '[]'

    def rpc_finfo(self,arg):
        print 'metronome __finfo'
        return '[]'

    def rpc_dinfo(self,arg):
        l=[]
        dsc=self.get_description()
        l.append(('dinfo_id',dsc))
        v1=self[2].get_value()
        l.append(('Tempo','%.1f' % v1))
        v2=self[3].get_value()
        l.append(('Beats',v2))
        if self.__playing:
            v3='Yes'
        else:
            v3='No'
        l.append(('Running',v3))
        if self[16].get_value():
            v5='Yes'
        else:
            v5='No'
        l.append(('Midi clock sync',v5))
        v6=self[17].get_value()
        l.append(('Midi clock latency (ms)',v6))
        
        return logic.render_term(T('keyval',tuple(l) ))

    def rpc_current(self,arg):
        #return '[]'
        uid=0
        return '[[%d,[]]]' % uid

    def rpc_setselected(self,arg):
        pass

    def rpc_activated(self,arg):
        return logic.render_term(('',''))


    def __setup_playstate(self,playing):
        self.__playing=playing
        print '__setup_playstate',self.__playing
        self.update()

    def __playchanged(self,d):
        if d.is_bool():
            self.__playing=d.as_bool()
            self.update()
            if self.__playing:
                self.pinger.play()
            else:
                self.pinger.stop()
      
    def __set_midi_clock_enable(self,e):
        print 'midi clock enable',e
        self.pinger.midi_clock_enable(e)
        self.update()
        return True
        
    def __midi_clock_set_latency(self,latency):
        # set midi clock latency in ms
        self.pinger.midi_clock_set_latency(latency)
        self.update()
        return True

    def __set_beat_flash_persistence(self,time):
        self.pinger.set_beat_flash_persistence(time)
        self[18].set_value(time)
        return True 

    def __preroll_set(self,t):
        self[9].set_value(t.as_bool())

    def __preroll(self,t):
        self.pinger.start_preroll(self[10].get_value() if t else 0)

    def __set_preroll_count(self,c):
        return True

    def __beats_set(self,t):
        self[3].set_value(t.as_float())
        self.update()

    def __tempo_set(self,t):
        #print 'tempo set to',t.as_float()
        self[2].set_value(t.as_float())
        self.update()

    def __set_tempo(self,t):
        print '__set_tempo to',t
        self.pinger.set_tempo(t)
        return False

    def __set_tempo_lbound(self,l):
        u=self[8].get_value()
        if u<=l:
            return False
        self.pinger.set_range(u,l)

    def __set_tempo_ubound(self,u):
        l=self[7].get_value()
        if l<1 or l>=u:
            return False
        self.pinger.set_range(u,l)

    def __set_beats(self,t):
        self.pinger.set_beats(t)
        return False

    def __start(self,subj):
        print 'start'
        self.pinger.play()
        self.__setup_playstate(True)
        return action.nosync_return()
    
    def __toggle(self,subj):
        print 'toggle'
        self.pinger.toggle()
        return action.nosync_return()

    def __stop(self,subj):
        print 'stop'
        self.pinger.stop()
        self.__setup_playstate(False)
        return action.nosync_return()

    def __status(self,subj):
        return 'dsc(~(s)"#14",None)'


agent.main(Agent)





