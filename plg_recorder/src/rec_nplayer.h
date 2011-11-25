
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

#ifndef __ARRANGER_PLAYER__
#define __ARRANGER_PLAYER__
#include <pirecorder_exports.h>
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>

namespace recorder
{
    class PIRECORDER_DECLSPEC_CLASS nplayer_t : public pic::nocopy_t
    {
        public:
            nplayer_t(const piw::cookie_t &c,unsigned poly,unsigned sig_data,unsigned sig_key,piw::clockdomain_ctl_t *);
            ~nplayer_t();

            piw::change_t play(unsigned note, unsigned velocity, unsigned long long length);
            bct_clocksink_t *get_clock();
            piw::change_nb_t control();

            class impl_t;

        private:
            impl_t *impl_;
    };
};

#endif
