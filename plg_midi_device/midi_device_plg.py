

import piw
from pi import agent, domain, bundles, atom, action, utils
from pi.logic.shortcuts import T
from . import midi_device_version as version

from .midi_device_plg_native import midi_device

OUT_KEY=1
OUT_STRIP_1=2
OUT_STRIP_2=3
OUT_BREATH=4
OUT_PEDAL_1=5
OUT_PEDAL_2=6
OUT_PEDAL_3=7
OUT_PEDAL_4=8


IN_MIDI=1
IN_LIGHT=2
OUT_MIDI=10

# this have to match mapping in midi_device.cpp
PRESSURE=0
ROLL=1
YAW=2
BREATH=3
STRIP_1=4
STRIP_2=5
PEDAL_1=6
PEDAL_2=7
PEDAL_3=8
PEDAL_4=9


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='midi device', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,False,names='key output')
        self[1][2] = bundles.Output(2,False,names='pressure output')
        self[1][3] = bundles.Output(3,False,names='roll output')
        self[1][4] = bundles.Output(4,False,names='yaw output')

        self[1][5] = bundles.Output(1,False,names='breath output')

        self[1][6] = bundles.Output(1,False,names='strip position output',ordinal=1)
        self[1][7] = bundles.Output(2,False,names='absolute strip output',ordinal=1)
        self[1][8] = bundles.Output(1,False,names='strip position output',ordinal=2)
        self[1][9] = bundles.Output(2,False,names='absolute strip output',ordinal=2)

        self[1][10] = bundles.Output(1,False,names='pedal output',ordinal=1)
        self[1][11] = bundles.Output(1,False,names='pedal output',ordinal=2)
        self[1][12] = bundles.Output(1,False,names='pedal output',ordinal=3)
        self[1][13] = bundles.Output(1,False,names='pedal output',ordinal=4)
 
		# key outputs
        self.koutput = bundles.Splitter(self.domain,self[1][1],self[1][2],self[1][3],self[1][4])
        self.kpoly = piw.polyctl(10,self.koutput.cookie(),False,5)
		# breath
        self.boutput = bundles.Splitter(self.domain,self[1][5])
		# strips
        self.s1output = bundles.Splitter(self.domain,self[1][6],self[1][7])
        self.s2output = bundles.Splitter(self.domain,self[1][8],self[1][9])
		# pedals
        self.poutput1 = bundles.Splitter(self.domain,self[1][10])
        self.poutput2 = bundles.Splitter(self.domain,self[1][11])
        self.poutput3 = bundles.Splitter(self.domain,self[1][12])
        self.poutput4 = bundles.Splitter(self.domain,self[1][13])

        self[5]=bundles.Output(OUT_MIDI,False,names="midi output")
        self.midi_output = bundles.Splitter(self.domain, self[5])  
        
		# now we need to set of outputs
        self.output=piw.sclone()
        self.output.set_filtered_output(OUT_KEY,self.kpoly.cookie(),piw.first_filter(OUT_KEY))
        self.output.set_filtered_output(OUT_STRIP_1,self.s1output.cookie(),piw.first_filter(OUT_STRIP_1))
        self.output.set_filtered_output(OUT_STRIP_2,self.s2output.cookie(),piw.first_filter(OUT_STRIP_2))
        self.output.set_filtered_output(OUT_BREATH,self.boutput.cookie(),piw.first_filter(OUT_BREATH)) 
        self.output.set_filtered_output(OUT_PEDAL_1,self.poutput1.cookie(),piw.first_filter(OUT_PEDAL_1))
        self.output.set_filtered_output(OUT_PEDAL_2,self.poutput2.cookie(),piw.first_filter(OUT_PEDAL_2))
        self.output.set_filtered_output(OUT_PEDAL_3,self.poutput3.cookie(),piw.first_filter(OUT_PEDAL_4))
        self.output.set_filtered_output(OUT_PEDAL_4,self.poutput4.cookie(),piw.first_filter(OUT_PEDAL_4))
        self.output.set_filtered_output(OUT_MIDI,self.midi_output.cookie(),piw.first_filter(OUT_MIDI))
        
        self.device = midi_device(self.domain, self.output.cookie())

        self[1][14] = atom.Atom(names='controller output',domain=domain.Aniso(),init=self.controllerinit())
              
        
        self.input = bundles.VectorInput(self.device.cookie(), self.domain, signals=(IN_MIDI,IN_LIGHT,))
        self[2] = atom.Atom(domain=domain.Aniso(), policy=self.input.vector_policy(IN_MIDI,False), names="midi input")
        self[4] = atom.Atom(domain=domain.Aniso(), protocols='revconnect', policy=self.input.vector_policy(IN_LIGHT,False,clocked=False), names="light input")

                
        self[3] = atom.Atom(names="inputs")
        self[3][1]  = atom.Atom(domain=domain.BoundedInt(0,16),init=0,policy=atom.default_policy(self.__channel),names='midi channel')
        self[3][2]  = atom.Atom(domain=domain.BoundedInt(0,64),init=4,policy=atom.default_policy(self.__velocity_sample),names='velocity sample')
        self[3][3]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__enable_notes),names='notes enable')
        self[3][4]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__enable_velocity),names='notes velocity enable')
        self[3][5]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_note_cc),names='note control')
        self[3][6]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_note_cc_pressure),names='note pressure control')
        # here we are just setting up some common ones to start with, and reasonable defaults
        self[3][7]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__enable_pb_roll),names='pitch bend roll enable')
        self[3][8]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_yaw_control),names='yaw control')
        self[3][9]  = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_chan_at_cc),names='channel pressure')
        self[3][10] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_poly_at_cc),names='polyphonic pressure')
        self[3][11] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_hires_cc),names='high resolution control')

        self[3][12]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=2,policy=atom.default_policy(self.__set_breath_control),names='breath control')
        self[3][13]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__set_breath_release),names='breath release')
        self[3][14]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__set_breath_bipolar),names='breath bipolar')
        self[3][15]  = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_breath_relative),names='breath relative')
        self[3][16]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=1,policy=atom.default_policy(self.__set_strip_1_control),names='strip 1 control')
        self[3][17]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__set_strip_1_release),names='strip 1 release')
        self[3][18]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__set_strip_1_bipolar),names='strip 1 bipolar')
        # self[3][19]  = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_strip_1_relative),names='strip 1 relative')
        self[3][20]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_strip_2_control),names='strip 2 control')
        self[3][21]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__set_strip_2_release),names='strip 2 release')
        self[3][22]  = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__set_strip_2_bipolar),names='strip 2 bipolar')
        # self[3][23]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__set_strip_2_relative),names='strip 1 relative')

        self[3][24]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_roll_control),names='roll control')
        self[3][25]  = atom.Atom(domain=domain.BoundedInt(0,5000), init=0, policy=atom.default_policy(self.__set_data_freq), names='data frequency')
        self[3][26]  = atom.Atom(domain=domain.BoundedInt(-1,127), init=-1, policy=atom.default_policy(self.__set_pressure_control), names='pressure control')

        self[3][27]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_hires_pressure_control),names='pressure high control')
        self[3][28]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_hires_roll_control),names='roll high control')
        self[3][29]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_hires_yaw_control),names='yaw high control')
        self[3][30]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_hires_pb_control),names='pitch bend high control')
        
        self[3][31]  = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__enable_control_notes),names='notes control enable')
        self[3][32]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_pedal_1_control),names='pedal 1 control')
        self[3][33]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_pedal_2_control),names='pedal 2 control')
        self[3][34]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_pedal_3_control),names='pedal 3 control')
        self[3][35]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=-1,policy=atom.default_policy(self.__set_pedal_4_control),names='pedal 4 control')

        self[3][36]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=0,policy=atom.default_policy(self.__set_off_colour),names='off colour')
        self[3][37]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=4,policy=atom.default_policy(self.__set_red_colour),names='red colour')
        self[3][38]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=124,policy=atom.default_policy(self.__set_orange_colour),names='orange colour')
        self[3][39]  = atom.Atom(domain=domain.BoundedInt(-1,127),init=87,policy=atom.default_policy(self.__set_green_colour),names='green colour')
        self[3][40]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__enable_pb_strip_1),names='pitch bend strip 1 enable')
        self[3][41]  = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__enable_pb_strip_2),names='pitch bend strip 2 enable')


    def controllerinit(self):
    	scale=piw.makestring('[0,1,2,3,4,5,6,7,8,9,10,11,12]',0)
    	octave=piw.makefloat_bounded(9,-1,0,-1,0)
        dict=utils.makedict({'columnlen':self.device.get_columnlen(),'columnoffset':self.device.get_columnoffset(),'courselen':self.device.get_courselen(),'courseoffset':self.device.get_courseoffset(),'octave':octave,'scale':scale},0)
        return dict

    def __channel(self,value):
        self[3][1].set_value(value)
        self.device.channel(value)
        return True

    def __velocity_sample(self,value):
        self[3][2].set_value(value)
        self.device.velocity_sample(value)
        return True

    def __enable_notes(self,value):
        self[3][3].set_value(value)
        self.device.enable_notes(value)
        return True

    def __enable_velocity(self,value):
        self[3][4].set_value(value)
        self.device.enable_velocity(value)
        return True

    def __set_note_cc(self,value):
        self[3][5].set_value(value)
        self.device.set_note_cc(value)
        return True

    def __set_note_cc_pressure(self,value):
        self[3][6].set_value(value)
        self.device.set_note_cc_pressure(value)
        return True
 
    def __enable_pb_roll(self,value):
        self[3][7].set_value(value)
        self.device.set_pb_map(ROLL,value)
        return True

    def __set_yaw_control(self,value):
        self[3][8].set_value(value)
        self.device.set_cc_map(YAW,value)
        return True

    def __set_chan_at_cc(self,value):
        self[3][9].set_value(value)
        self.device.enable_chan_at(PRESSURE,value)
        return True

    def __set_poly_at_cc(self,value):
        self[3][10].set_value(value)
        self.device.enable_poly_at(PRESSURE,value)
        return True

    def __set_hires_cc(self,value):
        self[3][11].set_value(value)
        self.device.enable_hires_cc(value)
        return True

    def __set_breath_control(self,value):
        self[3][12].set_value(value)
        self.device.set_cc_map(BREATH,value)
        return True

    def __set_breath_release(self,value):
        self[3][13].set_value(value)
        self.device.set_release_map(BREATH,value)
        return True

    def __set_breath_bipolar(self,value):
        self[3][14].set_value(value)
        self.device.set_bipolar_map(BREATH,value)
        return True

    def __set_breath_relative(self,value):
        self[3][15].set_value(value)
        self.device.set_relative_map(BREATH,value)
        return True

    def __set_strip_1_control(self,value):
        self[3][16].set_value(value)
        self.device.set_cc_map(STRIP_1,value)
        return True

    def __set_strip_1_release(self,value):
        self[3][17].set_value(value)
        self.device.set_release_map(STRIP_1,value)
        return True

    def __set_strip_1_bipolar(self,value):
        self[3][18].set_value(value)
        self.device.set_bipolar_map(STRIP_1,value)
        return True

    def __set_strip_1_relative(self,value):
        self[3][19].set_value(value)
        self.device.set_relative_map(STRIP_1,value)
        return True

    def __set_strip_2_control(self,value):
        self[3][20].set_value(value)
        self.device.set_cc_map(STRIP_2,value)
        return True

    def __set_strip_2_release(self,value):
        self[3][21].set_value(value)
        self.device.set_release_map(STRIP_2,value)
        return True

    def __set_strip_2_bipolar(self,value):
        self[3][22].set_value(value)
        self.device.set_bipolar_map(STRIP_2,value)
        return True

    def __set_strip_2_relative(self,value):
        self[3][23].set_value(value)
        self.device.set_relative_map(STRIP_2,value)
        return True
        
    def __set_roll_control(self,value):
        self[3][24].set_value(value)
        self.device.set_cc_map(ROLL,value)
        return True

    def __set_data_freq(self,value):
        self[3][25].set_value(value)
        self.device.set_data_freq(value)
        return True

    def __set_pressure_control(self,value):
        self[3][26].set_value(value)
        self.device.set_cc_map(PRESSURE,value)
        return True

    def __set_hires_pressure_control(self,value):
        self[3][27].set_value(value)
        self.device.set_hires_cc_map(PRESSURE,value)
        return True

    def __set_hires_roll_control(self,value):
        self[3][28].set_value(value)
        self.device.set_hires_cc_map(ROLL,value)
        return True

    def __set_hires_yaw_control(self,value):
        self[3][29].set_value(value)
        self.device.set_hires_cc_map(YAW,value)
        return True

    def __set_hires_pb_control(self,value):
        self[3][30].set_value(value)
        self.device.set_hires_pb_map(ROLL,value)
        return True

    def __enable_control_notes(self,value):
        self[3][31].set_value(value)
        self.device.enable_control_notes(value)
        return True

    def __set_pedal_1_control(self,value):
        self[3][32].set_value(value)
        self.device.set_cc_map(PEDAL_1,value)
        return True

    def __set_pedal_2_control(self,value):
        self[3][33].set_value(value)
        self.device.set_cc_map(PEDAL_2,value)
        return True

    def __set_pedal_3_control(self,value):
        self[3][34].set_value(value)
        self.device.set_cc_map(PEDAL_3,value)
        return True

    def __set_pedal_4_control(self,value):
        self[3][35].set_value(value)
        self.device.set_cc_map(PEDAL_4,value)
        return True

    def __set_off_colour(self,value):
        self[3][36].set_value(value)
        self.device.set_colour(0,value)
        return True

    def __set_red_colour(self,value):
        self[3][37].set_value(value)
        self.device.set_colour(2,value)
        return True

    def __set_orange_colour(self,value):
        self[3][38].set_value(value)
        self.device.set_colour(3,value)
        return True
    def __set_green_colour(self,value):
        self[3][39].set_value(value)
        self.device.set_colour(1,value)
        return True

    def __enable_pb_strip_1(self,value):
        self[3][40].set_value(value)
        self.device.set_pb_map(STRIP_1,value)
        return True

    def __enable_pb_strip_2(self,value):
        self[3][41].set_value(value)
        self.device.set_pb_map(STRIP_2,value)
        return True


#
# Define Agent as this agents top level class
#
agent.main(Agent)
