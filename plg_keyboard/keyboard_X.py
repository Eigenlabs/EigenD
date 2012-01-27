
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

import os
import itertools
import picross
import piw
import keyboard_native

from lib_alpha2 import ezload

from pi import atom, toggle, utils, bundles, domain, agent, paths, action, logic, policy, upgrade, resource, guid, audio, const, node
from pi.logic.shortcuts import *
from plg_keyboard import alpha_manager_version as version
from plg_keyboard import test

R = lambda a,b: range(a,b+1)

db_range = 70

def volume_function(f):
    if f<=0.01: return 0.0
    fn = f/100.0
    db = db_range*(1.0-fn)
    sc = pow(10.0,-db/20.0)
    return sc

def pedal_file(id):
    if id.startswith('<'):
        id = id[1:-1]
    return '%s-pedals'%id.replace('/','_')

class VirtualKey(atom.Atom):
    def __init__(self,keys):
        atom.Atom.__init__(self,names='k',protocols='virtual')
        self.choices=[]
        self.kbd_keys = keys

    def __key(self,*keys):
        x = ','.join(['cmp([dsc(~(parent)"#1","%(k)d"),dsc(~(parent)"#2","%(k)d"),dsc(~(parent)"#3","%(k)d"),dsc(~(parent)"#4","%(k)d"),dsc(~(parent)"#8","%(k)d")])' % dict(k=k) for k in keys])
        return '[%s]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',arg,(a,o)
        if not a and o is None: return self.__key(*R(1,self.kbd_keys))
        if o is None: return self.__key()
        o=int(o)
        if a: return '[]'
        if o<1 or o>self.kbd_keys: return self.__key()
        return self.__key(o)


class Keyboard(agent.Agent):
    def __init__(self,names,ordinal,dom,remove,keys):
        self.kbd_keys = keys
        agent.Agent.__init__(self,names=names,ordinal=ordinal,signature=version,subsystem='kbd',volatile=True,container=102,protocols='browse is_subsys')

        self.remover=remove

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self[1] = bundles.Output(1,False,names='activation output')
        self[2] = bundles.Output(2,False,names='pressure output')
        self[3] = bundles.Output(3,False,names='roll output')
        self[4] = bundles.Output(4,False,names='yaw output')

        self[5] = bundles.Output(1,False,names='strip position output',ordinal=1)

        self[7] = bundles.Output(1,False,names='breath output')
        self[10] = bundles.Output(2,False,names='absolute strip output',ordinal=1)

        self[12] = bundles.Output(1,False,names='pedal output',ordinal=1)
        self[13] = bundles.Output(1,False,names='pedal output',ordinal=2)
        self[14] = bundles.Output(1,False,names='pedal output',ordinal=3)
        self[15] = bundles.Output(1,False,names='pedal output',ordinal=4)

        self.led_backend = piw.functor_backend(1,True)
        self.status_mixer = piw.statusmixer(self.led_backend.cookie())
        self.led_input = bundles.VectorInput(self.status_mixer.cookie(),self.domain,signals=(1,))
        self[8] = atom.Atom(names='light input',protocols='revconnect',policy=self.led_input.vector_policy(1,False,False,auto_slot=True),domain=domain.Aniso())

        self[100] = VirtualKey(self.kbd_keys)

        self.koutput = bundles.Splitter(self.domain,self[1],self[2],self[3],self[4])
        self.kpoly = piw.polyctl(10,self.koutput.cookie(),False,5)
        self.s1output = bundles.Splitter(self.domain,self[5],self[10])
        self.boutput = bundles.Splitter(self.domain,self[7])
        self.poutput1 = bundles.Splitter(self.domain,self[12])
        self.poutput2 = bundles.Splitter(self.domain,self[13])
        self.poutput3 = bundles.Splitter(self.domain,self[14])
        self.poutput4 = bundles.Splitter(self.domain,self[15])

        self.kclone=piw.sclone()
        self.kclone.set_filtered_output(1,self.kpoly.cookie(),piw.first_filter(1))
        self.kclone.set_filtered_output(2,self.s1output.cookie(),piw.first_filter(2))
        self.kclone.set_filtered_output(5,self.boutput.cookie(),piw.first_filter(5)) 
        self.kclone.set_filtered_output(6,self.poutput1.cookie(),piw.first_filter(6))
        self.kclone.set_filtered_output(7,self.poutput2.cookie(),piw.first_filter(7))
        self.kclone.set_filtered_output(8,self.poutput3.cookie(),piw.first_filter(8))
        self.kclone.set_filtered_output(9,self.poutput4.cookie(),piw.first_filter(9))

        self[9] = atom.Atom(names='controller output',domain=domain.Aniso(),init=self.courses())

        self.add_verb2(4,'maximise([],None,role(None,[mass([pedal])]))',self.__maximise)
        self.add_verb2(5,'minimise([],none,role(None,[mass([pedal])]))',self.__minimise)

        self.__timestamp = piw.tsd_time()
        self.update()

    def rpc_enumerate(self,args):
        return logic.render_term((0,0))

    def rpc_cinfo(self,args):
        return '[]'

    def rpc_finfo(self,args):
        return '[]'

    def rpc_current(self,args):
        return '[[0,[]]]'

    def rpc_setselected(self,args):
        pass

    def rpc_activated(self,args):
        pass

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))
  
    def set_threshhold(self):
        self[241] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_min(1), protocols='input output explicit', names='pedal minimum threshold', ordinal=1, policy=atom.default_policy(lambda v: self.__set_min(1,v)))
        self[242] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_max(1), protocols='input output explicit', names='pedal maximum threshold', ordinal=1, policy=atom.default_policy(lambda v: self.__set_max(1,v)))
        self[243] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_min(2), protocols='input output explicit', names='pedal minimum threshold', ordinal=2, policy=atom.default_policy(lambda v: self.__set_min(2,v)))
        self[244] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_max(2), protocols='input output explicit', names='pedal maximum threshold', ordinal=2, policy=atom.default_policy(lambda v: self.__set_max(2,v)))
        self[245] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_min(3), protocols='input output explicit', names='pedal minimum threshold', ordinal=3, policy=atom.default_policy(lambda v: self.__set_min(3,v)))
        self[246] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_max(3), protocols='input output explicit', names='pedal maximum threshold', ordinal=3, policy=atom.default_policy(lambda v: self.__set_max(3,v)))
        self[247] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_min(4), protocols='input output explicit', names='pedal minimum threshold', ordinal=4, policy=atom.default_policy(lambda v: self.__set_min(4,v)))
        self[248] = atom.Atom(domain=domain.BoundedInt(0,4095), init=self.keyboard.get_pedal_max(4), protocols='input output explicit', names='pedal maximum threshold', ordinal=4, policy=atom.default_policy(lambda v: self.__set_max(4,v)))

        self[251] = atom.Atom(domain=domain.BoundedFloat(0,1), init=self.keyboard.get_threshold1(), protocols='input output', names='soft threshold', policy=atom.default_policy(self.keyboard.set_threshold1))
        self[249] = atom.Atom(domain=domain.BoundedFloat(0,1), init=self.keyboard.get_threshold2(), protocols='input output', names='hard threshold', policy=atom.default_policy(self.keyboard.set_threshold2))
        self[252] = atom.Atom(domain=domain.BoundedFloat(0,1), init=self.keyboard.get_roll_axis_window(), protocols='input output', names='roll axis window', policy=atom.default_policy(self.keyboard.set_roll_axis_window))
        self[253] = atom.Atom(domain=domain.BoundedFloat(0,1), init=self.keyboard.get_yaw_axis_window(), protocols='input output', names='yaw axis window', policy=atom.default_policy(self.keyboard.set_yaw_axis_window))

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.advertise('<keyboard>')

    def setup_leds(self):
        self.led_backend.set_functor(piw.pathnull(0),self.keyboard.led_functor())

    def __set_min(self,p,v):
        self.keyboard.set_pedal_min(p,v)
        return True

    def __set_max(self,p,v):
        self.keyboard.set_pedal_max(p,v)
        return True

    def __setpedal(self,p):
        if p>0 and p<=4:
            min = self.keyboard.get_pedal_min(p)
            max = self.keyboard.get_pedal_max(p)
            self[241+2*(p-1)].set_value(min)
            self[242+2*(p-1)].set_value(max)

    def __maximise(self,subj,pedal):
        pedal = int(action.mass_quantity(pedal))
        self.keyboard.learn_pedal_max(pedal)
        self.__setpedal(pedal)

    def __minimise(self,subj,pedal):
        pedal = int(action.mass_quantity(pedal))
        self.keyboard.learn_pedal_min(pedal)
        self.__setpedal(pedal)

    def courses(self):
        return utils.makedict({'courselen':piw.makestring('[24,24,24,24,24,12]',0)},0)

    def close_server(self):
        agent.Agent.close_server(self)
        if self.keyboard:
            self.keyboard.close()
            self.keyboard = None

    def name(self):
        return self.keyboard.name()

    def restart(self):
        self.keyboard.restart();

    def dead(self):
        print 'notify dead'
        self.close_server()
        print 'done close_server'
        self.remover()
        print 'done remove'


class MicrophoneOutput(audio.AudioOutput):
    mic_types = dict(dynamic=0,electret=1,condenser=2)

    def __init__(self,agent):
        self.__agent = agent
        audio.AudioOutput.__init__(self,self.__agent.audio_output,1,1,names='microphone')

        self[100] = toggle.Toggle(self.__enable,self.__agent.domain,container=(None,'microphone enable',self.__agent.verb_container()),names='enable',transient=True)
        self[101] = atom.Atom(domain=domain.String(),init='electret',names='type',policy=atom.default_policy(self.__type))
        self[102] = atom.Atom(domain=domain.BoundedInt(0,50,hints=(T('inc',1),T('biginc',5),T('control','updown'))),names='gain',init=30,policy=atom.default_policy(self.__gain))
        self[103] = toggle.Toggle(self.__pad,self.__agent.domain,container=(None,'microphone pad',self.__agent.verb_container()),names='pad')
        self[104] = toggle.Toggle(self.__loop_enable,self.__agent.domain,container=(None,'microphone loop',self.__agent.verb_container()),names='loop')
        self[105] = atom.Atom(domain=domain.BoundedInt(0,120,hints=(T('inc',1),T('biginc',5),T('control','updown'))),names='loop gain',init=100,policy=atom.default_policy(self.__loop_gain))
        self[106] = toggle.Toggle(self.__mute_enable,self.__agent.domain,container=(None,'microphone mute',self.__agent.verb_container()),names='automute')
        self[109] = atom.Atom(domain=domain.BoundedInt(0,4),names='quality',init=2,policy=atom.default_policy(self.__quality))

    def __quality(self,e):
        self.__agent.keyboard.set_mic_quality(e);

    def __loop_gain(self,e):
        self.__agent.keyboard.loopback_gain(volume_function(e))
        self.__agent.update()

    def __mute_enable(self,e):
        self.__agent.keyboard.mic_automute(e)
        self.__agent.update()

    def __loop_enable(self,e):
        self.__agent.keyboard.loopback_enable(e)
        self.__agent.update()

    def server_opened(self):
        audio.AudioOutput.server_opened(self)
        self.__agent.keyboard.mic_disabled(utils.notify(self.__notify))
        self.__agent.keyboard.set_mic_quality(self[109].get_value());
        self.__agent.keyboard.loopback_enable(self[104].get_value())
        self.__agent.keyboard.mic_automute(self[106].get_value())
        self.__agent.keyboard.loopback_gain(volume_function(self[105].get_value()))

        self.__agent.update()

    def __notify(self):
        self.__agent.update()
        self[100].notify()
        
    def __enable(self,e):
        self.__agent.keyboard.mic_enable(e)
        self.__agent.update()

    def __type(self,t):
        if t in self.mic_types:
            self.__agent.keyboard.mic_type(self.mic_types[t])
            self.__agent.update()
            return True
        print 'no such mic type',t
        return False

    def __gain(self,g):
        if g>9:
            g -= 9
        else:
            g = 0
        self.__agent.keyboard.mic_gain(g)
        self.__agent.update()
        return True

    def __pad(self,p):
        self.__agent.keyboard.mic_pad(p)
        self.__agent.update()


class HeadphoneInput(audio.AudioInput):
    def __init__(self,agent):
        self.__agent = agent
        audio.AudioInput.__init__(self,self.__agent.audio_input,1,2,names='headphone')
        self[100] = toggle.Toggle(self.__enable,self.__agent.domain,container=(None,'headphone',self.__agent.verb_container()),names='enable')
        self[101] = atom.Atom(domain=domain.BoundedInt(0,127,hints=(T('inc',1),T('biginc',5),T('control','updown'))),names='gain',init=70,policy=atom.default_policy(self.__gain))
        self[102] = atom.Atom(domain=domain.BoundedInt(0,4),names='quality',init=0,policy=atom.default_policy(self.__quality))
        self[103] = atom.Atom(domain=domain.Bool(),names='limit',init=True,policy=atom.default_policy(self.__limit))

    def server_opened(self):
        audio.AudioInput.server_opened(self)
        self.__agent.keyboard.set_hp_quality(self[102].get_value());

    def __quality(self,e):
        self.__agent.keyboard.set_hp_quality(e);

    def __enable(self,e):
        self.__agent.keyboard.headphone_enable(e)
        self.__agent.update()

    def __gain(self,g):
        self.__agent.keyboard.headphone_gain(g)
        self.__agent.update()
        return True

    def __limit(self,l):
        self.__agent.keyboard.headphone_limit(l)
        self.__agent.update()
        return True


class Keyboard_Alpha2( Keyboard ):
    def __init__(self,ordinal,dom,remove,usbname=None,usbdev=None):
        self.type = 1
        self.usbname = usbname
        self.usbdev = usbdev
        Keyboard.__init__(self,'alpha keyboard',ordinal,dom,remove,132)

        self[6] = bundles.Output(1,False,names='strip position output',ordinal=2)
        self[11] = bundles.Output(2,False,names='absolute strip output',ordinal=2)
        self.s2output = bundles.Splitter(self.domain,self[6],self[11])
        self.kclone.set_filtered_output(4,self.s2output.cookie(),piw.first_filter(4))

        self.test_setup()
        self.nativekeyboard_setup()
        self.set_threshhold() #define in base class 
        self.setup_leds() #define in base class 

    def rpc_dinfo(self,arg):
        if self.usbdev is None:
            return self.__dinfo_legacy(arg)
        else:
            return self.__dinfo(arg)

    def __dinfo(self,arg):
        l=[]
        dsc = self.get_description()
        l.append(('dinfo_id',dsc))
        l.append(('Microphone enabled','Yes' if self[101][100].get_value() else 'No'))
        l.append(('Microphone type',self[101][101].get_value()))
        l.append(('Microphone gain',self[101][102].get_value()))
        l.append(('Microphone pad','On' if self[101][103].get_value() else 'Off'))
        l.append(('Headphone enabled','Yes' if self[103][100].get_value() else 'No'))
        l.append(('Headphone gain',self[103][101].get_value()))
        l.append(('Direct Monitoring enabled','Yes' if self[101][104].get_value() else 'No'))
        l.append(('Direct Monitoring gain',self[101][105].get_value()))
        l.append(('Microphone Auto Mute',self[101][106].get_value()))
        return logic.render_term(T('keyval',tuple(l) ))

    def __dinfo_legacy(self,arg):
        l=[]
        dsc = self.get_description()
        l.append(('dinfo_id',dsc))
        l.append(('Microphone enabled','(unavailable)'))
        l.append(('Microphone type','(unavailable)'))
        l.append(('Microphone gain','(unavailable)'))
        l.append(('Microphone pad','(unavailable)'))
        l.append(('Headphone enabled','(unavailable)'))
        l.append(('Headphone gain','(unavailable)'))
        return logic.render_term(T('keyval',tuple(l) ))

    def audio_output_setup(self):
        self.audio_output = bundles.Splitter(self.domain)
        self[101] = MicrophoneOutput(self)

    def audio_input_setup(self):
        self.audio_input = bundles.ScalarInput(self.keyboard.audio_cookie(),self.domain,signals=(1,2))
        self[103] = HeadphoneInput(self)

    def test_setup( self):
        self.add_verb2(2,'record([],None,role(None,[mass([k])]))',self.__record)
        self[16] = bundles.Output(4,False,names='test running output')
        self.toutput = bundles.Splitter(self.domain,self[16])
        self.kclone.set_filtered_output(3,self.toutput.cookie(),piw.first_filter(3))

    def nativekeyboard_setup(self):
        if self.usbdev is not None:
            self.audio_output_setup()
            self.keyboard=keyboard_native.alpha2_bundle(self.usbdev,self.kclone.cookie(),self.audio_output.cookie(),utils.notify(self.dead))
            self.audio_input_setup()
        else:
            self.keyboard=keyboard_native.alpha2_bundle_legacy(self.usbname,self.kclone.cookie(),utils.notify(self.dead))

    def rpc_runtest(self,arg):
        return test.runtest(self.keyboard,arg)

    def rpc_arm_recording(self,arg):
        return test.arm_recording(self.keyboard,arg)

    def __record(self,subj,dummy):
        print 'record key',k
        return action.nosync_return()


class Keyboard_Tau( Keyboard ):
    
    def __init__(self,ordinal,dom,remove,usbname=None,usbdev=None):
        self.type = 2
        self.usbname = usbname
        self.usbdev = usbdev
        Keyboard.__init__(self,'tau keyboard',ordinal,dom,remove,92)
        
        self.nativekeyboard_setup()
        self.set_threshhold() #define in base class 
        self.setup_leds() #define in base class 

    def rpc_dinfo(self,arg):
        l=[]
        dsc=self.get_description()
        l.append(('dinfo_id',dsc))
        l.append(('Headphone enabled','Yes' if self[103][100].get_value() else 'No'))
        l.append(('Headphone gain',self[103][101].get_value()))
        return logic.render_term(T('keyval',tuple(l) ))

    def audio_input_setup(self):
        self.audio_input = bundles.ScalarInput(self.keyboard.audio_cookie(),self.domain,signals=(1,2))
        self[103] = HeadphoneInput(self)

    def nativekeyboard_setup(self):
        self.keyboard=keyboard_native.tau_bundle(self.usbdev,self.kclone.cookie(),utils.notify(self.dead))
        self.audio_input_setup()

    def courses(self):
        return utils.makedict({'courselen':piw.makestring('[16,16,20,20,12,4,4]',0)},0)


class Keyboard_Alpha1( Keyboard ):

    def __init__(self,usbname,ordinal,dom,remove):
        self.usbname = usbname
        Keyboard.__init__(self,'alpha keyboard',ordinal,dom,remove,132)
        self.nativekeyboard_setup()
        self.set_threshhold() #define in base class 
        self.setup_leds() #define in base class 

    def nativekeyboard_setup(self):
        self.keyboard=keyboard_native.bundle(self.usbname,self.kclone.cookie(),utils.notify(self.dead))


class BaseStation:
    def __init__(self,usbname,agent):
        self.__usbname = usbname
        self.__agent = agent
        self.__thing = piw.thing()
        self.__device = 0
        self.__instrument = 0
        self.__kbd = None
        piw.tsd_thing(self.__thing)
        self.__thing.set_slow_timer_handler(utils.notify(self.__poll))
        self.__counter = itertools.cycle([1]+[0]*20)
        self.__thing.timer_slow(1000)

    @utils.nothrow
    def __poll(self):
        if not self.__device:
            self.__device = picross.usbdevice(self.__usbname,0)
        basecfg = self.bs_config_read()
        instcfg = self.inst_config_read()
        if self.__counter.next():
            print 'basecfg: %s' % ''.join(map(lambda x:hex(ord(x))[2:].zfill(2),basecfg))
            print 'instcfg: %s' % ''.join(map(lambda x:hex(ord(x))[2:].zfill(2),instcfg))
        inst = ord(instcfg[0])
        if inst==self.__instrument:
            return
        self.__instrument = inst
        if inst:
            self.__kbd = self.__agent.create(self.__instrument,self.__usbname,self.__device)

    def bs_config_read(self):
        return self.__device.control_in(0x40|0x80,0xc2,0,0,64)

    def inst_config_read(self):
        return self.__device.control_in(0x40|0x80,0xc6,0,0,64)

    def bs_config_write(self,id,data):
        self.__device.control_out(0x40,0xc1,id,data,'')

    def inst_config_write(self,id,data):
        self.__device.control_out(0x40,0xc5,id,data,'')

    def close(self):
        if self.__kbd:
            k = self.__kbd
            self.__kbd = None
            k.dead()
        self.__thing.close_thing()
        self.__device = 0
        self.__instrument = 0


class KeyboardFactory( agent.Agent ):
    def __init__( self, address , ordinal):
        agent.Agent.__init__(self,signature=version, names='alpha manager',protocols='has_subsys', ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*', 0L))

        self.__thing = piw.thing()
        piw.tsd_thing(self.__thing)
        self.__thing.set_slow_dequeue_handler(utils.changify(self.__dequeue))

        self.__base = dict()
        
        self.__enum = []
        
        lsf = lambda c: picross.make_string_functor(utils.make_locked_callable(c))
        self.__enum.append(picross.enumerator(0x049f,0x505a,lsf(self.add_alpha1keyboard)))
        self.__enum.append(picross.enumerator(0xbeca,0x0102,lsf(self.add_alpha2keyboard)))
        self.__enum.append(picross.enumerator(0xbeca,0x0103,lsf(self.add_taukeyboard)))
        self.__enum.append(picross.enumerator(0x2139,0x0002,lsf(self.download_base_station)))
        self.__enum.append(picross.enumerator(0x2139,0x0003,lsf(self.download_psu)))
        self.__enum.append(picross.enumerator(0x2139,0x0104,lsf(self.add_base_station),lsf(self.del_base_station)))
        self.__enum.append(picross.enumerator(0x2139,0x0105,lsf(self.add_base_station),lsf(self.del_base_station)))

    def add_alpha1keyboard( self, usbname):
        msg = 'alpha1:%s' % usbname
        self.__thing.enqueue_slow(piw.makestring(msg,0))

    def add_taukeyboard( self, usbname ):
        msg = 'tau:%s' % usbname
        self.__thing.enqueue_slow( piw.makestring( msg,0))

    def add_alpha2keyboard( self,usbname ):
        msg = 'alpha2:%s' % usbname
        self.__thing.enqueue_slow(piw.makestring(msg,0))

    def download_base_station(self,usbname):
        firmware = ezload.bs_firmware()
        if firmware:
            print 'loading firmware to base station',usbname
            ezload.download(usbname,firmware)
        else:
            print 'master mode base station firmware not found'

    def download_psu(self,usbname):
        firmware = ezload.psu_firmware()
        if firmware:
            print 'loading firmware to PSU',usbname
            ezload.download(usbname,firmware)
        else:
            print 'master mode PSU firmware not found'

    def add_base_station(self,usbname):
        msg = 'base:%s' % usbname
        self.__thing.enqueue_slow(piw.makestring(msg,0))

    def del_base_station(self,usbname):
        print 'del',usbname
        msg = 'delbase:%s' % usbname
        self.__thing.enqueue_slow(piw.makestring(msg,0))

    def server_opened(self):
        agent.Agent.server_opened(self)
        for e in self.__enum:
            e.start()

    def server_closed(self):
        for e in self.__enum:
            e.stop()
        agent.Agent.server_closed(self)  

    def next_keyboard(self):
        i=0
        for ss in self.iter_subsystem():
            i=max(i,int(ss))
        return i+1

    def __dequeue(self,usbname):
        usbname=usbname.as_string()
        version,name = usbname.split(':') 
        
        print "evt :", version
        print "usb name :", name 

        if version=='delbase':
            bs = self.__base.get(name)
            print 'removing base station',name
            if bs:
                bs.close()
                del self.__base[name]
            return

        for (ssk,ssv) in self.iter_subsys_items():
            if ssv.usbname==name:
                print 'ignore existing kbd during add',name
                ssv.restart()
                return

        if version=='base':
            self.__base[name] = BaseStation(name,self)
            return
            
        i=self.next_keyboard()

        if version == 'alpha1': 
            k = Keyboard_Alpha1(name,i,self.domain,lambda: self.del_keyboard(i))
        elif version == 'alpha2':
            k = Keyboard_Alpha2(i,self.domain,lambda:self.del_keyboard(i),usbname=name)
        else:
            k = Keyboard_Tau(i,self.domain,lambda:self.del_keyboard(i),usbname=name)

        self.add_subsystem(str(i),k)
        print 'added keyboard',i,k.name()

    def create(self,type,name,device):
        for (ssk,ssv) in self.iter_subsys_items():
            if ssv.usbname==name:
                print 'ignore existing kbd during create',name
                ssv.restart()
                return
            
        i=self.next_keyboard()
        if type==1:
            k = Keyboard_Alpha2(i,self.domain,lambda:self.del_keyboard(i),usbname=name,usbdev=device)
        elif type==2:
            k = Keyboard_Tau(i,self.domain,lambda:self.del_keyboard(i),usbname=name,usbdev=device)
        else:
            return None

        self.add_subsystem(str(i),k)
        print 'added keyboard',i,k.name()
        return k

    def del_keyboard(self,i):
        print 'removed keyboard',i
        self.remove_subsystem(str(i))

class Upgrader(upgrade.Upgrader):
    def upgrade_9_0_to_10_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            if 'tau' in ssr.getmeta(const.meta_names):
                ssr.ensure_node(103,100,255,17)
                ssr.ensure_node(103,100,255,18)
        return True

    def upgrade_8_0_to_9_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            if 'tau' in ssr.getmeta(const.meta_names):
                print 'upgrading tau',ss
                ssr.ensure_node(103,1,255,1) 
                ssr.ensure_node(103,1,255,2) 
                ssr.ensure_node(103,1,255,3) 
                ssr.ensure_node(103,1,255,4).set_string("left")
                ssr.ensure_node(103,1,255,7).set_long(1)
                ssr.ensure_node(103,1,255,8).set_string("audio input")
                ssr.ensure_node(103,2,255,1) 
                ssr.ensure_node(103,2,255,2) 
                ssr.ensure_node(103,2,255,3) 
                ssr.ensure_node(103,2,255,4).set_string("right")
                ssr.ensure_node(103,2,255,7).set_long(2)
                ssr.ensure_node(103,2,255,8).set_string("audio input")
                ssr.ensure_node(103,100,1,255,1) 
                ssr.ensure_node(103,100,1,255,3) 
                ssr.ensure_node(103,100,1,255,8).set_string("status output")
                ssr.ensure_node(103,100,255,1) 
                ssr.ensure_node(103,100,255,3) 
                ssr.ensure_node(103,100,255,8).set_string("enable")
                ssr.ensure_node(103,101,255,1) 
                ssr.ensure_node(103,101,255,3) 
                ssr.ensure_node(103,101,255,8).set_string("gain")
                ssr.ensure_node(103,255,1) 
                ssr.ensure_node(103,255,8).set_string("headphone")
        return True

    def upgrade_7_0_to_8_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            path = resource.user_resource_file('keyboard',pedal_file(ss),version='')
            if not os.path.isfile(path):
                print 'no pedal data found in',path
                continue
            print 'loading pedals from',path
            for i,l in enumerate(open(path).readlines()):
                min,max = l.split()
                min = int(min)
                max = int(max)
                print 'read pedal',i+1,'min',min,'max',max
                tools.root(ss).ensure_node(241+2*i,254).set_float(min)
                tools.root(ss).ensure_node(242+2*i,254).set_float(max)
        return True

    def upgrade_6_0_to_7_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(101,255,1)
            ssr.ensure_node(101,255,8)
            ssr.ensure_node(103,255,1)
            ssr.ensure_node(103,255,8)
        return True

    def upgrade_5_0_to_6_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(255,6)
        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            print 'upgrading',ss
            ssr = tools.root(ss)
            n = ssr.get_node(10)
            if n:
                n.rename(names='absolute strip output')
        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            print 'upgrading',ss
            ssr = tools.root(ss)
            ssr.ensure_node(102).erase_children()
        return True

    def upgrade_2_0_to_3_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            print 'upgrading',ss
            ssr = tools.root(ss)
            ssr.rename(names='keyboard alpha')
        return True


