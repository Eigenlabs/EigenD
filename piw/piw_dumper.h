
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

#ifndef __PIW_DUMPER__
#define __PIW_DUMPER__
#include "piw_exports.h"
#include <piw/piw_client.h>

#define PIW_DUMP_SDATA   0x0001
#define PIW_DUMP_SSCALAR 0x0002
#define PIW_DUMP_SVECTOR 0x0004
#define PIW_DUMP_STIME   0x0008
#define PIW_DUMP_STYPE   0x0010
#define PIW_DUMP_ID      0x0020
#define PIW_DUMP_FDATA   0x0040
#define PIW_DUMP_FSCALAR 0x0080
#define PIW_DUMP_FVECTOR 0x0100
#define PIW_DUMP_FTIME   0x0200
#define PIW_DUMP_FTYPE   0x0400
#define PIW_DUMP_FLAGS   0x0800
#define PIW_DUMP_ADDRESS 0x1000

namespace piw
{
    PIW_DECLSPEC_FUNC(void) dump_client(piw::client_t *, unsigned flags, bool log);
};

#endif
