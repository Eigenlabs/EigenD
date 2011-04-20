
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

#include <picross/pic_float.h>
#include <piw/piw_clock.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_address.h>
#include <math.h>

#include "synth.h"
#include "synth_limiter.h"

#define IN_FC 1
#define IN_RESONANCE 2
#define IN_AUDIO 5
#define IN_NONLINEARITY 6
#define IN_MASK SIG4(IN_FC,IN_RESONANCE,IN_AUDIO,IN_NONLINEARITY)

#define OUT_LP 1
#define OUT_MASK SIG1(1)

#define TIMER_TICKS 10

#define DEFAULT_FREQ 440.0
#define DEFAULT_RESONANCE 0.5
#define MAX_RESONANCE 0.995


namespace
{
    struct synthfunc_t: piw::cfilterfunc_t
    {
        synthfunc_t(): current_freq_(DEFAULT_FREQ),current_resonance_(DEFAULT_RESONANCE),timer_(0)
        {
            tv2_=40000.f;
            ya_=0.f;yb_=0.f;yc_=0.f;yd_=0.f;ye_=0.f;
            wa_=0.f;wb_=0.f;wc_=0.f;
            last_=0.f;
        }

        void setfreq(const piw::data_nb_t &value)
        {
            current_freq_ = value.as_renorm_float(BCTUNIT_HZ,0,96000,0);
            current_freq_ = std::min(18000.f,current_freq_);
            current_freq_ = std::max(30.f,current_freq_);
        }

        void setq(const piw::data_nb_t &value)
        {
            current_resonance_ = value.as_renorm(0.0,MAX_RESONANCE,0.0);
            if(current_resonance_>MAX_RESONANCE) current_resonance_=MAX_RESONANCE;
        }

        void setnl(const piw::data_nb_t &value)
        {
            ya_=0.f;yb_=0.f;yc_=0.f;yd_=0.f;ye_=0.f;
            wa_=0.f;wb_=0.f;wc_=0.f;
            last_=0.f;
            float x = 1.0f - fabsf(value.as_norm());
            tv2_=40000.f*x*x*x*x;
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            //pic::logmsg() << "filter " << (void *)this << " prime";
            piw::data_nb_t d;

            current_freq_ = DEFAULT_FREQ;
            current_resonance_ = DEFAULT_RESONANCE;

            unsigned long long t=id.time();
            if(env->cfilterenv_latest(IN_FC,d,t)) setfreq(d);
            if(env->cfilterenv_latest(IN_RESONANCE,d,t)) setq(d);
            if(env->cfilterenv_latest(IN_NONLINEARITY,d,t)) setnl(d);
            env->cfilterenv_reset(IN_NONLINEARITY,t);
            env->cfilterenv_reset(IN_AUDIO,t);

            limiter_.release(false);
            timer_ = TIMER_TICKS;

            return true;
        }


        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs);
        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time);

        void synth_setup(float freq, float res);
        void synth_process(const float *input, float *lp, unsigned samp_from, unsigned samp_to);
        void synth_process1(float input, float *lp);

        float gc_;
        float ft_;
        float q_;
        float ya_,yb_,yc_,yd_,ye_;
        float wa_,wb_,wc_;
        float last_;
        float tv2_;

        float current_freq_, current_resonance_;
        unsigned timer_;
        synth::limiter_t limiter_;
    };
};


// filter based on antti huovilainen paper "non-linear implementation of the moog ladder filter"

void synthfunc_t::synth_process1(float in,float *lp)
{
    float x = 4.f*q_*gc_;

    // rand injects a tiny amount of noise (like analog filter)
    // which allows self-oscillation with high resonance (also fixes denormal issue)
    ya_ = ya_ + ft_*(pic::approx::tanh((in+(((rand()%2)-1)*1e-9)-x*last_)/tv2_)-wa_);
    wa_ = pic::approx::tanh(ya_/tv2_);
    yb_ = yb_ + ft_*(wa_-wb_);
    wb_ = pic::approx::tanh(yb_/tv2_);
    yc_ = yc_ + ft_*(wb_-wc_);
    wc_ = pic::approx::tanh(yc_/tv2_);
    yd_ = limiter_.process(yd_ + ft_*(wc_-pic::approx::tanh(yd_/tv2_)));

    last_ = (yd_+ye_)*0.5f;
    ye_ = yd_;

    ya_ = ya_ + ft_*(pic::approx::tanh((in-x*last_)/tv2_)-wa_);
    wa_ = pic::approx::tanh(ya_/tv2_);
    yb_ = yb_ + ft_*(wa_-wb_);
    wb_ = pic::approx::tanh(yb_/tv2_);
    yc_ = yc_ + ft_*(wb_-wc_);
    wc_ = pic::approx::tanh(yc_/tv2_);
    yd_ = limiter_.process(yd_ + ft_*(wc_-pic::approx::tanh(yd_/tv2_)));

    last_ = (yd_+ye_)*0.5f;
    ye_ = yd_;

	*lp = last_;
}

bool synthfunc_t::cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
{
    if(timer_ == 0)
    {
        //pic::logmsg() << "filter " << (void *)this << " average " << limiter_.average();
        if(limiter_.average()<1e-2)
        {
            //pic::logmsg() << "filter " << (void *)this << " turning off";
            ya_=0.f;yb_=0.f;yc_=0.f;yd_=0.f;ye_=0.f;
            wa_=0.f;wb_=0.f;wc_=0.f;
            last_=0.f;
            return false;
        }
        timer_ = TIMER_TICKS;
    }

    --timer_;

    //float sr=e->cfilterenv_clock()->get_sample_rate();
    float fc=current_freq_/sr;
    synth_setup(fc,current_resonance_);

    float *lp;
    float *lps;

    piw::data_nb_t dlp=piw::makenorm_nb(t,bs,&lp,&lps);

    piw::data_nb_t d;

    const float *audio_in=0;
    if(e->cfilterenv_nextsig(IN_AUDIO,d,t))
    {
        audio_in = d.as_array();
        timer_ = TIMER_TICKS;
    }
    //else
        //pic::logmsg() << "(no audio)";

    unsigned sig;
    unsigned samp_from = 0;
    unsigned samp_to = 0;

    while(e->cfilterenv_next(sig,d,t))
    {
        switch(sig)
        {
            case IN_FC:
                setfreq(d);
                fc=current_freq_/sr;
                synth_setup(fc,current_resonance_);
                break;

            case IN_RESONANCE:
                setq(d);
                synth_setup(fc,current_resonance_);
                break;

            case IN_NONLINEARITY:
                setnl(d);
                synth_setup(fc,current_resonance_);
                break;

        }

        samp_to = sample_offset(bs,d.time(),f,t);

        if(samp_to>samp_from)
        {
            synth_process(audio_in,lp,samp_from,samp_to);
            samp_from = samp_to;
        }
    }

    synth_process(audio_in,lp,samp_from,bs);

    *lps=lp[bs-1];

    e->cfilterenv_output(OUT_LP,dlp);

    return true;
}

void synthfunc_t::synth_process(const float *audio_in, float *lp, unsigned samp_from, unsigned samp_to)
{
    if(audio_in)
    {
        for(unsigned i=samp_from; i<samp_to; ++i)
        {
            synth_process1(audio_in[i],&lp[i]);
        }
    }
    else
    {
        for(unsigned i=samp_from; i<samp_to; ++i)
        {
            synth_process1(0.f,&lp[i]);
        }
    }
}

bool synthfunc_t::cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
{
    //pic::logmsg() << "filter " << (void *)this << " linger";
    limiter_.release(true);
    return true;
}

namespace synth
{
    struct synthfilter2_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new synthfunc_t(); }
        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };
}

void synthfunc_t::synth_setup(float fc, float res)
{
    float fc2 = fc*fc;
    float fc3 = fc*fc2;
    float fcorrection = 1.8730f*fc3 + 0.4955f*fc2 - 0.6490f*fc + 0.9988f;
    gc_ = -3.9364f*fc2 + 1.8409f*fc + 0.9968f;
    ft_ = tv2_*(1-pic::approx::exp(-PIC_PI*fc*fcorrection));
    q_ = res;
}

synth::synthfilter2_t::synthfilter2_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
piw::cookie_t synth::synthfilter2_t::cookie() { return impl_->cookie(); }
synth::synthfilter2_t::~synthfilter2_t() { delete impl_; }

