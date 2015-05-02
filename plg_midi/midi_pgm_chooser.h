/*
 Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __MIDI_PGM_CHOOSER__
#define __MIDI_PGM_CHOOSER__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_data.h>
#include <pimidi_exports.h>

namespace pi_midi
{
    class PIMIDI_DECLSPEC_CLASS midi_pgm_chooser_t
    {
        public:
            midi_pgm_chooser_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain);
            ~midi_pgm_chooser_t();
            piw::cookie_t cookie();
            piw::change_nb_t control();
        
            void reset();
            void up();
            void down();
            void program(unsigned);
            void bank(unsigned);
            void bank_mode(bool);
            void window(unsigned);
            void channel(unsigned);

            class impl_t;
        private:
            impl_t *impl_;
    };
};

#endif
