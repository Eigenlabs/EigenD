
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

#include "pia_dataqueue.h"
#include "pia_data.h"

#include <picross/pic_fastalloc.h>
#include <picross/pic_stl.h>
#include <picross/pic_log.h>
#include <picross/pic_time.h>


namespace
{
    struct queue_t : bct_dataqueue_s, virtual public pic::lckobject_t
    {
        queue_t(unsigned size);
        int write(const pia_data_nb_t &d);
        int read(pia_data_nb_t &, unsigned long long *i, unsigned long long t);
        int earliest(pia_data_nb_t &, unsigned long long *i, unsigned long long t);
        int latest(pia_data_nb_t &, unsigned long long *i, unsigned long long t);
        void subscribe(pia_dataqueue_t::subscriber_t *s);
        void broadcast(const pia_data_nb_t &d);
        void dump(pic::msg_t &o, bool full);

        unsigned size_;
        pic::lckvector_t<pia_data_nb_t>::nbtype queue_;
        unsigned long long write_;
        unsigned long long time_;
        pic::ilist_t<pia_dataqueue_t::subscriber_t,DATAQUEUE_SUBSCRIBER> subscribers_;
        pia_data_nb_t current_;
        unsigned dropped_;

        static void api_free(const bct_dataqueue_t);
        static int api_write(const bct_dataqueue_t, bct_data_t);
        static int api_read(const bct_dataqueue_t, bct_data_t *,unsigned long long *, unsigned long long);
        static int api_earliest(const bct_dataqueue_t, bct_data_t *,unsigned long long *, unsigned long long);
        static int api_latest(const bct_dataqueue_t, bct_data_t *,unsigned long long *, unsigned long long);
        static void api_dump(const bct_dataqueue_t,int);
        static bct_data_t api_current(const bct_dataqueue_t);

        static bct_dataqueue_host_ops_t dispatch__;
    };
}

int queue_t::write(const pia_data_nb_t &d)
{
    //PIC_ASSERT(d.time()!=0);
    PIC_ASSERT(!d.is_null());

    unsigned long long t = d.time();
    if(t<time_)
    {
        //pic::logmsg() << "time adjusted forward from " << t << " to " << time_;
        //t=time_;
        if(dropped_==0)
        {
            pic::logmsg() << "write dropped ts=" << t << " current=" << time_;
        }
        ++dropped_;
        return 1;
    }

    if(dropped_!=0)
    {
        pic::logmsg() << dropped_ << " write(s) dropped";
        dropped_ = 0;
    }
 
    if(write_!=0 && t==time_)
    {
        --write_;
    }

    current_ = d;
    queue_[write_%size_] = d;
    ++write_;
    time_ = t;

    broadcast(d);
    return 1;
}

void queue_t::broadcast(const pia_data_nb_t &d)
{
    pia_dataqueue_t::subscriber_t *s = subscribers_.head(), *n;
    while(s)
    {
        n = subscribers_.next(s);
        if(!s->receive_data(d))
        {
            subscribers_.remove(s);
        }
        s = n;
    }
}

int queue_t::read(pia_data_nb_t &d, unsigned long long *x, unsigned long long t)
{
    unsigned long long i = *x;

    if(i==write_)
    {
        return 0;
    }

    if(i>write_)
    {
        *x=write_;
        return 0;
    }

    if(write_>(i+size_))
    {
        pic::logmsg() << "read too far in past: read=" << i << " write=" << write_ << " size=" << size_;
        i=write_-size_;
        *x=i;
    }

    pia_data_nb_t r = queue_[i%size_];

    if(r.time()>t)
    {
        return 0;
    }

    d = r;
    return 1;
}

int queue_t::earliest(pia_data_nb_t &d, unsigned long long *i, unsigned long long t)
{
    if(i) *i = write_;

    if(write_==0)
    {
        return 0;
    }

    unsigned long long x = (write_>=size_) ? (write_-size_) : 0ULL;
    unsigned long long end = write_;

    for(; x<end; ++x)
    {
        pia_data_nb_t r = queue_[x%size_];
        if(r.time()>=t)
        {
            d = r;
            if(i) *i = x;
            return 1;
        }
    }

    return 0;
}

int queue_t::latest(pia_data_nb_t &d, unsigned long long *i, unsigned long long t)
{
    //if(i) *i = write_;

    if(write_==0)
    {
        return 0;
    }

    long long x = write_-1;
    long long start = (write_>=size_) ? (write_-size_) : 0ULL;

    for(; x>=start; --x)
    {
        pia_data_nb_t r = queue_[x%size_];
        if(r.time()<=t)
        {
            d = r;
            if(i) *i = x;
            return 1;
        }
    }

    if(i) *i = start;
    return 0;
}

void queue_t::subscribe(pia_dataqueue_t::subscriber_t *s)
{
    subscribers_.append(s);
}

bct_dataqueue_host_ops_t queue_t::dispatch__ = 
{
    api_free,
    api_write,
    api_read,
    api_earliest,
    api_latest,
    api_dump,
    api_current,
};

pia_dataqueue_t pia_dataqueue_t::alloc(unsigned size)
{
    queue_t *ret = new queue_t(size);
    ret->host_ops = &queue_t::dispatch__;
    ret->count = 1;
    return pia_dataqueue_t::from_given(ret);
}

queue_t::queue_t(unsigned size): size_(size), queue_(size), write_(0), time_(0), dropped_(0)
{
}

void pia_dataqueue_t::subscribe(subscriber_t *sub) { ((queue_t *)queue_)->subscribe(sub); }
int pia_dataqueue_t::write(const pia_data_nb_t &d) { return ((queue_t *)queue_)->write(d); }
int pia_dataqueue_t::read(pia_data_nb_t &d, unsigned long long *i, unsigned long long t) const { return ((queue_t *)queue_)->read(d,i,t); }
int pia_dataqueue_t::latest(pia_data_nb_t &d, unsigned long long *i, unsigned long long t) const { return ((queue_t *)queue_)->latest(d,i,t); }
int pia_dataqueue_t::earliest(pia_data_nb_t &d, unsigned long long *i, unsigned long long t) const { return ((queue_t *)queue_)->earliest(d,i,t); }
pia_data_nb_t pia_dataqueue_t::current_nb() const { return isvalid() ? ((queue_t *)queue_)->current_ : pia_data_nb_t(); }
pia_data_t pia_dataqueue_t::current_normal(pic::nballocator_t *a) const { return isvalid() ? ((queue_t *)queue_)->current_.make_normal(a, PIC_ALLOC_NORMAL) : pia_data_t(); }

void queue_t::api_free(const bct_dataqueue_t q) { delete (queue_t *)q; }
int queue_t::api_write(const bct_dataqueue_t q_, bct_data_t d)
{
    queue_t *q = (queue_t *)q_;
    return q->write(pia_data_nb_t::from_lent(d));
}

int queue_t::api_read(const bct_dataqueue_t q, bct_data_t *d, unsigned long long *i, unsigned long long t)
{
    pia_data_nb_t d2;
    int rc=((queue_t *)q)->read(d2,i,t);
    if(rc>0)
    {
        *d=d2.give();
    }
    return rc;
}

int queue_t::api_earliest(const bct_dataqueue_t q, bct_data_t *d, unsigned long long *i, unsigned long long t)
{
    pia_data_nb_t d2;
    int rc=((queue_t *)q)->earliest(d2,i,t);
    if(rc>0)
    {
        *d=d2.give();
    }
    return rc;
}

int queue_t::api_latest(const bct_dataqueue_t q, bct_data_t *d, unsigned long long *i, unsigned long long t)
{
    pia_data_nb_t d2;
    int rc=((queue_t *)q)->latest(d2,i,t);
    if(rc>0)
    {
        *d=d2.give();
    }
    return rc;
}

void queue_t::api_dump(const bct_dataqueue_t q_, int full)
{
    queue_t *q = (queue_t *)q_;
    pic::msg_t m(pic::logmsg());
    q->dump(m,full!=0);
}

bct_data_t queue_t::api_current(const bct_dataqueue_t q_)
{
    queue_t *q = (queue_t *)q_;
    return q?q->current_.give():0;
}

void queue_t::dump(pic::msg_t &o, bool full)
{
    o << (void *)this << " sz: " << size_ << " wptr: " << write_ << " mod: " << (write_%size_) << " cur: " << current_ <<  " @" << current_.time();
    if(full)
    {
        for(unsigned i=0; i<size_; ++i)
        {
            o << "\n" << i << ":" << queue_[i] << " @" << queue_[i].time();
        }
    }
}

