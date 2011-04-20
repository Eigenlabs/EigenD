
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

#include <picross/pic_log.h>
#include <piw/piw_clock.h>
#include <piw/piw_scroller2.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>

#include <map>

#define THRESHOLD 0.45
#define REPEAT_START 250
#define REPEAT_NORMAL 50
#define REPEAT_DEC 5
#define REPEAT_END 25

namespace
{
    struct scroller_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<0>
    {
        scroller_wire_t(piw::scroller2_t::impl_t *i,const piw::event_data_source_t &);
        ~scroller_wire_t() { invalidate(); }
        void invalidate();
        void wire_closed() { delete this; }
        void wire_latency() {}

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n);

        piw::data_t path_;
        piw::scroller2_t::impl_t *impl_;
    };
}

struct piw::scroller2_t::impl_t: piw::root_t, piw::clocksink_t, piw::thing_t
{
    impl_t(const piw::change_t &s): root_t(0), up_(0), scroll_target_(s), active_(0)
    {
        domain_.set_source(piw::makestring("*",0));
        domain_.sink(this,"scroller");
        piw::tsd_thing(this);
    }

    ~impl_t()
    {
        tracked_invalidate();
    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
        return new scroller_wire_t(this,es);
    }

    void root_closed() { root_t::disconnect(); root_clock(); }
    void root_opened() { root_clock(); root_latency(); }
    void root_latency() { set_sink_latency(get_latency()); }

    void root_clock()
    {
        if(up_)
        {
            remove_upstream(up_);
            up_ = 0;
        }

        bct_clocksink_t *s = get_clock();

        if(s)
        {
            up_ = s;
            add_upstream(s);
        }
    }

    void thing_dequeue_slow(const piw::data_t &d)
    {
        if(d.is_long())
        {
            scroll_target_(d);
        }
    }

    void event_start(scroller_wire_t *wire, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
    {
        if(active_)
        {
            return;
        }

        active_=wire;
        iter_ = b.iterator();
        tick_suppress(false);
        scroll_=false;
        age_=0;
        repeat_=REPEAT_START;
        activated_=false;
    }

    void event_end(scroller_wire_t *wire, unsigned long long t)
    {
        if(active_!=wire)
        {
            return;
        }

        active_=0;
        tick_suppress(true);

        if(!scroll_ && activated_)
        {
            enqueue_slow_nb(piw::makelong_nb(16,t));
        }
    }

    void event_reset(scroller_wire_t *wire, unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n)
    {
        if(active_!=wire)
        {
            return;
        }

        iter_->reset(s,t);
    }

    void clocksink_ticked(unsigned long long from, unsigned long long to)
    {
        unsigned long long len = (to-from)/1000ULL;

        if(!active_)
        {
            tick_suppress(true);
            return;
        }

        piw::data_nb_t d;
        float h=0.0,v=0.0;
        bool s = false;
        unsigned sv=0;

        if(iter_->latest(1,d,to)) h=d.as_norm();
        if(iter_->latest(2,d,to)) v=d.as_norm();
        if(iter_->latest(3,d,to)) if(d.as_norm()>0) activated_=true;

        if(h<-THRESHOLD || h>THRESHOLD)
        {
            if(h>THRESHOLD)
            {
                sv|=1;
            }
            else
            {
                sv|=2;
            }

            s=true;
            scroll_=true;
        }

        if(v<-THRESHOLD || v>THRESHOLD)
        {
            if(v>THRESHOLD)
            {
                sv|=4;
            }
            else
            {
                sv|=8;
            }

            s=true;
            scroll_=true;
        }

        if(s)
        {
            if(age_==0)
            {
                enqueue_slow_nb(piw::makelong_nb(sv,to));
                age_=repeat_;
            }
            else
            {
                if(age_<=len)
                {
                    enqueue_slow_nb(piw::makelong_nb(sv,to));

                    if(repeat_>REPEAT_END)
                    {
                        if(repeat_==REPEAT_START)
                        {
                            repeat_=REPEAT_NORMAL;
                        }
                        else
                        {
                            repeat_=repeat_-REPEAT_DEC;
                        }
                    }

                    age_=repeat_;
                }
                else
                {
                    age_=age_-len;
                }
            }
        }
        else
        {
            age_=0;
            repeat_=REPEAT_START;
        }
    }
    
    piw::clockdomain_ctl_t domain_;
    bct_clocksink_t *up_;
    std::map<piw::data_t, scroller_wire_t *> wires_;
    piw::change_t scroll_target_;
    scroller_wire_t *active_;
    piw::xevent_data_buffer_t::iter_t iter_;
    bool scroll_;
    unsigned age_;
    unsigned repeat_;
    bool activated_;
};

void scroller_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    impl_->event_start(this,id,b);
}

bool scroller_wire_t::event_end(unsigned long long t)
{
    impl_->event_end(this,t);
    return true;
}

void scroller_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    impl_->event_reset(this,s,t,o,n);
}

piw::scroller2_t::scroller2_t(const piw::change_t &s) : impl_(new impl_t(s))
{
}

piw::scroller2_t::~scroller2_t()
{
    delete impl_;
}

piw::cookie_t piw::scroller2_t::cookie()
{
    return piw::cookie_t(impl_);
}

void piw::scroller2_t::enable()
{
    impl_->tick_enable(true);
}

void piw::scroller2_t::disable()
{
    impl_->tick_disable();
}

scroller_wire_t::scroller_wire_t(piw::scroller2_t::impl_t *i,const piw::event_data_source_t &es) : path_(es.path()), impl_(i)
{
    impl_->wires_.insert(std::make_pair(path_,this));
    subscribe(es);
}

void scroller_wire_t::invalidate()
{ 
    unsubscribe();
    impl_->wires_.erase(path_);
}
