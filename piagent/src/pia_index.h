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

#ifndef __PIA_SRC_INDEX__
#define __PIA_SRC_INDEX__

#include <pibelcanto/plugin.h>
#include <picross/pic_ilist.h>
#include "pia_manager.h"

class pia_data_t;
struct pia_ctx_t;

struct pia_indexlist_t
{
    public:
        struct impl_t;

    public:
        pia_indexlist_t(pia::manager_t::impl_t *);
        ~pia_indexlist_t();
        void index(const pia_data_t &n, const pia_ctx_t &e, bct_index_t *w);
        void kill(const pia_ctx_t &e);
        void dump(const pia_ctx_t &e);
        void advertise(void *id, const pia_data_t &index, const pia_data_t &name, unsigned short cookie);
        void unadvertise(void *id, const pia_data_t &index, const pia_data_t &name);
        void killadvertise(void *id);

    private:
        impl_t *impl_;

};

#endif
