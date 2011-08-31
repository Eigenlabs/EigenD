
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

from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node
from plg_simple import stringer_version as version
import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        agent.Agent.__init__(self, signature=version, names='stringer',protocols='bind set',container=(5,'stringer',atom.VerbContainer(clock_domain=self.domain)),ordinal=ordinal)
        
        self[1] = atom.Atom()
        self[1][1] = bundles.Output(1,False,names='activation output', protocols='')
        self[1][2] = bundles.Output(2,False,names='pressure output', protocols='')
        self[1][3] = bundles.Output(3,False,names='roll output', protocols='')
        self[1][4] = bundles.Output(4,False,names='yaw output', protocols='')
        self[1][5] = bundles.Output(5,False,names='k number output', protocols='')
        self[1][6] = bundles.Output(6,False,names='key output', protocols='')

        self[2] = bundles.Output(1,False, names='controller output', continuous=True)

        self.output = bundles.Splitter(self.domain,*self[1].values())
        self.coutput = bundles.Splitter(self.domain,self[2])

        # stringer
        self.stringer = piw.stringer(self.output.cookie())

        # clone controller input to copy it to an output and the stringer
        self.cclone = piw.clone(True) 
        self.cclone.set_output(1,self.coutput.cookie())

        self.ctl_input = bundles.VectorInput(self.cclone.cookie(),self.domain,signals=(1,))
        self.data_input = bundles.VectorInput(self.stringer.data_cookie(),self.domain,signals=(1,2,3,4,6))

        self[4]=atom.Atom()
        self[4][1]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.data_input.merge_policy(1,False),names='activation input',protocols='nostage')
        self[4][2]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.data_input.vector_policy(2,False),names='pressure input',protocols='nostage')
        self[4][3]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.data_input.merge_policy(3,False),names='roll input',protocols='nostage')
        self[4][4]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.data_input.merge_policy(4,False),names='yaw input',protocols='nostage')
        self[4][6]=atom.Atom(domain=domain.Aniso(),policy=self.data_input.merge_policy(6,False), names='key input')
        self[4][5]=atom.Atom(domain=domain.Aniso(),policy=self.ctl_input.vector_policy(1,False),names='controller input')

        # self[5] is the verb container

        # enable, to deactivate the stringer
        self[6] = atom.Atom(domain=domain.Bool(),init=True,names="enable",policy=atom.default_policy(self.__enable))

        # status output to drive the talker lights
        self[7]=bundles.Output(1,False,names='status output')
        self.light_output=bundles.Splitter(self.domain,self[7])
        self.lights=piw.lightsource(piw.change_nb(),0,self.light_output.cookie())
        self.lights.set_size(1)
        self.__set_lights(self[6].get_value())

        # toggle enable verb, has status action to ensure the status output will be connected to a talker
        self.add_verb2(1,'enable([toggle],None)',callback=self.__toggle_enable,status_action=self.__status)

    # set status light colour
    def __set_lights(self,b):
        if b:
            self.lights.set_status(1,const.status_active)
        else:
            self.lights.set_status(1,const.status_inactive)

    # set the enable state
    def __enable(self, b):
        self.__set_lights(b)
        self.stringer.enable(b)
        return True

    # toggle the enable state
    def __toggle_enable(self,arg):
        b = not self[6].get_value()
        print 'toggle enable ->',b
        self.__enable(b)
        self[6].set_value(b)

    # return the status output for connection to a talker status input to control status light
    def __status(self,subj):
        return 'dsc(~(s)"#7",None)'


class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        print 'upgrading stringer',address
        root = tools.get_root(address)
        root.ensure_node(4,6).set_name('key input')
        root.ensure_node(1,5).set_name('k number output')
        root.ensure_node(1,6).set_name('key output')

    def phase2_1_0_1(self,tools,address):
        root = tools.get_root(address)
        root.mimic_connections((4,1),(4,6),'key output')

agent.main(Agent,Upgrader)
