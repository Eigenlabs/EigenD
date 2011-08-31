
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
import stk_native
from pi.logic.shortcuts import T
from pi import agent,atom,domain,bundles,action,async,utils,upgrade,policy
from plg_stk import cello_oscillator_version as version

def filtered_policy():
    return policy.LopassStreamPolicy(1000,0.98)

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version,names='cello oscillator',ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()

        self[1] = bundles.Output(1,True,names="audio output")

        self.output = bundles.Splitter(self.domain, self[1])
        self.inst = stk_native.cello(self.output.cookie(),self.domain)
        self.input = bundles.VectorInput(self.inst.cookie(), self.domain,signals=(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24))
        
        #self.bow_input = bundles.ScalarInput(self.inst.bow_cookie(), self.domain, signals=(1,2))

        param=(T('inc',0.02),T('biginc',0.2),T('control','updown'))
        self[2] = atom.Atom(names='inputs')
        # playing inputs
        # bow activation
        self[2][1] = atom.Atom(names='activation input', domain=domain.BoundedFloat(0,1), policy=self.input.merge_policy(1,False),protocols='nostage')
        # frequency
        self[2][2] = atom.Atom(names='frequency input', domain=domain.BoundedFloat(1,96000), policy=self.input.vector_policy(2,False))
        # bow pressure
        self[2][3] = atom.Atom(names='pressure input', domain=domain.BoundedFloat(0,1), policy=self.input.merge_policy(3,False),protocols='nostage')
        # bow position, tranverse position of bow relative to string
        self[2][4] = atom.Atom(names='bow position input', domain=domain.BoundedFloat(-1,1), policy=self.input.merge_nodefault_policy(22,False))
        # bow velocity
        self[2][5] = atom.Atom(names='bow velocity input', domain=domain.BoundedFloat(-1,1), policy=self.input.merge_nodefault_policy(23,False))
        # pitch sweep time (portomento) for mono mode
        self[2][6] = atom.Atom(names='pitch time input', domain=domain.BoundedFloat(0,100000,hints=param), init=10, policy=self.input.merge_policy(4,False))

        # TODO: remove
        # bow width
        #self[2][7] = atom.Atom(names='bow width input', domain=domain.BoundedFloat(0,1,hints=param), init=0, policy=self.input.merge_policy(5,False))

        # minimum frequency limit
        self[2][8] = atom.Atom(names='minimum frequency', domain=domain.BoundedFloat(0.1,20,hints=param), init=20, policy=self.input.merge_policy(6,False))

        # TODO: remove
        # string junction mode
        #self[2][9] = atom.Atom(names="mode", domain=domain.BoundedFloat(1,2), init=1, policy=self.input.merge_policy(7,False))

        
        # string to body filter parameters
        self[2][10] = atom.Atom(names="low filter frequency", domain=domain.BoundedFloat(0,96000), init=400, policy=self.input.merge_policy(8,False))
        self[2][11] = atom.Atom(names="low filter gain", domain=domain.BoundedFloat(-96,24), init=0, policy=self.input.merge_policy(9,False))
        self[2][12] = atom.Atom(names="low filter width", domain=domain.BoundedFloat(0.01,100,1), init=1, policy=self.input.merge_policy(10,False))
        self[2][13] = atom.Atom(names="mid filter frequency", domain=domain.BoundedFloat(0,96000), init=1000, policy=self.input.merge_policy(11,False))
        self[2][14] = atom.Atom(names="mid filter gain", domain=domain.BoundedFloat(-96,24), init=-24, policy=self.input.merge_policy(12,False))
        self[2][15] = atom.Atom(names="mid filter width", domain=domain.BoundedFloat(0.01,100,1), init=1, policy=self.input.merge_policy(13,False))
        self[2][16] = atom.Atom(names="high filter frequency", domain=domain.BoundedFloat(0,96000), init=10000, policy=self.input.merge_policy(14,False))
        self[2][17] = atom.Atom(names="high filter gain", domain=domain.BoundedFloat(-96,24), init=-40, policy=self.input.merge_policy(15,False))
        self[2][18] = atom.Atom(names="high filter width", domain=domain.BoundedFloat(0.01,100,1), init=1, policy=self.input.merge_policy(16,False))

        # string filter parameters
        # TODO: remove
        #self[2][19] = atom.Atom(names="string filter frequency", domain=domain.BoundedFloat(0,96000,1), init=800, policy=self.input.merge_policy(17,False))
        # TODO: remove
        #self[2][20] = atom.Atom(names="string filter width", domain=domain.BoundedFloat(0.01,100,1), init=0.8, policy=self.input.merge_policy(18,False))
        # polyphonic bow velocity, to bow independently with keys
        # TODO: remove
        #self[2][21] = atom.Atom(names='poly bow velocity input', domain=domain.BoundedFloat(-1,1,0), policy=self.input.merge_policy(19,False))
        # TODO: remove
        #self[2][22] = atom.Atom(names='poly bow enable', domain=domain.Bool(), init=False, policy=self.input.merge_policy(20,False))


        # velocity factor, determines velocity of bow from position difference
        self[2][23] = atom.Atom(names='bow velocity factor', domain=domain.BoundedFloat(0,100,1), init=1, policy=self.input.merge_policy(21,False))

#        self[2][70] = atom.Atom(names='parameter number', domain=domain.BoundedInt(0,10), policy=atom.default_policy(self.__set_param_num))
#        self[2][71] = atom.Atom(names='parameter value', domain=domain.BoundedFloat(-100,100), policy=atom.default_policy(self.__set_param_val))

#        self[2][24] = atom.Atom(names='finger pressure input', domain=domain.BoundedFloat(0,1), policy=self.input.merge_policy(22,False))
        
#        self[2][25] = atom.Atom(names='test input', ordinal=1, domain=domain.BoundedFloat(-20,0), init=0, policy=self.input.merge_policy(24,False))
#        self[2][26] = atom.Atom(names='test input', ordinal=2, domain=domain.BoundedFloat(-10,10), init=2, policy=self.input.merge_policy(26,False))

    def __set_mode(self,v):
        self.inst.set_mode(v)

    def __set_cutoff_frequency(self,f):
        self.inst.set_cutoff_frequency(f)

    def __set_param_num(self,n):
        print 'set param num',n
        self.inst.set_param_num(n)
        return False

    def __set_param_val(self,v):
        print 'set param val',v
        self.inst.set_param_val(v)
        return False


class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)

        node = root.get_node(2,23)
        if(node==None):
            print 'cello upgrade warning: no bow velocity factor node found so creating one'
            root.ensure_node(2,23)
            root.ensure_node(2,23,254).set_data(piw.makefloat(1.0,0))
            root.ensure_node(2,23,255)
            root.ensure_node(2,23,255,1)
            root.ensure_node(2,23,255,3)
            root.ensure_node(2,23).setmeta(8,'bow','velocity','factor')
        return True
  
agent.main(Agent,Upgrader)
