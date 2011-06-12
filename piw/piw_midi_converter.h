/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MIDI_CONVERTER_H_
#define MIDI_CONVERTER_H_

#include <piw/piw_exports.h>
#include <piw/piw_midi_from_belcanto.h>
#include <piw/piw_gui_mapper.h>
#include <piw/piw_control_mapping.h>

namespace piw
{
    struct PIW_DECLSPEC_CLASS midi_converter_t
    {
        midi_converter_t(piw::mapping_observer_t &, piw::midi_channel_delegate_t &, piw::clockdomain_ctl_t *, piw::midi_from_belcanto_t &, const std::string &);
        ~midi_converter_t();

        piw::clockdomain_ctl_t *clock_domain();

        piw::cookie_t parameter_input(unsigned);
        void set_mapping(const std::string &);
        std::string get_mapping();
        void parameter_name_changed(unsigned);
        void map_param(unsigned, piw::mapping_info_t);
        void map_midi(unsigned, piw::mapping_info_t);
        void unmap_param(unsigned, unsigned);
        void unmap_midi(unsigned, unsigned);
        bool is_mapped_param(unsigned, unsigned);
        bool is_mapped_midi(unsigned, unsigned);
        piw::mapping_info_t get_info_param(unsigned, unsigned);
        piw::mapping_info_t get_info_midi(unsigned, unsigned);
        void set_minimum_decimation(float);
        void set_midi_notes(bool);
        void set_midi_pitchbend(bool);
        void set_midi_channel(unsigned);
        void set_min_midi_channel(unsigned);
        void set_max_midi_channel(unsigned);
        void set_program_change(unsigned);
        void set_bank_change(unsigned);
        void close();

        int gc_clear();
        int gc_traverse(void *,void *);

        class impl_t;
    private:
        impl_t *impl_;
    };

}; // namespace piw

#endif /* MIDI_CONVERTER_H_ */
