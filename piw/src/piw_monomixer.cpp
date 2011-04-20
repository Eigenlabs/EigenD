
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
    struct monomixer_wire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        monomixer_wire_t(piw::monomixer_t::impl_t *impl,const piw::event_data_source_t &es);
        ~monomixer_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void ticked(unsigned long long t);

        piw::monomixer_t::impl_t *impl_;
        piw::dataqueue_t input_;
        unsigned long long index_;
    };
};

struct piw::monomixer_t::impl_t: piw::root_t, piw::root_ctl_t, piw::wire_ctl_t, piw::clocksink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
{
    impl_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c);
    ~impl_t() { invalidate(); }
    void invalidate();

    piw::wire_t *root_wire(const piw::event_data_source_t &es);

    void root_clock();
    void root_latency() { set_sink_latency(get_latency()); set_latency(get_latency()); }
    void root_opened();
    void root_closed() { delete this; }

    void source_ended(unsigned seq) { }

    void enqueue(const piw::data_nb_t &d);
    void clocksink_ticked(unsigned long long from,unsigned long long t);

    bct_clocksink_t *up_;
    std::list<monomixer_wire_t *> inputs_;
    int state_;
    pic::lcklist_t<piw::data_nb_t>::nbtype queue_;
    piw::xevent_data_buffer_t buffer_;
    pic::ilist_t<monomixer_wire_t> active_;
};

monomixer_wire_t::monomixer_wire_t(piw::monomixer_t::impl_t *impl, const piw::event_data_source_t &es) : impl_(impl), index_(0)
{
    impl_->inputs_.push_back(this);
    subscribe(es);
}

void monomixer_wire_t::event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei)
{
    piw::data_nb_t d2;
 
    input_=ei.signal(1);
    index_=0;
    input_.latest(d2,&index_,d.time());

    impl_->active_.append(this);

    if(impl_->state_==CLKSTATE_IDLE)
    {
        impl_->tick_suppress(false);
        impl_->state_=CLKSTATE_STARTUP;
    }
}

void monomixer_wire_t::ticked(unsigned long long t)
{
    piw::data_nb_t d;
    while(input_.read(d,&index_,t))
    {
        index_++;
        impl_->enqueue(d);
    }
}

void monomixer_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s==1)
    {
        piw::data_nb_t d2;
        input_=n;
        index_=0;
        input_.latest(d2,&index_,t);
    }
}

bool monomixer_wire_t::event_end(unsigned long long t)
{
    ticked(t);
    remove();
    return true;
}

void monomixer_wire_t::invalidate()
{
    unsubscribe();
    impl_->inputs_.remove(this);
    wire_t::disconnect();
}

piw::monomixer_t::impl_t::impl_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c): root_t(0), piw::event_data_source_real_t(piw::pathnull(0)), up_(0), state_(CLKSTATE_IDLE), buffer_(1,PIW_DATAQUEUE_SIZE_ISO)
{
    connect(c);
    d->sink(this,"monomixer");
    tick_enable(true);
    set_clock(this);
}

piw::wire_t *piw::monomixer_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    return new monomixer_wire_t(this,es);
}

void piw::monomixer_t::impl_t::root_clock()
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

void piw::monomixer_t::impl_t::root_opened()
{
    root_clock(); root_latency();
    connect_wire(this,source());
}

void piw::monomixer_t::impl_t::invalidate()
{
    source_shutdown();

    std::list<monomixer_wire_t *>::iterator i;

    while((i=inputs_.begin())!=inputs_.end())
    {
        delete (*i);
    }
}

void piw::monomixer_t::impl_t::clocksink_ticked(unsigned long long from,unsigned long long t)
{
    monomixer_wire_t *w=active_.head();
    if(!w)
    {
        state_=CLKSTATE_SHUTDOWN;
    }

    while(w)
    {
        w->ticked(t);
        w=active_.next(w);
    }

    unsigned bs = get_buffer_size();

    float *f,*fs;
    piw::data_nb_t d = piw::makenorm_nb(t,bs,&f,&fs);
    memset(f,0,bs*sizeof(float));

    while(queue_.size())
    {
        const float *df = queue_.front().as_array();
        unsigned dfl = std::min(bs,queue_.front().as_arraylen());
        pic::vector::vectadd(df,1,f,1,f,1,dfl);
        queue_.pop_front();
    }

    *fs=f[bs-1];
    buffer_.add_value(1,d);

    if(state_==CLKSTATE_STARTUP)
    {
        state_=CLKSTATE_RUNNING;
        source_start(0,piw::pathnull_nb(t),buffer_);
    }

    if(state_==CLKSTATE_SHUTDOWN)
    {
        source_end(t);
        state_ = CLKSTATE_IDLE;
        buffer_=piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_ISO);
        tick_suppress(true);
    }
}

void piw::monomixer_t::impl_t::enqueue(const piw::data_nb_t &d)
{
    queue_.push_back(d);
}

piw::monomixer_t::monomixer_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c) : impl_(new impl_t(d,c))
{
}

piw::monomixer_t::~monomixer_t()
{
    delete impl_;
}

piw::cookie_t piw::monomixer_t::cookie()
{
    return piw::cookie_t(impl_);
}
