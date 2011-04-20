
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

#include <piw/piw_clock.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_ring.h>
#include <piw/piw_address.h>
#include <picross/pic_log.h>
#include <math.h>
#include <map>

#include "synth.h"
#include "synth_sinetable.h"

#define IN_VELOCITY 1
#define IN_PRESSURE 10
#define IN_ATTACK 8
#define IN_DECAY 9
#define IN_RELEASE 11
#define IN_MASK SIG5(IN_VELOCITY,IN_PRESSURE,IN_ATTACK,IN_DECAY,IN_RELEASE)

#define THRU_MASK SIG1(IN_PRESSURE)

#define OUT_AMPLITUDE 1
#define OUT_MASK SIG1(OUT_AMPLITUDE)

#define DEFAULT_ATTACK_TIME 0.01
#define DEFAULT_DECAY_TIME 0.05
#define DEFAULT_RELEASE_TIME 0.25
#define DEFAULT_PRESSURE 0.0

#define STATE_OFF 0
#define STATE_ATTACK 1
#define STATE_DECAY 2
#define STATE_SUSTAIN 3
#define STATE_RELEASE 4

namespace
{
    struct cosine_ramp_t
    {
        void setup(float init, float target, float samples)
        {
            if(target > init)
            {
                height_=target-init;
                offset_=init;
                sign_=-1.0;
            }
            else
            {
                height_=init-target;
                offset_=target;
                sign_=1.0;
            }

            index_ = synth::sine_table_size/4;
            inc_ = (float)synth::sine_table_size/(2.0*samples);
        }

        float next()
        {
            float cos = interpolate(index_);
            float val = height_*0.5*(1.0+sign_*cos)+offset_;
            index_ += inc_;
            return val;
        }

        float interpolate(float f)
        {
            if(f>=0.75*float(synth::sine_table_size))
            {
                return -1.0*sign_;
            }
            unsigned i=(unsigned)f,j=(i+1);
            float frac=f-i;
            float y=synth::sine_table[i];
            return y+(frac*(synth::sine_table[j]-y));
        }

        float height_,sign_,index_,inc_,offset_;
    };

    struct state_t
    {
        state_t(unsigned s, float d=0, float p=0) : state(s),duration(d),param(p) {}
        unsigned state;
        float duration;
        float param;
    };

    struct asdrfunc_t: piw::cfilterfunc_t
    {
        asdrfunc_t() : current_pressure_(DEFAULT_PRESSURE),current_output_(0),attack_time_(DEFAULT_ATTACK_TIME),decay_time_(DEFAULT_DECAY_TIME),release_time_(DEFAULT_RELEASE_TIME), state_(STATE_OFF)
        {
        }

        bool cfilterfunc_end(piw::cfilterenv_t *e, unsigned long long t)
        {
            transitions_.clear();
            transitions_.insert(std::make_pair(t,state_t(STATE_RELEASE,release_time_)));
            unsigned long long t2 = t + (unsigned long long)(1000000.0*release_time_);
            //pic::logmsg() << "adsr linger " << (void *)this << ' ' << t << ' ' << t2;
            transitions_.insert(std::make_pair(t2,STATE_OFF));
            return true;
        }

        void setattack(const piw::data_nb_t &value)
        {
            attack_time_ = value.as_renorm_float(BCTUNIT_SECONDS,0,60,0);
        }

        void setdecay(const piw::data_nb_t &value)
        {
            decay_time_ = value.as_renorm_float(BCTUNIT_SECONDS,0,60,0);
        }

        void setrelease(const piw::data_nb_t &value)
        {
            release_time_ = value.as_renorm_float(BCTUNIT_SECONDS,0,60,0);
        }

        void setpressure(const piw::data_nb_t &value)
        {
            last_pressure_ = value;
            current_pressure_ = value.as_renorm(0,1,0);
        }

        void setup_env(unsigned long long t, float v)
        {
            transitions_.clear();
            transitions_.insert(std::make_pair(t,state_t(STATE_ATTACK,attack_time_,v*0.7)));
            t += (unsigned long long)(1000000.0*attack_time_);
            transitions_.insert(std::make_pair(t,state_t(STATE_DECAY,decay_time_)));
            t += (unsigned long long)(1000000.0*decay_time_);
            transitions_.insert(std::make_pair(t,STATE_SUSTAIN));
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long f, unsigned long long end,unsigned long sr, unsigned bs)
        {
            input(env,end);
            pic::lckmultimap_t<unsigned long long, state_t>::nbtype::iterator i,b,e;
            b = transitions_.begin();
            e = transitions_.upper_bound(end);

            if(b == e && state_ == STATE_OFF)
            {
                //pic::logmsg() << "adsr turning off " << (void *)this;
                return false;
            }

            //unsigned long sr = env->cfilterenv_clock()->get_sample_rate();
            float *out,*fs;
            unsigned pos = 0;
            piw::data_nb_t o = piw::makearray_nb(end,1,0,0,bs,&out,&fs);

            for(i = b; i != e; ++i)
            {
                unsigned len = ((i->first-end)*sr)/1000000;
                state_process(out,pos,len,i->first,bs);
                state_init(i->second,sr);
            }

            transitions_.erase(b,e);
            state_process(out,pos,bs-pos,end,bs);
            *fs=out[bs-1];
            env->cfilterenv_output(OUT_AMPLITUDE,o);
            last_pressure_ = piw::makenull_nb(0);
            return true;
        }

        void state_init(const state_t &s, unsigned long sr)
        {
            state_ = s.state;
            switch(state_)
            {
                case STATE_ATTACK:
                    ramp_.setup(current_output_,s.param,s.duration*(float)sr);
                    break;
                case STATE_DECAY:
                    ramp_.setup(current_output_,current_pressure_,s.duration*(float)sr);
                    break;
                case STATE_RELEASE:
                    ramp_.setup(current_output_,0,s.duration*(float)sr);
                    break;
            }
        }

        void state_process(float *b, unsigned &pos, unsigned len, unsigned long long end, unsigned bs)
        {
            len = std::min(len, bs-pos);
            switch(state_)
            {
                case STATE_ATTACK:
                case STATE_DECAY:
                case STATE_RELEASE:
                    for(; len > 0; --len)
                    {
                        b[pos] = ramp_.next();
                        ++pos;
                    }
                    break;

                case STATE_SUSTAIN:
                    if(last_pressure_.is_array())
                    {
                        const float *p = last_pressure_.as_array();
                        memcpy(b+pos,p+pos,len*sizeof(float));
                        pos += len;
                    }
                    else
                    {
                        for(; len > 0; --len)
                        {
                            b[pos] = current_pressure_;
                            ++pos;
                        }
                    }
                    break;

                case STATE_OFF:
                    for(; len > 0; --len)
                    {
                        b[pos] = 0;
                        ++pos;
                    }
                    break;
            }

            if(pos > 0)
            {
                current_output_ = b[pos-1];
            }
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            //pic::logmsg() << "adsr turning on " << id << ' ' << (void *)this;

            piw::data_nb_t d;
            unsigned long long t = id.time();
            if(env->cfilterenv_latest(IN_ATTACK,d,t)) setattack(d); 
            if(env->cfilterenv_latest(IN_DECAY,d,t)) setdecay(d); 
            if(env->cfilterenv_latest(IN_RELEASE,d,t)) setrelease(d); 
            if(env->cfilterenv_latest(IN_PRESSURE,d,t)) setpressure(d); 

            if(env->cfilterenv_latest(IN_VELOCITY,d,t))
            {
                float velocity = d.as_renorm(0,1,0);

                if(velocity)
                {
                    setup_env(id.time(),velocity);
                }

                return true;
            }

            pic::logmsg() << "no velocity";
            return false;
        }

        void input(piw::cfilterenv_t *env, unsigned long long t)
        {
            piw::data_nb_t d;

            if(env->cfilterenv_nextsig(IN_ATTACK,d,t)) setattack(d); 
            if(env->cfilterenv_nextsig(IN_DECAY,d,t)) setdecay(d); 
            if(env->cfilterenv_nextsig(IN_RELEASE,d,t)) setrelease(d); 
            if(env->cfilterenv_nextsig(IN_PRESSURE,d,t)) setpressure(d);
        }


        float current_pressure_;
        float current_output_;
        piw::data_nb_t last_pressure_;
        float attack_time_,decay_time_,release_time_;
        unsigned state_;
        cosine_ramp_t ramp_;
        pic::lckmultimap_t<unsigned long long, state_t>::nbtype transitions_;
    };
}

namespace synth
{
    struct adsr_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new asdrfunc_t(); }
        unsigned long long cfilterctl_thru() { return THRU_MASK; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };

    adsr_t::adsr_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    piw::cookie_t adsr_t::cookie() { return impl_->cookie(); }
    adsr_t::~adsr_t() { delete impl_; }
}

