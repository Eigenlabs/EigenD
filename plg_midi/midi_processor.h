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

#ifndef __MIDI_PROCESSOR_H__
#define __MIDI_PROCESSOR_H__

#include <piw/piw_bundle.h>
#include <pimidi_exports.h>

namespace pi_midi
{
    class PIMIDI_DECLSPEC_CLASS midi_processor_t
    {
        public:
            midi_processor_t(const piw::cookie_t &, piw::clockdomain_ctl_t *);
            ~midi_processor_t();
            piw::cookie_t cookie();

            void closed(bool);
            void noteon_enabled(bool);
            void noteoff_enabled(bool);
            void polypressure_enabled(bool);
            void cc_enabled(bool);
            void programchange_enabled(bool);
            void channelpressure_enabled(bool);
            void pitchbend_enabled(bool);
            void messages_enabled(bool);

            void clear_enabled_channels();
            void enable_channel(unsigned);
            void activate_enabled_channels();

            void clear_channel_mapping();
            void set_channel_mapping(unsigned, unsigned);
            void activate_channel_mapping();

            class impl_t;
        private:
            impl_t *impl_;
    };
};

#endif
