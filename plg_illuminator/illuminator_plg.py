#
# Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com
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

from pi import agent,atom,domain,policy,bundles,logic,action,utils
from . import illuminator_version as version

import piw

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='illuminator', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[1] = bundles.Output(1, False, names='light output',protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1])

        self.status_buffer = piw.statusbuffer(self.output.cookie())
        self.status_buffer.autosend(False)
 
        self[2] = atom.Atom(domain=domain.String(), init='[]', names='physical light map', policy=atom.default_policy(self.__physical_light_map))
        self[3] = atom.Atom(domain=domain.String(), init='[]', names='musical light map', policy=atom.default_policy(self.__musical_light_map))

        self.keyfunctor = piw.functor_backend(1, False)
        self.keyinput = bundles.VectorInput(self.keyfunctor.cookie(), self.domain, signals=(1,))
        self[4] = atom.Atom(domain=domain.Aniso(), policy=self.keyinput.vector_policy(1,False), names='key input')
        self.choicefunctor = utils.make_change_nb(piw.slowchange(utils.changify(self.__choice)))

        self.add_verb2(1,'clear([],None)', callback=self.__clear)
        self.add_verb2(2,'clear([],None,role(None,[matches([physical])]))', callback=self.__clear_physical)
        self.add_verb2(3,'clear([],None,role(None,[matches([musical])]))', callback=self.__clear_musical)
        self.add_verb2(4,'set([],None,role(None,[coord(physical,[row],[column])]),role(to,[abstract,matches([red])]))', callback=self.__set_physical)
        self.add_verb2(5,'set([],None,role(None,[coord(physical,[row],[column])]),role(to,[abstract,matches([green])]))', callback=self.__set_physical)
        self.add_verb2(6,'set([],None,role(None,[coord(physical,[row],[column])]),role(to,[abstract,matches([orange])]))', callback=self.__set_physical)
        self.add_verb2(7,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([red])]))', callback=self.__set_musical)
        self.add_verb2(8,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([green])]))', callback=self.__set_musical)
        self.add_verb2(9,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([orange])]))', callback=self.__set_musical)
        self.add_verb2(10,'set([un],None,role(None,[coord(physical,[row],[column])]))', callback=self.__unset_physical)
        self.add_verb2(11,'set([un],None,role(None,[coord(musical,[course],[key])]))', callback=self.__unset_musical)
        self.add_verb2(12,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([red])]))',self.__choose_physical)
        self.add_verb2(13,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([green])]))',self.__choose_physical)
        self.add_verb2(14,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([orange])]))',self.__choose_physical)
        self.add_verb2(15,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([red])]))',self.__choose_musical)
        self.add_verb2(16,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([green])]))',self.__choose_musical)
        self.add_verb2(17,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([orange])]))',self.__choose_musical)
        self.add_verb2(18,'choose([un],None)',self.__unchoose)
 
    def __physical_light_map(self,v):
        self[2].set_value(v)
        self.__update_lights()

    def __musical_light_map(self,v):
        self[3].set_value(v)
        self.__update_lights()

    def __clear(self,subj):
        self[2].set_value('[]')
        self[3].set_value('[]')
        self.__update_lights()

    def __clear_physical(self,subj,v):
        self[2].set_value('[]')
        self.__update_lights()

    def __clear_musical(self,subj,v):
        self[3].set_value('[]')
        self.__update_lights()

    def __set_physical(self,subject,key,colour):
        row,column = action.coord_value(key)
        colour = action.abstract_string(colour)
        phys = [x for x in logic.parse_clause(self[2].get_value()) if x[0][0] != row and x[0][1] != column]
        phys.append([[row,column],colour])
        self[2].set_value(logic.render_term(phys))
        self.__update_lights()

    def __set_musical(self,subject,key,colour):
        course,key = action.coord_value(key)
        colour = action.abstract_string(colour)
        mus = [x for x in logic.parse_clause(self[3].get_value()) if x[0][0] != course and x[0][1] != key]
        mus.append([[course,key],colour])
        self[3].set_value(logic.render_term(mus))
        self.__update_lights()

    def __unset_physical(self,subject,key):
        row,column = action.coord_value(key)
        phys = [x for x in logic.parse_clause(self[2].get_value()) if x[0][0] != row or x[0][1] != column]
        self[2].set_value(logic.render_term(phys))
        self.__update_lights()

    def __unset_musical(self,subject,key):
        course,key = action.coord_value(key)
        mus = [x for x in logic.parse_clause(self[3].get_value()) if x[0][0] != course or x[0][1] != key]
        self[3].set_value(logic.render_term(mus))
        self.__update_lights()

    def __update_lights(self):
        self.status_buffer.clear()
        self.__add_lights(False,self[2].get_value())
        self.__add_lights(True,self[3].get_value())
        self.status_buffer.send()
        return True

    def __colour_to_int(self,v):
        colour = str(v).lower()
        if colour == 'red' or colour == 'r':
            colour = 2 
        elif colour == 'green' or colour == 'g':
            colour = 1
        elif colour == 'orange' or colour == 'o':
            colour = 3 
        elif colour == 'off':
            colour = 0
        colour = int(colour)
        if colour < 0 or colour > 3:
            colour = 0
        return colour

    def __add_lights(self,musical,v):
        mapping = logic.parse_clause(v)
        for m in mapping:
            if 2 == len(m) and 2 == len(m[0]):
                self.status_buffer.set_status(musical,int(m[0][0]),int(m[0][1]),self.__colour_to_int(m[1]))

    def __choose_physical(self,subject,key,colour):
        self.__choose_base(False,action.abstract_string(colour))

    def __choose_musical(self,subject,key,colour):
        self.__choose_base(True,action.abstract_string(colour))

    def __choose_base(self,musical,colour):
        self.__choices = []
        self.__choosemusical = musical
        self.__choosecolour = colour

        self.status_buffer.clear()
        self.status_buffer.send()
        self.keyfunctor.set_gfunctor(self.choicefunctor)

    def __choice(self,v):
        choice = utils.key_to_lists(v)
        if not choice: return
        if not choice[4]: return
        # remove the hardness so that it's not part of the identity of the choice
        del choice[4]

        # if this choice is the same as the previous one
        # stop choose mode and store the new mapping
        if self.__choices and choice==self.__choices[-1]:
            self.__stop_choosing()
            self.__add_choices()
            return

        # if this is a new choice, store it and adapt the status leds
        if not choice in self.__choices:
            self.__choices.append(choice)
            if self.__choosemusical:
                key = choice[3]
            else:
                key = choice[1]
            self.status_buffer.set_status(self.__choosemusical,key[0],key[1],self.__colour_to_int(self.__choosecolour))
            self.status_buffer.send()

    def __stop_choosing(self):
        self.keyfunctor.clear_gfunctor()

    def __unchoose(self,subject):
        self.__stop_choosing()
        self.__update_lights()

    def __add_choices(self):
        if self.__choosemusical:
            existing = list(logic.parse_clause(self[3].get_value()))
            for choice in self.__choices:
                existing.append([[int(choice[3][0]),int(choice[3][1])],self.__choosecolour])
            self[3].set_value(logic.render_term(existing))
        else:
            existing = list(logic.parse_clause(self[2].get_value()))
            for choice in self.__choices:
                existing.append([[int(choice[1][0]),int(choice[1][1])],self.__choosecolour])
            self[2].set_value(logic.render_term(existing))

        self.__update_lights()


agent.main(Agent)

