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

#ifndef __PIW_TABLE__
#define __PIW_TABLE__
#include "piw_exports.h"
#include <picross/pic_table.h>
#include "piw_data.h"
#include <vector>

namespace piw
{
    PIW_DECLSPEC_FUNC(piw::d2d_nb_t) make_d2d_table(float domain_lo, float domain_hi, float domain_z, unsigned resolution, const piw::d2d_nb_t &proto);
    PIW_DECLSPEC_FUNC(piw::d2d_nb_t) make_d2d_const(float ubound, float lbound, float rest, float value);
    PIW_DECLSPEC_FUNC(pic::f2f_t) make_f2f_table(float domain_lo, float domain_hi, unsigned resolution, const pic::f2f_t &proto);
    PIW_DECLSPEC_FUNC(pic::f2f_t) make_f2f_identity();
};

#endif
