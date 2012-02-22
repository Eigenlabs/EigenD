
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

from pi import agent,atom,action,bundles,domain,node,upgrade
from . import recorder_version as version,recorder_native

import piw
import picross

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        agent.Agent.__init__(self, signature=version, names='player', ordinal=ordinal)

        self[2] = atom.Atom(names='outputs')
        self[2][1] = bundles.Output(1,False,names='pressure output', protocols='')
        self[2][2] = bundles.Output(2,False,names='key output', protocols='')

        self.output = bundles.Splitter(self.domain,self[2][1],self[2][2])
        self.player = recorder_native.nplayer(self.output.cookie(),16,1,2,self.domain)

        self.ctl_fb = piw.functor_backend(1,True)
        self.ctl_fb.set_functor(piw.pathnull(0),self.player.control())
        self.ctl_input = bundles.ScalarInput(self.ctl_fb.cookie(),self.domain,signals=(1,))

        self[1] = atom.Atom(names='inputs')
        self[1][1]=atom.Atom(domain=domain.Aniso(), policy=self.ctl_input.policy(1,False),names='controller input')

        self.add_verb2(1,'play([],None,role(None,[mass([note])]),role(with,[mass([velocity])]))',create_action=self.__playnv,clock=True)
        self.add_verb2(2,'play([],None,role(None,[mass([note])]),role(with,[mass([velocity])]),role(for,[mass([second])]))',create_action=self.__playnvl,clock=True)


    def __playnvl(self,ctx,subj,note,velocity,s):
        note = action.mass_quantity(note)
        velocity = action.mass_quantity(velocity)
        length = max(1,int(1000000*action.mass_quantity(s)))
        print 'play note',note,'velocity',velocity,'sec',s,'us',length
        return self.player.play(note,velocity,length),None

    def __playnv(self,ctx,subj,note,velocity):
        note = action.mass_quantity(note)
        velocity = action.mass_quantity(velocity)
        print 'play note',note,'velocity',velocity
        return self.player.play(note,velocity,500000),None


agent.main(Agent)
