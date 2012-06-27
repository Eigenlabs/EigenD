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

#ifndef __PIW_THING__
#define __PIW_THING__
#include "piw_exports.h"
#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>

#include "piw_data.h"
#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS thing_t: public pic::nocopy_t, public bct_thing_t, virtual public pic::tracked_t
    {
        public:
            thing_t();
            virtual ~thing_t() { tracked_invalidate(); close_thing(); }

            pic::notify_t trigger_fast_functor() { return pic::notify_t::method(this, &thing_t::trigger_fast); }
            pic::notify_t trigger_slow_functor() { return pic::notify_t::method(this, &thing_t::trigger_slow); }
            piw::change_nb_t enqueue_fast_functor() { return piw::change_nb_t::method(this, &thing_t::enqueue_fast_lopri); }
            piw::change_t enqueue_slow_functor() { return piw::change_t::method(this, &thing_t::enqueue_slow); }
            piw::change_nb_t enqueue_slow_nb_functor() { return piw::change_nb_t::method(this, &thing_t::enqueue_slow_nb); }

            void set_fast_timer_handler(pic::notify_t f) { _fftimer=f; }
            void set_slow_timer_handler(pic::notify_t f) { _fstimer=f; }
            void set_fast_trigger_handler(pic::notify_t f) { _fftrigger=f; }
            void set_slow_trigger_handler(pic::notify_t f) { _fstrigger=f; }
            void set_fast_dequeue_handler(piw::change_nb_t f) { _ffdequeue=f; }
            void set_slow_dequeue_handler(piw::change_t f) { _fsdequeue=f; }

            void clear_fast_timer_handler() { _fftimer.clear(); }
            void clear_slow_timer_handler() { _fstimer.clear(); }
            void clear_fast_trigger_handler() { _fftrigger.clear(); }
            void clear_slow_trigger_handler() { _fstrigger.clear(); }
            void clear_fast_dequeue_handler() { _ffdequeue.clear(); }
            void clear_slow_dequeue_handler() { _fsdequeue.clear(); }

            void trigger_fast() { PIC_ASSERT(open()); bct_thing_host_trigger_fast(this); }
            void trigger_slow() { PIC_ASSERT(open()); bct_thing_host_trigger_slow(this); }
            void timer_fast(unsigned long ms) { PIC_ASSERT(open()); PIC_ASSERT(bct_thing_host_timer_fast(this,ms,0)>=0); }
            void timer_fast_us(unsigned long ms, long us) { PIC_ASSERT(open()); PIC_ASSERT(bct_thing_host_timer_fast(this,ms,us)>=0); }
            void timer_slow(unsigned long ms) { PIC_ASSERT(open()); bct_thing_host_timer_slow(this,ms); }
            void cancel_timer_fast() { PIC_ASSERT(open()); bct_thing_host_cancel_timer_fast(this); }
            void cancel_timer_slow() { PIC_ASSERT(open()); bct_thing_host_cancel_timer_slow(this); }
            void flush_slow() { PIC_ASSERT(open()); bct_thing_host_flush_slow(this); }
            void enqueue_fast(const piw::data_nb_t &d,int p=0) { PIC_ASSERT(open()); bct_thing_host_queue_fast(this,d.realloc(PIC_ALLOC_NB).lend(),p); }
            void enqueue_slow(const piw::data_t &d) { PIC_ASSERT(open()); bct_thing_host_queue_slow(this,d.lend()); }
            void enqueue_slow_nb(const piw::data_nb_t &d) { PIC_ASSERT(open()); bct_thing_host_queue_slow(this,d.give_copy()); }
            void enqueue_fast_hipri(const piw::data_nb_t &d) { PIC_ASSERT(open()); bct_thing_host_queue_fast(this,d.realloc(PIC_ALLOC_NB).lend(),1); }
            void enqueue_fast_lopri(const piw::data_nb_t &d) { PIC_ASSERT(open()); bct_thing_host_queue_fast(this,d.realloc(PIC_ALLOC_NB).lend(),0); }
            void defer_delete(bool(*cb)(void*), void *d, unsigned long ms) { PIC_ASSERT(open()); bct_thing_host_defer_delete(this,cb,d,ms); }

            virtual void close_thing() { if(host_ops) { bct_thing_host_close(this); host_ops=0; } }
            inline bool open() { return host_ops!=0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

            int gc_traverse(void *v, void *a) const;
            int gc_clear();

        protected:
            virtual void thing_closed() {}
            virtual void thing_trigger_fast() { _fftrigger(); }
            virtual void thing_dequeue_fast(const piw::data_nb_t &d) { _ffdequeue(d); }
            virtual void thing_timer_fast() { _fftimer(); }
            virtual void thing_trigger_slow() { _fstrigger(); }
            virtual void thing_dequeue_slow(const piw::data_t &d) { _fsdequeue(d); }
            virtual void thing_timer_slow() { _fstimer(); }

        private:
            static void thing_attached_thunk(bct_thing_t *);
            static void thing_triggered_fast_thunk(bct_thing_t *, bct_entity_t);
            static void thing_triggered_slow_thunk(bct_thing_t *, bct_entity_t);
            static void thing_dequeue_fast_thunk(bct_thing_t *, bct_entity_t, bct_data_t);
            static void thing_dequeue_slow_thunk(bct_thing_t *, bct_entity_t, bct_data_t);
            static void thing_timer_fast_thunk(bct_thing_t *, bct_entity_t);
            static void thing_timer_slow_thunk(bct_thing_t *, bct_entity_t);
            static void thing_closed_thunk(bct_thing_t *, bct_entity_t);

            static bct_thing_plug_ops_t _dispatch;

            pic::flipflop_functor_t<pic::notify_t> _fftimer;
            pic::notify_t _fstimer;
            pic::flipflop_functor_t<pic::notify_t> _fftrigger;
            pic::notify_t _fstrigger;
            pic::flipflop_functor_t<piw::change_nb_t> _ffdequeue;
            piw::change_t _fsdequeue;
    };
};

#endif
