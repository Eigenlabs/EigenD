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

#ifndef __PIW_RANGER__
#define __PIW_RANGER__
#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS ranger_t
    {
        public:
            class impl_t;
        public:
            ranger_t(clockdomain_ctl_t *,const cookie_t &);
            ~ranger_t();
            cookie_t data_cookie();
            cookie_t ctl_cookie();
            void set_sticky(bool);
            void set_mono(bool);
            void set_absolute(bool);
            void set_curve(float c);
            void reset();
        private:
            impl_t *impl_;
    };
};

#endif
