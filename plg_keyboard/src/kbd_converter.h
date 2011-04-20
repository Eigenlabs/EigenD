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

#ifndef __KBD_CONVERTER__
#define __KBD_CONVERTER__

#include "kbd_exports.h"
#include <picross/pic_flipflop.h>
#include <picross/pic_fastalloc.h>

namespace kbd
{
    class PIKEYBOARDPLG_DECLSPEC_CLASS converter_t: public pic::lckobject_t
    {
        public:
            converter_t();
            ~converter_t();

            unsigned long reset(unsigned long sr, unsigned bs, unsigned q);
            bool write(const float *b, unsigned dl);
            void read(void (*consumer)(void *ctx, const float *b, unsigned dl, unsigned period), void *ctx);

        public:
            class impl_t;

        private:
            pic::flipflop_t<impl_t *> impl_;
    };
};

#endif

