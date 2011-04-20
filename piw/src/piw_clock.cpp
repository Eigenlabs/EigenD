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

#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <picross/pic_weak.h>

#define SINK_TOO_LONG   10000
#define SOURCE_TOO_LONG 10000

void piw::clocksource_t::closed_thunk(bct_clocksource_t *s_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<clocksource_t> k = (clocksource_t *)s_;

    try
    {
        k->clocksource_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_source();
        }
        CATCHLOG()
    }
}

static int ticker__(void *a1, void *a2)
{
    piw::clocksource_t *s = (piw::clocksource_t *)a1;
    unsigned long long *tp = (unsigned long long *)a2;
    s->tick(*tp);
    return 0;
}

void piw::clocksource_t::tick_slow(unsigned long long t)
{
    if(running())
    {
        piw::tsd_fastcall(ticker__,this,&t);
    }
}

bct_clocksource_plug_ops_t piw::clocksource_t::dispatch__ =
{
    closed_thunk
};

piw::clocksource_t::clocksource_t()
{
    host_ops = 0;
    plg_state = PLG_STATE_CLOSED;
    plug_ops = &dispatch__;
}

piw::clocksource_t::~clocksource_t()
{
    tracked_invalidate();
    close_source();
}

void piw::clocksource_t::set_details(unsigned bs, unsigned long sr)
{
    if(running())
    {
        bct_clocksource_host_set_details(this,bs,sr);
    }
}

void piw::clocksource_t::close_source()
{
    if(host_ops)
    {
        bct_clocksource_host_close(this);
        host_ops = 0;
    }
}

void piw::clocksource_t::tick(unsigned long long t)
{
    if(running())
    {
        piw::tsd_protect_t p;
        PIC_ASSERT(open());
        bct_clocksource_host_tick(this, t);
    }
}

void piw::clocksource_t::clocksource_closed()
{
    close_source();
}

void piw::clockdomain_t::source_changed_thunk(bct_clockdomain_t *d_, bct_entity_t x)
{
    tsd_setcontext(x);
    clockdomain_t *d = (clockdomain_t *)d_;
    try
    {
        d->clockdomain_source_changed();
    }
    CATCHLOG()
}

void piw::clockdomain_t::closed_thunk(bct_clockdomain_t *d_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<clockdomain_t> k = (clockdomain_t *)d_;

    try
    {
        k->clockdomain_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_domain();
        }
        CATCHLOG()
    }
}

bct_clockdomain_plug_ops_t piw::clockdomain_t::dispatch__ =
{
    source_changed_thunk,
    closed_thunk
};

piw::clockdomain_t::clockdomain_t()
{
    host_ops = 0;
    plg_state = PLG_STATE_CLOSED;
    plug_ops = &dispatch__;
}

piw::clockdomain_t::~clockdomain_t()
{
    tracked_invalidate();
    close_domain();
}

void piw::clockdomain_t::close_domain()
{
    if(host_ops)
    {
        bct_clockdomain_host_close(this);
        host_ops = 0;
    }
}

void piw::clockdomain_t::set_source(const piw::data_t &s)
{
    PIC_ASSERT(host_ops);
    bct_clockdomain_host_set_source(this, s.lend());
}

void piw::clockdomain_t::sink(bct_clocksink_t *s,const char *n)
{
    PIC_ASSERT(host_ops);
    bct_clockdomain_host_clocksink(this, s, n);
}

piw::data_t piw::clockdomain_t::source_name()
{
    PIC_ASSERT(host_ops);
    return piw::data_t::from_given(bct_clockdomain_host_source_name(this));
}

unsigned piw::clocksink_t::get_buffer_size()
{
    PIC_ASSERT(host_ops);
    return bct_clocksink_host_buffer_size(this);
}

unsigned long piw::clocksink_t::get_sample_rate()
{
    PIC_ASSERT(host_ops);
    return bct_clocksink_host_sample_rate(this);
}

unsigned piw::clockdomain_t::buffer_size()
{
    PIC_ASSERT(host_ops);
    return bct_clockdomain_host_buffer_size(this);
}

unsigned long piw::clockdomain_t::sample_rate()
{
    PIC_ASSERT(host_ops);
    return bct_clockdomain_host_sample_rate(this);
}

void piw::clockdomain_t::clockdomain_closed()
{
    close_domain();
}

void piw::clockdomain_t::clockdomain_source_changed()
{
}

void piw::clocksink_t::ticked_thunk(bct_clocksink_t *s_, bct_entity_t x, unsigned long long f, unsigned long long t)
{
    tsd_setcontext(x);
    clocksink_t *c = (clocksink_t *)s_;

    try
    {
        //unsigned l=c->latency_;
        //c->clocksink_ticked(f-l,t-l);
        unsigned long long t0 = tsd_time();
        c->clocksink_ticked(f,t);
        unsigned long long t1 = tsd_time();
        if((t1-t0)>SINK_TOO_LONG)
            pic::logmsg() << " ******* clock tick took " << (t1-t0) << "us  at " << (double)t0/1000000.0;
    }
    CATCHLOG()
}

void piw::clocksink_t::closed_thunk(bct_clocksink_t *s_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<clocksink_t> k = (clocksink_t *)s_;

    try
    {
        k->clocksink_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_sink();
        }
        CATCHLOG()
    }
}

bct_clocksink_plug_ops_t piw::clocksink_t::dispatch__ =
{
    ticked_thunk,
    closed_thunk
};

piw::clocksink_t::clocksink_t(): latency_(0)
{
    host_ops = 0;
    plg_state = PLG_STATE_CLOSED;
    plug_ops = &dispatch__;
}

piw::clocksink_t::~clocksink_t()
{
    tracked_invalidate();
    close_sink();
}

void piw::clocksink_t::close_sink()
{
    if(host_ops)
    {
        bct_clocksink_host_close(this);
        host_ops = 0;
    }
}

void piw::clocksink_t::tick()
{
    PIC_ASSERT(host_ops);
    bct_clocksink_host_tick(this);
}

void piw::clocksink_t::tick2(void (*cb)(unsigned long long,unsigned long long,void *),void *ctx)
{
    PIC_ASSERT(host_ops);
    bct_clocksink_host_tick2(this,cb,ctx);
}

unsigned long long piw::clocksink_t::current_tick()
{
    PIC_ASSERT(host_ops);
    return bct_clocksink_host_current_tick(this);
}

void piw::clocksink_t::add_downstream(bct_clocksink_t *down)
{
    if(bct_clocksink_host_add_upstream(down, this) < 0)
    {
        PIC_THROW("add_upstream failed");
    }
}

void piw::clocksink_t::remove_downstream(bct_clocksink_t *down)
{
    if(bct_clocksink_host_remove_upstream(down,this) < 0)
    {
        PIC_THROW("remove_upstream failed");
    }
}

void piw::clocksink_t::add_upstream(bct_clocksink_t *up)
{
    PIC_ASSERT(open());
    if(bct_clocksink_host_add_upstream(this, up) < 0)
    {
        PIC_THROW("add_upstream failed");
    }
}

void piw::clocksink_t::remove_upstream(bct_clocksink_t *up)
{
    if(!open())
    {
        return;
    }
    if(bct_clocksink_host_remove_upstream(this, up) < 0)
    {
        PIC_THROW("remove_upstream failed");
    }
}

void piw::clocksink_t::clocksink_closed()
{
    close_sink();
}

void piw::clocksink_t::tick_suppress(bool s)
{
    if(!open())
    {
        if(s)
        {
            return;
        }

        PIC_THROW("suppress failed");
    }

    if(bct_clocksink_host_suppress(this,s?1:0) < 0)
    {
        PIC_THROW("suppress failed");
    }
}

void piw::clocksink_t::tick_enable(bool s)
{
    if(!open() || bct_clocksink_host_enable(this,1,s?1:0)<0)
    {
        PIC_THROW("enable failed");
    }
}

void piw::clocksink_t::tick_disable()
{
    if(open())
    {
        if(bct_clocksink_host_enable(this,0,0) < 0)
        {
            PIC_THROW("enable failed");
        }
    }
}

struct piw::clockdomain_ctl_t::impl_t : piw::clockdomain_t
{
    impl_t(): listeners_(pic::notify_t::list()) {}
    void clockdomain_source_changed() { listeners_(); }
    void add_listener(const pic::notify_t &l) { pic::functorlist_connect(listeners_,l); }
    void remove_listener(const pic::notify_t &l) { pic::functorlist_disconnect(listeners_,l); }
    pic::notify_t listeners_;
};

piw::clockdomain_ctl_t::clockdomain_ctl_t(): domain_(new impl_t())
{
    tsd_clockdomain(domain_);
}

piw::clockdomain_ctl_t::~clockdomain_ctl_t()
{
    tracked_invalidate();
    delete domain_;
}

int piw::clockdomain_ctl_t::gc_traverse(void *v, void *a) const
{
    return domain_->listeners_.gc_traverse(v,a);
}

int piw::clockdomain_ctl_t::gc_clear()
{
    return domain_->listeners_.gc_clear();
}

void piw::clockdomain_ctl_t::add_listener(const pic::notify_t &l)
{
    domain_->add_listener(l);
}

void piw::clockdomain_ctl_t::remove_listener(const pic::notify_t &l)
{
    domain_->remove_listener(l);
}

void piw::clockdomain_ctl_t::set_source(const piw::data_t &n)
{
    domain_->set_source(n);
}

void piw::clockdomain_ctl_t::sink(bct_clocksink_t *s,const char *n)
{
    domain_->sink(s,n);
}

piw::data_t piw::clockdomain_ctl_t::get_source()
{
    return domain_->source_name();
}

unsigned piw::clockdomain_ctl_t::get_buffer_size()
{
    return domain_->buffer_size();
}

unsigned long piw::clockdomain_ctl_t::get_sample_rate()
{
    return domain_->sample_rate();
}
