
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

#include <picross/pic_float.h>

#include "synth.h"

#define IN_MASK SIG24(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24)

#define OUT_AUDIO 1
#define OUT_MASK SIG1(OUT_AUDIO)

namespace
{
    struct sumfunc_t: piw::cfilterfunc_t
    {
        sumfunc_t() {}

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            unsigned long long t=id.time();
            for(unsigned i=1; i<=24; ++i)
            {
                env->cfilterenv_reset(i,t);
            }
            return true;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to, unsigned long samplerate, unsigned buffersize)
        {
            float *f,*fs;
            piw::data_nb_t buffer;
            buffer = piw::makenorm_nb(to,buffersize,&f,&fs);
            memset(f,0,buffersize*sizeof(float));

            piw::data_nb_t d;
            unsigned sig;
            while(env->cfilterenv_next(sig,d,to))
            {
                const float *df = d.as_array();
                unsigned dfl = std::min(buffersize,d.as_arraylen());
                pic::vector::vectadd(df,1,f,1,f,1,dfl);
            }
            env->cfilterenv_output(OUT_AUDIO,buffer);

            return true;
        }

        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long to)
        {
            return false;
        }
    };
};

namespace synth
{
    struct polysummer_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this, o, d) {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new sumfunc_t(); }
        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };
}

synth::polysummer_t::polysummer_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
piw::cookie_t synth::polysummer_t::cookie() { return impl_->cookie(); }
synth::polysummer_t::~polysummer_t() { delete impl_; }

