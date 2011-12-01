
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
from plg_stk import sax_oscillator_version as version

def filtered_policy():
    return policy.LopassStreamPolicy(1000,0.97)

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        # TODO: change to blownstring, change in SConscript also!
        agent.Agent.__init__(self, signature=version,names='sax oscillator',ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()

        self[1] = bundles.Output(1,True,names="audio output")

        self.output = bundles.Splitter(self.domain, self[1])
        self.inst = stk_native.blownstring(self.output.cookie(),self.domain)
        self.input = bundles.VectorInput(self.inst.cookie(), self.domain,signals=(1,2,3,4,5,6,7))

        param=(T('inc',0.02),T('biginc',0.2),T('control','updown'))
        self[2] = atom.Atom()
        self[2][1] = atom.Atom(names='activation input',domain=domain.BoundedFloat(0,1), policy=self.input.merge_policy(1,False))
        self[2][2] = atom.Atom(names='freakency input',domain=domain.BoundedFloat(1,96000), policy=self.input.vector_policy(2,filtered_policy()))
        self[2][3] = atom.Atom(names='pressure input', domain=domain.BoundedFloat(0,1), policy=self.input.merge_policy(3,False))
        self[2][4] = atom.Atom(names='reed stiffness input', domain=domain.BoundedFloat(0,1,hints=param), init=0.5, policy=self.input.merge_policy(4,False))
        self[2][5] = atom.Atom(names='noise gain input', domain=domain.BoundedFloat(0,1,hints=param), init=0.3, policy=self.input.merge_policy(5,False))
        self[2][6] = atom.Atom(names='blow position input', domain=domain.BoundedFloat(0,1,hints=param), init=0.45, policy=self.input.merge_policy(6,False))
        self[2][7] = atom.Atom(names='vibrato gain input', domain=domain.BoundedFloat(0,1,hints=param), init=0, policy=self.input.merge_policy(7,False))


agent.main(Agent)
