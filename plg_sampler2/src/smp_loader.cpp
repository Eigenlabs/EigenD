
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
#include <piw/piw_sample.h>

#include <plg_sampler2/smp_loader.h>

#define IN_FREQ 1
#define IN_VELOCITY 3

#define OUT_LOADER_ACTIVATION 1
#define OUT_LOADER_AHDSR_DELAY 2
#define OUT_LOADER_AHDSR_ATTACK 3
#define OUT_LOADER_AHDSR_HOLD 4
#define OUT_LOADER_AHDSR_DECAY 5
#define OUT_LOADER_AHDSR_SUSTAIN 6
#define OUT_LOADER_AHDSR_RELEASE 7

#define DEFAULT_FREQ 440.0

namespace
{
    struct loaderctl_t;

    struct loaderfunc_t: piw::cfilterfunc_t
    {
        loaderfunc_t(loaderctl_t *ctl): ctl_(ctl), running_(false) {}

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs) { return running_; }
        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long);
        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id);

        loaderctl_t *ctl_;
        bool running_;
        piw::data_nb_t id_;
    };

    struct loaderctl_t: piw::cfilterctl_t
    {
        loaderctl_t();
        ~loaderctl_t();

        void attach_sampler(sampler2::player_t *s, piw::clocksink_t *c);
        void detach_sampler();

        void load(const piw::presetref_t &p) { synth_.set(p); }
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new loaderfunc_t(this); }
        piw::voiceref_t find_voice(float v,float f);
        void start_voice(const piw::data_nb_t &id,const piw::voiceref_t &);

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return 5; }
        unsigned long long cfilterctl_outputs() { return 127; }

        pic::flipflop_t<piw::presetref_t> synth_;
        pic::weak_t<sampler2::player_t> link_;
    };
}

struct sampler2::loader_t::impl_t: pic::lckobject_t
{
    impl_t(player_t *s,const piw::cookie_t &c, piw::clockdomain_ctl_t *d);
    ~impl_t();

    piw::cookie_t cookie() { return filter_.cookie(); }
    void load(const piw::presetref_t &p) { ctl_.load(p); }

    loaderctl_t ctl_;
    piw::cfilter_t filter_;
};

piw::voiceref_t loaderctl_t::find_voice(float v,float f)
{
    if(v>0.0)
    {
        pic::flipflop_t<piw::presetref_t>::guard_t g(synth_);

        if(g.value().isvalid())
        {
            return g.value()->find_zone(v,f);
        }
    }

    return piw::voiceref_t();
}

void loaderctl_t::start_voice(const piw::data_nb_t &id,const piw::voiceref_t &voice)
{
    if(link_.isvalid())
    {
        pic::weak_t<sampler2::player_t>::guard_t g(link_);

        if(g.value())
        {
            g.value()->define_voice(id,voice);
        }
    }
}

bool loaderfunc_t::cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
{
    //pic::logmsg() << "loader start:"; env->cfilterenv_dump(false);
    piw::data_nb_t d;
    float freq,vel;

    running_=false;

    
    if(env->cfilterenv_latest(IN_FREQ,d,id.time()))
    {
        freq = d.as_renorm_float(BCTUNIT_HZ,0,96000,0);
        if(!freq) pic::logmsg() << "*************** freq=0";
    }
    else
    {
        freq = DEFAULT_FREQ;
    }

    if(env->cfilterenv_latest(IN_VELOCITY,d,id.time()))
    {
        vel = d.as_renorm(0,1,0);
    }
    else
    {
        vel = 0;
    }

    if(vel<=0.0)
    {
        return false;
    }

    //pic::logmsg() << "velocity: " << vel;
    piw::voiceref_t voice = ctl_->find_voice(vel,freq);

    if(!voice.isvalid())
    {
        pic::logmsg() << "loader didnt find voice for vel=" << vel << " freq=" << freq;
        return false;
    }

    float ede,ea,eh,edc,es,er;
    voice->envelope(vel,freq,&ede,&ea,&eh,&edc,&es,&er);

    //pic::logmsg() << "loader found voice vel=" << vel << " freq=" << freq << " time=" << id.time() << " delay=" << ede << " attack=" << ea << " hold=" << eh << " decay=" << edc << " sustain=" << es << " release=" << er;

    env->cfilterenv_output(OUT_LOADER_AHDSR_DELAY,piw::makefloat_bounded_units_nb(BCTUNIT_SECONDS,60000.0,0.0,0.0,ede,id.time()));
    env->cfilterenv_output(OUT_LOADER_AHDSR_ATTACK,piw::makefloat_bounded_units_nb(BCTUNIT_SECONDS,60000.0,0.0,0.0,ea,id.time()));
    env->cfilterenv_output(OUT_LOADER_AHDSR_HOLD,piw::makefloat_bounded_units_nb(BCTUNIT_SECONDS,60000.0,0.0,0.0,eh,id.time()));
    env->cfilterenv_output(OUT_LOADER_AHDSR_SUSTAIN,piw::makefloat_bounded_nb(1.0,0.0,0.0,es,id.time()));
    env->cfilterenv_output(OUT_LOADER_AHDSR_DECAY,piw::makefloat_bounded_units_nb(BCTUNIT_SECONDS,60000.0,0.0,0.0,edc,id.time()));
    env->cfilterenv_output(OUT_LOADER_AHDSR_RELEASE,piw::makefloat_bounded_units_nb(BCTUNIT_SECONDS,60000.0,0.0,0.0,er,id.time()));
    env->cfilterenv_output(OUT_LOADER_ACTIVATION,piw::makebool_nb(true,id.time()+1));

    id_=id;
    ctl_->start_voice(id,voice);
    running_=true;
    return true;
}

bool loaderfunc_t::cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long)
{
    if(running_)
    {
        ctl_->start_voice(id_,piw::voiceref_t());
        running_=false;
    }

    return false;
}

sampler2::loader_t::impl_t::impl_t(player_t *s,const piw::cookie_t &c, piw::clockdomain_ctl_t *d): filter_(&ctl_,c,d)
{
    ctl_.attach_sampler(s,filter_.sink());
}

sampler2::loader_t::impl_t::~impl_t()
{
    ctl_.detach_sampler();
}

loaderctl_t::loaderctl_t()
{
}

loaderctl_t::~loaderctl_t()
{
}

void loaderctl_t::attach_sampler(sampler2::player_t *s, piw::clocksink_t *c)
{
    if(s->attach_loader(c))
    {
        link_=s;
    }
}

void loaderctl_t::detach_sampler()
{
    if(link_.isvalid())
    {
        pic::weak_t<sampler2::player_t>::guard_t g(link_);

        if(g.value())
        {
            g.value()->detach_loader();
        }
    }

    link_.clear();
}

sampler2::loader_t::loader_t(player_t *b,const piw::cookie_t &c, piw::clockdomain_ctl_t *d)
{
    impl_=new impl_t(b,c,d);
}

sampler2::loader_t::~loader_t()
{
    delete impl_;
}

void sampler2::loader_t::load(const piw::presetref_t &p)
{
    impl_->load(p);
}

piw::cookie_t sampler2::loader_t::cookie()
{
    return impl_->cookie();
}
