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

#ifndef __PIUTIL_PARSE__
#define __PIUTIL_PARSE__

#ifdef __cplusplus
extern "C" {
#endif
#include "pie_exports.h"

typedef int (*piu_input_t)(void *,int);

PIE_DECLSPEC_FUNC(void) pie_skipspace(piu_input_t r, void *ra);
PIE_DECLSPEC_FUNC(int) pie_parsebuffer(unsigned char *buf, unsigned size, piu_input_t r, void *ra);
PIE_DECLSPEC_FUNC(int) pie_parsestring(unsigned char *buf, unsigned size, piu_input_t r, void *ra);
PIE_DECLSPEC_FUNC(int) pie_parsestring2(unsigned char *buf,unsigned size, piu_input_t r, void *ra,const char *term, const char *incl);
PIE_DECLSPEC_FUNC(int) pie_parseaddress(char *server, unsigned slen, unsigned char *path, unsigned plen, piu_input_t r, void *ra);
PIE_DECLSPEC_FUNC(int) pie_parsepath(unsigned char *buf,unsigned size,piu_input_t r, void *ra);

#ifdef __cplusplus
}
#endif

#endif
