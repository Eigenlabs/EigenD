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

#ifndef __PIW_FASTDATA__
#define __PIW_FASTDATA__

#include "piw_exports.h"

#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>
#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_flipflop.h>

#include "piw_data.h"
#include "piw_tsd.h"
#include "piw_dataqueue.h"

namespace piw
{
    typedef pic::functor_t<void(const piw::data_nb_t &, const dataqueue_t &)> f_event_sender_t;
    typedef pic::functor_t<bool(const piw::data_nb_t &, const dataqueue_t &)> f_event_receiver_t;

    class PIW_DECLSPEC_CLASS fastdata_t: public pic::nocopy_t, public bct_fastdata_t, virtual public pic::tracked_t
    {
        public:
            fastdata_t(int (*rcv)(void *,bct_entity_t,bct_data_t,bct_dataqueue_t), int (*rcv2)(void *,bct_entity_t,bct_data_t), void *rcvctx, unsigned short flags = 0);
            fastdata_t(unsigned short flags = 0);
            virtual ~fastdata_t() { close_fastdata(); }

            void send_slow(const piw::data_t &d,const dataqueue_t &q) const
            {
                piw::tsd_protect_t p;
                PIC_ASSERT(open());
                PIC_WARN(d.time()!=0ULL);
                PIC_ASSERT(bct_fastdata_host_send(this,d.give_copy(PIC_ALLOC_NB),q.lend())>=0);
            }
            void send_fast(const piw::data_nb_t &d,const dataqueue_t &q) const
            {
                PIC_ASSERT(open());
                piw::tsd_protect_t p;
                PIC_ASSERT(bct_fastdata_host_send_fast(this,d.lend(),q.lend())>=0);
            }

            void set_upstream(bct_fastdata_t *f) { PIC_ASSERT(open()); PIC_ASSERT(bct_fastdata_host_set_upstream(this,f)>=0); }
            void clear_upstream() { set_upstream(0); }
            void enable(bool enable, bool ping, bool supress) { int rs=supress?1:0; PIC_ASSERT(open()); PIC_ASSERT(bct_fastdata_host_enable(this,enable?1:0,&rs,ping?1:0)>=0); }
            void suppress(bool s, bool p) { PIC_ASSERT(open()); PIC_ASSERT(bct_fastdata_host_suppress(this,s?1:0,p?1:0)>=0); }
            void suppress() { suppress(true,false); }
            piw::data_nb_t current(bool id) const { PIC_ASSERT(open()); return piw::data_nb_t::from_given(bct_fastdata_host_current(this,id?1:0)); }
            dataqueue_t current_queue() { PIC_ASSERT(open()); return dataqueue_t::from_given(bct_fastdata_host_current_queue(this)); }

            virtual void close_fastdata() { if(host_ops) { bct_fastdata_host_close(this); host_ops=0; } }
            bool open() const { return host_ops!=0; }
            bool closing() const { return plg_state==PLG_STATE_CLOSED; }
            bool running() const { return open() && !closing(); }

            void set_change_handler(const change_nb_t &f) { fchanged_.set(f); }
            void clear_change_handler() { fchanged_.clear(); }

            void set_id_change_handler(const f_event_receiver_t &f) { ichanged_.set(f); }
            void clear_id_change_handler() { ichanged_.clear(); }

            f_event_sender_t id_sender() { return sender_; }
            int gc_traverse(void *v, void *a) const;
            int gc_clear();
            void ping(unsigned long long time, const dataqueue_t &q);
            void fastdata_subscribe() { PIC_ASSERT(bct_fastdata_host_subscribe(this)>=0); }

        protected:
            virtual void fastdata_closed() { close_fastdata(); }
            virtual bool fastdata_receive_event(const piw::data_nb_t &d,const dataqueue_t &q);
            virtual bool fastdata_receive_data(const piw::data_nb_t &d);

        private:
            static void fastdata_attached_thunk(bct_fastdata_t *);
            static int fastdata_receive_event_thunk(void *, bct_entity_t, bct_data_t, bct_dataqueue_t);
            static int fastdata_receive_data_thunk(void *, bct_entity_t, bct_data_t);
            static void fastdata_closed_thunk(bct_fastdata_t *, bct_entity_t);

        private:
            bct_fastdata_plug_ops_t dispatch_;
            pic::flipflop_functor_t<change_nb_t> fchanged_;
            pic::flipflop_functor_t<f_event_receiver_t> ichanged_;
            f_event_sender_t sender_;
    };

    class PIW_DECLSPEC_CLASS fastdata_receiver_t: public fastdata_t
    {
        public:
            fastdata_receiver_t(const change_nb_t &handler): fastdata_t(0) { set_change_handler(handler); }
            bool fastdata_receive_event(const piw::data_nb_t &d, const dataqueue_t &q) { ping(d.time(),q); return true; }
    };
};

#endif
