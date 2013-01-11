
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

from pi import agent,atom,bundles,domain,logic,utils
from . import labeler_version as version

import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='labeler', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[1] = atom.Atom(domain=domain.Aniso(hints=(logic.make_term('continuous'),)), names='controller output', policy=atom.readonly_policy())

        self[2] = atom.Atom(domain=domain.String(), init='label', names='category', policy=atom.default_policy(self.set_category))
        self[3] = atom.Atom(domain=domain.String(), init='', names='label', policy=atom.default_policy(self.set_label))

        self.ctl_functor = piw.functor_backend(1, True)
        self.ctl_input = bundles.VectorInput(self.ctl_functor.cookie(), self.domain, signals=(1,))
        self[4] = atom.Atom(domain=domain.Aniso(), policy=self.ctl_input.vector_policy(1,False), names='controller input')
        self.ctl_functor.set_functor(piw.pathnull(0), utils.make_change_nb(piw.slowchange(utils.changify(self.__controller_input))))

        self.__ctl = []
 
    def set_category(self,v):
        self[2].set_value(v)
        self.__update_labels()
 
    def set_label(self,v):
        self[3].set_value(v)
        self.__update_labels()

    def __controller_input(self,c):
        self.__ctl = utils.dict_items(c);
        self.__update_labels()

    def __update_labels(self):
        # extract all control stream entries into a new list
        # extract the labels entry into a deticated list, if it exists
        new_ctl = []
        labels = []
        for e in self.__ctl:
            if e[0] == 'labels':
                labels = utils.tuple_items(e[1])
            else:
                new_ctl.append(e)

        # append the local category and label to the labels list
        category = self[2].get_value()
        label = self[3].get_value()
        if category and len(category) > 0 and label and len(label) > 0:
            labels.append(utils.maketuple([piw.makestring(category,0), piw.makestring(label,0)],0))

        # reintegrate the labels list and set the new control stream value
        new_ctl.append(['labels',utils.maketuple(labels,0)])
        self[1].set_value(utils.makedict(new_ctl,0))

agent.main(Agent)
