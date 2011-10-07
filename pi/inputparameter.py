
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

import re, piw
from pi import action,atom,bundles,domain,errors,const

# The parameter delegate requires the following methods:
#
# cookie parameter_input(i)          : which gives access to a cookie for a distinct parameter input
# void parameter_name_changed(i)     : notification that the name of a parameter input has changed
# void map_param(i,mapping_info)     : which sets up a host automation mapping for a parameter
# void map_midi(i,mapping_info)      : which sets up a midi mapping for a parameter
# void unmap_param(i,j)              : which removes a parameter mapping for host automation
# void unmap_midi(i,j)               : which removes a parameter mapping for midi
# bool is_mapped_param(i,j)          : which checks if a parameter mapping exists for host automation
# bool is_mapped_midi(i,j)           : which checks if a parameter mapping exists for midi
# mapping_info get_info_param(i,j)   : which retrieves the info of a parameter mapping for host automation
# mapping_info get_info_midi(i,j)    : which retrieves the info of a parameter mapping for midi
# void set_minimum_decimation(float) : which sets the minimum data decimation rate
# void set_midi_notes(bool)          : which enables or disables midi notes
# void set_midi_pitchbend(bool)      : which enables or disables midi pitchbend
class Parameter(atom.Atom):
    def __init__(self,k,delegate,clockdomain):
        self.__delegate = delegate
        self.__input = bundles.VectorInput(self.__delegate.parameter_input(k),clockdomain,signals=(1,))
        self.__index = k
        atom.Atom.__init__(self,domain=domain.Aniso(),policy=self.__input.vector_policy(1,False),
                            names='parameter',protocols='input explicit',ordinal=k)

    def property_change(self,key,value):
        if key in ['name','ordinal']:
            self.__delegate.parameter_name_changed(self.__index)


class List(atom.Atom):
    def __init__(self,delegate,clockdomain,verbcontainer):
        self.__delegate = delegate
        atom.Atom.__init__(self, container=(None,'parameters',verbcontainer))
        # legacy verbs
        self.add_verb2(1,'map([],~a,role(None,[abstract]),role(to,[numeric]),role(with,[numeric]))',callback=self.__map_legacy)
        self.add_verb2(2,'map([un],~a,role(None,[abstract]),role(to,[numeric]))',callback=self.__unmap)
        # current mapping verbs
        self.add_verb2(3,'map([un],~a,role(None,[abstract]),role(for,[abstract]))',callback=self.__unmap)
        self.add_verb2(4,'map([],~a,role(None,[abstract]),role(for,[abstract]))',callback=self.__map)
        self.add_verb2(5,'set([],~a,role(None,[matches([enable])]),role(from,[abstract]),role(for,[abstract]))',callback=self.__set_enable)
        self.add_verb2(6,'set([un],~a,role(None,[matches([enable])]),role(from,[abstract]),role(for,[abstract]))',callback=self.__unset_enable)
        self.add_verb2(7,'set([],~a,role(None,[matches([scale])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_scale)
        self.add_verb2(8,'set([],~a,role(None,[matches([low])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_low)
        self.add_verb2(9,'set([],~a,role(None,[matches([base])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_base)
        self.add_verb2(10,'set([],~a,role(None,[matches([high])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_high)
        self.add_verb2(11,'set([],~a,role(None,[matches([decimation])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_decimation)
        self.add_verb2(12,'set([],~a,role(None,[matches([scope])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_scope)
        self.add_verb2(13,'set([],~a,role(None,[matches([resolution])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_resolution)
        self.add_verb2(14,'set([],~a,role(None,[matches([secondary])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_secondary)
        self.add_verb2(15,'set([],~a,role(None,[matches([origin,return])]),role(from,[abstract]),role(for,[abstract]))',callback=self.__set_origin_return)
        self.add_verb2(16,'set([un],~a,role(None,[matches([origin,return])]),role(from,[abstract]),role(for,[abstract]))',callback=self.__unset_origin_return)
        self.add_verb2(17,'set([],~a,role(None,[matches([curve])]),role(from,[abstract]),role(for,[abstract]),role(to,[abstract]))',callback=self.__set_curve)
        self.add_verb2(18,'set([],~a,role(None,[matches([minimum,decimation])]),role(to,[numeric]))',callback=self.__set_minimum_decimation)
        self.add_verb2(19,'set([],~a,role(None,[matches([midi,notes])]))',callback=self.__set_notes)
        self.add_verb2(20,'set([un],~a,role(None,[matches([midi,notes])]))',callback=self.__unset_notes)
        self.add_verb2(21,'set([],~a,role(None,[matches([midi,pitch,bend])]))',callback=self.__set_pitchbend)
        self.add_verb2(22,'set([un],~a,role(None,[matches([midi,pitch,bend])]))',callback=self.__unset_pitchbend)
        for i in range(1,33):
            self[i] = Parameter(i,delegate,clockdomain)

    def __map_legacy(self,a,fr,fo,to):
        return self.__set_scale(a,'scale',fr,fo,to)

    def __unmap(self,a,fr,fo):
        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.unmap_param(iparam,oparam)
            elif midi != -1:
                self.__delegate.unmap_midi(iparam,midi)
        except RuntimeError,e:
            return e.message

    def __map(self,a,fr,fo):
        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info)
            elif midi != -1:
                self.__delegate.map_midi(iparam,info)
        except RuntimeError,e:
            return e.message

    def __set_enable(self,a,prop,fr,fo):
        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_enabled(True))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_enabled(True))
        except RuntimeError,e:
            return e.message

    def __unset_enable(self,a,prop,fr,fo):
        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_enabled(False))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_enabled(False))

        except RuntimeError,e:
            return e.message

    def __set_scale(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to)
        to_val = float(to_str)
        if to_val < -10 or to_val > 10:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_scale(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_scale(to_val))
        except RuntimeError,e:
            return e.message

    def __set_low(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to)
        to_val = float(to_str)/100.0
        if to_val < 0 or to_val > 1:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_lo(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_lo(to_val))
        except RuntimeError,e:
            return e.message

    def __set_base(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to)
        to_val = float(to_str)/100.0
        if to_val < 0 or to_val > 1:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_base(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_base(to_val))
        except RuntimeError,e:
            return e.message

    def __set_high(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to)
        to_val = float(to_str)/100.0
        if to_val < 0 or to_val > 1:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_hi(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_hi(to_val))
        except RuntimeError,e:
            return e.message

    def __set_decimation(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to)
        to_val = float(to_str)
        if to_val < 0 or to_val > 100:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_decimation(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_decimation(to_val))
        except RuntimeError,e:
            return e.message

    def __set_scope(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to).lower()
        to_val = -1;
        to_match = None
        if to_str == 'note':
            to_val = const.scope_pernote;
        elif to_str == 'global':
            to_val = const.scope_global;
        else:
            to_match = re.match('^channel\s(\d+)$',to_str,re.IGNORECASE)
            if to_match:
                to_val = int(to_match.group(1))
                if to_val < 1 or to_val > 16:
                    return errors.invalid_thing(to_str, 'set')
            else:
                return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if to_match:
                if oparam != -1:
                    self.__delegate.map_param(iparam,info.clone_with_channelscope(to_val))
                elif midi != -1:
                    self.__delegate.map_midi(iparam,info.clone_with_channelscope(to_val))
            else:
                if oparam != -1:
                    self.__delegate.map_param(iparam,info.clone_with_scope(to_val))
                elif midi != -1:
                    self.__delegate.map_midi(iparam,info.clone_with_scope(to_val))
        except RuntimeError,e:
            return e.message

    def __set_resolution(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to).lower()
        to_val = int(to_str)
        if to_val != const.resolution_bits_7 and to_val != const.resolution_bits_14:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_resolution(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_resolution(to_val))
        except RuntimeError,e:
            return e.message

    def __set_secondary(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to)
        to_val = int(to_str)
        if to_val < 0 or to_val > 127:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_secondarycc(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_secondarycc(to_val))
        except RuntimeError,e:
            return e.message

    def __set_origin_return(self,a,prop,fr,fo):
        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_origin_return(True))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_origin_return(True))
        except RuntimeError,e:
            return e.message

    def __unset_origin_return(self,a,prop,fr,fo):
        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_origin_return(False))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_origin_return(False))

        except RuntimeError,e:
            return e.message

    def __set_curve(self,a,prop,fr,fo,to):
        to_str = action.abstract_string(to).lower()

        if 'linear'==to_str:
            to_val = 0;
        elif 'cubic'==to_str:
            to_val = 1;
        elif 'quadratic'==to_str:
            to_val = 2;
        elif 'step'==to_str:
            to_val = 3;
        else:
            return errors.invalid_thing(to_str, 'set')

        try:
            (iparam,oparam,midi,info) = self.__map_from_for(fr,fo)
            if oparam != -1:
                self.__delegate.map_param(iparam,info.clone_with_curve(to_val))
            elif midi != -1:
                self.__delegate.map_midi(iparam,info.clone_with_curve(to_val))
        except RuntimeError,e:
            return e.message

    def __map_from_for(self,fr,fo):
        from_str = action.abstract_string(fr)
        for_str = action.abstract_string(fo).lower()

        iparam_number = 0
        oparam_number = -1
        midi_number = -1

        # determine the input parameter number
        from_match = re.match('^parameter\s+(\d+)$',from_str,re.IGNORECASE)
        if not from_match:
            for i in range(1,17):
                param_name = self[i].get_property_string('name')
                param_ordinal = self[i].get_property_long('ordinal')
                if param_ordinal:
                    param_name = param_name+' '+str(param_ordinal)
                if from_str.lower() == param_name.lower():
                    iparam_number = i
                    break;
        else:
            iparam_number = int(from_match.group(1))

        if iparam_number < 1 or iparam_number > 16:
            raise RuntimeError(errors.invalid_thing(from_str, 'map'))

        # determine the output parameter or midi mapping
        oparam_match = re.match('^\d+$',for_str,re.IGNORECASE)
        midi_match = re.match('^midi\s(\d+)$',for_str,re.IGNORECASE)
        if oparam_match:
            oparam_number = int(oparam_match.group(0))
        elif midi_match:
            midi_number = int(midi_match.group(1))
        else:
            if for_str == "polyphonic aftertouch":
                midi_number = 128
            elif for_str == "program change":
                midi_number = 129
            elif for_str == "channel aftertouch":
                midi_number = 130
            elif for_str == "pitch wheel":
                midi_number = 131
            elif for_str == "legato trigger":
                midi_number = 132

        # validate the results
        if oparam_number < 0 and (midi_number < 0 or midi_number > 131):
            raise RuntimeError(errors.invalid_thing(for_str, 'map'))

        print 'iparam:',iparam_number,'oparam:',oparam_number,'midi:',midi_number

        info = None
        if oparam_number != -1:
            if self.__delegate.is_mapped_param(iparam_number,oparam_number):
                info = self.__delegate.get_info_param(iparam_number,oparam_number);
            else:
                info = piw.mapping_info(oparam_number);
        elif midi_number != -1:
            if self.__delegate.is_mapped_midi(iparam_number,midi_number):
                info = self.__delegate.get_info_midi(iparam_number,midi_number)
            else:
                info = piw.mapping_info(midi_number);

        return (iparam_number,oparam_number,midi_number,info)

    def __set_minimum_decimation(self,a,prop,to):
        try:
            to_str = action.abstract_string(to)
            to_val = float(to_str)
            self.__delegate.set_minimum_decimation(to_val)
        except RuntimeError,e:
            return e.message

    def __set_notes(self,a,prop):
        try:
            self.__delegate.set_midi_notes(True)
        except RuntimeError,e:
            return e.message

    def __unset_notes(self,a,prop):
        try:
            self.__delegate.set_midi_notes(False)
        except RuntimeError,e:
            return e.message

    def __set_pitchbend(self,a,prop):
        try:
            self.__delegate.set_midi_pitchbend(True)
        except RuntimeError,e:
            return e.message

    def __unset_pitchbend(self,a,prop):
        try:
            self.__delegate.set_midi_pitchbend(False)
        except RuntimeError,e:
            return e.message
