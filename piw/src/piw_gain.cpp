
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

#include <piw/piw_gain.h>
#include <piw/piw_cfilter.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>
#include <piw/piw_address.h>
#include <vector>

namespace
{
    struct gainbase_t : piw::cfilterfunc_t
    {
        gainbase_t() : current_gain_(0)
        {
            last_audio_.reserve(2);
            current_audio_.reserve(2);
        }

        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
        {
            return false;
        }

        unsigned long long input_audio(unsigned index, const piw::data_nb_t &d)
        {
            if(index>=last_audio_.size())
            {
                last_audio_.resize(index+1);
                current_audio_.resize(index+1);
            }

            last_audio_[index] = d;
            current_audio_[index] = d.as_norm();
            return 1ULL<<(index+1);
        }

        unsigned long long input_gain(const piw::data_nb_t &d)
        {
            last_gain_ = d;
            current_gain_ = d.as_norm();
            return 1;
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            unsigned long long t=id.time();
            for(unsigned i=1; i<=MAX_SIGNALS; ++i)
            {
                env->cfilterenv_reset(i,t);
            }

            return true;
        }

        unsigned long long input(piw::cfilterenv_t *env,unsigned long long t)
        {
            unsigned long long changed = 0;
            unsigned sig;
            piw::data_nb_t val;

            while(env->cfilterenv_next(sig,val,t))
            {
                if(sig==1)
                {
                    changed |= input_gain(val);
                }
                else
                {
                    changed |= input_audio(sig-2,val);
                }
            }

            return changed;
        }

        pic::lckvector_t<float>::nbtype current_audio_;
        float current_gain_;
        pic::lckvector_t<piw::data_nb_t>::nbtype last_audio_;
        piw::data_nb_t last_gain_;
    };

    struct gainfunc_t: gainbase_t
    {
        gainfunc_t(const pic::f2f_t &f) : functor_(f)
        {
        }

        void do_neither(unsigned i, float *audio_out, unsigned bs)
        {
            float g = functor_(current_gain_);
            for(unsigned n=0; n<bs; ++n)
            {
                audio_out[n]=current_audio_[i]*g;
            }
        }

        void do_audio(unsigned i,float *audio_out, unsigned bs)
        {
            float g = functor_(current_gain_);
            const float *audio_in = last_audio_[i].as_array();
            unsigned dfl = std::min(bs,last_audio_[i].as_arraylen());

            for(unsigned n=0; n<dfl; ++n)
            {
                audio_out[n]=audio_in[n]*g;
            }
        }

        void do_gain(unsigned i,float *audio_out, unsigned bs)
        {
            const float *gain_in = last_gain_.as_array();

            for(unsigned n=0; n<bs; ++n)
            {
                float g = functor_(gain_in[n]);
                audio_out[n]=current_audio_[i]*g;
            }
        }

        void do_both(unsigned i,float *audio_out, unsigned bs)
        {
            const float *gain_in = last_gain_.as_array();
            const float *audio_in = last_audio_[i].as_array();
            unsigned dfl = std::min(last_gain_.as_arraylen(),std::min(bs,last_audio_[i].as_arraylen()));

            for(unsigned n=0; n<dfl; ++n)
            {
                float g = functor_(gain_in[n]);
                audio_out[n]=audio_in[n]*g;
            }
        }

        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
            unsigned long long changed = input(e,t);

            for(unsigned i = 0; i < last_audio_.size(); ++i)
            {
                float *buffer_out,*fs;
                piw::data_nb_t audio_out = piw::makenorm_nb(t,bs,&buffer_out,&fs);
                do_signal(i,buffer_out,changed,bs);
                *fs=buffer_out[bs-1];
                e->cfilterenv_output(i+2,audio_out);
            }

            last_audio_.resize(0);

            return true;
        }

        void do_signal(unsigned i,float *buffer_out, unsigned long long changed, unsigned bs)
        {
            bool audio_changed = changed&(1ULL<<(i+1));
            bool gain_changed = changed&1;

            if(audio_changed)
            {
                if(gain_changed)
                {
                    do_both(i,buffer_out,bs);
                }
                else
                {
                    do_audio(i,buffer_out,bs);
                }
            }
            else
            {
                if(gain_changed)
                {
                    do_gain(i,buffer_out,bs);
                }
                else
                {
                    do_neither(i,buffer_out,bs);
                }
            }
        }

        pic::flipflop_functor_t<pic::f2f_t> functor_;
    };

    struct linear_gainfunc_t: gainbase_t
    {
        void do_neither(unsigned i, float *audio_out, unsigned bs)
        {
            float g = current_gain_;
            for(unsigned n=0; n<bs; ++n)
            {
                audio_out[n]=current_audio_[i]*g;
            }
        }

        void do_audio(unsigned i,float *audio_out, unsigned bs)
        {
            float g = current_gain_;
            const float *audio_in = last_audio_[i].as_array();
            unsigned dfl = std::min(bs,last_audio_[i].as_arraylen());

            for(unsigned n=0; n<dfl; ++n)
            {
                audio_out[n]=audio_in[n]*g;
            }
        }

        void do_gain(unsigned i,float *audio_out, unsigned bs)
        {
            const float *gain_in = last_gain_.as_array();

            for(unsigned n=0; n<bs; ++n)
            {
                float g = gain_in[n];
                audio_out[n]=current_audio_[i]*g;
            }
        }
        void do_both(unsigned i, float *audio_out, unsigned bs)
        {
            const float *gain_in = last_gain_.as_array();
            const float *audio_in = last_audio_[i].as_array();
            unsigned dfl = std::min(last_gain_.as_arraylen(),std::min(bs,last_audio_[i].as_arraylen()));
            pic::vector::vectmul(gain_in,1,audio_in,1,audio_out,1,dfl);
        }

        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
            unsigned long long changed = input(e,t);

            for(unsigned i = 0; i < last_audio_.size(); ++i)
            {
                float *buffer_out,*fs;
                piw::data_nb_t audio_out = piw::makenorm_nb(t,bs,&buffer_out,&fs);
                do_signal(i,buffer_out,changed,bs);
                *fs=buffer_out[bs-1];
                e->cfilterenv_output(i+2,audio_out);
            }

            return true;
        }

        void do_signal(unsigned i,float *buffer_out, unsigned long long changed,unsigned bs)
        {
            bool audio_changed = changed&(1ULL<<(i+1));
            bool gain_changed = changed&1;

            if(audio_changed)
            {
                if(gain_changed)
                {
                    do_both(i,buffer_out,bs);
                }
                else
                {
                    do_audio(i,buffer_out,bs);
                }
            }
            else
            {
                if(gain_changed)
                {
                    do_gain(i,buffer_out,bs);
                }
                else
                {
                    do_neither(i,buffer_out,bs);
                }
            }
        }
    };
}

namespace piw
{
    struct gain_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const pic::f2f_t &f,const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d), functor_(f) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new gainfunc_t(functor_); }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return ~0ULL; }
        unsigned long long cfilterctl_outputs() { return ~0ULL; }

        pic::f2f_t functor_;
    };

    gain_t::gain_t(const pic::f2f_t &f,const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(f,o,d)) {}
    piw::cookie_t gain_t::cookie() { return impl_->cookie(); }
    gain_t::~gain_t() { delete impl_; }

    int gain_t::gc_traverse(void *v, void *a) const 
    {
        return impl_->functor_.gc_traverse(v,a);
    }

    int gain_t::gc_clear()
    {
        return impl_->functor_.gc_clear();
    }

    struct linear_gain_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new linear_gainfunc_t(); }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return ~0ULL; }
        unsigned long long cfilterctl_outputs() { return ~0ULL; }
    };

    linear_gain_t::linear_gain_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    piw::cookie_t linear_gain_t::cookie() { return impl_->cookie(); }
    linear_gain_t::~linear_gain_t() { delete impl_; }
}


