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

#ifndef __PIA_SRC_THING__
#define __PIA_SRC_THING__

#include <pibelcanto/plugin.h>
#include <picross/pic_ilist.h>

struct pia_ctx_t;

class pia_thinglist_t
{
    public:
        struct impl_t;

    public:
        pia_thinglist_t();
        ~pia_thinglist_t();

        void kill(const pia_ctx_t &);
        void dump(const pia_ctx_t &);
        void thing(const pia_ctx_t &, bct_thing_t *);

    private:
        impl_t *impl_;
};

#endif
