
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

#include "piw_exports.h"
#include <piw/piw_data.h>

namespace piw
{
    PIW_DECLSPEC_FUNC(dd2d_nb_t) make_pitchbender(float range,float offset);

    class PIW_DECLSPEC_CLASS fast_pitchbender_t: public pic::nocopy_t
    {
        public:
            fast_pitchbender_t();
            ~fast_pitchbender_t();

            change_nb_t set_range();  // semitones
            change_nb_t set_offset(); // semitones
            dd2d_nb_t bend_functor();

            class impl_t;

        private:
            impl_t *impl_;
    };
}

