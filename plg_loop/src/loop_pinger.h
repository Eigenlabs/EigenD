
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

#ifndef __LOOP_PINGER__
#define __LOOP_PINGER__

#include <piw/piw_clock.h>
#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <plg_loop/src/loop_exports.h>
namespace loop
{
    class PILOOP_DECLSPEC_CLASS pinger_t
    {
        public:
            pinger_t(const piw::cookie_t &c, const piw::cookie_t &c1, const piw::cookie_t &c3,piw::clockdomain_ctl_t *domain,const piw::change_t &, const piw::change_t &, const piw::change_t &);
            ~pinger_t();
            piw::cookie_t cookie();
            void set_tempo(float t);
            void set_beats(float b);
            void set_range(float u, float l);
            void start_preroll(unsigned p);
            void midi_clock_enable(bool e);
            void midi_clock_set_latency(float l);
            void set_beat_flash_persistence(unsigned d);
            void play();
            void stop();
            void toggle();

            int gc_traverse(void *,void *) const;
            int gc_clear();

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif
