
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

#include <piw/piw_panner.h>
#include <piw/piw_cfilter.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>
#include <piw/piw_address.h>
#include <vector>

namespace
{
    struct pannerfunc_t: piw::cfilterfunc_t
    {
        pannerfunc_t(const pic::f2f_t &f) : functor_(f), current_panner_(0)
        {
            last_audio_.reserve(2);
            current_audio_.reserve(2);
        }

        void do_neither(float *audio_out_l, float *audio_out_r, unsigned bs)
        {
            float l = functor_(current_panner_);
            float r = functor_(-current_panner_);

            for(unsigned n=0; n<bs; ++n)
            {
                audio_out_l[n]=current_audio_[0]*l;
                audio_out_r[n]=current_audio_[1]*r;
            }
        }

        void do_audio(float *audio_out_l, float *audio_out_r, unsigned bs)
        {
            float l = functor_(current_panner_);
            float r = functor_(current_panner_);

            const float *audio_in_l = last_audio_[0].as_array();
            const float *audio_in_r = last_audio_[1].as_array();
            unsigned dfl = std::min(bs,std::min(last_audio_[0].as_arraylen(),last_audio_[1].as_arraylen()));

            for(unsigned n=0; n<dfl; ++n)
            {
                audio_out_l[n]=audio_in_l[n]*l;
                audio_out_r[n]=audio_in_r[n]*r;
            }
        }

        void do_panner(float *audio_out_l, float *audio_out_r, unsigned bs)
        {
            const float *panner_in = last_panner_.as_array();
            unsigned dfl = std::min(bs,last_panner_.as_arraylen());

            for(unsigned n=0; n<dfl; ++n)
            {
                float l = functor_(panner_in[n]);
                float r = functor_(-panner_in[n]);
                audio_out_l[n]=current_audio_[0]*l;
                audio_out_r[n]=current_audio_[1]*r;
            }
        }

        void do_both(float *audio_out_l, float *audio_out_r, unsigned bs)
        {
            const float *panner_in = last_panner_.as_array();
            const float *audio_in_l = last_audio_[0].as_array();
            const float *audio_in_r = last_audio_[1].as_array();
            unsigned dfl = std::min(last_panner_.as_arraylen(),std::min(bs,std::min(last_audio_[0].as_arraylen(),last_audio_[1].as_arraylen())));

            for(unsigned n=0; n<dfl; ++n)
            {
                float l = functor_(panner_in[n]);
                float r = functor_(-panner_in[n]);
                audio_out_l[n]=audio_in_l[n]*l;
                audio_out_r[n]=audio_in_r[n]*r;
            }
        }

        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
            unsigned long long changed = input(e,t);

            float *buffer_out_l,*fsl;
            float *buffer_out_r,*fsr;
            piw::data_nb_t audio_out_l = piw::makenorm_nb(t,bs,&buffer_out_l,&fsl);
            piw::data_nb_t audio_out_r = piw::makenorm_nb(t,bs,&buffer_out_r,&fsr);
            do_signal(buffer_out_l,buffer_out_r,changed,bs);
            *fsl=buffer_out_l[bs-1];
            *fsr=buffer_out_r[bs-1];
            e->cfilterenv_output(1,audio_out_l);
            e->cfilterenv_output(2,audio_out_r);

            last_audio_.resize(0);

            return true;
        }

        void do_signal(float *buffer_out_l, float *buffer_out_r, unsigned long long changed, unsigned bs)
        {
            bool audio_changed = changed&6;
            bool panner_changed = changed&1;

            if(audio_changed)
            {
                if(panner_changed)
                {
                    do_both(buffer_out_l,buffer_out_r,bs);
                }
                else
                {
                    do_audio(buffer_out_l,buffer_out_r,bs);
                }
            }
            else
            {
                if(panner_changed)
                {
                    do_panner(buffer_out_l,buffer_out_r,bs);
                }
                else
                {
                    do_neither(buffer_out_l,buffer_out_r,bs);
                }
            }
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

        unsigned long long input_panner(const piw::data_nb_t &d)
        {
            last_panner_ = d;
            current_panner_ = d.as_norm();
            return 1;
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            unsigned long long t=id.time();

            env->cfilterenv_reset(1,t);
            env->cfilterenv_reset(2,t);
            env->cfilterenv_reset(3,t);

            return true;
        }

        unsigned long long input(piw::cfilterenv_t *env,unsigned long long t)
        {
            unsigned long long changed = 0;
            unsigned sig;
            piw::data_nb_t val;

            while(env->cfilterenv_next(sig,val,t))
            {
                switch(sig)
                {
                    case 1: changed |= input_panner(val); break;
                    case 2: changed |= input_audio(0,val); break;
                    case 3: changed |= input_audio(1,val); break;
                }
            }

            return changed;
        }

        pic::flipflop_functor_t<pic::f2f_t> functor_;
        float current_panner_;
        pic::lckvector_t<float>::nbtype current_audio_;
        pic::lckvector_t<piw::data_nb_t>::nbtype last_audio_;
        piw::data_nb_t last_panner_;
    };
}

namespace piw
{
    struct panner_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const pic::f2f_t &f,const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d), functor_(f) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new pannerfunc_t(functor_); }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return SIG4(1,2,3,4); }
        unsigned long long cfilterctl_outputs() { return SIG2(1,2); }

        pic::f2f_t functor_;
    };

    panner_t::panner_t(const pic::f2f_t &f,const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(f,o,d)) {}
    piw::cookie_t panner_t::cookie() { return impl_->cookie(); }
    panner_t::~panner_t() { delete impl_; }

    int panner_t::gc_traverse(void *v, void *a) const 
    {
        return impl_->functor_.gc_traverse(v,a);
    }

    int panner_t::gc_clear()
    {
        return impl_->functor_.gc_clear();
    }

}


