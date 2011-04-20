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

#ifndef __PIA_SRC_BCLOCK__
#define __PIA_SRC_BCLOCK__

#include <picross/pic_stdint.h>
#include "pia_bkernel.h"

#define PIE_CLOCK_TICK_FREQ 50

class pie_clock_t
{
    public:
        class impl_t;
    public:
        pie_clock_t(pie_bkernel_t *kernel);
        ~pie_clock_t();
        void pie_clock_data(const void *payload, unsigned paylen);
        void pie_clock_tick(void);
        int pie_clock_term(void);
        uint64_t pie_clock_get(void);
        long pie_clock_get_delay(void);
        int64_t pie_clock_get_delta(void);
        uint64_t pie_clock_get_id(void);
        int pie_clock_get_mode(void);
        void pie_clock_set_loop(uint64_t id);
    private:
        impl_t *impl_;
};

#endif
