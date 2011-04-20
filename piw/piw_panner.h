
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

#ifndef __PIW_PANNER__
#define __PIW_PANNER__
#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <picross/pic_functor.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS panner_t
    {
        public:
            panner_t(const pic::f2f_t &f,const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~panner_t();

            piw::cookie_t cookie();

            int gc_traverse(void *,void *) const;
            int gc_clear();

            class impl_t;
        private:
            impl_t *impl_;
    };

}

#endif
