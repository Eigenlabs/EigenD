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

#ifndef __PIW_FUNCTION2__
#define __PIW_FUNCTION2__

#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_data.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS function2_t
    {
        public:
            class impl_t;
        public:
            function2_t(bool thru, unsigned input1, unsigned input2, unsigned output, const piw::data_t &i1, const piw::data_t &i2, const cookie_t &o);
            ~function2_t();
            piw::cookie_t cookie();
            void set_functor(const piw::dd2d_nb_t &f);
            int gc_traverse(void *,void *) const;
            int gc_clear();
        private:
            impl_t *impl_;
    };

    PIW_DECLSPEC_FUNC(piw::dd2d_nb_t) add(float fact);
};

#endif
