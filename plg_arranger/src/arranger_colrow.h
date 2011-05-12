
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

#ifndef __ARRANGER_COLROW__
#define __ARRANGER_COLROW__
#include <piarranger_exports.h>

namespace arranger
{
    typedef std::pair<unsigned,unsigned> colrow_t;

    inline colrow_t decode(unsigned long long t)
    {
        colrow_t cr;
        cr.second = t&0xffffffff;
        t >>= 32;
        cr.first = t&0xffffffff;
        return cr;
    }

    inline unsigned long long encode(const colrow_t &cr)
    {
        unsigned long long t = cr.first;
        t <<= 32;
        t |= cr.second;
        return t;
    }
}

#endif
