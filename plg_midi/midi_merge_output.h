
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

#ifndef __PLG_MIDI_MERGEOUT__
#define __PLG_MIDI_MERGEOUT__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <plg_midi/midi_exports.h>

namespace pi_midi
{
    class PIMIDI_DECLSPEC_CLASS midi_merge_output_t
    {
        public:
            class impl_t;

        public:
            midi_merge_output_t(const piw::change_nb_t &midi_output_functor, piw::clockdomain_ctl_t *clk_domain);
            ~midi_merge_output_t();
            piw::cookie_t cookie();

        private:
            impl_t *impl_;
    };
};

#endif // __PLG_MIDI_MERGEOUT__
