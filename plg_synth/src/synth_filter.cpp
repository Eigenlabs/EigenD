
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
#include <picross/pic_fastalloc.h>
#include <piw/piw_clock.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_address.h>
#include <math.h>

#include "synth.h"
#include "synth_limiter.h"

#define IN_FC 1
#define IN_RESONANCE 2
#define IN_AUDIO 5
#define IN_MASK SIG3(IN_FC,IN_RESONANCE,IN_AUDIO)

#define OUT_LP 1
#define OUT_HP 2
#define OUT_BP 3
#define OUT_MASK SIG3(OUT_LP,OUT_HP,OUT_BP)

#define TIMER_TICKS 10

#define DEFAULT_FREQ 440.0
#define DEFAULT_RESONANCE 0.5
#define MAX_RESONANCE 0.997

namespace
{
    struct synthfunc_t: piw::cfilterfunc_t
    {
        synthfunc_t(): coeff_(0.0),feedback_(0.0),lowpass_(0.0),current_freq_(DEFAULT_FREQ),current_resonance_(DEFAULT_RESONANCE),timer_(0)
        {
            memset(state_,0,sizeof(state_));
        }

        void setfreq(const piw::data_nb_t &value)
        {
            float v1 = value.as_renorm_float(BCTUNIT_HZ,0,96000,0);
            current_freq_ = std::min(18000.f,v1);
            current_freq_ = std::max(30.f,current_freq_);
            //pic::logmsg() << "freq " << value << ' ' << value.units() << ':' << v1 << "->" << current_freq_ << ' ' << value.time();
        }

        void setq(const piw::data_nb_t &value)
        {
            current_resonance_ = value.as_renorm(0.0,MAX_RESONANCE,0.0);
            if(current_resonance_>MAX_RESONANCE) current_resonance_=MAX_RESONANCE;
            //pic::logmsg() << "resonance " << value << "->" << current_resonance_ << ' ' << value.time();
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            piw::data_nb_t d;

            current_freq_ = DEFAULT_FREQ;
            current_resonance_ = DEFAULT_RESONANCE;

            unsigned long long t=id.time();

            if(env->cfilterenv_latest(IN_FC,d,t)) setfreq(d);
            if(env->cfilterenv_latest(IN_RESONANCE,d,t)) setq(d);

            env->cfilterenv_reset(IN_AUDIO,t);

            limiter_.release(false);
            timer_ = TIMER_TICKS;

            return true;
        }


        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs);
        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time);

        void synth_setup(float freq, float res);
        void synth_process(const float *input, float *lp, float *hp, float *bp, unsigned samp_from, unsigned samp_to);
        void synth_process1(float input, float *lp, float *hp, float *bp);

        double coeff_, feedback_;
        double lowpass_;
        double state_[4];

        float current_freq_, current_resonance_;
        unsigned timer_;
        synth::limiter_t limiter_;
    };
};


// filter based on PD extern by dfl@ccrma.stanford.edu
static float gaintable__[199] = {
    0.999969, 0.990082, 0.980347, 0.970764, 0.961304, 0.951996, 0.94281, 0.933777, 0.924866, 0.916077,
    0.90741, 0.898865, 0.890442, 0.882141 , 0.873962, 0.865906, 0.857941, 0.850067, 0.842346, 0.834686,
    0.827148, 0.819733, 0.812378, 0.805145, 0.798004, 0.790955, 0.783997, 0.77713, 0.770355, 0.763672,
    0.75708 , 0.75058, 0.744141, 0.737793, 0.731537, 0.725342, 0.719238, 0.713196, 0.707245, 0.701355,
    0.695557, 0.689819, 0.684174, 0.678558, 0.673035, 0.667572, 0.66217, 0.65686, 0.651581, 0.646393,
    0.641235, 0.636169, 0.631134, 0.62619, 0.621277, 0.616425, 0.611633, 0.606903, 0.602234, 0.597626,
    0.593048, 0.588531, 0.584045, 0.579651, 0.575287 , 0.570953, 0.566681, 0.562469, 0.558289, 0.554169,
    0.550079, 0.546051, 0.542053, 0.538116, 0.53421, 0.530334, 0.52652, 0.522736, 0.518982, 0.515289,
    0.511627, 0.507996 , 0.504425, 0.500885, 0.497375, 0.493896, 0.490448, 0.487061, 0.483704, 0.480377,
    0.477081, 0.473816, 0.470581, 0.467377, 0.464203, 0.46109, 0.457977, 0.454926, 0.451874, 0.448883,
    0.445892, 0.442932, 0.440033, 0.437134, 0.434265, 0.431427, 0.428619, 0.425842, 0.423096, 0.42038,
    0.417664, 0.415009, 0.412354, 0.409729, 0.407135, 0.404572, 0.402008, 0.399506, 0.397003, 0.394501,
    0.392059, 0.389618, 0.387207, 0.384827, 0.382477, 0.380127, 0.377808, 0.375488, 0.37323, 0.370972,
    0.368713, 0.366516, 0.364319, 0.362122, 0.359985, 0.357849, 0.355713, 0.353607, 0.351532, 0.349457,
    0.347412, 0.345398, 0.343384, 0.34137, 0.339417, 0.337463, 0.33551, 0.333588, 0.331665, 0.329773,
    0.327911, 0.32605, 0.324188, 0.322357, 0.320557, 0.318756, 0.316986, 0.315216, 0.313446, 0.311707,
    0.309998, 0.308289, 0.30658, 0.304901, 0.303223, 0.301575, 0.299927, 0.298309, 0.296692, 0.295074,
    0.293488, 0.291931, 0.290375, 0.288818, 0.287262, 0.285736, 0.284241, 0.282715, 0.28125, 0.279755,
    0.27829, 0.276825, 0.275391, 0.273956, 0.272552, 0.271118, 0.269745, 0.268341, 0.266968, 0.265594,
    0.264252, 0.262909, 0.261566, 0.260223, 0.258911, 0.257599, 0.256317, 0.255035, 0.25375 };
#include <picross/pic_config.h>
#ifndef PI_WINDOWS
static pic::lckarray_t<float,199> locker__(gaintable__);
#else
#pragma message ("warning: gain table not mlocked on windows")
#endif

static inline float __interp(float r, float a, float b)
{
    return (1-r)*a + r*b;
}

static inline double __clip(double x)
{
    if(!pic::isnormal(x))
    {
        return 0.0;
    }
    if(x>0.9999)
    {
        return 0.9999;
    }
    if(x<-0.9999)
    {
        return -0.9999;
    }
    return x;
}

void synthfunc_t::synth_setup(float fc, float res)
{
    double fc1 = std::max(0.001,(double)fc*2);
    double fc2 = fc1*fc1;
	double fc3 = fc1*fc2;
    coeff_ = __clip(-0.69346 * fc3 - 0.59515 * fc2 + 3.2937 * fc1 - 1.0072);
	float ix, ixfrac;
	int ixint;
	ix = coeff_ * 99;
	ixint = (int)ix;
	ixfrac = ix - ixint;
    feedback_ = res*__interp(ixfrac, gaintable__[ixint + 99], gaintable__[ixint + 100]);
}

#define POLE(pole) { double p=state_[pole]; output=output+coeff_*(output-p); state_[pole]=output; output=output+p; }

void synthfunc_t::synth_process1(float input,float *lp,float *hp,float *bp )
{
    // rand injects a tiny amount of noise (like analog filter)
    // which allows self-oscillation with high resonance (also fixes denormal issue)
	double output = 0.25*(input+(((rand()%2)-1)*1e-9)-feedback_*lowpass_);
    POLE(0)
    POLE(1)
    POLE(2)
    POLE(3)
    output = limiter_.process(output);
    lowpass_ = output;
	*lp = lowpass_;
	*hp = input-output;
	*bp = 3*state_[2]-lowpass_;
}

bool synthfunc_t::cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
{
    if(timer_ == 0)
    {
        if(limiter_.average()<1e-2)
        {
            //pic::logmsg() << "filter " << (void *)this << " turning off";
            memset(state_,0,sizeof(state_));
            lowpass_ = 0.f;
            return false;
        }
        timer_ = TIMER_TICKS;
    }

    --timer_;

    //float sr=(float)e->cfilterenv_clock()->get_sample_rate();
    float fc=current_freq_/sr;
    synth_setup(fc,current_resonance_);

    float *lp,*hp,*bp;
    float *lps,*hps,*bps;

    piw::data_nb_t dlp=piw::makenorm_nb(t,bs,&lp,&lps);
    piw::data_nb_t dhp=piw::makenorm_nb(t,bs,&hp,&hps);
    piw::data_nb_t dbp=piw::makenorm_nb(t,bs,&bp,&bps);

    piw::data_nb_t d;

    const float *audio_in=0;
    if(e->cfilterenv_nextsig(IN_AUDIO,d,t))
    {
        if(d.as_arraylen()>=bs)
        {
            audio_in = d.as_array();
        }

        timer_ = TIMER_TICKS;
    }

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

        }

        samp_to = sample_offset(bs,d.time(),f,t);

        if(samp_to>samp_from)
        {
            if(audio_in)
                synth_process(audio_in,lp,hp,bp,samp_from,samp_to);
            samp_from = samp_to;
        }
    }

    if(audio_in)
        synth_process(audio_in,lp,hp,bp,samp_from,bs);

    *lps=lp[bs-1];
    *hps=hp[bs-1];
    *bps=bp[bs-1];

    e->cfilterenv_output(OUT_LP,dlp);
    e->cfilterenv_output(OUT_HP,dhp);
    e->cfilterenv_output(OUT_BP,dbp);

    return true;
}

void synthfunc_t::synth_process(const float *audio_in, float *lp, float *hp, float *bp, unsigned samp_from, unsigned samp_to)
{
    if(audio_in)
    {
        for(unsigned i=samp_from; i<samp_to; ++i)
        {
            synth_process1(audio_in[i],&lp[i],&hp[i],&bp[i]);
        }
    }
    else
    {
        for(unsigned i=samp_from; i<samp_to; ++i)
        {
            synth_process1(0.f,&lp[i],&hp[i],&bp[i]);
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
    struct synthfilter_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new synthfunc_t(); }
        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };
}

synth::synthfilter_t::synthfilter_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
piw::cookie_t synth::synthfilter_t::cookie() { return impl_->cookie(); }
synth::synthfilter_t::~synthfilter_t() { delete impl_; }

