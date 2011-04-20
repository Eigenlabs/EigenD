
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

#ifndef __PICROSS_PIC_MLOCK__
#define __PICROSS_PIC_MLOCK__

#include "pic_config.h"

#ifdef PI_MACOSX

#define PIC_FASTCODE  __attribute__((section("__TEXT,__fastcode")))
#define PIC_FASTDATA  __attribute__((section("__DATA,__fastdata")))
#define PIC_FASTMARK_CPP  namespace { static const PIC_FASTCODE __attribute__((used)) unsigned fastmark__ = 0; }
#define PIC_FASTMARK_C  static const PIC_FASTCODE __attribute__((used)) unsigned fastmark__ = 0;

#endif

#ifdef PI_LINUX

#define PIC_FASTCODE
#define PIC_FASTDATA
#define PIC_FASTMARK_CPP
#define PIC_FASTMARK_C

#endif

#ifdef PI_WINDOWS

#define PIC_FASTCODE
#define PIC_FASTDATA
#define PIC_FASTMARK_CPP
#define PIC_FASTMARK_C

#endif


#endif
