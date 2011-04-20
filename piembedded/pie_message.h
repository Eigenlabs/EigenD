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

#ifndef __PIE__MESSAGE__
#define __PIE__MESSAGE__

#include "pie_exports.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <picross/pic_stdint.h>

PIE_DECLSPEC_FUNC(int) pie_setheader(unsigned char *b, unsigned l, uint16_t cookie, uint32_t sseq,uint32_t dseq, uint32_t nseq, uint32_t tseq, uint32_t tseq2);
PIE_DECLSPEC_FUNC(int) pie_setstanza(unsigned char *b, unsigned l, unsigned bt, const unsigned char *pref, unsigned pl, unsigned char suffix);
PIE_DECLSPEC_FUNC(int) pie_setevthdr(unsigned char *b, unsigned l, uint32_t dseq, uint32_t nseq, uint32_t tseq);
PIE_DECLSPEC_FUNC(int) pie_settevtpath(unsigned char *b, unsigned l, unsigned char path, uint32_t tseq);
PIE_DECLSPEC_FUNC(int) pie_settsetpath(unsigned char *b, unsigned l, unsigned char path);
PIE_DECLSPEC_FUNC(int) pie_setlastpath(unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_settsetlist(unsigned char *b, unsigned l, unsigned dl, const void *dp);

PIE_DECLSPEC_FUNC(int) pie_setdata(unsigned char *b, unsigned l, unsigned df, uint16_t dl, const void *dp);

PIE_DECLSPEC_FUNC(unsigned) pie_datalen(unsigned dl);
PIE_DECLSPEC_FUNC(unsigned) pie_headerlen(void);
PIE_DECLSPEC_FUNC(unsigned) pie_stanzalen_req(unsigned pl, unsigned char suffix);
PIE_DECLSPEC_FUNC(unsigned) pie_stanzalen_tevt(unsigned pl, unsigned paths, unsigned char suffix);
PIE_DECLSPEC_FUNC(unsigned) pie_stanzalen_tset(unsigned pl, unsigned paths, unsigned char suffix);
PIE_DECLSPEC_FUNC(unsigned) pie_stanzalen_devt(unsigned pl, unsigned dl, unsigned char suffix);
PIE_DECLSPEC_FUNC(unsigned) pie_stanzalen_fevt(unsigned pl, unsigned dl, unsigned char suffix);
PIE_DECLSPEC_FUNC(unsigned) pie_stanzalen_dset(unsigned pl, unsigned dl, unsigned char suffix);

PIE_DECLSPEC_FUNC(int) pie_getheader(const unsigned char *b, unsigned l, uint16_t *cookie, uint32_t *sseq,uint32_t *dseq, uint32_t *nseq, uint32_t *tseq, uint32_t *tseq2);
PIE_DECLSPEC_FUNC(int) pie_getstanza(const unsigned char *b, unsigned l, unsigned *bt, const unsigned char **pref, unsigned *pl);
PIE_DECLSPEC_FUNC(int) pie_getevthdr(const unsigned char *b, unsigned l, uint32_t *dseq, uint32_t *nseq, uint32_t *tseq);
PIE_DECLSPEC_FUNC(int) pie_gettevtpath(const unsigned char *b, unsigned l, unsigned char *path, uint32_t *tseq);
PIE_DECLSPEC_FUNC(int) pie_gettsetpath(const unsigned char *b, unsigned l, unsigned char *path);
PIE_DECLSPEC_FUNC(int) pie_gettsetlist(const unsigned char *b, unsigned l, const unsigned char **dp, unsigned *lp);

PIE_DECLSPEC_FUNC(int) pie_getdata(const unsigned char *b, unsigned l, unsigned *df, uint16_t *dl, const unsigned char **dp);

PIE_DECLSPEC_FUNC(int) pie_skipevthdr(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skiptset(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skiptevt(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skipdata(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skipdset(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skipdevt(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skipdata(const unsigned char *b, unsigned l);
PIE_DECLSPEC_FUNC(int) pie_skipstanza(const unsigned char *b, unsigned l, unsigned char bt);

PIE_DECLSPEC_FUNC(int) pie_setindex(unsigned char *, unsigned, uint16_t, uint16_t dl, const unsigned char *dp);
PIE_DECLSPEC_FUNC(int) pie_getindex(const unsigned char *, unsigned, uint16_t *, uint16_t *dl, const unsigned char **dp);

PIE_DECLSPEC_FUNC(int) pie_setrpc(unsigned char *b, unsigned l, const unsigned char *p, unsigned pl, unsigned bt, const uint64_t *cookie, unsigned nl, const unsigned char *np, int st, uint16_t dl, const void *dp);
PIE_DECLSPEC_FUNC(int) pie_getrpc(const unsigned char *b, unsigned l, const unsigned char **p, unsigned *pl, unsigned *bt, uint64_t *cookie, unsigned *nl, const unsigned char **np, int *st, uint16_t *dl, const unsigned char **dp);

#ifdef __cplusplus
}
#endif

#endif
