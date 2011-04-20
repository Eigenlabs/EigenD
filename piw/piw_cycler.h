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

#ifndef __PIW_CYCLER__
#define __PIW_CYCLER__
#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS cycler_t
    {
        public:
            class impl_t;
        public:
            cycler_t(clockdomain_ctl_t *,int poly, const cookie_t &, const cookie_t &,bool cycle);
            ~cycler_t();
            void set_poly(unsigned poly);
            void set_cycle(bool cycle);
            void set_maxdamp(float damp);
            void set_invert(bool invert);
            void set_curve(float);
            cookie_t main_cookie();
            cookie_t feedback_cookie();

        private:
            impl_t *impl_;
    };
};

#endif
