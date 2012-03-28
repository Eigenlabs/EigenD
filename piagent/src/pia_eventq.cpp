
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

#include "pia_eventq.h"

#include <picross/pic_safeq.h>
#include <picross/pic_ilist.h>
#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_thread.h>

/*
 *  Synchronisation.  When a job is cancelled, the guarantee is that
 *  on return from the cancel operation, the job is not currently and
 *  will not run.  The exception is when cancel is called from within
 *  a callback function.
 *
 *  To do this, we have a fast internal queue which is serviced before
 *  running any job.  Invocation and Cancellation requests are put on
 *  this queue.  There is a gate which is closed when the loop is
 *  servicing jobs.  The queue runner closes the gate before servicing
 *  the fast queue.
 *  
 *  To cancel a job, we queue the cancellation request and then wait
 *  for the gate.  This ensures that no jobs are currently running, and
 *  we know that the cancellation will be processed before any
 *  subsequent jobs
 */

namespace
{
    template <class T> struct job_t;
    template <class T> struct idle_t;
    template <class T> struct idlecall_t;
    template <class T> struct fastjob_t;
    template <class T> struct slowjob_t;
    template <class T> struct evttimer_t;
}

template <class T> struct pia_eventq_impl_t
{
    pia_eventq_impl_t(pic::nballocator_t *a, void (*pinger)(void *), void *ctx): allocator_(a), pinger_(pinger), ctx_(ctx) { }

    void idle(const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T & d);
    void idlecall(const pia_cref_t &cp, void (*cb)(void *), void *ctx);
    void fastjob(const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T & d);
    void slowjob(const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T & d);
    void timer(const pia_cref_t &cp, void (*cb)(void *), void *ctx, unsigned long ms,long us);

    bool run(unsigned long long);
    unsigned long long next();

    void wait();
    void add(void (*)(void *,void *,void *,void *),void *,void *,void *,void *);

    pic::safeq_t safe_;

    pic::ilist_t<idle_t<T> > idle_;
    pic::ilist_t<idlecall_t<T> > idlecall_;
    pic::ilist_t<fastjob_t<T> > fastjob_;
    pic::ilist_t<slowjob_t<T> > slowjob_;
    pic::ilist_t<evttimer_t<T> > timer_;

    pic::nballocator_t * const allocator_;
    void (*pinger_)(void *);
    void *ctx_;

    void (*icurrent_)(void *,const T &);
    void (*iccurrent_)(void *);
    void (*fcurrent_)(void *,const T &);
    void (*scurrent_)(void *,const T &);
    void (*tcurrent_)(void *);

    void *icurrentctx_;
    void *iccurrentctx_;
    void *icallcurrentctx_;
    void *fcurrentctx_;
    void *scurrentctx_;
    void *tcurrentctx_;
};

namespace
{
    template <class T> struct job_t: virtual public pic::lckobject_t
    {
        inline job_t(pia_eventq_impl_t<T> *q,const pia_cref_t &cp): queue_(q),cpoint_(cp) { }
        pia_eventq_impl_t<T> *queue_;
        pia_cref_t cpoint_;
    };

    template <class T> struct idle_t: public job_t<T>, virtual public pic::lckobject_t, pic::element_t<>
    {
        inline idle_t(pia_eventq_impl_t<T> *q, const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T &d): job_t<T>(q,cp), cb_(cb), ctx_(ctx), d_(d.give_copy(q->allocator_)) { }
        bool run();

        void (*cb_)(void *, const T &);
        void *ctx_;
        bct_data_t d_;
    };

    template <class T> struct idlecall_t: public job_t<T>, virtual public pic::lckobject_t, pic::element_t<>
    {
        inline idlecall_t(pia_eventq_impl_t<T> *q, const pia_cref_t &cp, void (*cb)(void *), void *ctx): job_t<T>(q,cp), cb_(cb), ctx_(ctx) { }
        bool run();

        void (*cb_)(void *);
        void *ctx_;
    };

    template <class T> struct fastjob_t: public job_t<T>, virtual public pic::lckobject_t, pic::element_t<>
    {
        inline fastjob_t(pia_eventq_impl_t<T> *q, const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T &d): job_t<T>(q,cp), cb_(cb), ctx_(ctx), d_(d.give_copy(q->allocator_)) { }
        bool run();

        void (*cb_)(void *, const T &);
        void *ctx_;
        bct_data_t d_;
    };

    template <class T> struct slowjob_t: public job_t<T>, virtual public pic::lckobject_t, pic::element_t<>
    {
        inline slowjob_t(pia_eventq_impl_t<T> *q, const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T &d): job_t<T>(q,cp), cb_(cb), ctx_(ctx), d_(d.give_copy(q->allocator_)) { }
        bool run();

        void (*cb_)(void *, const T &);
        void *ctx_;
        bct_data_t d_;
    };

    template <class T> struct evttimer_t: public job_t<T>, virtual public pic::lckobject_t, pic::element_t<>
    {
        evttimer_t(pia_eventq_impl_t<T> *q, const pia_cref_t &cp, void (*cb)(void *), void *ctx, unsigned long ms, long us): job_t<T>(q,cp), cb_(cb), ctx_(ctx), period_(ms*1000+us) { }
        void insert();
        bool run();

        void (*cb_)(void *);
        void *ctx_;
        unsigned long long period_;
        unsigned long long scheduled_;
    };

}

template <class T> bool slowjob_t<T>::run()
{
    pia_cguard_t g(job_t<T>::cpoint_);

    if(g.enabled())
    {
        try
        {
            (cb_)(ctx_,T::from_given(d_));
        }
        CATCHLOG()

        return true;
    }

    return false;
}

template <class T> bool fastjob_t<T>::run()
{
    pia_cguard_t g(job_t<T>::cpoint_);

    if(g.enabled())
    {
        try
        {
            (cb_)(ctx_,T::from_given(d_));
        }
        CATCHLOG()

        return true;
    }

    return false;
}

template <class T> bool evttimer_t<T>::run()
{
    pia_cguard_t g(job_t<T>::cpoint_);

    if(g.enabled())
    {
        try
        {
            (cb_)(ctx_);
        }
        CATCHLOG()

        return true;
    }

    return false;
}

template <class T> bool idle_t<T>::run()
{
    pia_cguard_t g(job_t<T>::cpoint_);

    if(g.enabled())
    {
        try
        {
            (cb_)(ctx_,T::from_given(d_));
        }
        CATCHLOG()

        return true;
    }

    return false;
}

template <class T> bool idlecall_t<T>::run()
{
    pia_cguard_t g(job_t<T>::cpoint_);

    if(g.enabled())
    {
        try
        {
            (cb_)(ctx_);
        }
        CATCHLOG()

        return true;
    }

    return false;
}

template <class T> void evttimer_t<T>::insert()
{
    evttimer_t<T> *t = job_t<T>::queue_->timer_.head();

    if(!t)
    {
        job_t<T>::queue_->timer_.append(this);
        return;
    }

    while(t && t->scheduled_<scheduled_)
    {
        t=job_t<T>::queue_->timer_.next(t);
    }

    if(t)
    {
        job_t<T>::queue_->timer_.insert(this,t);
    }
    else
    {
        job_t<T>::queue_->timer_.append(this);
    }
}


/*
 * pia_eventq_impl_t
 */

template <class T> void pia_eventq_impl_t<T>::add(void (*cb)(void *,void *,void *,void *), void *c1, void *c2, void *c3,void *c4)
{
    if(safe_.add(cb,c1,c2,c3,c4))
    {
        if(pinger_)
        {
            try
            {
                (pinger_)(ctx_);
            }
            CATCHLOG()
        }
    }
}

static void synchronizer(void *gate_, void *, void *,void *)
{
    ((pic::semaphore_t *)gate_)->up();
}

template <class T> void pia_eventq_impl_t<T>::wait()
{
    pic::semaphore_t gate;
    add(synchronizer,&gate,0,0,0);
    gate.untimeddown();
}

template <class T> static void evttimer_insert(void *ctx1, void *, void *,void *)
{
    evttimer_t<T> *t = (evttimer_t<T> *)ctx1;

    t->scheduled_=pic_microtime()+t->period_;
    t->insert();
}

template <class T> static void idle_insert(void *ctx1, void *, void *,void *)
{
    idle_t<T> *i = (idle_t<T> *)ctx1;
    i->queue_->idle_.append(i);
}

template <class T> static void idlecall_insert(void *ctx1, void *, void *,void *)
{
    idlecall_t<T> *i = (idlecall_t<T> *)ctx1;
    i->queue_->idlecall_.append(i);
}

template <class T> static void fastjob_insert(void *ctx1, void *ctx2, void *,void *)
{
    fastjob_t<T> *e = (fastjob_t<T> *)ctx1;
    e->queue_->fastjob_.append(e);
}

template <class T> static void slowjob_insert(void *ctx1, void *ctx2, void *,void *)
{
    slowjob_t<T> *e = (slowjob_t<T> *)ctx1;
    e->queue_->slowjob_.append(e);
}

template <class T> void pia_eventq_impl_t<T>::idle(const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T & d)
{
    idle_t<T> *i = new idle_t<T>(this,cp,cb,ctx,d);
    add(idle_insert<T>,i,0,0,0);
}

template <class T> void pia_eventq_impl_t<T>::idlecall(const pia_cref_t &cp, void (*cb)(void *), void *ctx)
{
    idlecall_t<T> *i = new idlecall_t<T>(this,cp,cb,ctx);
    add(idlecall_insert<T>,i,0,0,0);
}

template <class T> void pia_eventq_impl_t<T>::slowjob(const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T & d)
{
    slowjob_t<T> *i = new slowjob_t<T>(this,cp,cb,ctx,d);
    add(slowjob_insert<T>,i,0,0,0);
}

template <class T> void pia_eventq_impl_t<T>::fastjob(const pia_cref_t &cp, void (*cb)(void *, const T &), void *ctx, const T & d)
{
    fastjob_t<T> *i = new fastjob_t<T>(this,cp,cb,ctx,d);
    add(fastjob_insert<T>,i,0,0,0);
}

template <class T> void pia_eventq_impl_t<T>::timer(const pia_cref_t &cp, void (*cb)(void *), void *ctx, unsigned long ms,long us)
{
    evttimer_t<T> *i = new evttimer_t<T>(this,cp,cb,ctx,ms,us);
    add(evttimer_insert<T>,i,0,0,0);
}

template <class T> bool pia_eventq_impl_t<T>::run(unsigned long long now)
{
    evttimer_t<T> *t;
    fastjob_t<T> *e;
    slowjob_t<T> *s;
    idle_t<T> *i;
    idlecall_t<T> *ic;
    bool a = false;

    safe_.run();

    while((t=timer_.head())!=0  && t->scheduled_ <= now)
    {
        tcurrent_=t->cb_;
        tcurrentctx_=t->ctx_;
        t->remove();
        if(t->run())
        {
            t->scheduled_+=t->period_;
            t->insert();
        }
        else
        {
            delete t;
        }
        tcurrent_=0;
        tcurrentctx_=0;
        a=true;
        safe_.run();
    }

restart:

    while((e=fastjob_.head())!=0)
    {
        fcurrent_=e->cb_;
        fcurrentctx_=e->ctx_;
        e->remove();

        e->run();
        a=true;
        delete e;
        fcurrent_=0;
        fcurrentctx_=0;
        safe_.run();
    }

    if((ic=idlecall_.head())!=0)
    {
        iccurrent_=ic->cb_;
        iccurrentctx_=ic->ctx_;
        ic->remove();
        ic->run();
        a=true;
        delete ic;
        iccurrent_=0;
        iccurrentctx_=0;
        safe_.run();
        goto restart;
    }

    if((i=idle_.head())!=0)
    {
        icurrent_=i->cb_;
        icurrentctx_=i->ctx_;
        i->remove();
        i->run();
        a=true;
        delete i;
        icurrent_=0;
        icurrentctx_=0;
        safe_.run();
        goto restart;
    }

    if((s=slowjob_.head())!=0)
    {
        scurrent_=s->cb_;
        scurrentctx_=s->ctx_;
        s->remove();
        s->run();
        a=true;
        delete s;
        scurrent_=0;
        scurrentctx_=0;
        safe_.run();
        goto restart;
    }

    return a;
}

template <class T> unsigned long long pia_eventq_impl_t<T>::next()
{
    evttimer_t<T> *t = timer_.head();
    return t?t->scheduled_:~0ULL;
}


/*
 * pia_eventq_t::impl_t
 */

struct pia_eventq_t::impl_t: virtual public pic::lckobject_t
{
    impl_t(pic::nballocator_t *a, void (*pinger)(void *), void *ctx): impl_(a, pinger, ctx) { }

    pia_eventq_impl_t<pia_data_t> impl_;
};


/*
 * pia_eventq_t
 */

pia_eventq_t::pia_eventq_t(pic::nballocator_t *a, void (*pinger)(void *), void *ctx): impl_(new impl_t(a,pinger,ctx))
{
}

pia_eventq_t::~pia_eventq_t()
{
    delete impl_;
}

void pia_eventq_t::idlecall(const pia_cref_t &cp,void (*cb)(void *), void *ctx)
{
    impl_->impl_.idlecall(cp,cb,ctx);
}

void pia_eventq_t::idle(const pia_cref_t &cp,void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t & d)
{
    impl_->impl_.idle(cp,cb,ctx,d);
}

void pia_eventq_t::slowjob(const pia_cref_t &cp,void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t & d)
{
    impl_->impl_.slowjob(cp,cb,ctx,d);
}

void pia_eventq_t::fastjob(const pia_cref_t &cp,void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t & d)
{
    impl_->impl_.fastjob(cp,cb,ctx,d);
}

void pia_eventq_t::timer(const pia_cref_t &cp,void (*cb)(void *), void *ctx, unsigned long ms, long us)
{
    impl_->impl_.timer(cp,cb,ctx,ms,us);
}

unsigned long long pia_eventq_t::next()
{
    return impl_->impl_.next();
}

bool pia_eventq_t::run(unsigned long long now)
{
    return impl_->impl_.run(now);
}


/*
 * pia_eventq_nb_t::impl_nb_t
 */

struct pia_eventq_nb_t::impl_nb_t: virtual public pic::lckobject_t
{
    impl_nb_t(pic::nballocator_t *a, void (*pinger)(void *), void *ctx): impl_(a, pinger, ctx) { }

    pia_eventq_impl_t<pia_data_nb_t> impl_;
};


/*
 * pia_eventq_nb_t
 */

pia_eventq_nb_t::pia_eventq_nb_t(pic::nballocator_t *a, void (*pinger)(void *), void *ctx): impl_(new impl_nb_t(a,pinger,ctx))
{
}

pia_eventq_nb_t::~pia_eventq_nb_t()
{
    delete impl_;
}

void pia_eventq_nb_t::idlecall(const pia_cref_t &cp,void (*cb)(void *), void *ctx)
{
    impl_->impl_.idlecall(cp,cb,ctx);
}

void pia_eventq_nb_t::idle(const pia_cref_t &cp,void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t & d)
{
    impl_->impl_.idle(cp,cb,ctx,d);
}

void pia_eventq_nb_t::slowjob(const pia_cref_t &cp,void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t & d)
{
    impl_->impl_.slowjob(cp,cb,ctx,d);
}

void pia_eventq_nb_t::fastjob(const pia_cref_t &cp,void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t & d)
{
    impl_->impl_.fastjob(cp,cb,ctx,d);
}

void pia_eventq_nb_t::timer(const pia_cref_t &cp,void (*cb)(void *), void *ctx, unsigned long ms, long us)
{
    impl_->impl_.timer(cp,cb,ctx,ms,us);
}

unsigned long long pia_eventq_nb_t::next()
{
    return impl_->impl_.next();
}

bool pia_eventq_nb_t::run(unsigned long long now)
{
    return impl_->impl_.run(now);
}


/*
 * pia_job_t
 */

void pia_job_t::idlecall(pia_eventq_t *l, void (*cb)(void *), void *ctx)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.idlecall(cpoint_,cb,ctx);
}

void pia_job_t::idle(pia_eventq_t *l, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t & d)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.idle(cpoint_,cb,ctx,d);
}

void pia_job_t::slowjob(pia_eventq_t *l, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t & d)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.slowjob(cpoint_,cb,ctx,d);
}

void pia_job_t::fastjob(pia_eventq_t *l, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t & d)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.fastjob(cpoint_,cb,ctx,d);
}

void pia_job_t::timer(pia_eventq_t *l, void (*cb)(void *), void *ctx, unsigned long ms, long us)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.timer(cpoint_,cb,ctx,ms,us);
}


/*
 * pia_job_nb_t
 */

void pia_job_nb_t::idlecall(pia_eventq_nb_t *l, void (*cb)(void *), void *ctx)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.idlecall(cpoint_,cb,ctx);
}

void pia_job_nb_t::idle(pia_eventq_nb_t *l, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t & d)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.idle(cpoint_,cb,ctx,d);
}

void pia_job_nb_t::slowjob(pia_eventq_nb_t *l, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t & d)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.slowjob(cpoint_,cb,ctx,d);
}

void pia_job_nb_t::fastjob(pia_eventq_nb_t *l, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t & d)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.fastjob(cpoint_,cb,ctx,d);
}

void pia_job_nb_t::timer(pia_eventq_nb_t *l, void (*cb)(void *), void *ctx, unsigned long ms, long us)
{
    cancel();
    cpoint_=pia_make_cpoint();
    l->impl_->impl_.timer(cpoint_,cb,ctx,ms,us);
}
