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

#ifndef __MIDI_FROM_BELCANTO_H__
#define __MIDI_FROM_BELCANTO_H__

#include <midilib_exports.h>

#include <picross/pic_functor.h>
#include <piw/piw_data.h>
#include <piw/piw_bundle.h>

#include <lib_midi/control_mapping.h>
#include <lib_midi/control_params.h>

namespace piw { class clockdomain_ctl_t; }

namespace midi
{

    typedef pic::functor_t<void(const piw::data_nb_t&)> resend_current_t;

    struct MIDILIB_DECLSPEC_CLASS midi_from_belcanto_t
    {
        midi_from_belcanto_t(const piw::cookie_t &, piw::clockdomain_ctl_t *);
        ~midi_from_belcanto_t();

        piw::cookie_t cookie();
        control_mapping_t &get_mapping(unsigned);
        void set_resend_current(midi::resend_current_t);
        void set_midi_channel(unsigned);
        void set_min_midi_channel(unsigned);
        void set_max_midi_channel(unsigned);
        void set_omni(bool);
        void set_program_change(unsigned);
        piw::change_nb_t change_program();
        void set_bank_change(unsigned);
        piw::change_nb_t change_bank();
        void set_cc(unsigned, unsigned);
        piw::change_nb_t change_cc();
        void set_control_interval(float);
        void set_midi(pic::lckvector_t<midi_data_t>::nbtype &);
        void set_send_notes(bool);
        void set_send_pitchbend(bool);
        void set_send_hires_velocity(bool);
        unsigned get_active_midi_channel(const piw::data_nb_t &);

        class impl_t;
    private:
        impl_t *impl_;
    };

}; // namespace midi


#endif /* __MIDI_FROM_BELCANTO_H__ */
