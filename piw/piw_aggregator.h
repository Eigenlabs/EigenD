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

#ifndef __PIW_AGGEGRATOR__
#define __PIW_AGGEGRATOR__
#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS aggregator_t
    {
        public:
            class impl_t;
        public:
            aggregator_t(const cookie_t &c, clockdomain_ctl_t *d);
            ~aggregator_t();
            unsigned find_unused();
            cookie_t get_output(unsigned name);
            cookie_t get_filtered_output(unsigned name, const piw::d2d_nb_t &filter);
            void clear_output(unsigned name);
            unsigned size();
            int gc_traverse(void *, void *) const;
            int gc_clear();
        private:
            impl_t *root_;
    };
};

#endif
