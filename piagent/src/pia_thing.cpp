
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

#include "pia_thing.h"
#include "pia_glue.h"
#include "pia_error.h"

#include <picross/pic_strbase.h>
#include <picross/pic_ilist.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_log.h>

#include <stdlib.h>

namespace
{
    struct tnode_t: pic::element_t<>, virtual public pic::lckobject_t
    {
        tnode_t(pia_thinglist_t::impl_t *tl,const pia_ctx_t &e, bct_thing_t *s);
        ~tnode_t();

        static void api_close(bct_thing_host_ops_t **t_);
        static void api_queue_slow(bct_thing_host_ops_t **t_, bct_data_t d);
        static void api_queue_fast(bct_thing_host_ops_t **t_, bct_data_t d, int p);
        static void api_defer_delete(bct_thing_host_ops_t **t_, bool(*cb)(void*), void *d, unsigned long ms);
        static void api_trigger_fast(bct_thing_host_ops_t **t_);
        static void api_trigger_slow(bct_thing_host_ops_t **t_);
        static int api_timer_fast(bct_thing_host_ops_t **t_, unsigned long ms, long us);
        static void api_cancel_timer_fast(bct_thing_host_ops_t **t_);
        static int api_timer_slow(bct_thing_host_ops_t **t_, unsigned long ms);
        static void api_cancel_timer_slow(bct_thing_host_ops_t **t_);
        static void api_flush_slow(bct_thing_host_ops_t **t_);

        static void timer_appq_callback(void *t_, const pia_data_t &d);
        static void timer_main_callback(void *t_);
        static void timer_fast_callback(void *t_);
        static void close_callback(void *t_, const pia_data_t & d);
        static void dequeue_slow_callback(void *t_, const pia_data_t &d);
        static void dequeue_fast_callback(void *t_, const pia_data_nb_t &d);
        static void dequeue_fast_callback_deactive(void *t_, const pia_data_t &d);
        static void trigger_slow_callback(void *t_, const pia_data_t &d);
        static void trigger_fast_callback(void *t_, const pia_data_nb_t &d);

        void detach(int);

        void close();
        void queue_slow(const pia_data_t & d);
        void queue_fast(const pia_data_nb_t & d, int p);
        void defer_delete(bool(*cb)(void*), void *d, unsigned long ms);
        void trigger_fast();
        void trigger_slow();
        static int timer_fast(void *, void *, void *, void *);
        static int cancel_timer_fast(void *,void *,void *,void *);
        void timer_slow(unsigned long ms);
        void cancel_timer_slow();
        void flush_slow();

        bct_thing_host_ops_t *host_ops_;

        pia_thinglist_t::impl_t *list_;
        bct_thing_t *thing_;

        pia_ctx_t entity_;

        pic::flipflop_t<bool> open_;
        pic_atomic_t fast_latch_;
        pic_atomic_t slow_latch_;

        pia_job_t job_slow_timer_;
        pia_job_t job_close_;

        pia_cref_t timer_fast_cpoint_;
        pia_cref_t timer_slow_cpoint_;
        pia_cref_t trigger_slow_cpoint_;
        pia_cref_t dequeue_slow_cpoint_;
        pia_cref_t fast_cpoint_;
        pia_cref_t cpoint_;

        static bct_thing_host_ops_t dispatch__;
    };
};

struct pia_thinglist_t::impl_t
{
    pic::ilist_t<tnode_t> things_;
};

void tnode_t::dequeue_slow_callback(void *t_, const pia_data_t & d)
{
    tnode_t *t = (tnode_t *)t_;
    bct_thing_plug_dequeue_slow(t->thing_,t->entity_->api(),d.lend());
}

void tnode_t::dequeue_fast_callback_deactive(void *t_, const pia_data_t & d)
{
    tnode_t *t = (tnode_t *)t_;
    bct_thing_plug_dequeue_fast(t->thing_,t->entity_->api(),d.lend());
}

void tnode_t::dequeue_fast_callback(void *t_, const pia_data_nb_t & d)
{
    tnode_t *t = (tnode_t *)t_;
    bct_thing_plug_dequeue_fast(t->thing_,t->entity_->api(),d.lend());
}

void tnode_t::trigger_slow_callback(void *t_, const pia_data_t & d)
{
    tnode_t *t = (tnode_t *)t_;
    pic_atomiccas(&t->slow_latch_,1,0);
    bct_thing_plug_triggered_slow(t->thing_,t->entity_->api());
}

void tnode_t::trigger_fast_callback(void *t_, const pia_data_nb_t & d)
{
    tnode_t *t = (tnode_t *)t_;
    pic_atomiccas(&t->fast_latch_,1,0);
    bct_thing_plug_triggered_fast(t->thing_,t->entity_->api());
}

void tnode_t::close_callback(void *t_, const pia_data_t & d)
{
    tnode_t *t = (tnode_t *)t_;
    bct_thing_plug_closed(t->thing_,t->entity_->api());
}

void tnode_t::timer_fast_callback(void *t_)
{
    tnode_t *t = (tnode_t *)t_;
    bct_thing_plug_timer_fast(t->thing_,t->entity_->api());
}

void tnode_t::timer_appq_callback(void *t_, const pia_data_t &d)
{
    tnode_t *t = (tnode_t *)t_;
    bct_thing_plug_timer_slow(t->thing_,t->entity_->api());
}

void tnode_t::timer_main_callback(void *t_)
{
    tnode_t *t = (tnode_t *)t_;
    t->entity_->appq()->idle(t->timer_slow_cpoint_,timer_appq_callback, t, pia_data_t());
}

void tnode_t::detach(int e)
{
    if(!open_.current())
    {
        return;
    }

    entity_->glue()->fastcall(cancel_timer_fast,this,0,0,0);

    open_.set(false);

    job_slow_timer_.cancel();
    job_close_.cancel();
    timer_slow_cpoint_->disable();
    trigger_slow_cpoint_->disable();
    dequeue_slow_cpoint_->disable();
    fast_cpoint_->disable();
    cpoint_->disable();

    remove();
    thing_->plg_state = PLG_STATE_DETACHED;

    if(e)
    {
        job_close_.idle(entity_->appq(),close_callback,this,pia_data_t());
    }
}

tnode_t::~tnode_t()
{
}

void tnode_t::close()
{
    detach(0);
    thing_->plg_state = PLG_STATE_CLOSED;
    delete this;
}

void tnode_t::api_close(bct_thing_host_ops_t **t_)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());

        t->close();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::queue_slow(const pia_data_t & d)
{
    pic::flipflop_t<bool>::guard_t g(open_);

    if(g.value())
    {
        entity_->appq()->idle(dequeue_slow_cpoint_,dequeue_slow_callback, this, d);
    }
}

void tnode_t::api_queue_slow(bct_thing_host_ops_t **t_, bct_data_t d)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        t->queue_slow(pia_data_t::from_lent(d));
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::queue_fast(const pia_data_nb_t & d, int p)
{
    pic::flipflop_t<bool>::guard_t g(open_);

    if(g.value())
    {
        if(p!=0)
        {
            if(entity_->glue()->fast_active())
            {
                entity_->glue()->fastq()->fastjob(fast_cpoint_,dequeue_fast_callback, this, d);
            }
            else
            {
                entity_->appq()->fastjob(fast_cpoint_,dequeue_fast_callback_deactive, this, d.make_normal(entity_->glue()->allocator(), PIC_ALLOC_NB));
            }
        }
        else
        {
            if(entity_->glue()->fast_active())
            {
                entity_->glue()->fastq()->idle(fast_cpoint_,dequeue_fast_callback, this, d);
            }
            else
            {
                entity_->appq()->idle(fast_cpoint_,dequeue_fast_callback_deactive, this, d.make_normal(entity_->glue()->allocator(), PIC_ALLOC_NB));
            }
        }
    }
}

void tnode_t::api_queue_fast(bct_thing_host_ops_t **t_, bct_data_t d, int p)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        t->queue_fast(pia_data_nb_t::from_lent(d),p);
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::defer_delete(bool(*cb)(void*), void *d, unsigned long ms)
{
    pic::flipflop_t<bool>::guard_t g(open_);

    if(g.value())
    {
        entity_->glue()->defer_delete(cb,d,ms);
    }
}

void tnode_t::api_defer_delete(bct_thing_host_ops_t **t_, bool(*cb)(void*), void *d, unsigned long ms)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        t->defer_delete(cb,d,ms);
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::trigger_fast()
{
    pic::flipflop_t<bool>::guard_t g(open_);

    if(g.value() && entity_->glue()->fast_active())
    {
        if(pic_atomiccas(&fast_latch_,0,1))
        {
            entity_->glue()->fastq()->idle(fast_cpoint_,trigger_fast_callback, this, pia_data_nb_t());
        }
    }
}

void tnode_t::api_trigger_fast(bct_thing_host_ops_t **t_)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        t->trigger_fast();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::trigger_slow()
{
    pic::flipflop_t<bool>::guard_t g(open_);

    if(g.value())
    {
        if(pic_atomiccas(&slow_latch_,0,1))
        {
            entity_->appq()->idle(trigger_slow_cpoint_,trigger_slow_callback, this, pia_data_t());
        }
    }
}

void tnode_t::flush_slow()
{
    pic::flipflop_t<bool>::guard_t g(open_);

    if(g.value())
    {
        trigger_slow_cpoint_->disable();
        dequeue_slow_cpoint_->disable();
        trigger_slow_cpoint_=pia_make_cpoint();
        dequeue_slow_cpoint_=pia_make_cpoint();
    }
}

void tnode_t::api_trigger_slow(bct_thing_host_ops_t **t_)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        t->trigger_slow();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::api_flush_slow(bct_thing_host_ops_t **t_)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        t->flush_slow();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

int tnode_t::timer_fast(void *self_, void *ms_, void *us_, void *)
{
    tnode_t *self = (tnode_t *)self_;
    unsigned long ms = *(unsigned long *)ms_;
    long us = *(long *)us_;

    pic::flipflop_t<bool>::guard_t g(self->open_);

    if(g.value() && self->entity_->glue()->fast_active())
    {
        self->timer_fast_cpoint_->disable();
        self->timer_fast_cpoint_=pia_make_cpoint();

        if(ms)
        {
            self->entity_->glue()->fastq()->timer(self->timer_fast_cpoint_,timer_fast_callback,self,ms,us);
        }
    }

    return 1;
}

int tnode_t::api_timer_fast(bct_thing_host_ops_t **t_, unsigned long ms, long us)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());
        t->entity_->glue()->fastcall(timer_fast,t,&ms,&us,0);
        return 0;
    }
    PIA_CATCHLOG_EREF(t->entity_)
    return -1;
}

int tnode_t::cancel_timer_fast(void *self_, void *,void *,void *)
{
    tnode_t *self = (tnode_t *)self_;

    pic::flipflop_t<bool>::guard_t g(self->open_);

    if(g.value())
    {
        self->timer_fast_cpoint_->disable();
        self->timer_fast_cpoint_=pia_make_cpoint();
    }

    return 1;
}

void tnode_t::api_cancel_timer_fast(bct_thing_host_ops_t **t_)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());
        t->entity_->glue()->fastcall(cancel_timer_fast,t,0,0,0);
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void tnode_t::timer_slow(unsigned long ms)
{
    if(open_.current())
    {
        job_slow_timer_.cancel();
        timer_slow_cpoint_->disable();
        timer_slow_cpoint_=pia_make_cpoint();

        if(ms)
        {
            job_slow_timer_.timer(entity_->glue()->mainq(),timer_main_callback,this,ms);
        }
    }
}

int tnode_t::api_timer_slow(bct_thing_host_ops_t **t_, unsigned long ms)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());

        t->timer_slow(ms);
        return 0;
    }
    PIA_CATCHLOG_EREF(t->entity_)
    return -1;
}

void tnode_t::cancel_timer_slow()
{
    if(open_.current())
    {
        job_slow_timer_.cancel();
        timer_slow_cpoint_->disable();
        timer_slow_cpoint_=pia_make_cpoint();
    }
}

void tnode_t::api_cancel_timer_slow(bct_thing_host_ops_t **t_)
{
    tnode_t *t = PIC_STRBASE(tnode_t,t_,host_ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());

        t->cancel_timer_slow();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

bct_thing_host_ops_t tnode_t::dispatch__ =
{
    api_close,
    api_trigger_fast,
    api_trigger_slow,
    api_queue_fast,
    api_queue_slow,
    api_defer_delete,
    api_timer_fast,
    api_timer_slow,
    api_cancel_timer_fast,
    api_cancel_timer_slow,
    api_flush_slow,
};

pia_thinglist_t::pia_thinglist_t()
{
    impl_ = new impl_t;
}

pia_thinglist_t::~pia_thinglist_t()
{
    try
    {
        kill(0);
    }
    PIA_CATCHLOG_PRINT()

    delete impl_;
}

void pia_thinglist_t::thing(const pia_ctx_t &e, bct_thing_t *s)
{
    new tnode_t(impl_,e,s);
}

tnode_t::tnode_t(pia_thinglist_t::impl_t *tl,const pia_ctx_t &e, bct_thing_t *s): list_(tl), thing_(s), entity_(e)
{
    open_.set(true);

    timer_slow_cpoint_ = pia_make_cpoint();
    trigger_slow_cpoint_ = pia_make_cpoint();
    dequeue_slow_cpoint_ = pia_make_cpoint();
    timer_fast_cpoint_=pia_make_cpoint();
    fast_cpoint_=pia_make_sync_cpoint();
    cpoint_ = pia_make_cpoint();

    fast_latch_=0;
    slow_latch_=0;

    host_ops_=&dispatch__;
    thing_->host_ops=&host_ops_;
    thing_->plg_state = PLG_STATE_OPENED;

    list_->things_.append(this);
}

void pia_thinglist_t::dump(const pia_ctx_t &e)
{
    tnode_t *i;

    i = impl_->things_.head();

    while(i)
    {
        if(i->entity_.matches(e))
        {
            pic::logmsg() << "thing " << i->entity_->tag();
        }

        i = impl_->things_.next(i);
    }
}

void pia_thinglist_t::kill(const pia_ctx_t &e)
{
    tnode_t *i,*n;

    i = impl_->things_.head();

    while(i)
    {
        n = impl_->things_.next(i);

        if(i->entity_.matches(e))
        {
            i->detach(1);
        }

        i=n;
    }
}
