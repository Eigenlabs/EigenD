
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

#include "angle_radius.h"

#define _USE_MATH_DEFINES

#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <picross/pic_ilist.h>
#include <math.h>
#include <map>


#define ROLL    1 
#define YAW     2

#define ANGLE   1 
#define RADIUS  2

namespace
{
    struct angle_radius_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_sink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        angle_radius_wire_t(prim::angle_radius_t::impl_t *p, const piw::event_data_source_t &);
        ~angle_radius_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
        bool event_end(unsigned long long);
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
        void process(unsigned, const piw::data_nb_t &, unsigned long long);
        void ticked(unsigned long long f, unsigned long long t);
        void source_ended(unsigned);

        prim::angle_radius_t::impl_t *root_;
        piw::xevent_data_buffer_t::iter_t input_;
        piw::xevent_data_buffer_t output_;
        unsigned long long last_from_;
        bool roll_set_;
        bool yaw_set_;
        float roll_;
        float yaw_;
    };
};

struct prim::angle_radius_t::impl_t: piw::root_t, piw::root_ctl_t, virtual pic::lckobject_t, virtual pic::tracked_t, piw::clocksink_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c): root_t(0), up_(0)
    {
        connect(c);
        cd->sink(this,"angle_radius");
        tick_enable(true);
        set_clock(this);
    }

    ~impl_t() { tracked_invalidate(); invalidate(); }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        angle_radius_wire_t *w;

        for(w=tickers_.head(); w!=0; w=tickers_.next(w))
        {
            w->ticked(f,t);
        }
    }

    void add_ticker(angle_radius_wire_t *w)
    {
        if(!tickers_.head())
        {
            tick_suppress(false);
        }
        
        tickers_.append(w);
    }

    void del_ticker(angle_radius_wire_t *w)
    {
        tickers_.remove(w);

        if(!tickers_.head())
        {
            tick_suppress(true);
        }
        
    }

    void invalidate()
    {
        tick_disable();

        pic::lckmap_t<piw::data_t,angle_radius_wire_t *>::lcktype::iterator ci;
        while((ci=children_.begin())!=children_.end())
        {
            delete ci->second;
        }
    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
       pic::lckmap_t<piw::data_t,angle_radius_wire_t *>::lcktype::iterator ci;

        if((ci=children_.find(es.path()))!=children_.end())
        {
            delete ci->second;
        }

        return new angle_radius_wire_t(this, es);
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

    pic::lckmap_t<piw::data_t, angle_radius_wire_t *>::lcktype children_;
    pic::ilist_t<angle_radius_wire_t> tickers_;
    bct_clocksink_t *up_;
};

angle_radius_wire_t::angle_radius_wire_t(prim::angle_radius_t::impl_t *p, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), root_(p), last_from_(0), roll_set_(false), yaw_set_(false), roll_(0.f), yaw_(0.f)
{
    root_->children_.insert(std::make_pair(path(),this));
    root_->connect_wire(this,source());
    subscribe_and_ping(es);
}

static int __wire_invalidator(void *w_, void *_)
{
    angle_radius_wire_t *w = (angle_radius_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

void angle_radius_wire_t::invalidate()
{
    source_shutdown();
    unsubscribe();

    piw::tsd_fastcall(__wire_invalidator, this, 0);
    if(root_)
    {
        root_->children_.erase(path());
        root_ = 0;
    }
}

void angle_radius_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    output_ = piw::xevent_data_buffer_t(SIG2(ANGLE,RADIUS),PIW_DATAQUEUE_SIZE_NORM);
    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

    source_start(seq,id,output_);

    root_->add_ticker(this);
}

void angle_radius_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    input_->set_signal(s,n);
    input_->reset(s,t);
    ticked(last_from_,t);
}

void angle_radius_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
    switch(s)
    {
        case ROLL:
            roll_ = d.as_norm();
            roll_set_ = true;
            break;

        case YAW:
            yaw_ = d.as_norm();
            yaw_set_ = true;
            break;
    }

    if(roll_set_ && yaw_set_)
    {
        float angle = atan2(roll_, yaw_);
        float radius = sqrt(pow(roll_,2) + pow(yaw_,2));

        unsigned long long t = piw::tsd_time();
        output_.add_value(ANGLE,piw::makefloat_bounded_nb(M_PI,-1.f*M_PI,0,angle,t));
        output_.add_value(RADIUS,piw::makefloat_bounded_nb(M_SQRT2,0,0,radius,t));

        roll_set_ = false;
        yaw_set_ = false;
    }
}

void angle_radius_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

    piw::data_nb_t d;
    unsigned s;

    while(input_->next(SIG2(ROLL,YAW),s,d,t))
    {
        process(s,d,t);
    }
}

bool angle_radius_wire_t::event_end(unsigned long long t)
{
    ticked(last_from_,t);
    root_->del_ticker(this);
    return source_end(t);
}

void angle_radius_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}

piw::cookie_t prim::angle_radius_t::cookie()
{
    return piw::cookie_t(impl_);
}

prim::angle_radius_t::angle_radius_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &cookie): impl_(new impl_t(cd, cookie))
{
}

prim::angle_radius_t::~angle_radius_t()
{
    delete impl_;
}
