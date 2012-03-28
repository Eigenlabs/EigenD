
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

#ifndef __PIW_CORRELATOR__
#define __PIW_CORRELATOR__

#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_policy.h>

#define INPUT_MERGE     0
#define INPUT_LATCH     1
#define INPUT_INPUT     2
#define INPUT_LINGER    3

namespace piw
{
    class PIW_DECLSPEC_CLASS correlator_t: virtual public pic::tracked_t
    {
        public:
            correlator_t(clockdomain_ctl_t *, const std::string &sigmap, const d2d_nb_t &filter, const cookie_t &,unsigned threshold,unsigned poly);
            ~correlator_t();
            clocksink_t *clocksink();
            void clock_plumbed(unsigned signal,bool status);
            void plumb_input(unsigned signal, unsigned id, const piw::data_t &wire, int priority, unsigned type, fastdata_t *data, const converter_ref_t &cvt, const d2d_nb_t &filter);
            void unplumb_input(unsigned signal, unsigned id, const piw::data_t &wire, int priority);

            void set_latency(unsigned signal, unsigned id, unsigned latency);
            void remove_latency(unsigned signal, unsigned id);

            void kill();

            int gc_clear();
            int gc_traverse(void *,void *) const;

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif

