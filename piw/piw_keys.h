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

#ifndef __PIW_KEYS_
#define __PIW_KEYS_

#include "piw_exports.h"

#include <piw/piw_data.h>

namespace piw
{
    PIW_DECLSPEC_FUNC(unsigned) calc_keynum(piw::data_nb_t geo, int row, int col);
    PIW_DECLSPEC_FUNC(piw::data_nb_t) key_position(unsigned key, const piw::data_nb_t &lengths, unsigned long long t);
};

#endif
