
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
from . import strummer_version as version
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
        self.strummer = piw.strummer(self.output.cookie(),self.domain)
        self.strum_input = bundles.VectorInput(self.strummer.strum_cookie(),self.domain,signals=(1,2,3,4,5,))
        self.data_input = bundles.VectorInput(self.strummer.data_cookie(),self.domain,signals=(1,2,3,4,))

        self[4]=atom.Atom(names='inputs')

        self[4][1]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.strum_input.vector_policy(1,False),names='strum breath input',protocols='nostage')
        self[4][2]=atom.Atom(domain=domain.Aniso(),policy=self.strum_input.vector_policy(2,False),names='strum key input',protocols='nostage')
        self[4][3]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.strum_input.merge_policy(3,False),names='strum pressure input',protocols='nostage')
        self[4][4]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.strum_input.merge_policy(4,False),names='strum roll input',protocols='nostage')
        self[4][5]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.strum_input.merge_policy(5,False),names='strum yaw input',protocols='nostage')

        self[4][6]=atom.Atom(domain=domain.Aniso(),policy=self.data_input.vector_policy(1,False),names='key input',protocols='nostage')
        self[4][7]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.data_input.merge_policy(2,False),names='pressure input',protocols='nostage')
        self[4][8]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.data_input.merge_policy(3,False),names='roll input',protocols='nostage')
        self[4][9]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.data_input.merge_policy(4,False),names='yaw input',protocols='nostage')

        self[2]=atom.Atom(names='controls')
        self[2][1] = atom.Atom(domain=domain.Bool(),init=True,names='enable',policy=atom.default_policy(self.strummer.enable))
        self[2][2] = atom.Atom(domain=domain.BoundedInt(0,2000),init=0,names='trigger window',policy=atom.default_policy(self.strummer.set_trigger_window))
        self[2][3] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='strum breath scale',policy=atom.default_policy(self.strummer.set_strum_breath_scale))
        self[2][4] = atom.Atom(domain=domain.BoundedFloat(0,10),init=0.0,names='pressure scale',policy=atom.default_policy(self.strummer.set_pressure_scale))
        self[2][5] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='strum pressure scale',policy=atom.default_policy(self.strummer.set_strum_pressure_scale))
        self[2][6] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='roll scale',policy=atom.default_policy(self.strummer.set_roll_scale))
        self[2][7] = atom.Atom(domain=domain.BoundedFloat(0,10),init=0.0,names='strum roll scale',policy=atom.default_policy(self.strummer.set_strum_roll_scale))
        self[2][8] = atom.Atom(domain=domain.BoundedFloat(0,10),init=1.0,names='yaw scale',policy=atom.default_policy(self.strummer.set_yaw_scale))
        self[2][9] = atom.Atom(domain=domain.BoundedFloat(0,10),init=0.0,names='strum yaw scale',policy=atom.default_policy(self.strummer.set_strum_yaw_scale))
        self[2][10] = atom.Atom(domain=domain.String(), init='[]', names='breath course map', policy=atom.default_policy(self.__set_breath_course_map))
        self[2][11] = atom.Atom(domain=domain.String(), init='[]', names='key course map', protocols='keytocourse', policy=atom.default_policy(self.__set_key_course_map))
        self[2][12] = atom.Atom(domain=domain.Bool(),init=True,names='strum note end',policy=atom.default_policy(self.strummer.set_strum_note_end))

        self.strummer.enable(True)
        self.strummer.set_trigger_window(0)
        self.strummer.set_strum_breath_scale(1.0)
        self.strummer.set_pressure_scale(0.0)
        self.strummer.set_strum_pressure_scale(1.0)
        self.strummer.set_roll_scale(1.0)
        self.strummer.set_strum_roll_scale(0.0)
        self.strummer.set_yaw_scale(1.0)
        self.strummer.set_strum_yaw_scale(0.0)
        self.__set_breath_course_map('[]')
        self.__set_key_course_map('[]')
        self.strummer.set_strum_note_end(True)

    def __set_breath_course_map(self,value):
        self[2][10].set_value(value)
        mapping = logic.parse_clause(value)
        self.strummer.clear_breath_courses()
        for i in mapping:
            self.strummer.add_breath_course(i)

    def __set_key_course_map(self,value):
        self[2][11].set_value(value)
        mapping = logic.parse_clause(value)
        self.strummer.clear_key_courses()
        for i in mapping:
            if 3 == len(i):
                self.strummer.add_key_course(i[0],i[1],i[2])

agent.main(Agent)
