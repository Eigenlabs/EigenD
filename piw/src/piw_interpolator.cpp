
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

#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_policy.h>

#include "piw_isosupport.h"

#define IDLE_TICKS 100

#define SMALL_BUF (PLG_CLOCK_BUFFER_SIZE/32)
#define LARGE_BUF (PLG_CLOCK_BUFFER_SIZE)

/* input freqs in Hz/1000 */
#define INFREQ_44 1378125
#define INFREQ_48 1500000
#define INFREQ_96 1500000

#define CHEAP_ISO 1

#ifdef CHEAP_ISO

typedef piw::linear_interp_t<piw::isofilter_t<piw::zerohold_t<INFREQ_44,SMALL_BUF,7>,piw::cheb6_t,SMALL_BUF,7>,32,LARGE_BUF,0> interpolator_44_t;
typedef piw::linear_interp_t<piw::isofilter_t<piw::zerohold_t<INFREQ_48,SMALL_BUF,7>,piw::cheb6_t,SMALL_BUF,7>,32,LARGE_BUF,0> interpolator_48_t;
typedef piw::linear_interp_t<piw::isofilter_t<piw::zerohold_t<INFREQ_96,SMALL_BUF,7>,piw::cheb6_t,SMALL_BUF,7>,64,LARGE_BUF,0> interpolator_96_t;

#else

// 44100Hz sample rate
// 1378.125 * (4*4*2) = 44100
typedef piw::isofilter_t<piw::zerohold_t<INFREQ_44, SMALL_BUF, 7>, piw::cheb6_t, SMALL_BUF, 7> stage1_44_t;
typedef piw::isofilter_t<piw::upsampler_t<stage1_44_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> stage2_44_t;
typedef piw::isofilter_t<piw::upsampler_t<stage2_44_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> stage3_44_t;
typedef piw::isofilter_t<piw::upsampler_t<stage3_44_t, 2, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> interpolator_44_t;

// 48000Hz sample rate
// 1500 * (4*4*2) = 48000
typedef piw::isofilter_t<piw::zerohold_t<INFREQ_48, SMALL_BUF, 7>, piw::cheb6_t, SMALL_BUF, 7> stage1_48_t;
typedef piw::isofilter_t<piw::upsampler_t<stage1_48_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> stage2_48_t;
typedef piw::isofilter_t<piw::upsampler_t<stage2_48_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> stage3_48_t;
typedef piw::isofilter_t<piw::upsampler_t<stage3_48_t, 2, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> interpolator_48_t;

// 96000Hz sample rate
// 1500 * (4*4*4) = 96000
typedef piw::isofilter_t<piw::zerohold_t<INFREQ_96, SMALL_BUF, 7>, piw::cheb6_t, SMALL_BUF, 7> stage1_96_t;
typedef piw::isofilter_t<piw::upsampler_t<stage1_96_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> stage2_96_t;
typedef piw::isofilter_t<piw::upsampler_t<stage2_96_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> stage3_96_t;
typedef piw::isofilter_t<piw::upsampler_t<stage3_96_t, 4, LARGE_BUF, 7>, piw::cheb6_t, LARGE_BUF, 7> interpolator_96_t;

#endif

namespace
{
    struct interpolating_converter_t: piw::converter_t
    {
        interpolating_converter_t(float u, float l, float z): ubound_(u), lbound_(l), rest_(z), sample_rate_(48000), idle_(IDLE_TICKS)
        {
            sample_rate_changed();
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q,unsigned long long t)
        {
            idle_=IDLE_TICKS;
            index_=0;
            iqueue_=q;
            oqueue_=piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_ISO);

            piw::data_nb_t d;
            if(iqueue_.latest(d,&index_,t))
            {
                index_++;
                reset_(this,d.as_denorm_float());
            }
            else
            {
                reset_(this,rest_);
            }

            return oqueue_;
        }


        int ticked(unsigned long long from,unsigned long long now, unsigned long sr, unsigned bs)
        {
            if(idle_==0)
            {
                idle_=IDLE_TICKS;
                return TICK_SUPPRESS;
            }

            --idle_;


            piw::data_nb_t d;
            while(iqueue_.read(d,&index_,now))
            {
                idle_ = IDLE_TICKS;
                input_(this,d,ubound_,lbound_,rest_);
                index_++;
            }

            float *f,*fs;
            piw::data_nb_t out=piw::makearray_nb(now,ubound_, lbound_, rest_, bs,&f,&fs);
            memset(f,0,bs*sizeof(float));
            process_(this,bs,now);
            const float *iso = output_(this, bs);

            for(unsigned i = 0; i < bs; ++i)
            {
                f[i] = piw::normalise(ubound_,lbound_,rest_,iso[i]);
            }

            *fs = f[bs-1];
            oqueue_.write_fast(out);

            return TICK_ENABLE;
        }

        void sample_rate_changed()
        {
            switch(sample_rate_)
            {
                case 44100:
                    input_=input44; process_=process44; output_=output44; reset_=reset44; break;
                case 48000:
                    input_=input48; process_=process48; output_=output48; reset_=reset48; break;
                case 96000:
                    input_=input96; process_=process96; output_=output96; reset_=reset96; break;
                default:
                    pic::logmsg() << "unsupported sample rate " << sample_rate_;
                    input_=input48; process_=process48; output_=output48; reset_=reset48; break;
            }

            reset_(this,rest_);
        }

        static void input44(interpolating_converter_t *i, const piw::data_nb_t &d, float u, float l, float z) { i->i44_.input(d,u,l,z); }
        static void input48(interpolating_converter_t *i, const piw::data_nb_t &d, float u, float l, float z) { i->i48_.input(d,u,l,z); }
        static void input96(interpolating_converter_t *i, const piw::data_nb_t &d, float u, float l, float z) { i->i96_.input(d,u,l,z); }
        static void process44(interpolating_converter_t *i, unsigned n, unsigned long long t) { i->i44_.process(n,t); }
        static void process48(interpolating_converter_t *i, unsigned n, unsigned long long t) { i->i48_.process(n,t); }
        static void process96(interpolating_converter_t *i, unsigned n, unsigned long long t) { i->i96_.process(n,t); }
        static const float *output44(interpolating_converter_t *i, unsigned n) { return i->i44_.read(n); }
        static const float *output48(interpolating_converter_t *i, unsigned n) { return i->i48_.read(n); }
        static const float *output96(interpolating_converter_t *i, unsigned n) { return i->i96_.read(n); }
        static void reset44(interpolating_converter_t *i,float v) { i->i44_.reset(v); }
        static void reset48(interpolating_converter_t *i,float v) { i->i48_.reset(v); }
        static void reset96(interpolating_converter_t *i,float v) { i->i96_.reset(v); }

        interpolator_44_t i44_;
        interpolator_48_t i48_;
        interpolator_96_t i96_;

        void (*input_)(interpolating_converter_t *i, const piw::data_nb_t &d, float u, float l, float z);
        void (*process_)(interpolating_converter_t *i, unsigned n, unsigned long long t);
        const float * (*output_)(interpolating_converter_t *i, unsigned n);
        void (*reset_)(interpolating_converter_t *i,float v);

        float ubound_, lbound_, rest_;
        unsigned long sample_rate_;
        piw::dataqueue_t iqueue_, oqueue_;
        unsigned long long index_;
        unsigned idle_;
    };
};

piw::converter_ref_t piw::interpolating_converter(float u, float l, float z)
{
    return pic::ref(new interpolating_converter_t(u,l,z));
}

