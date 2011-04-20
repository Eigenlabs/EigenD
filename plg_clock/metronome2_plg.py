
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

from pi import agent,atom,action,domain,bundles,utils,logic,node
from plg_clock import metronome2_version as version
import clock_native
import piw

class TempoChange(node.Server):
    def __init__(self,clock):
        node.Server.__init__(self,change=self.__change)
        self.__clock = clock
        self.__time = None

    def __change(self,value):
        print 'change:',value
        if value.is_string():
            a = value.as_string().split(':')
            if len(a)==2 and a[0] and a[1]:
                self.setup(float(a[0]),float(a[1]))
            return
        self.clear()

    def clear(self):
        if self.__time is not None:
            self.__clock.del_change(self.__time,self.__tempo)
            self.__time = None

    def setup(self,time,tempo):
        self.clear()
        self.__time = time
        self.__tempo = tempo
        self.__clock.add_change(self.__time,self.__tempo)
        self.set_data(piw.makestring('%f:%f' % (time,tempo),0))

    def compare(self,time,tempo):
        return self.__time == time and self.__tempo == tempo

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,signature=version,names='metronome', ordinal=ordinal)

        self.beat_changes = node.Server(creator=self.__beat_creator,wrecker=self.__beat_wrecker)
        self.bar_changes = node.Server(creator=self.__bar_creator,wrecker=self.__bar_wrecker)

        self.set_private(node.Server())
        self.get_private()[1] = self.beat_changes
        self.get_private()[2] = self.bar_changes

        self[2] = atom.Atom()
        self[2][1] = bundles.Output(1, False, names='running output')
        self[2][2] = bundles.Output(2, False, names='time output')
        self[2][3] = bundles.Output(3, False, names='song beat output')
        self[2][4] = bundles.Output(5, False, names='tempo output')
        self[2][5] = bundles.Output(8, False, names='bar beat output')

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.output = bundles.Splitter(self.domain,*self[2].values())
        self.barclock = clock_native.clock(6,1.0,4.0,self.output.cookie(),self.domain)
        self.beatclock = clock_native.clock(2,-60.0,60.0,self.barclock.cookie(),self.domain)
        self.input = bundles.ScalarInput(self.beatclock.cookie(), self.domain, signals=(1,2))

        self[1] = atom.Atom()
        self[1][1] = atom.Atom(domain=domain.Bool(), init=False, policy=self.input.policy(1,False), names='running input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(0,10000.0), policy=self.input.policy(2,False), names='time input')

        self[3] = atom.Atom()
        self[3][1] = action.Verb('set(none,none,role(at,[mass([second])]),role(to,[abstract]),role(none,[matches([tempo])]))', lambda s,a,t,x: self.__add(a,t))
        self[3][2] = action.Verb('set(dont,none,role(at,[mass([second])]),role(to,[abstract]),role(none,[matches([tempo])]))', lambda s,a,t,x: self.__del(a,t))

    def __beat_creator(self,index):
        return TempoChange(self.beatclock)

    def __beat_wrecker(self,index,object):
        object.clear()

    def __bar_creator(self,index):
        return TempoChange(self.barclock)

    def __bar_wrecker(self,index,object):
        object.clear()

    def __add(self,time,value):
        print 'add tempo change to',value,'at',time
        time = action.mass_quantity(time)
        value = action.mass_quantity(value)
        print 'add tempo change to',value,'at',time
        c = TempoChange(self.beatclock)
        self.beat_changes.add(c)
        c.setup(time,value)

    def __del(self,time,value):
        time = action.mass_quantity(time)
        value = action.mass_quantity(value)
        print 'del tempo change to',value,'at',time
        for (i,c) in self.beat_changes.iteritems():
            if c.compare(time,value):
                del self.beat_changes[i]
                c.clear()
                return

agent.main(Agent)
