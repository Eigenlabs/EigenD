
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

#include "clk_clock.h"

#include <piw/piw_cfilter.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_tsd.h>
#include <piw/piw_clock.h>

#include <map>
#include <cmath>

namespace
{
    float cvt2bound(float units,float lbound, float ubound)
    {
        if(units<=0)
        {
            return -units/lbound;
        }
        else
        {
            return ubound*units;
        }
    }

    float cvt2external(float units,float internal)
    {
        if(units<=0)
        {
            return -units/internal;
        }
        else
        {
            return internal*units;
        }
    }

    float cvt2internal(float units,float external)
    {
        if(units<=0)
        {
            return -units/external;
        }
        else
        {
            return external/units;
        }
    }

    struct point_t
    {
        point_t(float time,float tempo): time(time), tempo(tempo) {}

        float time;
        float tempo;
    };

    typedef std::multimap<float,point_t> timemap_t;

    struct timeline_t
    {
        void recalculate(float units)
        {
            timemap_t::iterator i;
            float in_stamp = 0.0;
            float out_stamp = 0.0;
            float tempo = 1.0;

            float lbound = 1.0e10;
            ubound = 0.0;

            for(i=map.begin(); i!=map.end(); i++)
            {
                i->second.time = out_stamp+(i->first-in_stamp)/tempo;

                in_stamp = i->first;
                out_stamp = i->second.time;
                tempo = i->second.tempo;

                if(tempo>ubound) ubound=tempo;
                if(tempo<lbound) lbound=tempo;
            }

            bound = cvt2bound(units,lbound,ubound);
        }

        void interpolate(float inclock, float *quot, float *mod, float *tempo) const
        {
            timemap_t::const_iterator i(map.lower_bound(inclock));
            i--;

            float inclock_base = i->first;
            float outclock_base = i->second.time;
            float outclock_tempo = i->second.tempo;
            float outclock = outclock_base+(inclock-inclock_base)/outclock_tempo;
            float outmod = (outclock-floor(outclock))*outclock_tempo;

            if(quot) *quot = outclock;
            if(mod) *mod = outmod;
            if(tempo) *tempo = outclock_tempo;
        }

        void lookup(float outclock, float *inclock) const
        {
            timemap_t::const_iterator i;

            for(i=map.begin(); i!=map.end(); i++)
            {
                if(i->second.time > outclock)
                {
                    break;
                }
            }

            i--;
            *inclock = i->first+(outclock-i->second.time)*i->second.tempo;
        }

        timemap_t map;
        float bound;
        float ubound;
    };

    struct parms_t
    {
        parms_t(float u): units(u) { }

        pic::flipflop_t<timeline_t> timeline;
        float units;
    };

    struct event_t
    {
        event_t(unsigned si,const piw::data_nb_t &v): signal_index(si), value(v) {}

        unsigned signal_index;
        piw::data_nb_t value;
    };

    struct clock_filt_t: piw::cqfilterfunc_t
    {
        clock_filt_t(parms_t *p): parms_(p), next_clock_(0)
        {
        }

        ~clock_filt_t()
        {
        }

        unsigned long cfilterfunc_init(piw::cfilterenv_t *env, unsigned long long)
        {
            transport_running_ = false;
            return env->cfilterenv_prime();
        }

        unsigned long cfilterfunc_filter(piw::cfilterenv_t *env, unsigned index, const piw::data_nb_t &value)
        {
            piw::data_nb_t d;

            if(value.as_arraylen()==0)
            {
                d=piw::makefloat_bounded_nb(1,0,0,0,next_clock_);
            }
            else
            {
                d=value;
            }

            cfilterenv_enqueue(index,value);
            return 1<<index;
        }

        void transport_event(piw::cfilterenv_t *env, unsigned long long t, float f)
        {
            bool r = (f>0.0);

            if(r != transport_running_)
            {
                transport_running_ = r;
                pic::msg() << "transport has " << (r?"started":"stopped") << pic::log;
            }
        }

        float interpolate_in(unsigned long long t)
        {
            return (inclock_zero_+inclock_ratio_*(float)(t-inclock_offset_));
        }


        void time_event(piw::cfilterenv_t *env, const piw::data_nb_t &v)
        {
            float f = v.as_denorm_float();
            unsigned long long t = v.time();

            if(!transport_running_)
            {
                inclock_offset_ = t;
                inclock_zero_ = f;
                inclock_ratio_ = 0.001;
            }
            else
            {
                inclock_ratio_ = (f-inclock_zero_)/((float)(t-inclock_offset_));
                //printf("time event inclk %f\n",f);
            }

            float u = v.as_array_ubound();

            if(u!=inclock_upper_)
            {
                inclock_upper_=u;
                pic::flipflop_t<timeline_t>::guard_t(parms_->timeline).value().interpolate(inclock_upper_,&outclock_upper_,0,0);
            }
        }

        bool cfilterfunc_ticked(piw::cfilterenv_t *env,unsigned long long f, unsigned long long now,unsigned long)
        {
            unsigned i;
            piw::data_nb_t d;

            while(cfilterenv_dequeue(&i, &d, f, now))
            {
                switch(i)
                {
                    case 0: transport_event(env,d.time(),d.as_renorm(0.0,1.0,0.0)); break;
                    case 1: time_event(env,d); break;
                }
            }

            next_clock_=now+1;

            if(!transport_running_)
            {
                return true;
            }

            float inclock = interpolate_in(now);
            float outclock;
            float outmod;
            float tempo;

            pic::flipflop_t<timeline_t>::guard_t g(parms_->timeline);
            g.value().interpolate(inclock,&outclock,&outmod,&tempo);

            //printf("now %llu inclock %f outclk %f outmod %f\n",now,inclock,outclock,outmod);
            env->cfilterenv_output(0,piw::makefloat_bounded_nb(outclock_upper_,0,0,outclock,now));
            env->cfilterenv_output(1,piw::makefloat_bounded_nb(tempo,0,0,outmod,now));
            env->cfilterenv_output(2,piw::makefloat_bounded_nb(g.value().bound,0,0,cvt2external(parms_->units,tempo),now));
            env->cfilterenv_output(3,piw::makefloat_bounded_nb(g.value().ubound,0,0,tempo,now));

            return true;
        }

        parms_t *parms_;
        bool transport_running_;

        unsigned long long next_clock_;
        unsigned long long inclock_offset_;
        float inclock_zero_;
        float inclock_ratio_;
        float inclock_upper_;
        float outclock_upper_;
    };

    struct clock_ctl_t: public piw::cfilterctl_t
    {
        clock_ctl_t(unsigned o, float u, float i): offset(o), parms(u)
        {
            parms.timeline.alternate().map.insert(std::make_pair(0,point_t(0,cvt2internal(u,i))));
            parms.timeline.alternate().recalculate(parms.units);
            parms.timeline.exchange();
        }

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_nb_t &)
        {
            return new clock_filt_t(&parms);
        }

        void cfilterctl_delete(piw::cfilterfunc_t *f)
        {
            delete f;
        }

        unsigned cfilterctl_latency()
        {
            return 0;
        }

        void add_change(float time, float tempo)
        {
            timeline_t &t(parms.timeline.alternate());

            tempo = cvt2internal(parms.units,tempo);

            t.map.insert(std::make_pair(time,point_t(0,tempo)));
            t.recalculate(parms.units);
            parms.timeline.exchange();
        }

        bool del_change(float time, float tempo)
        {
            timeline_t &t(parms.timeline.alternate());
            timemap_t::iterator l = t.map.lower_bound(time);
            timemap_t::iterator u = t.map.upper_bound(time);

            tempo = cvt2internal(parms.units,tempo);

            while(l!=u)
            {
                if(l->first==time && l->second.tempo==tempo)
                {
                    t.map.erase(l);
                    t.recalculate(parms.units);
                    parms.timeline.exchange();
                    return true;
                }

                l++;
            }

            return false;
        }

        float convert_i2o(float time)
        {
            timeline_t &t(parms.timeline.alternate());
            float out;
            t.interpolate(time,&out,0,0);
            return out;
        }

        float convert_o2i(float time)
        {
            timeline_t &t(parms.timeline.alternate());
            float out;
            t.lookup(time,&out);
            return out;
        }

        unsigned cfilterctl_inputs()
        {
            return 2;
        }

        unsigned cfilterctl_input(unsigned i)
        {
            return i+1;
        }

        unsigned cfilterctl_outputs()
        {
            return 4;
        }

        unsigned cfilterctl_output(unsigned o)
        {
            return o+offset+1;
        }

        bool cfilterctl_thru()
        {
            return true;
        }

        unsigned offset;
        parms_t parms;
    };
};

struct clocks::clock_t::impl_t
{
    impl_t(unsigned o, float u, float i,const piw::cookie_t &c, piw::clockdomain_ctl_t *d): controller(o,u,i), filter(&controller,c,d) { }
    ~impl_t() { }

    clock_ctl_t controller;
    piw::cfilter_t filter;
};

clocks::clock_t::clock_t(unsigned o,float u,float i,const piw::cookie_t &c, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,u,i,c,d)) {}
clocks::clock_t::~clock_t() { delete impl_; }
piw::cookie_t clocks::clock_t::cookie() { return impl_->filter.cookie(); }
void clocks::clock_t::add_change(float time, float tempo) { impl_->controller.add_change(time,tempo); }
bool clocks::clock_t::del_change(float time, float tempo) { return impl_->controller.del_change(time,tempo); }
float clocks::clock_t::convert_i2o(float time) { return impl_->controller.convert_i2o(time); }
float clocks::clock_t::convert_o2i(float time) { return impl_->controller.convert_o2i(time); }
