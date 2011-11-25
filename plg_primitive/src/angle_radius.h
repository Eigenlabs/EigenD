
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

#ifndef __ANGLE_RADIUS__
#define __ANGLE_RADIUS__

#include "primitive_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace prim
{
    class PRIMITIVE_PLG_DECLSPEC_CLASS angle_radius_t : public pic::nocopy_t
    {
        public:
            angle_radius_t(piw::clockdomain_ctl_t *, const piw::cookie_t &);
            ~angle_radius_t();

            piw::cookie_t cookie();

            class impl_t;
            
        private:
            impl_t *impl_;
    };
}

#endif

