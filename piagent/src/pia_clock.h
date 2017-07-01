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

#ifndef __PIA_CLOCK__
#define __PIA_CLOCK__

#include <pibelcanto/plugin.h>
#include "pia_manager.h"

struct pia_ctx_t;
class pia_data_t;

struct pia_clocklist_t
{
    public:
        struct impl_t;

    public:
        pia_clocklist_t(pia::manager_t::impl_t *g);
        ~pia_clocklist_t();

        void add_source(const pia_data_t &n, unsigned bs, unsigned long sr, const pia_ctx_t &e, bct_clocksource_t *c);
        void add_domain(const pia_ctx_t &e, bct_clockdomain_t *c);
        void kill(const pia_ctx_t &e);
        void dump(const pia_ctx_t &e);
        int setdownstream(void *, void *);
        int cleardownstream(void *, void *);
        void *addnotify(bct_clocksink_t *, void (*)(void *), void *);
        void cancelnotify(void *);

    private:
        impl_t *impl_;
};

#endif
