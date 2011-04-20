
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

#include <picross/pic_stl.h>
#include <picross/pic_float.h>

#include <piw/piw_cfilter.h>
#include <piw/piw_clock.h>
#include <piw/piw_address.h>
#include <piw/piw_sampler.h>
#include <piw/piw_phase.h>

#include <plg_sampler2/smp_player.h>

#include <math.h>
#include <list>
#include <map>

#define IN_FREQ 1
#define IN_DETUNE 2
#define IN_ACTIVATION 3

#define OUT_LEFT 1
#define OUT_RIGHT 2

#define DEFAULT_FREQ 440.0
#define DEFAULT_DETUNE 1.0

#define ACTIVATION_TICKS 50

namespace
{
    struct playerctl_t;
    struct loaderctl_t;

    struct playerfunc_t: piw::cfilterfunc_t
    {
        playerfunc_t(playerctl_t *ctl);

        void process(unsigned from,unsigned to);
        void event(const piw::voiceref_t &,unsigned long long t);

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id);
        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs);
        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long);

        bool setfreq(const piw::data_nb_t &value);
        bool setdetune(const piw::data_nb_t &value);
        piw::voiceref_t setactivation(const piw::data_nb_t &value);
        void recalc_freq(piw::cfilterenv_t *);

        playerctl_t *ctl_;
        piw::sampler_voiceref_t voice_;
        piw::sampler_voiceref_t fading1_;
        piw::sampler_voiceref_t fading2_;
        float current_freq_, current_detune_;
        piw::data_nb_t outdata0_;
        piw::data_nb_t outdata1_;
        float *outbuffer0_;
        float *outbuffer1_;
        piw::data_nb_t id_;
        bool activated_;
        float activation_;
        unsigned count_;
    };

    struct playerctl_t: piw::cfilterctl_t, pic::lckobject_t
    {
        playerctl_t(): fade_(true), playercount_(0) {}

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new playerfunc_t(this); }

        void define_voice(const piw::data_nb_t &id,const piw::voiceref_t &voice);
        piw::voiceref_t find_voice(const piw::data_nb_t &id);

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return 7; }
        unsigned long long cfilterctl_outputs() { return 3; }

        pic::lckmap_t<piw::data_nb_t,piw::voiceref_t,piw::path_less>::nbtype id2voice_;
        bool fade_;
        unsigned long long playercount_;
    };

}

struct sampler2::player_t::impl_t
{
    impl_t(const piw::cookie_t &c,piw::clockdomain_ctl_t *d);
    ~impl_t();

    piw::cookie_t cookie() { return filter_.cookie(); }
    playerctl_t ctl_;
    piw::cfilter_t filter_;
    bct_clocksink_t *attached_;
};

piw::voiceref_t playerctl_t::find_voice(const piw::data_nb_t &id)
{
    pic::lckmap_t<piw::data_nb_t,piw::voiceref_t,piw::path_less>::nbtype::iterator i;
    piw::voiceref_t v;

    if((i=id2voice_.find(id))!=id2voice_.end())
    {
        v = i->second;
        id2voice_.erase(i);
    }

    return v;
}

void playerctl_t::define_voice(const piw::data_nb_t &id, const piw::voiceref_t &voice)
{
    id2voice_.erase(id);

    if(voice.isvalid())
    {
        id2voice_.insert(std::make_pair(id,voice));
    }
}

playerfunc_t::playerfunc_t(playerctl_t *ctl): ctl_(ctl), current_freq_(DEFAULT_FREQ), current_detune_(DEFAULT_DETUNE)
{
}

bool playerfunc_t::cfilterfunc_end(piw::cfilterenv_t *e, unsigned long long time)
{
    return activated_;
}

void playerfunc_t::process(unsigned from,unsigned to)
{
    if(fading2_.isvalid())
    {
        if(fading2_->fade(outbuffer0_,outbuffer1_,from,to))
        {
            fading2_.clear();
        }
    }

    if(fading1_.isvalid())
    {
        if(fading1_->fade(outbuffer0_,outbuffer1_,from,to))
        {
            fading1_.clear();
        }
    }

    if(voice_.isvalid())
    {
        if(voice_->write(outbuffer0_,outbuffer1_,from,to))
        {
            voice_.clear();
            ctl_->playercount_--;
            //pic::logmsg() << "player voice count " << ctl_->playercount_;
        }
    }
}

void playerfunc_t::event(const piw::voiceref_t &v, unsigned long long t)
{
    if(voice_.isvalid())
    {
        fading2_ = fading1_;
        fading1_ = voice_;
        voice_.clear();
        ctl_->playercount_--;
        //pic::logmsg() << "player voice count " << ctl_->playercount_;
    }

    if(v.isvalid())
    {
        voice_=piw::create_player(v);
        ctl_->playercount_++;
        //pic::logmsg() << "player voice count " << ctl_->playercount_;
        if(!ctl_->fade_) 
            voice_->disable_fadein();
        //pic::logmsg() << "voice started at " << t;
    }
    else
    {
        //pic::logmsg() << "voice ended at " << t;
    }
}

bool playerfunc_t::cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs)
{
    float *fs;
    outdata0_ = piw::makenorm_nb(to,bs,&outbuffer0_,&fs); *fs=0;
    outdata1_ = piw::makenorm_nb(to,bs,&outbuffer1_,&fs); *fs=0;
    memset(outbuffer0_,0,bs*sizeof(float));
    memset(outbuffer1_,0,bs*sizeof(float));

    piw::data_nb_t d;
    unsigned sig;

    unsigned samp_from = 0;
    unsigned samp_to = 0;

    while(env->cfilterenv_next(sig,d,to))
    {
        samp_to = sample_offset(bs,d.time(),from,to);

        if(samp_to > samp_from)
        {
            process(samp_from,samp_to);
            samp_from = samp_to;
        }

        switch(sig)
        {
            case 0:
                event(piw::voiceref_t(),d.time());
                break;

            case IN_FREQ:
                setfreq(d);
                recalc_freq(env);
                break;

            case IN_DETUNE:
                setdetune(d);
                recalc_freq(env);
                break;

            case IN_ACTIVATION:
                //pic::logmsg() << "activation at " << d.time() << " samp_from=" << samp_from;
                if(d.as_norm()!=activation_)
                {
                    activation_ = d.as_norm();
                    event(setactivation(d),d.time());
                    recalc_freq(env);
                }
                break; 
        }
    }

    if(samp_from==0)
    {
        if(count_<ACTIVATION_TICKS) 
        {
            count_++;
        }
        else
        {
            if(!fading1_.isvalid() && !fading2_.isvalid() && !voice_.isvalid())
            {
                current_freq_ = DEFAULT_FREQ;
                current_detune_ = DEFAULT_DETUNE;
                //pic::logmsg() << "sampler turning off " << (void *)this;
                return false;
            }
        }
    }

    process(samp_from,bs);

    env->cfilterenv_output(1,outdata0_);
    env->cfilterenv_output(2,outdata1_);

    return true;
}

void playerfunc_t::recalc_freq(piw::cfilterenv_t *env)
{
    if(voice_.isvalid())
    {
        unsigned long sr = env->cfilterenv_clock()->get_sample_rate();
        float f = current_freq_*current_detune_;
        voice_->recalc_freq(sr,f);
    }
}

sampler2::player_t::player_t(const piw::cookie_t &c,piw::clockdomain_ctl_t *d): impl_(new impl_t(c,d))
{
}

sampler2::player_t::~player_t()
{
    tracked_invalidate();
    delete impl_;
}

piw::cookie_t sampler2::player_t::cookie()
{
    return impl_->cookie();
}

bool sampler2::player_t::attach_loader(bct_clocksink_t *c)
{
    if(!impl_->attached_)
    {
        impl_->filter_.sink()->add_upstream(c);
        impl_->attached_ = c;
        //pic::logmsg() << "sampler loader attached " << (void *)c;
        return true;
    }

    return false;
}

void sampler2::player_t::detach_loader()
{
    if(impl_->attached_)
    {
        impl_->filter_.sink()->remove_upstream(impl_->attached_);
        impl_->attached_=0;
    }
}

void sampler2::player_t::define_voice(const piw::data_nb_t &id, const piw::voiceref_t &voice)
{
    impl_->ctl_.define_voice(id,voice);
}

bool playerfunc_t::cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
{
    //pic::logmsg() << "player start:"; env->cfilterenv_dump(false);
    count_=0;
    activated_=false;
    activation_=0.0;
    current_freq_ = DEFAULT_FREQ;
    current_detune_ = DEFAULT_DETUNE;
    id_=id;

    piw::data_nb_t d;
    unsigned long long t = id.time();
    if(env->cfilterenv_latest(IN_FREQ,d,t)) setfreq(d);
    if(env->cfilterenv_latest(IN_DETUNE,d,t)) setdetune(d);
    env->cfilterenv_reset(IN_ACTIVATION,t);

    return true;
}

piw::voiceref_t playerfunc_t::setactivation(const piw::data_nb_t &value)
{
    bool act = (value.as_norm()>0.0);

    if(act!=activated_)
    {
        activated_=act;
        if(act)
        {
            return ctl_->find_voice(id_);
        }
    }

    return piw::voiceref_t();
}

bool playerfunc_t::setfreq(const piw::data_nb_t &value)
{
    current_freq_ = value.as_renorm_float(BCTUNIT_HZ,0,96000,0);
    return true;
}

bool playerfunc_t::setdetune(const piw::data_nb_t &value)
{
    current_detune_ = powf(2.0, value.as_renorm_float(-1200,1200,0)/1200.0);
    return true;
}

sampler2::player_t::impl_t::impl_t(const piw::cookie_t &c,piw::clockdomain_ctl_t *d): filter_(&ctl_,c,d), attached_(0)
{
}

sampler2::player_t::impl_t::~impl_t()
{
}

void sampler2::player_t::set_fade(bool fade)
{
    impl_->ctl_.fade_=fade;
}
