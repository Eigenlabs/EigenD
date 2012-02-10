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
    enum hardness_t {KEY_LIGHT, KEY_SOFT, KEY_HARD};

    PIW_DECLSPEC_FUNC(piw::data_nb_t) makekey(unsigned pseq, int row, int col, unsigned mseq, int course, int key, hardness_t hardness, unsigned long long t);
    PIW_DECLSPEC_FUNC(bool) is_key(const piw::data_t &d);
    PIW_DECLSPEC_FUNC(bool) decode_key(const piw::data_nb_t &d, unsigned *pseq=0, float *row=0, float *col=0, unsigned *mseq=0, float *course=0, float *key=0, hardness_t *hardness=0);
    PIW_DECLSPEC_FUNC(unsigned) key_sequential(const piw::data_t &lengths, int x, int y);
    PIW_DECLSPEC_FUNC(void) key_coordinates(unsigned sequential, const piw::data_nb_t &lengths, int *x, int *y);
};

#endif
