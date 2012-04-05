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

#ifndef __PIW_DATAQUEUE__
#define __PIW_DATAQUEUE__
#include "piw_exports.h"
#include <pibelcanto/plugin.h>
#include <piw/piw_data.h>

#define PIW_DATAQUEUE_SIZE_ISO 4
#define PIW_DATAQUEUE_SIZE_TINY 8
#define PIW_DATAQUEUE_SIZE_NORM 128

namespace piw
{
    inline void piw_dataqueue_incref(bct_dataqueue_t q) { if(q) pic_atomicinc(&q->count); }
    inline void piw_dataqueue_decref(bct_dataqueue_t q) { if(q) if(pic_atomicdec(&q->count)==0) bct_dataqueue_free(q); }

    class PIW_DECLSPEC_CLASS dataqueue_t: public pic::tracked_t
    {
        public:
            dataqueue_t(): queue_(0) {}
            dataqueue_t(const dataqueue_t &q): queue_(q.queue_) { piw_dataqueue_incref(queue_); }
            dataqueue_t &operator=(const dataqueue_t &q) { if(queue_!=q.queue_) { clear(); queue_=q.queue_; piw_dataqueue_incref(queue_); } return *this; }
            ~dataqueue_t();

            bct_dataqueue_t lend() const { return queue_; }
            bct_dataqueue_t give() const { piw_dataqueue_incref(queue_); return queue_; }
            void clear();
            bool isvalid() const { return queue_!=0; }

            void write_slow(const piw::data_base_t &d);
            void write_fast(const piw::data_nb_t &d);
            void trigger_slow();
            void clear_fast();

            // read next value, up to and including t
            bool read(piw::data_nb_t &d,unsigned long long *i,unsigned long long t) const;
            // read latest value in queue up to and including t, and initialise *i
            bool latest(piw::data_nb_t &d,unsigned long long *i,unsigned long long t) const;
            // read earliest value at t or after, and initialise *i
            bool earliest(piw::data_nb_t &d,unsigned long long *i,unsigned long long t) const;

            piw::data_nb_t current() const;

            static dataqueue_t from_lent(bct_dataqueue_t r) { piw_dataqueue_incref(r); return r; }
            static dataqueue_t from_given(bct_dataqueue_t r) { return r; }

            change_nb_t sender_fast() { return piw::change_nb_t::method(this,&dataqueue_t::send_fast__); }
            change_t sender_slow() { return piw::change_t::method(this,&dataqueue_t::send_slow__); }

            void dump(bool full) const { if(isvalid()) bct_dataqueue_dump(queue_,full?1:0); else pic::logmsg() << "(no queue)"; }

        private:
            dataqueue_t(bct_dataqueue_t q);
            static int write_restamp__(void *q_, void *d_);
            void send_fast__(const piw::data_nb_t &d);
            void send_slow__(const piw::data_t &d);

        private:
            bct_dataqueue_t queue_;
    };
}

#endif
