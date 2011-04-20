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

#ifndef __PIW_NORMALIZE__
#define __PIW_NORMALIZE__

#include "piw_exports.h"
#include "piw_data.h"

namespace piw
{
    PIW_DECLSPEC_FUNC(d2d_nb_t) bint_normalizer(int min, int max, int rest);
    PIW_DECLSPEC_FUNC(d2d_nb_t) bool_normalizer();
    PIW_DECLSPEC_FUNC(d2d_nb_t) bfloat_normalizer(float min, float max, float rest);
    PIW_DECLSPEC_FUNC(d2d_nb_t) null_normalizer();
    PIW_DECLSPEC_FUNC(d2d_nb_t) string_normalizer();
    PIW_DECLSPEC_FUNC(d2d_nb_t) string_denormalizer();

    PIW_DECLSPEC_FUNC(d2d_nb_t) bint_denormalizer(int min, int max, int rest);
    PIW_DECLSPEC_FUNC(d2d_nb_t) bfloat_denormalizer(float min, float max, float rest);
    PIW_DECLSPEC_FUNC(d2d_nb_t) bool_denormalizer();
    PIW_DECLSPEC_FUNC(d2d_nb_t) null_denormalizer();

    PIW_DECLSPEC_FUNC(change_nb_t) d2d_convert(const piw::d2d_nb_t &a, const change_nb_t &d);
    PIW_DECLSPEC_FUNC(change_nb_t) d2d_chain(const piw::d2d_nb_t &a, const piw::d2d_nb_t &b, const change_nb_t &d);

}

#endif
