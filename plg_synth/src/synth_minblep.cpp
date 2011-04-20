
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

#include <picross/pic_config.h>


#include "synth.h"
#include "synth_blepdata.h"
#include <piw/piw_cfilter.h>
#include <piw/piw_clock.h>
#include <piw/piw_address.h>
#include <picross/pic_float.h>
#include <picross/pic_time.h>
#include <cmath>

#define BUFFER_SPACE  (PLG_CLOCK_BUFFER_SIZE*8)
#define BUFFER_LEN    (BUFFER_SPACE+TABLE_SAMPLES)

#define IN_VOL 1
#define IN_FREQ 2
#define IN_PARAM 3
#define IN_DETUNE 4
#define IN_MASK SIG4(IN_VOL,IN_FREQ,IN_PARAM,IN_DETUNE)

#define OUT_AUDIO 1
#define OUT_MASK SIG1(OUT_AUDIO)

#define DEFAULT_VOLUME 0.0
#define DEFAULT_FREQ 440.0
#define DEFAULT_PARAM 0.5
#define DEFAULT_DETUNE 0.0

namespace 
{
    struct minblep_buffer_t: piw::cfilterfunc_t
    {
        minblep_buffer_t(float p) : index_(0),phase_(p),current_volume_(DEFAULT_VOLUME),current_freq_(DEFAULT_FREQ),current_param_(DEFAULT_PARAM),current_detune_(powf(2.0,DEFAULT_DETUNE/1200.0)), on_(false)
        {
            memset(buffer_,0,BUFFER_LEN*sizeof(float));
        }

        void reserve(unsigned len)
        {
            PIC_ASSERT(len<=BUFFER_SPACE);
            if(index_+len >= BUFFER_SPACE)
            {
                memcpy(buffer_,buffer_+index_,TABLE_SAMPLES*sizeof(float));
                memset(buffer_+TABLE_SAMPLES,0,BUFFER_SPACE*sizeof(float));
                index_=0;
            }
        }

        void interpolate(float inc, float offset, float sign, const float *tbl, const float *dtbl)
        {
			if(!pic::isnormal(inc))
                return;
            // phase into minblep table
            float mbphase = phase_-offset;
            if(mbphase<0.0f) mbphase=0.0f;
            if(mbphase>=inc) mbphase=0.0f;

            // phase in oversample steps
            float ophase = TABLE_OVERSAMPLEF*mbphase/inc;
            if(ophase < 0.0f) ophase=0.0f;
            if(ophase >= TABLE_OVERSAMPLEF) ophase=0.0f;

            // index into minblep table
            unsigned ix = (unsigned)ophase;
            if(ix >= TABLE_OVERSAMPLE) ix=0;

            // interpolation x offset
            float dx = ophase-(float)ix;

            // current output position
            float *b = buffer_+index_;

            // number of samples to fill
            unsigned n = (TABLE_SIZE-ix)/TABLE_OVERSAMPLE;

            const float *t = tbl+ix, *d = dtbl+ix;
            for(unsigned i=0; i<n; ++i,++b,t+=TABLE_OVERSAMPLE,d+=TABLE_OVERSAMPLE)
                *b += (sign*((*t)+dx*(*d)));
        }

        void step(float inc, float offset, float sign)
        {
            interpolate(inc,offset,sign,blepstep__,blepdelta__);
        }

        void cusp(float inc, float offset, float sign)
        {
            interpolate(inc,offset,sign*inc,blepcusp__,blepcdelta__);
        }

        void setfreq(const piw::data_nb_t &d)
        {
            current_freq_ = std::max(0.5f,d.as_renorm_float(BCTUNIT_HZ,0,96000,440));

        }
        void setparam(const piw::data_nb_t &d) { current_param_ = d.as_norm(); }
        void setdetune(const piw::data_nb_t &d) { current_detune_ = powf(2.0, d.as_renorm(-1200,1200,0)/1200.0); }

        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long t)
        {
            on_=false;
            return false;
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            on_=true;
            current_volume_=DEFAULT_VOLUME;
            current_freq_=DEFAULT_FREQ;
            current_param_=DEFAULT_PARAM;
            current_detune_=DEFAULT_DETUNE;

            unsigned long long t=id.time();
            piw::data_nb_t d;
            if(env->cfilterenv_latest(IN_FREQ,d,t)) setfreq(d);
            if(env->cfilterenv_latest(IN_PARAM,d,t)) setparam(d);
            if(env->cfilterenv_latest(IN_DETUNE,d,t)) setdetune(d);
            env->cfilterenv_reset(IN_VOL,t);

            return true;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long t,unsigned long sr, unsigned bs)
        {
            if(!on_)
            {
                return false;
            }

            float *out, *fs;
            
            piw::data_nb_t o = piw::makenorm_nb(t,bs,&out,&fs);
            reserve(bs);

            //float sr = (float)env->cfilterenv_clock()->get_sample_rate();
            float inc = current_freq_*current_detune_/sr;

            piw::data_nb_t d;

            const float *audio_in=0;
            if(env->cfilterenv_nextsig(IN_VOL,d,t))
            {
                audio_in = d.as_array();
                current_volume_ = d.as_renorm(0,1,0);
            }

            unsigned sig;
            unsigned samp_from = 0;
            unsigned samp_to = 0;

            while(env->cfilterenv_next(sig,d,t))
            {
                switch(sig)
                {
                    case IN_FREQ:
                        setfreq(d);
                        inc = current_freq_*current_detune_/sr;
                        break;

                    case IN_DETUNE:
                        setdetune(d);
                        inc = current_freq_*current_detune_/sr;
                        break;

                    case IN_PARAM:
                        setparam(d);
                        break;
                }

                samp_to = sample_offset(bs,d.time(),from,t);

                if(samp_to>samp_from)
                {
                    process(audio_in,out,inc,samp_from,samp_to);
                    samp_from = samp_to;
                }
            }

            process(audio_in,out,inc,samp_from,bs);

            //pic::logmsg() << "sawtooth output " << o;
            *fs=out[bs-1];
            env->cfilterenv_output(OUT_AUDIO,o);

            return true;
        }

        virtual void process(const float *audio_in, float *out, float inc, unsigned from, unsigned to) = 0;

        float buffer_[BUFFER_LEN];
        unsigned index_;
        float phase_;
        float current_volume_, current_freq_, current_param_, current_detune_;
        bool on_;
    };

    struct sawfunc_t : minblep_buffer_t
    {
        sawfunc_t(): minblep_buffer_t(0.5) {}

        void process(const float *audio_in,float *out,float inc,unsigned from,unsigned to)
        {
            if(audio_in)
            {
                for(unsigned i=from; i<to; ++i,++index_)
                {
                    if(phase_ >= 1.0)
                    {
                        phase_ -= 1.0;
                        step(inc, 0.0, 1.0);
                    }

                    buffer_[index_+TABLE_DELAY] += (0.5-phase_);
                    out[i] = buffer_[index_]*piw::denormalise(1.f,0.f,0.f,audio_in[i]);
                    phase_+=inc;
                }
            }
            else
            {
                for(unsigned i=from; i<to; ++i,++index_)
                {
                    if(phase_ >= 1.0)
                    {
                        phase_ -= 1.0;
                        step(inc, 0.0, 1.0);
                    }

                    buffer_[index_+TABLE_DELAY] += (0.5-phase_);
                    out[i] = current_volume_*buffer_[index_];
                    phase_+=inc;
                }
            }
        }
    };

    struct rectfunc_t : minblep_buffer_t
    {
        rectfunc_t(): minblep_buffer_t(0.0) { state_ = 0; }

        void process(const float *audio_in,float *out, float inc, unsigned from, unsigned to)
        {
            if(audio_in)
            {
                float p = piw::denormalise(0.9f,0.1f,0.5f,current_param_);

                for(unsigned i=from; i<to; ++i,++index_)
                {
                    switch(state_)
                    {
                        case 0:
                            if(phase_ >= p)
                            {
                                step(inc, p, -1.0);
                                state_=1;
                            }
                            break;

                        case 1:
                            if(phase_ >= 1.0)
                            {
                                phase_ -= 1.0;
                                step(inc, 0.0, 1.0);
                                state_=0;
                            }
                            break;
                    }

                    buffer_[index_+TABLE_DELAY] += (state_?-0.5:0.5);
                    out[i] = buffer_[index_]*piw::denormalise(1.f,0.f,0.f,audio_in[i]);
                    phase_+=inc;
                }
            }
            else
            {
                float p = (current_param_*0.5+1)/4;
                for(unsigned i=from; i<to; ++i,++index_)
                {
                    switch(state_)
                    {
                        case 0:
                            if(phase_ >= p)
                            {
                                step(inc, p, -1.0);
                                state_=1;
                            }
                            break;

                        case 1:
                            if(phase_ >= 1.0)
                            {
                                phase_ -= 1.0;
                                step(inc, 0.0, 1.0);
                                state_=0;
                            }
                            break;
                    }

                    buffer_[index_+TABLE_DELAY] += (state_?-0.5:0.5);
                    out[i] = current_volume_*buffer_[index_];
                    phase_+=inc;
                }
            }
        }

        int state_;
    };

    struct trifunc_t: minblep_buffer_t
    {
        trifunc_t(): minblep_buffer_t(0.0) { state_ = 0; }

        void process(const float *audio_in,float *out, float inc,unsigned from, unsigned to)
        {
            if(audio_in)
            {
                for(unsigned i=from; i<to; ++i,++index_)
                {
                    switch(state_)
                    {
                        case 0:
                            if(phase_ >= 0.5)
                            {
                                cusp(inc, 0.5, -4.0);
                                state_=1;
                            }
                            break;

                        case 1:
                            if(phase_ >= 1.0)
                            {
                                phase_ -= 1.0;
                                cusp(inc, 0, 4.0);
                                state_=0;
                            }
                            break;
                    }

                    if(state_==0)
                        buffer_[index_+TABLE_DELAY] += ((phase_*2.0)-0.5);
                    if(state_==1)
                        buffer_[index_+TABLE_DELAY] += (1.5-(phase_*2.0));

                    out[i] = buffer_[index_]*piw::denormalise(1.f,0.f,0.f,audio_in[i]);
                    phase_+=inc;
                }
            }
            else
            {
                for(unsigned i=from; i<to; ++i,++index_)
                {
                    switch(state_)
                    {
                        case 0:
                            if(phase_ >= 0.5)
                            {
                                cusp(inc, 0.5, -4.0);
                                state_=1;
                            }
                            break;

                        case 1:
                            if(phase_ >= 1.0)
                            {
                                phase_-=1.0;
                                cusp(inc, 0, 4.0);
                                state_=0;
                            }
                            break;
                    }

                    if(state_==0)
                        buffer_[index_+TABLE_DELAY] += ((phase_*2.0)-0.5);
                    if(state_==1)
                        buffer_[index_+TABLE_DELAY] += (1.5-(phase_*2.0));

                    out[i] = current_volume_*buffer_[index_];
                    phase_+=inc;
                }
            }
        }

        int state_;
    };
}


namespace synth
{
    struct sawtooth_t::impl_t: piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new sawfunc_t; }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };

    sawtooth_t::sawtooth_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    piw::cookie_t sawtooth_t::cookie() { return impl_->cookie(); }
    sawtooth_t::~sawtooth_t() { delete impl_; }

    struct rect_t::impl_t: piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new rectfunc_t; }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };

    rect_t::rect_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    piw::cookie_t rect_t::cookie() { return impl_->cookie(); }
    rect_t::~rect_t() { delete impl_; }

    struct triangle_t::impl_t: piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new trifunc_t; }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };

    triangle_t::triangle_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    piw::cookie_t triangle_t::cookie() { return impl_->cookie(); }
    triangle_t::~triangle_t() { delete impl_; }
}


