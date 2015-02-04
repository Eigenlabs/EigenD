# agent for Ableton Push


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

IN_MIDI=1
IN_LIGHT=2
OUT_MIDI=10

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='ableton push', ordinal=ordinal)

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

		#values required for push
        self.device.channel(1)
        self.device.velocity_sample(4)
        self.device.enable_notes(True)
        self.device.enable_velocity(True)
        self.device.enable_poly_at(PRESSURE,True)
        self.device.set_data_freq(0)
        self.device.enable_control_notes(True)
        self.device.set_colour(1,87) # green
        self.device.set_colour(2,4) # red
        self.device.set_colour(3,124) #orange
        self.device.set_pb_map(STRIP_1,True)
        
    def controllerinit(self):
    	scale=piw.makestring('[0,1,2,3,4,5,6,7,8,9,10,11,12]',0)
    	octave=piw.makefloat_bounded(9,-1,0,-1,0)
        dict=utils.makedict({'columnlen':self.device.get_columnlen(),'columnoffset':self.device.get_columnoffset(),'courselen':self.device.get_courselen(),'courseoffset':self.device.get_courseoffset(),'octave':octave,'scale':scale},0)
        return dict


#
# Define Agent as this agents top level class
#
agent.main(Agent)
