

import piw
from pi import agent, domain, bundles, atom, action
from pi.logic.shortcuts import T
from . import midi_monitor_version as version

from .midi_monitor_plg_native import midi_monitor

IN_CONTROL=1
IN_MIDI=2

OUT_LIGHT=1

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='midi monitor', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(OUT_LIGHT, False, names='light output', protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1][1])

        self.monitor = midi_monitor(self.domain, self.output.cookie())
        
        self.ctlr_fb = piw.functor_backend(1,True)
        self.ctlr_fb.set_functor(piw.pathnull(0),self.monitor.control())
        self.ctlr_input = bundles.ScalarInput(self.ctlr_fb.cookie(),self.domain,signals=(IN_CONTROL,))
        self[2] = atom.Atom(domain=domain.Aniso(),policy=self.ctlr_input.policy(IN_CONTROL,False),names='controller input')

        self.midi_input = bundles.VectorInput(self.monitor.cookie(), self.domain, signals=(IN_MIDI,))
        self[3] = atom.Atom(domain=domain.Aniso(), policy=self.midi_input.vector_policy(IN_MIDI,False), names="midi input")
        
        
        self[4] = atom.Atom(names="inputs")
        self[4][1] = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__enable_notes),names='enable notes')
        self[4][2] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__enable_poly_pressure),names='enable polyphonic pressure')
        self[4][3] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__enable_cc_as_key),names='enable control with key')
        self[4][4] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__enable_cc_as_course),names='enable control with course')
        
        self[4][5] = atom.Atom(domain=domain.BoundedInt(0,16),init=0,policy=atom.default_policy(self.__channel),names='channel')
        self[4][6] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__nearest_match),names='altered note')
        self[4][7] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__first_match),names='base note')
        
        self[4][8] = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__use_velocity_as_state),names='velocity for status')
        self[4][9] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__use_channel_as_state),names='channel for status')
        
        self[4][10] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__use_physical_mapping),names='use physical mapping')
        
        self[4][11] = atom.Atom(domain=domain.BoundedInt(0,127),init=0,policy=atom.default_policy(self.__control_offset),names='control offset')
        
    def __enable_notes(self,value):
        self[4][1].set_value(value)
        self.monitor.enable_notes(value);
        return True

    def __enable_poly_pressure(self,value):
        self[4][2].set_value(value)
        self.monitor.enable_poly_pressure(value);
        return True

    def __enable_cc_as_key(self,value):
        self[4][3].set_value(value)
        self.monitor.enable_cc_as_key(value);
        return True

    def __enable_cc_as_course(self,value):
        self[4][4].set_value(value)
        self.monitor.enable_cc_as_course(value);
        return True

    def __channel(self,value):
        self[4][5].set_value(value)
        self.monitor.channel(value);
        return True

    def __nearest_match(self,value):
        self[4][6].set_value(value)
        self.monitor.nearest_match(value);
        return True

    def __first_match(self,value):
        self[4][7].set_value(value)
        self.monitor.first_match(value);
        return True

    def __use_velocity_as_state(self,value):
        self[4][8].set_value(value)
        self.monitor.use_velocity_as_state(value);
        return True

    def __use_channel_as_state(self,value):
        self[4][9].set_value(value)
        self.monitor.use_channel_as_state(value);
        return True

    def __use_physical_mapping(self,value):
        self[4][10].set_value(value)
        self.monitor.use_physical_mapping(value);
        return True

    def __control_offset(self,value):
        self[4][11].set_value(value)
        self.monitor.control_offset(value);
        return True

#
# Define Agent as this agents top level class
#
agent.main(Agent)
