
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
#include <piw/piw_phase.h>
#include <piw/piw_tsd.h>
#include "loop_clicker.h"
#include <cmath>
#include <picross/pic_time.h>
#include <lib_samplerate/lib_samplerate.h>

#define IN_RUNNING 1
#define IN_BEAT 2
#define OUT_AUDIO 1

namespace
{
    struct clickfilter_t: piw::cfilterfunc_t
    {
        clickfilter_t(loop::clicker_t::impl_t *p): impl_(p), running_(false), inclock_ratio_(0.0), inclock_zero_(1e30), phase_(0), samplesize_(0), sampledata_(0), samplerate_(0), on_(false)
        {
        }

        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
        {
            pic::logmsg() << "event end";
            tick_running(time,0);
            on_ = false;
            return false;
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            running_=false; inclock_ratio_=0.0; inclock_zero_=1e30;
            on_=true;
            env->cfilterenv_reset(IN_RUNNING,id.time());
            env->cfilterenv_reset(IN_BEAT,id.time());
            return true;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env,unsigned long long f, unsigned long long now,unsigned long sr, unsigned bs);

        void tick_running(unsigned long long t, double v)
        {
            bool r = (v>0.0);

            if(r != running_)
            {
                running_ = r;

                if(!r)
                {
                    inclock_zero_ = 1e30;
                    inclock_ratio_=0.0;
                    phase_ = 0;
                    sample_ = loop::samplearray2ref_t();
                }

                pic::msg() << "transport has " << (r?"started":"stopped") << pic::log;
            }
        }

        void tick_clock(unsigned long long t, double v)
        {
            if(running_)
            {
                if(v > inclock_zero_)
                {
                    inclock_ratio_ = (v-inclock_zero_)/(t-inclock_offset_);
                }

                inclock_offset_ = t;
                inclock_zero_ = v;
            }
        }

        double interpolate_in(unsigned long long t)
        {
            return (inclock_zero_+inclock_ratio_*(double)(t-inclock_offset_));
        }

        unsigned click(bool accent, double beatoffset, double srate);

        piw::data_nb_t render_audio(unsigned len, unsigned long long t, float sr,unsigned bs);

        loop::clicker_t::impl_t *impl_;
        bool running_;
        double lastfloor_;
        double inclock_ratio_;
        double inclock_zero_;
        unsigned long long inclock_offset_;
        loop::samplearray2ref_t sample_;
        piw::phase_t phase_;
        unsigned long samplesize_;
        const float *sampledata_;
        float samplerate_;
        bool on_;
    };
};

struct loop::clicker_t::impl_t: piw::cfilterctl_t, piw::cfilter_t
{
    impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d, const loop::samplearray2ref_t &a, const loop::samplearray2ref_t &b): piw::cfilter_t(this,o,d), sounding_(true), accent_(a), beat_(b)
    {
    }

    piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &)
    {
        return new clickfilter_t(this);
    }

    void cfilterctl_delete(piw::cfilterfunc_t *f)
    {
        delete f;
    }

    unsigned cfilterctl_latency() { return 0; }

    unsigned long long cfilterctl_thru() { return 0; }
    unsigned long long cfilterctl_inputs() { return 3; }
    unsigned long long cfilterctl_outputs() { return 1; }

    bool sounding_;
    loop::samplearray2ref_t accent_;
    loop::samplearray2ref_t beat_;
};

bool clickfilter_t::cfilterfunc_process(piw::cfilterenv_t *env,unsigned long long f, unsigned long long now,unsigned long sr, unsigned bs)
{
    unsigned i;
    piw::data_nb_t d;

    if(!on_)
    {
        return false;
    }

    while(env->cfilterenv_next(i, d, now))
    {
        if(d.time() <= f)
            continue;

        switch(i)
        {
            case IN_RUNNING: tick_running(d.time(),d.as_norm()); break;
            case IN_BEAT: tick_clock(d.time(),d.as_renorm_float(BCTUNIT_BEATS,0,96000,0)); break;
        }
    }

    if(impl_->sounding_ && inclock_ratio_)
    {
        double nowbeat = interpolate_in(now);
        double nowfloor = floor(nowbeat);
        unsigned len = bs;
        float sr = (float)env->cfilterenv_clock()->get_sample_rate();

        if(nowfloor != lastfloor_)
        {
            bool b = (nowfloor==0);
            double beatoffset = nowbeat-nowfloor;
            len = click(b,beatoffset,sr);
        }

        if(sample_.isvalid() && len<=bs)
            env->cfilterenv_output(OUT_AUDIO,render_audio(len,now,sr,bs));

        lastfloor_ = nowfloor;
    }

    return true;
}

unsigned clickfilter_t::click(bool accent, double beatoffset, double srate)
{
    double diff_us = beatoffset/inclock_ratio_;
    sample_ = accent ? impl_->accent_ : impl_->beat_;

    if(sample_.isvalid())
    {
        phase_ = 0;
        samplesize_ = sample_->size;
        sampledata_ = sample_->data;
        samplerate_ = sample_->rate;
        return (unsigned)(diff_us*srate/1000000.0);
    }

    return 0;
}

piw::data_nb_t clickfilter_t::render_audio(unsigned len, unsigned long long t, float sr,unsigned bs)
{
    PIC_ASSERT(len<=bs);
    piw::phase_t inc(samplerate_/sr);

    float *auv, *aus;
    piw::data_nb_t d = piw::makenorm_nb(t,bs,&auv,&aus);
    memset(auv,0,bs*sizeof(float));

    unsigned long i = (unsigned long)(bs-len);
    unsigned idx = 0;

    for(; i<bs; ++i)
    {
        idx = phase_.index();
        unsigned i2 = idx;

        float x0 = (i2<samplesize_) ? sampledata_[i2++] : 0.f;
        float x1 = (i2<samplesize_) ? sampledata_[i2++] : 0.f;
        float x2 = (i2<samplesize_) ? sampledata_[i2++] : 0.f;
        float x3 = (i2<samplesize_) ? sampledata_[i2++] : 0.f;

        auv[i] = phase_.interpolate0_flt(x0,x1,x2,x3);
        phase_.advance(inc);
    }

    *aus = auv[bs-1];

    if(idx>=samplesize_)
        sample_ = loop::samplearray2ref_t();

    return d;
}

loop::clicker_t::clicker_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d, const loop::samplearray2ref_t &a, const loop::samplearray2ref_t &b): impl_(new impl_t(c,d,a,b))
{
}

void loop::clicker_t::play(bool on)
{
    impl_->sounding_ = on;
}

loop::clicker_t::~clicker_t()
{
    delete impl_;
}

piw::cookie_t loop::clicker_t::cookie()
{
    return impl_->cookie();
}

loop::samplearray2ref_t loop::canonicalise_samples(const std::string &data,float rate)
{
    long n = data.size();
    samplearray2ref_t ret(pic::ref(new samplearray2_t(n/2)));

    for(long i=0; i<n; i+=2)
    {
        unsigned char lo = (unsigned char)data[i];
        unsigned char hi = (unsigned char)data[i+1];
        short s = (hi<<8)|(lo);
        ret->data[i/2] = ((float)s)/32768.f;
    }

    ret->rate=rate;

    return ret;
}

struct loop::xplayer_t::impl_t: piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, piw::clocksink_t
{
    impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d, const loop::samplearray2ref_t &a): event_data_source_real_t(piw::pathnull(0)), file_(a), position_(0), domain_(d), gain_(0.5), ratio_(1.0)
    {
        d->sink(this,"xplayer");
        set_clock(this);
        connect(o);
        connect_wire(this,source());
        output_.set_signal(1,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_ISO));
        output_.set_signal(2,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_ISO));
        start_slow(piw::pathnull(0),output_);
        d->add_listener(pic::notify_t::method(this,&impl_t::sample_rate_changed));
        int e;
        src_state_ = src_new(SRC_SINC_MEDIUM_QUALITY,2,&e);
        tick_enable(false);
    }

    static int __changed(void *i_, void *sr_)
    {
        impl_t *i = (impl_t *)i_;
        unsigned long sr = *(unsigned long *)sr_;
        i->ratio_ = (double)sr/(double)i->file_->rate;
        return 0;
    }

    void sample_rate_changed()
    {
        unsigned long sr = domain_->get_sample_rate();
        piw::tsd_fastcall(__changed,this,&sr);
    }

    ~impl_t()
    {
        tick_disable();
        end_slow(piw::tsd_time());
        source_shutdown();
        src_delete(src_state_);
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        pic::flipflop_t<bool>::guard_t g(play_);
        if(!g.value())
            return;

        unsigned bs = get_buffer_size();
        float output[PLG_CLOCK_BUFFER_SIZE*2];
        unsigned len = 0;

        while(len<bs)
        {
            SRC_DATA src_data;
            src_data.input_frames = (file_->size-position_)/2;
            src_data.output_frames = bs-len;
            src_data.end_of_input = 0;
            src_data.src_ratio = ratio_;
            src_data.data_in = file_->data+position_;
            src_data.data_out = output+2*len;
            src_process(src_state_, &src_data);
            len += src_data.output_frames_gen;
            position_ += 2*src_data.input_frames_used;
            if(position_>=file_->size)
                position_ = 0;
        }

        float *lbuf, *rbuf, *ls, *rs;
        piw::data_nb_t ldata = piw::makenorm_nb(t,bs,&lbuf,&ls);
        piw::data_nb_t rdata = piw::makenorm_nb(t,bs,&rbuf,&rs);

        for(unsigned i=0; i<bs; i++)
        {
            lbuf[i] = gain_*output[2*i+0];
            rbuf[i] = gain_*output[2*i+1];
        }

        *ls = lbuf[bs-1];
        *rs = rbuf[bs-1];

        output_.add_value(1,ldata);
        output_.add_value(2,rdata);
    }

    loop::samplearray2ref_t file_;
    unsigned position_;
    piw::xevent_data_buffer_t output_;
    pic::flipflop_t<bool> play_;
    piw::clockdomain_ctl_t *domain_;
    float gain_;
    double ratio_;
    SRC_STATE *src_state_;
};

loop::xplayer_t::xplayer_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d, const loop::samplearray2ref_t &audio): impl_(new impl_t(c,d,audio))
{
}

loop::xplayer_t::~xplayer_t()
{
    delete impl_;
}

void loop::xplayer_t::play(bool b)
{
    impl_->play_.set(b);
}

static int __setgain(void *i_, void *g_)
{
    loop::xplayer_t::impl_t *i = (loop::xplayer_t::impl_t *)i_;
    float g = *(float *)g_;
    i->gain_ = g;
    return 0;
}

void loop::xplayer_t::set_gain(float g)
{
    piw::tsd_fastcall(__setgain,impl_,&g);
}
