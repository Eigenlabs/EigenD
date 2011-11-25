
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

#include <piw/piw_ranger.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_ufilter.h>
#include <picross/pic_ilist.h>
#include <cmath>
#include <map>

namespace
{
    struct ranger_static_t: piw::wire_ctl_t, piw::event_data_source_real_t
    {
        ranger_static_t(piw::ranger_t::impl_t *p): piw::event_data_source_real_t(piw::pathnull(0)), parent_(p), sticky_(false), mono_(false), stickysave_(0), absolute_(true)
        {
            min_=-1.0; max_=1.0; rst_=0.0; val_=0.0;
            output_ = piw::xevent_data_buffer_t(7,PIW_DATAQUEUE_SIZE_NORM);
        }

        ~ranger_static_t()
        {
            disconnect();
            source_shutdown();
        }

        void connect();

        static int __enabler(void *w, void *a)
        {
            ranger_static_t *s = (ranger_static_t *)w;
            unsigned long long t = piw::tsd_time();
            piw::data_nb_t id = piw::pathnull_nb(t);
            s->source_start(0,id,s->output_);
            s->output(t);
            return 0;
        }

        void set_all(unsigned long long t, float max, float min, float rst)
        {
            min_=min; max_=max; rst_=rst;
            output(t);
        }

        void set_min(unsigned long long t, float v)
        {
            min_=v;
            output(t);
        }

        void set_max(unsigned long long t, float v)
        {
            max_=v;
            output(t);
        }

        void set_rst(unsigned long long t, float v)
        {
            rst_=v;
            output(t);
        }

        void set_val(unsigned long long t, double v)
        {
            val_ += v;
            output(t);
        }

        double start(unsigned long long t, float v)
        {
            val_ += v;
            output(t);
            return sticky_?0.0:v;
        }

        void stop(unsigned long long t, double l, double v)
        {
            if(!sticky_)
            {
                val_ -= l;
                val_ -= v;
                output(t);
            }
            else
            {
                stickysave_ += v;
                stickysave_ += l;
            }
        }

        void setabsolute(bool s)
        {
            if(s!=absolute_)
            {
                absolute_ = s;
                output(piw::tsd_time());
            }
        }

        void setsticky(bool s)
        {
            if(s!=sticky_)
            {
                sticky_ = s;

                if(!sticky_)
                {
                    val_ -= stickysave_;
                    output(piw::tsd_time());
                }
            }
        }

        piw::data_nb_t get_val(unsigned long long t, float d)
        {
            float v;

            d = std::min(1.0f,std::max(-1.0f,d));

            if(d>=0)
            {
                v=(max_-rst_)*d+rst_;
            }
            else
            {
                v=(rst_-min_)*d+rst_;
            }

            //pic::logmsg() << "output " << v << " min=" << min_ << " max=" << max_ << " rest=" << rst_ << " input=" << d << " abs " << absolute_;

            float el = std::min(std::min(max_,min_),rst_);
            float eu = std::max(std::max(max_,min_),rst_);

            if(absolute_)
            {
                return piw::makefloat_bounded_units_nb(BCTUNIT_GLOBAL,eu,el,rst_,v,t);
            }
            else
            {
                return piw::makefloat_bounded_units_nb(BCTUNIT_RATIO,eu,el,rst_,v,t);
            }
        }

        void output(unsigned long long t)
        {
            output_.add_value(1,get_val(t,val_));
        }

        piw::ranger_t::impl_t *parent_;
        piw::xevent_data_buffer_t output_;
        float min_;
        float max_;
        float rst_;
        double val_;
        bool sticky_;
        bool mono_;
        double stickysave_;
        bool absolute_;
    };

    struct ranger_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_sink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        ranger_wire_t(piw::ranger_t::impl_t *p, const piw::event_data_source_t &);
        ~ranger_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
        void ticked(unsigned long long f, unsigned long long t);
        void source_ended(unsigned seq);

        piw::ranger_t::impl_t *parent_;
        piw::xevent_data_buffer_t::iter_t input_;
        piw::xevent_data_buffer_t output_;
        unsigned seq_;
        bool static_;
        double bias_;
        double last_;
        double initial_;
    };

    struct ranger_ctl_t: piw::ufilterctl_t, piw::ufilterfunc_t, piw::ufilter_t
    {
        ranger_ctl_t(piw::ranger_t::impl_t *s): piw::ufilter_t(this,piw::cookie_t(0)), impl_(s), up_(0)
        {
        }

        virtual piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &) { return this; }
        virtual void ufilterctl_delete(piw::ufilterfunc_t *f) { }
        virtual void ufilterctl_clock(bct_clocksink_t *c);
        virtual unsigned ufilterctl_latency() { return 0; }
        virtual unsigned long long ufilterctl_thru() { return 0; }
        virtual unsigned long long ufilterctl_inputs() { return SIG3(1,2,3); }
        virtual unsigned long long ufilterctl_outputs() { return 0; }

        virtual void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
        {
        }

        virtual void ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id);
        virtual void ufilterfunc_data(piw::ufilterenv_t *e,unsigned s,const piw::data_nb_t &d);

        piw::ranger_t::impl_t *impl_;
        bct_clocksink_t *up_;
    };

};

struct piw::ranger_t::impl_t: root_t, root_ctl_t, virtual pic::lckobject_t, virtual pic::tracked_t, piw::clocksink_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c): root_t(0), static_(this), ctl_(this), up_(0), curve_(0)
    {
        connect(c);
        static_.connect();
        cd->sink(this,"ranger");
        tick_enable(true);
        set_clock(this);
    }

    ~impl_t() { tracked_invalidate(); invalidate(); }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        ranger_wire_t *w;

        for(w=tickers_.head(); w!=0; w=tickers_.next(w))
        {
            w->ticked(f,t);
        }
    }

    void add_ticker(ranger_wire_t *w)
    {
        if(!tickers_.head())
        {
            tick_suppress(false);
        }
        
        tickers_.append(w);
    }

    void del_ticker(ranger_wire_t *w)
    {
        tickers_.remove(w);

        if(!tickers_.head())
        {
            tick_suppress(true);
        }
        
    }

    void invalidate()
    {
        pic::lckmap_t<piw::data_t,ranger_wire_t *>::lcktype::iterator ci;
        tick_disable();

        while((ci=children_.begin())!=children_.end())
        {
            delete ci->second;
        }
    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
       pic::lckmap_t<piw::data_t,ranger_wire_t *>::lcktype::iterator ci;

        if((ci=children_.find(es.path()))!=children_.end())
        {
            delete ci->second;
        }

        return new ranger_wire_t(this,es);
    }

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }

    void root_clock()
    {
        if(up_)
        {
            remove_upstream(up_);
            up_=0;
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

    float curve(float in)
    {
        if(in==0.f)
            return 0.f;
        if(curve_==0.f)
            return in;

#ifndef PI_WINDOWS			
        float sign = std::signbit(in)?-1.f:1.f;
#else
		float sign = (in<0)?-1.f:1.f;
#endif
        in = fabsf(in);

        if(curve_>0.f)
            return sign*(1.f-powf(1.f-in,1.f+curve_));
        else
            return sign*pow(in,1.f-curve_);
    }

    ranger_static_t static_;
    ranger_ctl_t ctl_;
    pic::lckmap_t<piw::data_t, ranger_wire_t *>::lcktype children_;
    pic::ilist_t<ranger_wire_t> tickers_;
    bct_clocksink_t *up_;
    float curve_;
};

void ranger_ctl_t::ufilterctl_clock(bct_clocksink_t *c)
{
    if(up_)
    {
        impl_->remove_upstream(up_);
        up_=0;
    }

    up_=c;

    if(up_)
    {
        impl_->add_upstream(up_);
    }
}

void ranger_ctl_t::ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
{
    unsigned long long t=id.time();
    piw::data_nb_t d;
    float min = -1,max = 1,rst = 0; 

    if(e->ufilterenv_latest(1,d,t)) min = d.as_renorm_float(-10000000,10000000,0);
    if(e->ufilterenv_latest(2,d,t)) max = d.as_renorm_float(-10000000,10000000,0);
    if(e->ufilterenv_latest(3,d,t)) rst = d.as_renorm_float(-10000000,10000000,0);

    impl_->static_.set_all(t,max,min,rst);
    e->ufilterenv_start(t);
}

void ranger_ctl_t::ufilterfunc_data(piw::ufilterenv_t *e,unsigned s,const piw::data_nb_t &d)
{
    switch(s)
    {
        case 1: impl_->static_.set_min(d.time(),d.as_renorm_float(-10000000,10000000,0)); break;
        case 2: impl_->static_.set_max(d.time(),d.as_renorm_float(-10000000,10000000,0)); break;
        case 3: impl_->static_.set_rst(d.time(),d.as_renorm_float(-10000000,10000000,0)); break;
    }
}

void ranger_static_t::connect()
{
    parent_->connect_wire(this,source());
    piw::tsd_fastcall(__enabler,this,0);
}

ranger_wire_t::ranger_wire_t(piw::ranger_t::impl_t *p, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), parent_(p)
{
    parent_->children_.insert(std::make_pair(path(),this));
    parent_->connect_wire(this,source());
    subscribe_and_ping(es);
}

void ranger_wire_t::invalidate()
{
    source_shutdown();
    unsubscribe();

    if(parent_)
    {
        parent_->children_.erase(path());
        parent_=0;
    }
}

void ranger_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    unsigned long long t = id.time();
    piw::data_nb_t d;

    seq_ = seq;
    output_ = piw::xevent_data_buffer_t(SIG1(1),PIW_DATAQUEUE_SIZE_NORM);
    input_ = b.iterator();

    if(id.as_pathlen()==0 || parent_->static_.mono_)
    {
        static_=true;
        float n = 0;

        if(input_->latest(1,d,t))
        {
            n=d.as_norm();
        }

        float f = parent_->curve(n);
        bias_ = parent_->static_.start(t,f);
        last_ = f;
        initial_ = f;
        //pic::logmsg() << "start(static):" << d.as_norm() << "->" << f << " bias=" << bias_;
    }
    else
    {
        static_=false;

        if(input_->latest(1,d,t))
        {
            float f = parent_->curve(d.as_norm());
            //pic::logmsg() << "start(unstatic):" << d.as_norm() << "->" << f;
            output_.add_value(1,parent_->static_.get_val(t,f));
        }
        else
        {
            output_.add_value(1,parent_->static_.get_val(t,0));
        }

        source_start(seq,id,output_);
    }

    parent_->add_ticker(this);

}

void ranger_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    if(s==1)
    {
        input_->set_signal(1,n);
        input_->reset(1,t);
    }
}

void ranger_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    piw::data_nb_t d;
    unsigned s;

    while(input_->next(SIG1(1),s,d,t))
    {
        double f = parent_->curve(d.as_norm());
        if(static_)
        {
            //pic::logmsg() << "ticked(static):" << d.as_norm() << "->" << f;
            double delta = f-last_;
            parent_->static_.set_val(d.time(),delta);
            
            last_ = f;
        }
        else
        {
            //pic::logmsg() << "ticked(unstatic):" << d.as_norm() << "->" << f;
            output_.add_value(1,parent_->static_.get_val(d.time(),f));
        }
    }
}

bool ranger_wire_t::event_end(unsigned long long t)
{
    parent_->del_ticker(this);

    if(static_)
    {
        parent_->static_.stop(t,last_-initial_,bias_);
    }
    else
    {
        return source_end(t);
    }

    return true;
}

void ranger_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}

piw::cookie_t piw::ranger_t::ctl_cookie()
{
    return impl_->ctl_.cookie();
}

piw::cookie_t piw::ranger_t::data_cookie()
{
    return piw::cookie_t(impl_);
}

piw::ranger_t::ranger_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &cookie): impl_(new impl_t(cd,cookie))
{
}

piw::ranger_t::~ranger_t()
{
    delete impl_;
}

static int __setmono(void *i_, void *s_)
{
    piw::ranger_t::impl_t *i = (piw::ranger_t::impl_t *)i_;
    bool s = *(bool *)s_;
    i->static_.mono_ = s;
    return 0;
}

void piw::ranger_t::set_mono(bool s)
{
    piw::tsd_fastcall(__setmono,impl_,&s);
}

static int __setabsolute(void *i_, void *s_)
{
    piw::ranger_t::impl_t *i = (piw::ranger_t::impl_t *)i_;
    i->static_.setabsolute(*(bool *)s_);
    return 0;
}

void piw::ranger_t::set_absolute(bool s)
{
    piw::tsd_fastcall(__setabsolute,impl_,&s);
}


static int __setsticky(void *i_, void *s_)
{
    piw::ranger_t::impl_t *i = (piw::ranger_t::impl_t *)i_;
    i->static_.setsticky(*(bool *)s_);
    return 0;
}

void piw::ranger_t::set_sticky(bool s)
{
    piw::tsd_fastcall(__setsticky,impl_,&s);
}

static int __reset(void *i_, void *)
{
    piw::ranger_t::impl_t *i = (piw::ranger_t::impl_t *)i_;
    i->static_.val_ = i->static_.rst_;
    return 0;
}

static int __setcurve(void *i_, void *f_)
{
    piw::ranger_t::impl_t *i = (piw::ranger_t::impl_t *)i_;
    float f = *(float *)f_;
    i->curve_ = f;
    return 0;
}

void piw::ranger_t::set_curve(float c)
{
    piw::tsd_fastcall(__setcurve,impl_,&c);
}

void piw::ranger_t::reset()
{
    piw::tsd_fastcall(__reset,impl_,0);
}
