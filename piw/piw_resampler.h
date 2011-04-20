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

#ifndef __PIW_RESAMPLER__
#define __PIW_RESAMPLER__

#include "piw_exports.h"
#include <picross/pic_flipflop.h>
#include <picross/pic_fastalloc.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS resampler_t: public pic::lckobject_t
    {
        public:
            resampler_t(const char *label, unsigned channels, unsigned quality);
            ~resampler_t();
            
            void resampler_write_interleaved(const float *d, unsigned dl);
            unsigned resampler_read_interleaved(unsigned bs, float *outbuffer);
            void set_quality(unsigned quality);
            void reset();

        public:
            class impl_t;

        private:
            pic::flipflop_t<impl_t *> impl_;
    };
}

#endif
