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

#ifndef __PIA_SRC_FASTDATA__
#define __PIA_SRC_FASTDATA__

#include <pibelcanto/plugin.h>
#include <picross/pic_ilist.h>
#include "pia_data.h"
#include "pia_dataqueue.h"

struct pia_ctx_t;

class pia_fastdatalist_t
{
    public:
        struct impl_t;
    public:
        pia_fastdatalist_t();
        ~pia_fastdatalist_t();

        void fastdata(const pia_ctx_t &e, bct_fastdata_t *s);
        void kill(const pia_ctx_t &e);
        void dump(const pia_ctx_t &e);

    private:
        impl_t *impl_;
};

class pia_fastdata_t: bct_fastdata_t, virtual public pic::lckobject_t
{
    public:
        pia_fastdata_t(const pia_ctx_t &, unsigned flags, int (*rcv)(void *, bct_entity_t, bct_data_t, bct_dataqueue_t), int (*rcv2)(void *, bct_entity_t, bct_data_t), void *rcv_ctx);
        virtual ~pia_fastdata_t();

        void fastdata_clear_downstream(void *tag);
        void fastdata_add_downstream(bct_fastdata_t *d, void *tag = 0);
        void fastdata_set_upstream(bct_fastdata_t *u, void *tag = 0);
        void fastdata_enable(bool,bool);
        pia_data_nb_t fastdata_current_nb(bool);
        pia_data_t fastdata_current_normal(pic::nballocator_t *a, bool);
        pia_dataqueue_t fastdata_current_queue();
        void fastdata_send(const pia_data_t &,const pia_dataqueue_t &);
        void fastdata_send(const pia_data_nb_t &,const pia_dataqueue_t &);

    private:
        bct_fastdata_plug_ops_t dispatch_;
};

#endif
