
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

#include <piw/piw_cfilter.h>
#include <piw/piw_clock.h>
#include <picross/pic_log.h>

#include "synth.h"
#include "synth_sinetable.h"

#include <math.h>

#define IN_VOL 1
#define IN_FREQ 2
#define IN_DETUNE 4
#define IN_MASK SIG3(IN_VOL,IN_FREQ,IN_DETUNE)
#define OUT_AUDIO 1
#define OUT_MASK 1

#define DEFAULT_VOLUME 0.0
#define DEFAULT_FREQ 440.0
#define DEFAULT_DETUNE 0.0

namespace
{
    struct wavetable_t: piw::cfilterfunc_t
    {
        wavetable_t(const float *samples, unsigned size): samples_(samples), size_(size), count_(0.0),current_volume_(DEFAULT_VOLUME),current_freq_(DEFAULT_FREQ),current_detune_(powf(2.0,DEFAULT_DETUNE/1200.0)), on_(false)
        {
        }

        void setfreq(const piw::data_nb_t &value) { current_freq_ = value.as_renorm_float(BCTUNIT_HZ,0,96000,440); }
        void setdetune(const piw::data_nb_t &value) { current_detune_ = powf(2.0, value.as_renorm(-1200,1200,0)/1200.0); }

        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long t)
        {
            on_=false;
            return false;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long t,unsigned long sr, unsigned bs)
        {
            if(!on_)
            {
                return false;
            }

            float *out, *fs;

            piw::data_nb_t o = piw::makenorm_nb(t,bs,&out,&fs);

            float inc = size_*current_freq_*current_detune_/sr;

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
                        inc = size_*current_freq_*current_detune_/sr;
                        break;

                    case IN_DETUNE:
                        setdetune(d);
                        inc = size_*current_freq_*current_detune_/sr;
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

            *fs=out[bs-1];
            //pic::logmsg() << "sine output " << o << ' ' << *fs << ' ' << current_freq_ << ' ' << inc << ' ' << sr << ' ' << current_detune_ << ' ' << current_volume_;
            env->cfilterenv_output(OUT_AUDIO,o);

            return true;
        }

        void process(const float *vol,float *out, float inc, unsigned from, unsigned to)
        {
            if(vol)
            {
                for(unsigned i=from; i<to; ++i)
                {
                    if(count_>=size_)
                    {
                        count_-=size_;
                    }

                    out[i]=interpolate(count_)*vol[i];
                    count_+=inc;
                }
            }
            else
            {
                for(unsigned i=from; i<to; ++i)
                {
                    if(count_>=size_)
                    {
                        count_-=size_;
                    }

                    out[i]=interpolate(count_)*current_volume_;
                    count_+=inc;
                }
            }
        }

        float interpolate(float f) const
        {
            unsigned i=(unsigned)f,j=(i+1)%size_;
            float frac=f-i;
            float y=samples_[i];
            return y+(frac*(samples_[j]-y));
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            on_=true;
            current_volume_=DEFAULT_VOLUME;
            current_freq_=DEFAULT_FREQ;
            current_detune_=DEFAULT_DETUNE;

            unsigned long long t=id.time();
            piw::data_nb_t d;
            if(env->cfilterenv_latest(IN_FREQ,d,t)) setfreq(d);
            if(env->cfilterenv_latest(IN_DETUNE,d,t)) setdetune(d);
            env->cfilterenv_reset(IN_VOL,t);

            return true;
        }

        const float *samples_;
        unsigned size_;
        float count_;
        float current_volume_, current_freq_, current_detune_;
        piw::data_nb_t last_volume_;
        bool on_;
    };
}


namespace synth
{
    struct sine_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new wavetable_t(sine_table, sine_table_size); }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };

    sine_t::sine_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    piw::cookie_t sine_t::cookie() { return impl_->cookie(); }
    sine_t::~sine_t() { delete impl_; }
}
