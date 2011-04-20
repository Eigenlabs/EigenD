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

#ifndef __PIUTIL_STRING__
#define __PIUTIL_STRING__
#include "pie_exports.h"
#include "pie_parse.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct pie_strreader_s
{
    const char *ptr;
    unsigned len;
} pie_strreader_t;

typedef struct pie_strwriter_s
{
    char *ptr;
    unsigned len;
} pie_strwriter_t;

PIE_DECLSPEC_FUNC(int) pie_readstr(void *a, int p);
PIE_DECLSPEC_FUNC(void) pie_writestr(void *a, char p);

PIE_DECLSPEC_FUNC(void) pie_readstr_init(pie_strreader_t *a, const char *, unsigned);
PIE_DECLSPEC_FUNC(void) pie_writestr_init(pie_strwriter_t *a, char *, unsigned);

#ifdef __cplusplus
}
#endif

#endif
