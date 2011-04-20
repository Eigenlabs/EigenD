
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
#include <piw/piw_scroller.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_table.h>
#include <piw/piw_fastdata.h>

#include <map>
#include <cmath>

#define TAPPING_INTERVAL_END 100000ULL
#define THRESHOLD 80.0f

namespace
{
    struct scroller_wire_t;

    struct scroller_axis_t:piw::fastdata_t
    {
        scroller_axis_t(scroller_wire_t *w,float scale,float *sum,unsigned scheme): fastdata_t(PLG_FASTDATA_SENDER),wire_(w),scale_(scale),sum_(sum),scheme_(scheme),max_(0.0f)
        {
            piw::tsd_fastdata(this);
            enable(true,false,false);
        }

        bool fastdata_receive_event(const piw::data_nb_t &d,const piw::dataqueue_t &q)
        {
            if(!d.is_null())
            {
                fastdata_t::ping(d.time(),q);
                return true;
            }
            return false;
        }

        bool fastdata_receive_data(const piw::data_nb_t &d)
        {
            const float *v = d.as_array();
            unsigned len = d.as_arraylen();
            unsigned n=len;
            float c =0.0;
            float o = *sum_;
            if (scheme_==0)
            {
                pic::ftable_t &tbl = table();

                while(len>0)
                {
                    max_+=std::abs(v[0]);
                    c=scale_*v[0];
                    *sum_ += tbl.interp(c);

                    if (*sum_>1.0) *sum_=1.0;
                    if (*sum_<-1.0) *sum_=-1.0;

                    v++; len--;
                }
            }
            else if (scheme_==1)
            {
                *sum_=0;
                o=0.0;
                while(len>0)
                {
                    max_+=std::abs(v[0]);
                    c=scale_*v[0];
                    *sum_+=c;
                    if (*sum_>1.0) *sum_=1.0;
                    if (*sum_<-1.0) *sum_=-1.0;

                    v++; len--;
                }
                *sum_=*sum_/n;
            }

            if(*sum_ != o)
            {
                ping(d.time());
            }

            return true;
        }

        void ping(unsigned long long t);
        pic::ftable_t &table();

        scroller_wire_t *wire_;
        float scale_;
        float *sum_;
        unsigned int scheme_;
        float max_;
    };

    struct scroller_activation_t:piw::fastdata_t
    {
        scroller_activation_t(scroller_wire_t *w,bool act): fastdata_t(PLG_FASTDATA_SENDER),wire_(w),act_(act)
        {
            piw::tsd_fastdata(this);
            enable(true,false,false);
        }

        piw::dataqueue_t input_;
        bool fastdata_receive_event(const piw::data_nb_t &d,const piw::dataqueue_t &q)
        {
            if(!d.is_null())
            {
                input_=q;
                fastdata_t::ping(d.time(),q);
                return true;
            }
            return false;
        }

        bool fastdata_receive_data(const piw::data_nb_t &d)
        {
            const float *v = d.as_array();
            unsigned len = d.as_arraylen();

            while(len>0)
            {
                if (v[0]>0)
                {
                    act_=true;
                    return false;
                }

                v++; len--;
            }

            return true;
        }

        void set_act(bool a)
        {
            act_=a;
        }

        bool get_act()
        {
            return act_;
        }

        scroller_wire_t *wire_;
        bool act_;
    };


    struct scroller_wire_t: piw::wire_t, piw::event_data_sink_t
    {
        scroller_wire_t(piw::scroller_t::impl_t *i,const piw::event_data_source_t &);
        ~scroller_wire_t() { invalidate(); }
        void invalidate();
        void wire_closed() { delete this; }
        void wire_latency() {}

        void ping(unsigned long long t);
        void ping_tap();

        piw::xevent_data_buffer_t buf_;
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
        {
            buf_=b;
            h_.max_ = 0.0f;
            v_.max_ = 0.0f;
            a_.set_act(false);
            h_.send_fast(id,b.signal(1));
            v_.send_fast(id,b.signal(2));
            a_.send_fast(id,b.signal(3));
            t_=id.time();
            id_=id;
        }

        void event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n)
        {
            if(s==1) h_.send_fast(id_,n);
            if(s==2) v_.send_fast(id_,n);
            if(s==3) a_.send_fast(id_,n);
        }

        bool event_end(unsigned long long t)
        {
            piw::data_nb_t a;
            id_=piw::data_nb_t();

            if(t_)
            {
                if (a_.get_act())
                {
                    pic::logmsg() << "tap OK " << t-t_;
                    ping_tap();
                }
                else
                {
                    pic::logmsg() << "No good - activation is false ";
                }
            }

            return true;
        }

        piw::data_nb_t id_;
        piw::data_t path_;
        piw::scroller_t::impl_t *impl_;
        scroller_axis_t h_,v_;
        scroller_activation_t a_;
        unsigned long long t_;
    };
}

struct piw::scroller_t::impl_t: piw::root_t, piw::clocksink_t, piw::thing_t
{
    impl_t(piw::scrolldelegate_t *,float,float,unsigned long);

    ~impl_t()
    {
        tracked_invalidate();
    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
        return new scroller_wire_t(this,es);
    }

    void root_closed() { root_t::disconnect(); root_clock(); }
    void root_clock();
    void root_opened() { root_clock(); root_latency(); }
    void root_latency() { set_sink_latency(get_latency()); }

    void thing_trigger_slow()
    {
        delegate_->scroll(hsum_,vsum_);
    }

    void thing_dequeue_slow(const piw::data_t &)
    {
        delegate_->tap();
    }

    void ping_tap()
    {
        enqueue_slow_nb(piw::data_nb_t());
    }

    void ping()
    {
        if(!changed_)
        {
            tick_suppress(false);
            changed_=true;
        }
    }

    void clocksink_ticked(unsigned long long from, unsigned long long to)
    {
        if(to>=stamp_)
        {
            stamp_=to+interval_;
            tick_suppress(true);
            trigger_slow();
            changed_=false;
        }
    }
    
    void set_scheme(unsigned int s)
    {
        scheme_ = s;
    }


    void reseth(float h)
    {
        hsum_ = h;
    }

    void resetv(float v)
    {
        vsum_ = v;
    }

    piw::clockdomain_ctl_t domain_;
    piw::scrolldelegate_t *delegate_;
    bct_clocksink_t *up_;
    float hscale_;
    float vscale_;
    pic::ftable_t powtable_;
    unsigned long long interval_;

    bool changed_;
    unsigned long long stamp_;

    float hsum_;
    float vsum_;
    unsigned int scheme_;

    std::map<piw::data_t, scroller_wire_t *> wires_;
};

void scroller_wire_t::ping_tap()
{
    float max = std::max(h_.max_,v_.max_);
    pic::logmsg() << "max excursion " << max;
    if(max>THRESHOLD)
    {
        return;
    }
    impl_->ping_tap();
}

void scroller_wire_t::ping(unsigned long long t)
{
    if(t_)
    {
        if(t<t_+TAPPING_INTERVAL_END)
        {
            return;
        }

        pic::logmsg() << "Ping " << t-t_;
        t_=0;
    }

    impl_->ping();
}

void scroller_axis_t::ping(unsigned long long t)
{
    wire_->ping(t);
}

pic::ftable_t &scroller_axis_t::table()
{
    return wire_->impl_->powtable_;
}

piw::scroller_t::impl_t::impl_t(piw::scrolldelegate_t *d,float hscale, float vscale, unsigned long interval): root_t(0), delegate_(d), up_(0), hscale_(hscale), vscale_(vscale), powtable_(200), interval_(interval*1000), changed_(false), stamp_(0), hsum_(0), vsum_(0), scheme_(0)
{
    domain_.set_source(piw::makestring("*",0));
    domain_.sink(this,"scroller");

    piw::tsd_thing(this);

    unsigned i = 0;
    float b=0.0;
    float y=0.0;

    powtable_.set_bounds(-1.0,1.0);

    for(; i<100; ++i)
    {
        if (b<0.02)
        {
            y=0.0;
        }
        else
        {
            y=b*b*b*b*b*b;
        }
        powtable_.set_entry(100+i,y);
        powtable_.set_entry(99-i,-1*y);
        b +=0.01;
    }
}

void piw::scroller_t::impl_t::root_clock()
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

piw::scroller_t::scroller_t(piw::scrolldelegate_t *d,float x, float y, unsigned long i) : impl_(new impl_t(d,x,y,i))
{
}

piw::scroller_t::~scroller_t()
{
    delete impl_;
}

piw::cookie_t piw::scroller_t::cookie()
{
    return piw::cookie_t(impl_);
}

void piw::scroller_t::reset(float h, float v)
{
    impl_->reseth(h);
    impl_->resetv(v);
}

void piw::scroller_t::set_scheme(unsigned s)
{
    impl_->set_scheme(s);
    impl_->reseth(0.0);
    impl_->resetv(0.0);
}


void piw::scroller_t::reset_h(float h)
{
    impl_->reseth(h);
}

void piw::scroller_t::reset_v(float v)
{
    impl_->resetv(v);
}

void piw::scroller_t::enable()
{
    impl_->tick_enable(false);
}

void piw::scroller_t::disable()
{
    impl_->tick_disable();
}

scroller_wire_t::scroller_wire_t(piw::scroller_t::impl_t *i,const piw::event_data_source_t &es) : path_(es.path()), impl_(i), h_(this,impl_->hscale_,&impl_->hsum_,impl_->scheme_), v_(this,impl_->vscale_,&impl_->vsum_,impl_->scheme_),a_(this,false),t_(0)
{
    impl_->wires_.insert(std::make_pair(path_,this));
    subscribe(es);
}

void scroller_wire_t::invalidate()
{ 
    unsubscribe();
    impl_->wires_.erase(path_);
}
