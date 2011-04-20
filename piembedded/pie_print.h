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

#ifndef __PIUTIL_PRINT__
#define __PIUTIL_PRINT__

#include "pie_exports.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*piu_output_t)(void *, char);

PIE_DECLSPEC_FUNC(void) pie_print(unsigned,const void *,piu_output_t, void *);
PIE_DECLSPEC_FUNC(void) pie_printfull(unsigned,const void *,piu_output_t, void *);
PIE_DECLSPEC_FUNC(void) pie_printmsg(const void *, unsigned, int, piu_output_t, void *);
PIE_DECLSPEC_FUNC(void) pie_printbuffer(const unsigned char *, unsigned, piu_output_t, void *);
PIE_DECLSPEC_FUNC(void) pie_printstring(const char *, unsigned, piu_output_t, void *);
PIE_DECLSPEC_FUNC(void) pie_printdict(const unsigned char *, unsigned, piu_output_t, void *);

#ifdef __cplusplus
}
#endif

#endif
