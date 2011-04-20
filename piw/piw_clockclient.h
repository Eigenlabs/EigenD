
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

#ifndef __PIW_CLOCKCLIENT__
#define __PIW_CLOCKCLIENT__
#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS clockinterp_t : public pic::nocopy_t
    {
        public:
            clockinterp_t(unsigned n);
            ~clockinterp_t();

            void recv_clock(unsigned n, const piw::data_nb_t &d);
            double get_clock(unsigned n);

            bool clockvalid(unsigned n);
            unsigned long long interpolate_time(unsigned n, float c, unsigned long long lb);
            unsigned long long interpolate_time_backward(unsigned n, float c);
            unsigned long long interpolate_time_noncyclic(unsigned n, float c, unsigned long long lb);
            float interpolate_clock(unsigned n, unsigned long long t);
            float get_mod(unsigned n);
            double get_speed(unsigned n);
            unsigned long long get_time(unsigned n);

            class impl_t;

        private:
            impl_t *impl_;
    };
};

#endif
