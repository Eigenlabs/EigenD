
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

from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node,logic
from . import strummer_version as version, strummer_native

import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        agent.Agent.__init__(self, signature=version, names='strummer',protocols='', ordinal=ordinal)
        
        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,False,names='key output')
        self[1][2] = bundles.Output(2,False,names='pressure output')
        self[1][3] = bundles.Output(3,False,names='roll output')
        self[1][4] = bundles.Output(4,False,names='yaw output')

        self.output = bundles.Splitter(self.domain,*self[1].values())
        self.strummer = strummer_native.cstrummer(self.output.cookie(),self.domain)
        self.aggregator = piw.aggregator(self.strummer.cookie(),self.domain)
        self.strum_input = bundles.VectorInput(self.aggregator.get_filtered_output(1,piw.null_filter()),self.domain,signals=(1,2,3,4,5,))
        self.data_input = bundles.VectorInput(self.aggregator.get_filtered_output(2,piw.null_filter()),self.domain,signals=(1,2,3,4,))

        self[4] = atom.Atom(names='inputs')

        self[4][1] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.strum_input.vector_policy(1,False),names='strum breath input',protocols='nostage')
        self[4][2] = atom.Atom(domain=domain.Aniso(),policy=self.strum_input.vector_policy(2,False),names='strum key input',protocols='nostage')
        self[4][3] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.strum_input.merge_policy(3,False),names='strum pressure input',protocols='nostage')
        self[4][4] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.strum_input.merge_policy(4,False),names='strum roll input',protocols='nostage')
        self[4][5] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.strum_input.merge_policy(5,False),names='strum yaw input',protocols='nostage')

        self[4][6] = atom.Atom(domain=domain.Aniso(),policy=self.data_input.vector_policy(1,False),names='course key input',protocols='nostage')
        self[4][7] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.data_input.merge_policy(2,False),names='course pressure input',protocols='nostage')
        self[4][8] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.data_input.merge_policy(3,False),names='course roll input',protocols='nostage')
        self[4][9] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.data_input.merge_policy(4,False),names='course yaw input',protocols='nostage')

        self[2] = atom.Atom(names='controls')
        self[2][1] = atom.Atom(domain=domain.Bool(),init=True,names='enable',policy=atom.default_policy(self.__enable))
        self[2][2] = atom.Atom(domain=domain.BoundedInt(0,2000),init=0,names='trigger window',policy=atom.default_policy(self.__set_trigger_window))
        self[2][3] = atom.Atom(domain=domain.BoundedFloat(0,10),init=0.0,names='course pressure scale',policy=atom.default_policy(self.__set_course_pressure_scale))
        self[2][4] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='course roll scale',policy=atom.default_policy(self.__set_course_roll_scale))
        self[2][5] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='course yaw scale',policy=atom.default_policy(self.__set_course_yaw_scale))
        self[2][6] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='strum breath scale',policy=atom.default_policy(self.__set_strum_breath_scale))
        self[2][7] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='strum pressure scale',policy=atom.default_policy(self.__set_strum_pressure_scale))
        self[2][8] = atom.Atom(domain=domain.BoundedFloat(0,10),init=0.0,names='strum roll scale',policy=atom.default_policy(self.__set_strum_roll_scale))
        self[2][9] = atom.Atom(domain=domain.BoundedFloat(0,10),init=0.0,names='strum yaw scale',policy=atom.default_policy(self.__set_strum_yaw_scale))
        self[2][10] = atom.Atom(domain=domain.Bool(),init=True,names='strum note end',policy=atom.default_policy(self.__set_strum_note_end))
        self[2][11] = atom.Atom(domain=domain.Bool(),init=False,names='open course enable',policy=atom.default_policy(self.__open_course_enable))
        self[2][12] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.0,names='open course pressure default',policy=atom.default_policy(self.__open_course_pressure_default))
        self[2][13] = atom.Atom(domain=domain.BoundedFloat(-1,1),init=0.0,names='open course roll default',policy=atom.default_policy(self.__open_course_roll_default))
        self[2][14] = atom.Atom(domain=domain.BoundedFloat(-1,1),init=0.0,names='open course yaw default',policy=atom.default_policy(self.__open_course_yaw_default))
        self[2][15] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.0,names='strum roll pressure mix',policy=atom.default_policy(self.__set_strum_roll_pressure_mix))
        self[2][16] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.0,names='strum yaw pressure mix',policy=atom.default_policy(self.__set_strum_yaw_pressure_mix))
        self[2][17] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.1,names='course mute threshold',policy=atom.default_policy(self.__set_course_mute_threshold))
        self[2][18] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.1,names='strum mute threshold',policy=atom.default_policy(self.__set_strum_mute_threshold))
        self[2][19] = atom.Atom(domain=domain.BoundedInt(0,1000),init=20,names='course mute interval',policy=atom.default_policy(self.__set_course_mute_interval))
        self[2][20] = atom.Atom(domain=domain.BoundedInt(0,1000),init=20,names='strum mute interval',policy=atom.default_policy(self.__set_strum_mute_interval))
        self[2][21] = atom.Atom(domain=domain.Bool(),init=True,names='polyphonic courses enable',policy=atom.default_policy(self.__set_poly_courses_enable))
        self[2][22] = atom.Atom(domain=domain.Bool(),init=True,names='course mute enable',policy=atom.default_policy(self.__set_course_mute_enable))
        self[2][23] = atom.Atom(domain=domain.Bool(),init=True,names='strum mute enable',policy=atom.default_policy(self.__set_strum_mute_enable))
        self[2][24] = atom.Atom(domain=domain.BoundedFloat(0,1),init=0.6,names='pulloff threshold',policy=atom.default_policy(self.__set_pulloff_threshold))
        self[2][25] = atom.Atom(domain=domain.BoundedInt(0,1000),init=10,names='pulloff interval',policy=atom.default_policy(self.__set_pulloff_interval))
        self[2][26] = atom.Atom(domain=domain.Bool(),init=True,names='pulloff enable',policy=atom.default_policy(self.__set_pulloff_enable))

        self[3] = atom.Atom(domain=domain.String(),init='[]',names='courses',policy=atom.default_policy(self.__set_courses),protocols='course_editor')

        self.add_verb2(1,'set([],~a,role(None,[matches([breath,modulation])]),role(for,[mass([course])]))',callback=self.__set_breath_modulation)
        self.add_verb2(2,'set([un],~a,role(None,[matches([breath,modulation])]),role(for,[mass([course])]))',callback=self.__unset_breath_modulation)
        self.add_verb2(3,'set([],~a,role(None,[matches([open])]),role(as,[mass([key])]),role(for,[mass([course])]))',callback=self.__set_open_course_key)
        self.add_verb2(4,'set([],~a,role(None,[matches([open])]),role(as,[coord(physical,[column],[row])]),role(for,[mass([course])]))',callback=self.__set_open_course_physical)
        self.add_verb2(5,'add([],~a,role(None,[matches([strum,key])]),role(with,[coord(physical,[column],[row])]),role(for,[mass([course])]))',callback=self.__add_strum_key)
        self.add_verb2(6,'clear([],~a,role(None,[matches([all,strum,key])]),role(for,[mass([course])]))',callback=self.__clear_strum_key)

        self.strumconfig = strummer_native.strumconfig()

    def __update_strumconfig(self):
        self.strummer.set_strumconfig(self.strumconfig)
        self[3].set_value(self.strumconfig.encode_courses())

    def __enable(self,v):
        self.strumconfig.enable(v)
        self.__update_strumconfig()

    def __set_trigger_window(self,v):
        self.strumconfig.set_trigger_window(v)
        self.__update_strumconfig()

    def __set_course_pressure_scale(self,v):
        self.strumconfig.set_course_pressure_scale(v)
        self.__update_strumconfig()

    def __set_course_roll_scale(self,v):
        self.strumconfig.set_course_roll_scale(v)
        self.__update_strumconfig()

    def __set_course_yaw_scale(self,v):
        self.strumconfig.set_course_yaw_scale(v)
        self.__update_strumconfig()

    def __set_strum_breath_scale(self,v):
        self.strumconfig.set_strum_breath_scale(v)
        self.__update_strumconfig()

    def __set_strum_pressure_scale(self,v):
        self.strumconfig.set_strum_pressure_scale(v)
        self.__update_strumconfig()

    def __set_strum_roll_scale(self,v):
        self.strumconfig.set_strum_roll_scale(v)
        self.__update_strumconfig()

    def __set_strum_yaw_scale(self,v):
        self.strumconfig.set_strum_yaw_scale(v)
        self.__update_strumconfig()

    def __set_strum_note_end(self,v):
        self.strumconfig.set_strum_note_end(v)
        self.__update_strumconfig()

    def __set_poly_courses_enable(self,v):
        self.strumconfig.set_poly_courses_enable(v)
        self.__update_strumconfig()

    def __open_course_enable(self,v):
        self.strumconfig.open_course_enable(v)
        self.__update_strumconfig()

    def __open_course_pressure_default(self,v):
        self.strumconfig.open_course_pressure_default(v)
        self.__update_strumconfig()

    def __open_course_roll_default(self,v):
        self.strumconfig.open_course_roll_default(v)
        self.__update_strumconfig()

    def __open_course_yaw_default(self,v):
        self.strumconfig.open_course_yaw_default(v)
        self.__update_strumconfig()

    def __set_strum_roll_pressure_mix(self,v):
        self.strumconfig.set_strum_roll_pressure_mix(v)
        self.__update_strumconfig()

    def __set_strum_yaw_pressure_mix(self,v):
        self.strumconfig.set_strum_yaw_pressure_mix(v)
        self.__update_strumconfig()

    def __set_course_mute_threshold(self,v):
        self.strumconfig.set_course_mute_threshold(v)
        self.__update_strumconfig()

    def __set_course_mute_interval(self,v):
        self.strumconfig.set_course_mute_interval(v)
        self.__update_strumconfig()

    def __set_course_mute_enable(self,v):
        self.strumconfig.set_course_mute_enable(v)
        self.__update_strumconfig()

    def __set_strum_mute_threshold(self,v):
        self.strumconfig.set_strum_mute_threshold(v)
        self.__update_strumconfig()

    def __set_strum_mute_interval(self,v):
        self.strumconfig.set_strum_mute_interval(v)
        self.__update_strumconfig()

    def __set_strum_mute_enable(self,v):
        self.strumconfig.set_strum_mute_enable(v)
        self.__update_strumconfig()

    def __set_pulloff_threshold(self,v):
        self.strumconfig.set_pulloff_threshold(v)
        self.__update_strumconfig()

    def __set_pulloff_interval(self,v):
        self.strumconfig.set_pulloff_interval(v)
        self.__update_strumconfig()

    def __set_pulloff_enable(self,v):
        self.strumconfig.set_pulloff_enable(v)
        self.__update_strumconfig()

    def __set_breath_modulation(self,a,subj,course):
        number = int(action.mass_quantity(course))
        self.strumconfig.add_breath_course(number)
        self.__update_strumconfig()

    def __unset_breath_modulation(self,a,subj,course):
        number = int(action.mass_quantity(course))
        self.strumconfig.remove_breath_course(number)
        self.__update_strumconfig()

    def __set_open_course_key(self,a,subj,key,course):
        number = int(action.mass_quantity(course))
        key = int(action.mass_quantity(key))
        self.strumconfig.set_open_course_key(number,key)
        self.__update_strumconfig()

    def __set_open_course_physical(self,a,subj,phys,course):
        number = int(action.mass_quantity(course))
        open_col,open_row = action.coord_value(phys)
        self.strumconfig.set_open_course_physical(number,open_col,open_row)
        self.__update_strumconfig()

    def __add_strum_key(self,a,subj,phys,course):
        number = int(action.mass_quantity(course))
        strum_col,strum_row = action.coord_value(phys)
        self.strumconfig.add_key_course(number,strum_col,strum_row)
        self.__update_strumconfig()

    def __clear_strum_key(self,a,subj,course):
        number = int(action.mass_quantity(course))
        self.strumconfig.clear_key_course(number)
        self.__update_strumconfig()

    def __set_courses(self,v):
        self[3].set_value(v)
        courses = logic.parse_clause(v)
        self.strumconfig.clear_breath_courses()
        self.strumconfig.clear_key_courses()
        self.strumconfig.clear_open_courses_info()
        for c in courses:
            number = int(c[0])
            breath_mod = ('true' == c[1].lower())
            if breath_mod:
                self.strumconfig.add_breath_course(number)
            open_key = int(c[2])
            open_col = int(c[3])
            open_row = int(c[4])
            self.strumconfig.set_open_course_info(number,open_key,open_col,open_row)
            for strum_key in c[5]:
                self.strumconfig.add_key_course(number,strum_key[0],strum_key[1])
        self.__update_strumconfig()

agent.main(Agent)
