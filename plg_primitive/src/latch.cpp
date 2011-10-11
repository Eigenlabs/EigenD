
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

#include "latch.h"

#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <picross/pic_ilist.h>
#include <cmath>
#include <map>

namespace
{
    struct latch_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_sink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        latch_wire_t(prim::latch_t::impl_t *p, const piw::event_data_source_t &);
        ~latch_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
        bool event_end(unsigned long long);
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
        void process(unsigned, const piw::data_nb_t &, unsigned long long);
        void ticked(unsigned long long f, unsigned long long t);
        void source_ended(unsigned);

        prim::latch_t::impl_t *root_;
        piw::xevent_data_buffer_t::iter_t input_;
        piw::xevent_data_buffer_t output_;
        unsigned seq_;
        unsigned long long last_from_;
        float last_value_[4];
    };
};

struct prim::latch_t::impl_t: piw::root_t, piw::root_ctl_t, virtual pic::lckobject_t, virtual pic::tracked_t, piw::clocksink_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c): root_t(0), up_(0), minimum_(0.5f), controller_(2)
    {
        connect(c);
        cd->sink(this,"latch");
        tick_enable(true);
        set_clock(this);
    }

    ~impl_t() { tracked_invalidate(); invalidate(); }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        latch_wire_t *w;

        for(w=tickers_.head(); w!=0; w=tickers_.next(w))
        {
            w->ticked(f,t);
        }
    }

    void add_ticker(latch_wire_t *w)
    {
        if(!tickers_.head())
        {
            tick_suppress(false);
        }
        
        tickers_.append(w);
    }

    void del_ticker(latch_wire_t *w)
    {
        tickers_.remove(w);

        if(!tickers_.head())
        {
            tick_suppress(true);
        }
        
    }

    void invalidate()
    {
        pic::lckmap_t<piw::data_t,latch_wire_t *>::lcktype::iterator ci;
        tick_disable();

        while((ci=children_.begin())!=children_.end())
        {
            delete ci->second;
        }
    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
       pic::lckmap_t<piw::data_t,latch_wire_t *>::lcktype::iterator ci;

        if((ci=children_.find(es.path()))!=children_.end())
        {
            delete ci->second;
        }

        return new latch_wire_t(this, es);
    }

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }

    void root_clock()
    {
        if(up_)
        {
            remove_upstream(up_);
            up_ = 0;
        }

        up_=get_clock();

        if(up_)
        {
            add_upstream(up_);
        }
    }

    void root_latency()
    {
        set_latency(get_latency());
    }

    pic::lckmap_t<piw::data_t, latch_wire_t *>::lcktype children_;
    pic::ilist_t<latch_wire_t> tickers_;
    bct_clocksink_t *up_;
    float minimum_;
    unsigned controller_;
};

latch_wire_t::latch_wire_t(prim::latch_t::impl_t *p, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), root_(p), last_from_(0)
{
    root_->children_.insert(std::make_pair(path(),this));
    root_->connect_wire(this,source());
    subscribe_and_ping(es);
}

void latch_wire_t::invalidate()
{
    source_shutdown();
    unsubscribe();

    if(root_)
    {
        root_->children_.erase(path());
        root_ = 0;
    }
}

void latch_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    piw::data_nb_t d;

    seq_ = seq;
    output_ = piw::xevent_data_buffer_t(SIG4(1,2,3,4),PIW_DATAQUEUE_SIZE_NORM);
    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

    for(int i = 0; i < 4; ++i)
    {
        last_value_[i] = 0;
    }

    for(int s = 1; s <= 4; ++s)
    {
        if(input_->latest(s,d,t))
        {
            process(s,d,t);
        }
    }

    source_start(seq,id,output_);

    root_->add_ticker(this);

}

void latch_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s==1)
    {
        input_->set_signal(1,n);
        input_->reset(1,t);
    }
}

void latch_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
    if(s == 1 || s == root_->controller_)
    {
        last_value_[s-1] = d.as_norm();
        output_.add_value(s,d);
    }
    else
    {
        if(fabs(last_value_[root_->controller_-1]) >= root_->minimum_)
        {
            last_value_[s-1] = d.as_norm();
            output_.add_value(s,d);
        }
        else
        {
            output_.add_value(s,piw::makefloat_nb(t,last_value_[s-1]));
        }
    }
}

void latch_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

    piw::data_nb_t d;
    unsigned s;

    while(input_->next(SIG4(1,2,3,4),s,d,t))
    {
        process(s,d,t);
    }
}

bool latch_wire_t::event_end(unsigned long long t)
{
    ticked(last_from_,t);
    root_->del_ticker(this);
    return source_end(t);
}

void latch_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}

piw::cookie_t prim::latch_t::cookie()
{
    return piw::cookie_t(impl_);
}

prim::latch_t::latch_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &cookie): impl_(new impl_t(cd, cookie))
{
}

prim::latch_t::~latch_t()
{
    delete impl_;
}

static int __set_minimum(void *i_, void *m_)
{
    prim::latch_t::impl_t *i = (prim::latch_t::impl_t *)i_;
    float m = *(float *)m_;
    i->minimum_ = m;
    return 0;
}

void prim::latch_t::set_minimum(float m)
{
    piw::tsd_fastcall(__set_minimum,impl_,&m);
}

static int __set_controller(void *i_, void *c_)
{
    prim::latch_t::impl_t *i = (prim::latch_t::impl_t *)i_;
    unsigned c = *(unsigned *)c_;
    i->controller_ = c;
    return 0;
}

void prim::latch_t::set_controller(unsigned c)
{
    if(c<=1) return;
    piw::tsd_fastcall(__set_controller,impl_,&c);
}

