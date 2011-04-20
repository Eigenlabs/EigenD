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

#ifndef __PIA_SRC_DATAQUEUE__
#define __PIA_SRC_DATAQUEUE__

#include <pibelcanto/plugin.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_ilist.h>
#include "pia_data.h"

inline void pia_dataqueue_incref(bct_dataqueue_t q) { if(q) pic_atomicinc(&q->count); }
inline void pia_dataqueue_decref(bct_dataqueue_t q) { if(q) if(pic_atomicdec(&q->count)==0) bct_dataqueue_free(q); }
//inline void pia_dataqueue_incref(bct_dataqueue_t q) { if(q) ++q->count; }
//inline void pia_dataqueue_decref(bct_dataqueue_t q) { if(q) { if(--q->count==0) bct_dataqueue_free(q); } }

#define DATAQUEUE_SUBSCRIBER 2

class pia_dataqueue_t : public pic::lckobject_t
{
    public:
        pia_dataqueue_t() : queue_(0) {}
        pia_dataqueue_t(const pia_dataqueue_t &q): queue_(q.queue_) { pia_dataqueue_incref(queue_); }
        ~pia_dataqueue_t() { pia_dataqueue_decref(queue_); }
        pia_dataqueue_t &operator=(const pia_dataqueue_t &q) { if(queue_!=q.queue_) { pia_dataqueue_decref(queue_); queue_=q.queue_; pia_dataqueue_incref(queue_); } return *this; }

        static pia_dataqueue_t alloc(unsigned);
        static pia_dataqueue_t from_lent(bct_dataqueue_t q) { pia_dataqueue_incref(q); return pia_dataqueue_t(q); }
        static pia_dataqueue_t from_given(bct_dataqueue_t q) { return pia_dataqueue_t(q); }

        bct_dataqueue_t lend() const { return queue_; }
        bct_dataqueue_t give() const { pia_dataqueue_incref(queue_); return queue_; }

        class subscriber_t : public pic::element_t<DATAQUEUE_SUBSCRIBER>
        {
            public:
                virtual ~subscriber_t() {}
                virtual bool receive_data(const pia_data_nb_t &d) = 0;
        };

        void subscribe(subscriber_t *);

        int write(const pia_data_nb_t &d);
        int read(pia_data_nb_t &,unsigned long long *i, unsigned long long t) const;
        int latest(pia_data_nb_t &,unsigned long long *i, unsigned long long t) const;
        int earliest(pia_data_nb_t &,unsigned long long *i, unsigned long long t) const;
        pia_data_nb_t current_nb() const;
        pia_data_t current_normal(pic::nballocator_t *a) const;
        bool isvalid() const { return queue_!=0; }

    private:
        pia_dataqueue_t(bct_dataqueue_t q): queue_(q) {}
        bct_dataqueue_t queue_;
};

class pia_dataqueuedrop_t: public pic::nocopy_t
{
    public:
        pia_dataqueuedrop_t() { set(pia_dataqueue_t()); }
        pia_dataqueuedrop_t(const pia_dataqueue_t &q) { set(q); }
        ~pia_dataqueuedrop_t() { set(pia_dataqueue_t()); }
        void set(const pia_dataqueue_t &q) { drop_.set(q.lend()); }
        pia_dataqueue_t get() { return pia_dataqueue_t::from_given(drop_.get()); }
    private:
        pic::datadrop_t<bct_dataqueue_t,pia_dataqueue_incref,pia_dataqueue_decref> drop_;
};

#endif
