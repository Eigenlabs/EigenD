
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

#ifndef __PIW_WAVRECORDER__
#define __PIW_WAVRECORDER__
#include "piw_exports.h"
#include <piw/piw_bundle.h>

namespace piw
{
    class clockdomain_ctl_t; 

    class PIW_DECLSPEC_CLASS wavrecorder_t : public pic::nocopy_t
    {
        public:
            wavrecorder_t(clockdomain_ctl_t *);
            ~wavrecorder_t();
            void setfile(const char *filename);
            cookie_t cookie();
            change_nb_t record();
            
            class impl_t;
        private:
            impl_t *impl_;
    };
}
#endif
