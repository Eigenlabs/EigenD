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

#ifndef __MIDI_CONVERTER_H__
#define __MIDI_CONVERTER_H__

#include <midilib_exports.h>
#include <lib_midi/midi_from_belcanto.h>
#include <lib_midi/control_mapper_gui.h>
#include <lib_midi/control_mapping.h>

namespace midi
{
    struct MIDILIB_DECLSPEC_CLASS midi_converter_t
    {
        midi_converter_t(mapping_observer_t &, midi_channel_delegate_t &, piw::clockdomain_ctl_t *, midi_from_belcanto_t &, const std::string &);
        ~midi_converter_t();

        piw::clockdomain_ctl_t *clock_domain();

        piw::cookie_t parameter_input(unsigned);
        void set_mapping(const std::string &);
        void set_title(const std::string &);
        std::string get_mapping();
        void parameter_name_changed(unsigned);
        void map_param(unsigned, mapping_info_t);
        void map_midi(unsigned, mapping_info_t);
        void unmap_param(unsigned, unsigned);
        void unmap_midi(unsigned, unsigned);
        bool is_mapped_param(unsigned, unsigned);
        bool is_mapped_midi(unsigned, unsigned);
        mapping_info_t get_info_param(unsigned, unsigned);
        mapping_info_t get_info_midi(unsigned, unsigned);
        void set_minimum_decimation(float);
        void set_midi_notes(bool);
        void set_midi_pitchbend(bool);
        void set_midi_hires_velocity(bool);
        void set_midi_channel(unsigned);
        void set_min_midi_channel(unsigned);
        void set_max_midi_channel(unsigned);
        void set_program_change(unsigned);
        piw::change_nb_t change_program();
        void set_bank_change(unsigned);
        piw::change_nb_t change_bank();
        void set_cc(unsigned, unsigned);
        piw::change_nb_t change_cc();
        void close();

        int gc_clear();
        int gc_traverse(void *,void *);

        class impl_t;
    private:
        impl_t *impl_;
    };

}; // namespace midi

#endif /* __MIDI_CONVERTER_H__ */
