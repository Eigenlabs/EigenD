
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

#include <piw/piw_mixer.h>
#include <piw/piw_clock.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_tsd.h>

#include <picross/pic_float.h>

#define CLKSTATE_IDLE 0
#define CLKSTATE_SHUTDOWN 1
#define CLKSTATE_RUNNING 2
#define CLKSTATE_STARTUP 3

namespace
{
    struct stereosummer_wire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        stereosummer_wire_t(piw::stereosummer_t::impl_t *impl,const piw::event_data_source_t &es);
        ~stereosummer_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned s,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void ticked(unsigned long long t);

        piw::stereosummer_t::impl_t *impl_;
        piw::xevent_data_buffer_t::iter_t iterator_;
    };
};

struct piw::stereosummer_t::impl_t: piw::root_t, piw::root_ctl_t, piw::wire_ctl_t, piw::clocksink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, virtual public pic::tracked_t
{
    impl_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c, unsigned n);

    ~impl_t()
    {
        tracked_invalidate();
        invalidate();
    }

    void invalidate();

    piw::wire_t *root_wire(const piw::event_data_source_t &es);

    void root_clock();
    void root_latency() { set_sink_latency(get_latency()); set_latency(get_latency()); }
    void root_opened();
    void root_closed() {}

    void source_ended(unsigned) { }

    void enqueue(unsigned s,const piw::data_nb_t &d);
    void set_channels(unsigned);

    void clocksink_ticked(unsigned long long from,unsigned long long t);

    bct_clocksink_t *up_;
    std::list<stereosummer_wire_t *> inputs_;
    int state_;
    unsigned channels_;
    unsigned long long sigmask_;

    pic::lckvector_t<pic::lcklist_t<piw::data_nb_t>::nbtype>::nbtype queue_;
    piw::xevent_data_buffer_t buffer_;
    pic::ilist_t<stereosummer_wire_t> active_;
};

stereosummer_wire_t::stereosummer_wire_t(piw::stereosummer_t::impl_t *impl, const piw::event_data_source_t &es) : impl_(impl)
{
    impl_->inputs_.push_back(this);
    subscribe(es);
}

void stereosummer_wire_t::event_start(unsigned s,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei)
{
    piw::data_nb_t d2;
    iterator_=ei.iterator();
    for(unsigned c=0;c<impl_->channels_;c++)
    {
        iterator_->reset(c+1,d.time());
    }

    impl_->active_.append(this);

    if(impl_->state_==CLKSTATE_IDLE)
    {
        impl_->tick_suppress(false);
        impl_->state_=CLKSTATE_STARTUP;
    }
}

void stereosummer_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    iterator_->set_signal(s,n);
    iterator_->reset(s,t);
}

void stereosummer_wire_t::ticked(unsigned long long t)
{
    for(unsigned c=0;c<impl_->channels_;c++)
    {
        piw::data_nb_t d;
        while(iterator_->nextsig(c+1,d,t))
            impl_->enqueue(c,d);
    }
}

bool stereosummer_wire_t::event_end(unsigned long long et)
{
    iterator_.clear();
    remove();
    return true;
}

void stereosummer_wire_t::invalidate()
{
    unsubscribe();
    wire_t::disconnect();
    impl_->inputs_.remove(this);
}

piw::stereosummer_t::impl_t::impl_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c, unsigned n): root_t(0), piw::event_data_source_real_t(piw::pathnull(0)), up_(0), state_(CLKSTATE_IDLE), channels_(n)
{
    queue_.resize(n);
    sigmask_=(1ULL<<n)-1;
    connect(c);

    d->sink(this,"stereosummer");
    tick_enable(true);
    set_clock(this);

}

static int resizer__(void *self_, void *c_)
{
    piw::stereosummer_t::impl_t *self = (piw::stereosummer_t::impl_t *)self_;
    unsigned c = *(unsigned *)c_;

    while(c>self->channels_)
    {
        self->channels_++;
        self->queue_.resize(self->channels_);
        self->buffer_.set_signal(self->channels_,piw::tsd_dataqueue(2));
    }

    while(c<self->channels_)
    {
        self->buffer_.set_signal(self->channels_,piw::dataqueue_t());
        self->channels_--;
        self->queue_.resize(self->channels_);
    }

    self->sigmask_=(1ULL<<c)-1;

    return 0;
}

void piw::stereosummer_t::impl_t::set_channels(unsigned c)
{
    piw::tsd_fastcall(resizer__,this,&c);
}

piw::wire_t *piw::stereosummer_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    return new stereosummer_wire_t(this,es);
}

void piw::stereosummer_t::impl_t::root_clock()
{
    bct_clocksink_t *s(get_clock());

    if(up_)
    {
        remove_upstream(up_);
        up_ = 0;
    }
    if(s)
    {
        up_ = s;
        add_upstream(s);
    }
}

void piw::stereosummer_t::impl_t::root_opened()
{
    root_clock(); root_latency();
    connect_wire(this,source());
}

void piw::stereosummer_t::impl_t::invalidate()
{
    std::list<stereosummer_wire_t *>::iterator i;
    source_shutdown();

    while((i=inputs_.begin())!=inputs_.end())
    {
        delete (*i);
    }
}

void piw::stereosummer_t::impl_t::clocksink_ticked(unsigned long long from,unsigned long long t)
{
    if(state_==CLKSTATE_STARTUP)
    {
        buffer_=piw::xevent_data_buffer_t(sigmask_,PIW_DATAQUEUE_SIZE_ISO);
    }

    stereosummer_wire_t *w=active_.head();
    if(!w)
    {
        state_=CLKSTATE_SHUTDOWN;
    }

    while(w)
    {
        w->ticked(t);
        w=active_.next(w);
    }

    float *f,*fs;
    piw::data_nb_t d;
    unsigned bs = get_buffer_size();

    for(unsigned c=0;c<channels_;c++)
    {
        if(queue_[c].size())
        {
            d = piw::makenorm_nb(t,bs,&f,&fs);
            memset(f,0,bs*sizeof(float));

            while(queue_[c].size())
            {
                const float *df = queue_[c].front().as_array();
                unsigned dfl = std::min(bs,queue_[c].front().as_arraylen());
                pic::vector::vectadd(df,1,f,1,f,1,dfl);
                queue_[c].pop_front();
            }

            *fs=f[bs-1];

            buffer_.add_value(c+1,d);
        }
    }

    if(state_==CLKSTATE_STARTUP)
    {
        state_=CLKSTATE_RUNNING;
        source_start(0,piw::pathnull_nb(t),buffer_);
    }

    if(state_==CLKSTATE_SHUTDOWN)
    {
        source_end(t);
        state_ = CLKSTATE_IDLE;
        tick_suppress(true);
    }
}

void piw::stereosummer_t::impl_t::enqueue(unsigned c,const piw::data_nb_t &d)
{
    if(c<channels_)
    {
        queue_[c].push_back(d);
        if(state_==CLKSTATE_SHUTDOWN)
            state_ = CLKSTATE_RUNNING;
    }
}

void piw::stereosummer_t::set_channels(unsigned n)
{
    impl_->set_channels(n);
}

piw::stereosummer_t::stereosummer_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c, unsigned n) : impl_(new impl_t(d,c,n))
{
}

piw::stereosummer_t::~stereosummer_t()
{
    delete impl_;
}

piw::cookie_t piw::stereosummer_t::cookie()
{
    return piw::cookie_t(impl_);
}
