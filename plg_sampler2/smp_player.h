
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

#ifndef __SAMPLER2_PLAYER__
#define __SAMPLER2_PLAYER__

#include <pisampler2_exports.h>
#include <picross/pic_ref.h>
#include <picross/pic_weak.h>

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_data.h>
#include <piw/piw_sample.h>

namespace sampler2
{
    class PISAMPLER2_DECLSPEC_CLASS player_t: public pic::tracked_t
    {
        private:
            class impl_t;

        public:
            player_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d);
            ~player_t();
            piw::cookie_t cookie();
            void define_voice(const piw::data_nb_t &id, const piw::voiceref_t &v);
            bool attach_loader(bct_clocksink_t *);
            void detach_loader();
            void set_fade(bool);

        private:
            impl_t *impl_;
    };
}

#endif
