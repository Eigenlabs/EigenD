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

#ifndef __PIE_WIRE__
#define __PIE_WIRE__



#ifdef __cplusplus
extern "C" {
#endif

#include <picross/pic_stdint.h>
#include "pie_exports.h"

PIE_DECLSPEC_FUNC(int) pie_setu16(unsigned char *b, unsigned l, uint16_t v);
PIE_DECLSPEC_FUNC(int) pie_setu32(unsigned char *b, unsigned l, uint32_t v);
PIE_DECLSPEC_FUNC(int) pie_set32(unsigned char *b, unsigned l, int32_t v);
PIE_DECLSPEC_FUNC(int) pie_setu64(unsigned char *b, unsigned l, uint64_t v);
PIE_DECLSPEC_FUNC(int) pie_setf32(unsigned char *b, unsigned l, float v);
PIE_DECLSPEC_FUNC(int) pie_setf64(unsigned char *b, unsigned l, double v);

PIE_DECLSPEC_FUNC(int) pie_getu16(const unsigned char *b, unsigned l, uint16_t *v);
PIE_DECLSPEC_FUNC(int) pie_getu32(const unsigned char *b, unsigned l, uint32_t *v);
PIE_DECLSPEC_FUNC(int) pie_get32(const unsigned char *b, unsigned l, int32_t *v);
PIE_DECLSPEC_FUNC(int) pie_getu64(const unsigned char *b, unsigned l, uint64_t *v);
PIE_DECLSPEC_FUNC(int) pie_getf32(const unsigned char *b, unsigned l, float *v);
PIE_DECLSPEC_FUNC(int) pie_getf64(const unsigned char *b, unsigned l, double *v);

#ifdef __cplusplus
}
#endif

#endif
