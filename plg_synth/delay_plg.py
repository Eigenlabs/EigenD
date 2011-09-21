
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

import sys
import piw
import picross
import synth_native
from pi import agent,atom,bundles,domain,paths,upgrade,policy,utils,action,async,collection
from plg_synth import delay_version as version

class Channel(atom.Atom):
    
    def __init__(self,chan,input,chan_num,vc,default_tap_time):
        print 'Tap, channel init ',chan
        # number of signals
        sigs=8
        self.input = input
        # by default, feedback and output pan to own channel only
        if chan=="left":
            l_pan_init=1
            r_pan_init=0
        else:
            l_pan_init=0
            r_pan_init=1
        
        atom.Atom.__init__(self,names="%s channel"%chan)
                
        # delay time in beat or seconds, beats +ve, seconds -ve, this is a temporary fix until units are used in domains!
        self[1]=atom.Atom(domain=domain.BoundedFloat(-4,4), names="time", init=default_tap_time, policy=self.input.policy(chan_num*sigs+1,False), container=vc)
        # feedback
        self[2]=atom.Atom(domain=domain.BoundedFloat(-2,2), init=0.5, names="feedback", protocols='input', policy=self.input.policy(chan_num*sigs+2,False))
        # filter enable
        self[3]=atom.Atom(domain=domain.Bool(), init=True, names="filter", protocols='input', policy=self.input.policy(chan_num*sigs+3,False))
        # filter cutoff
        self[4]=atom.Atom(domain=domain.BoundedFloat(0,20000), init=2000, names="cutoff", protocols='input', policy=self.input.policy(chan_num*sigs+4,False))
        # feedback to left channel, explicit stops an implicit connect (e.g. to an left audio input)
        self[5]=atom.Atom(domain=domain.BoundedFloat(0,1), init=l_pan_init, names="left gain", protocols='input explicit', policy=self.input.policy(chan_num*sigs+5,False))
        # feedback to right channel
        self[6]=atom.Atom(domain=domain.BoundedFloat(0,1), init=r_pan_init, names="right gain", protocols='input explicit', policy=self.input.policy(chan_num*sigs+6,False))
        # other feedback channels...

        # output to left channel
        self[7]=atom.Atom(domain=domain.BoundedFloat(0,1), init=l_pan_init, names="left volume", protocols='input explicit', policy=self.input.policy(chan_num*sigs+7,False))
        # output to right channel
        self[8]=atom.Atom(domain=domain.BoundedFloat(0,1), init=r_pan_init, names="right volume", protocols='input explicit', policy=self.input.policy(chan_num*sigs+8,False))
        # other output channels...

        # delay time set verbs
        self[1].add_verb2(1,'set([],None,role(None,[instance(~self)]),role(to,[mass([second])]))',callback=self.set_time_secs)
        self[1].add_verb2(2,'set([],None,role(None,[instance(~self)]),role(to,[mass([beat])]))',callback=self.set_time_beats)
        self[1].add_verb2(3,'set([],None,role(None,[instance(~self)]),role(to,[numeric]))',callback=self.set_time_beats_default)
        print 'Channel, verbs done'

    def set_time_secs(self,subj,dummy,arg):
        time_secs = action.mass_quantity(arg)
        print "set time secs",arg,"->",time_secs,self.id()
        # time in seconds is -ve for now!
        self[1].get_policy().set_value(-time_secs)
        return action.nosync_return()

    def set_time_beats(self,subj,dummy,arg):
        time_beats = action.mass_quantity(arg)
        print "set time beats",arg,"->",time_beats,self.id()
        self[1].get_policy().set_value(time_beats)
        return action.nosync_return()

    def set_time_beats_default(self,subj,dummy,arg):
        time_beats = float(action.abstract_string(arg))
        print "set time beats default",arg,"->",time_beats,self.id()
        self[1].get_policy().set_value(time_beats)
        return action.nosync_return()


class Tap(atom.Atom):
    
    def __init__(self,agent,tapno,default_tap_time):
        print 'Tap, tap init ',tapno
        
        self.__agent = agent
        self.__tapno = tapno
        atom.Atom.__init__(self,ordinal=tapno,names='tap',container=(None,'tap%s'%tapno,agent[8]),protocols='remove')
        
        self.input = bundles.ScalarInput(agent.aggregator.get_output(tapno), agent.domain, signals=(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16))

        self[1]=Channel("left",self.input,0,(None,'left%s'%tapno,agent[8]),default_tap_time)
        self[2]=Channel("right",self.input,1,(None,'right%s'%tapno,agent[8]),default_tap_time)

        # other channels ...

    def destroy_tap(self):
        self.__agent.aggregator.clear_output(self.__tapno)

        
class TapList(collection.Collection):

    def __init__(self,agent):
        self.__agent = agent
        collection.Collection.__init__(self,names='taps',creator=self.__create_tap,wrecker=self.__wreck_tap,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)

    # create tap, called when rebuilding state
    def __create_tap(self,tapno):
        print "TapList: create tap ",tapno
        tap = Tap(self.__agent,tapno,self.__agent.default_tap_time)
        self.__agent.default_tap_time += self.__agent.default_tap_interval
        return tap 
    
    # wreck tap, called when rebuilding state
    def __wreck_tap(self,tapno,tapatom):
        print "TapList: wreck tap",tapno
        tapatom.destroy_tap()

    def create_tap(self,ordinal=None):
        o = ordinal or self.find_hole()
        e = Tap(self.__agent,o,self.__agent.default_tap_time)
        self[o] = e
        self.__agent.update()
        return e
    
    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        e=self.create_tap(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,key,inst,ordinal):
        inst.destroy_tap()
        yield async.Coroutine.success()



class Agent(agent.Agent):

    def __init__(self,address, ordinal):
        print "Delay Init"
        # verb container, used by all taps
        agent.Agent.__init__(self,signature=version,names='delay',container=8,ordinal=ordinal)

        # the agent event clock
        self.domain = piw.clockdomain_ctl()

        # outputs 
        self[1]=bundles.Output(1,True,names="right audio output")
        self[2]=bundles.Output(2,True,names="left audio output")
        self.output = bundles.Splitter(self.domain, self[1], self[2])

        # the delay class, takes cookies: [audio in, tap], returns [audio out]
        self.delay = piw.delay(self.output.cookie(),self.domain)
        
        # input has the correlator and a bundle style output
        self.input = bundles.ScalarInput(self.delay.audio_cookie(), self.domain, signals=(1,2,3,4,5,6))
        # aggregator for tap parameters, combines parameter signals into a single stream
        self.aggregator = piw.aggregator(self.delay.tap_cookie(),self.domain);

        # audio inputs
        # use vector policy inputs
        self[3]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="left audio input", policy=self.input.nodefault_policy(1,True))
        self[4]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="right audio input", policy=self.input.nodefault_policy(2,True))

        # more inputs...
        
        
        # default tap creation interval
        self[5]=atom.Atom(domain=domain.BoundedFloat(0,100000), init=0.5, names="tap interval", policy=atom.default_policy(self.__set_tap_interval))
        # TODO: these should be just policy so that these parameters can cause an event to start without needing audio events - merge_policy used for debugging
        # wet/dry mix
        self[6]=atom.Atom(domain=domain.BoundedFloat(0,1), init=0.5, names="mix", protocols='input', policy=self.input.merge_policy(3,False))
        # master feedback, 'protocols='input' is metadata that labels this as an input without it appearing in Belcanto
        self[9]=atom.Atom(domain=domain.BoundedFloat(0,2), init=1, names="master feedback", protocols='input', policy=self.input.merge_policy(4,False))
        # input from the metronome to determine tap times from the tempo
        self[10]=atom.Atom(domain=domain.BoundedFloat(0,100000), init=120, names="tempo input", policy=self.input.merge_policy(5,False))
        # offset feedback, 
        self[11]=atom.Atom(domain=domain.BoundedFloat(-2,2), init=0, names="offset feedback", protocols='input', policy=self.input.merge_policy(6,False))

        
        # verb to create a tap, no tap number given
        self.add_verb2(1,'create([],None,role(None,[matches([tap])]))',self.__create_tap)
        
        # verb to uncreate a tap, tap number is given, #7 is self[7]
        self.add_verb2(2,'create([un],None,role(None,[partof(~(a)"#7")]))',self.__uncreate_tap)

        # verb to reset delay lines
        self.add_verb2(3, 'clear([],None)', self.__reset_delay_lines)

        # create tap at default time, 1/8 beat
        self.default_tap_time = 0.5
        # default delay interval is 1/8 beat after previous time
        self.default_tap_interval = 0.5

        # the list of taps
        self[7] = TapList(self)

        # effect enable
        self[12]=atom.Atom(domain=domain.Bool(), init=True, names="enable", protocols='input', policy=atom.default_policy(self.__set_enable))

        # enable time, time to fade in and out when enabling in ms
        self[13] = atom.Atom(names='enable time input', domain=domain.BoundedFloat(0,100000), init=100, policy=atom.default_policy(self.__set_enable_time))

        print "create default tap..."

        # create a single default tap
        self[7][1] = Tap(self,1,self.default_tap_time)
        self.default_tap_time += self.default_tap_interval

        print "done."
        
    def __create_tap(self,subject,dummy):
        print 'Delay, start create tap '
        tapno = self[7].find_hole()        
        self[7][tapno] = Tap(self,tapno,self.default_tap_time)
        self.default_tap_time += self.default_tap_interval
        
    def __uncreate_tap(self,subject,tap):
        subject = action.concrete_object(tap)
        print 'Delay, un create tap ',subject
        
        for k,v in self[7].iteritems():
            if v.id()==subject:
                self[7][k].destroy_tap()
                del self[7][k]
                return

    def __set_tap_interval(self,val):
        print 'Set tap interval ',val
        self.default_tap_interval = val  
        return True

    def __reset_delay_lines(self,subject):
        print 'Delay, Reset'        
        self.delay.reset_delay_lines()

    # set enable
    def __set_enable(self,e):
        self.delay.set_enable(e)
        return True

    # set enable time
    def __set_enable_time(self,t):
        self.delay.set_enable_time(t)
        return True


class Upgrader(upgrade.Upgrader):
    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)
        root.remove(9)
        root.remove(12)
        return True
      
    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(8).erase_children()
        return True

agent.main(Agent, Upgrader)












