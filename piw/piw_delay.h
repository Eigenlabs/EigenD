
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


#ifndef __PIW_DELAY__
#define __PIW_DELAY__
#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS delay_t : public pic::nocopy_t
    {
        public:
            delay_t(const cookie_t &o, clockdomain_ctl_t *d);
            ~delay_t();

            cookie_t audio_cookie();
            cookie_t tap_cookie();
            void reset_delay_lines();
            void set_enable(bool enable);
            void set_enable_time(float time);

            class impl_t;
        private:
            impl_t *impl_;

    };

}

#endif
