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

#ifndef __PIEMBEDDED_IOSTREAM__
#define __PIEMBEDDED_IOSTREAM__

#include <iostream>

namespace pie
{
    inline void ostreamwriter(void *a, char ch)
    {
        std::ostream *o = (std::ostream *)a;
        o->put(ch);
    }

    inline int istreamreader(void *a, int p)
    {
        std::istream *i = (std::istream *)a;

        if(i->eof())
        {
            return -1;
        }

        return p?(i->peek()):(i->get());
    }
}

#endif
