
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

#include <piw/piw_velocitydetect.h>
#include <piw/piw_ufilter.h>
#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>

#define NON_LINEARITY 4.0

namespace
{
    struct velctl_t;

    struct velfunc_t : piw::ufilterfunc_t
    {
        velfunc_t(velctl_t *c);
        void ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id);
        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t &ed);
        void ufilterfunc_end(piw::ufilterenv_t *env,unsigned long long t);
        void detector_init();
        void detector(piw::ufilterenv_t *env,const piw::data_nb_t &d);

        unsigned vcount_;
        float sumx_,sumy_,sumxy_,sumxsq_,x_;
        bool started_;

        velctl_t *ctl_;
    };

    struct velctl_t : piw::ufilterctl_t, virtual public pic::lckobject_t
    {
        velctl_t(unsigned p,unsigned v) : pressure_(p), velocity_(v), samples_(4),curve_(NON_LINEARITY), scale_(4.0) {}
        piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &) { return new velfunc_t(this); }
        unsigned long long ufilterctl_inputs();
        unsigned long long ufilterctl_thru();
        unsigned long long ufilterctl_outputs();

        unsigned pressure_, velocity_;
        unsigned samples_;
        float curve_;
        float scale_;
    };
}

velfunc_t::velfunc_t(velctl_t *c) : vcount_(0), ctl_(c)
{
}

void velfunc_t::detector_init()
{
    vcount_=0;
    sumx_=sumy_=sumxy_=sumxsq_=0.0;
    x_=0.0;

    sumx_+=x_;
    sumxsq_+=x_*x_;
    x_++;

    sumx_+=x_;
    sumxsq_+=x_*x_;
    x_++;
}

void velfunc_t::detector(piw::ufilterenv_t *env,const piw::data_nb_t &d)
{
    unsigned s = ctl_->samples_;
    float v = d.as_norm();

    if(vcount_<s)
    {
        sumx_+=x_;
        sumy_+=v;
        sumxy_+=(x_*v);
        sumxsq_+=(x_*x_);
        vcount_++;
        x_++;
        return;
    }

    double vel_raw = ctl_->scale_*(x_*sumxy_-(sumx_*sumy_))/(x_*sumxsq_-(sumx_*sumx_));
    double vel = 1-pow((double)(1-vel_raw),(double)(ctl_->curve_));

    //pic::logmsg() << "velocity: " << vel_raw << "->" << vel;

    env->ufilterenv_output(ctl_->velocity_,piw::makefloat_bounded_nb(1,0,0,std::min(1.0,vel),d.time()));
    env->ufilterenv_start(d.time());
    started_=true;
}

void velfunc_t::ufilterfunc_end(piw::ufilterenv_t *env,unsigned long long t)
{
    if(!started_)
    {
        pic::logmsg() << "velocity aborted after " << vcount_;
        env->ufilterenv_start(t-1);
    }
}

void velfunc_t::ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
{
    started_=false;
    detector_init();
}

void velfunc_t::ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t &d)
{
    if(!started_ && sig==ctl_->pressure_)
    {
        detector(env,d);
    }
}

unsigned long long velctl_t::ufilterctl_inputs()
{
    return SIG1(pressure_);
}

unsigned long long velctl_t::ufilterctl_outputs()
{
    return SIG1(velocity_);
}

unsigned long long velctl_t::ufilterctl_thru()
{
    return ~ufilterctl_outputs();
}

namespace piw
{
    struct velocitydetect_t::impl_t
    {
        impl_t(const piw::cookie_t &c,unsigned p,unsigned v) : ctl_(p,v), filter_(&ctl_,c) {}
        piw::cookie_t cookie() { return filter_.cookie(); }
        velctl_t ctl_;
        piw::ufilter_t filter_;
    };
}

piw::velocitydetect_t::velocitydetect_t(const piw::cookie_t &c,unsigned p,unsigned v) : impl_(new impl_t(c,p,v)) {}
piw::velocitydetect_t::~velocitydetect_t() { delete impl_; }
piw::cookie_t piw::velocitydetect_t::cookie() { return impl_->cookie(); }

static int __set_samples(void *a, void *b)
{
    piw::velocitydetect_t::impl_t *i = (piw::velocitydetect_t::impl_t *)a;
    unsigned n = *(unsigned *)b;
    i->ctl_.samples_ = n;
    return 0;
}

static int __set_curve(void *a, void *b)
{
    piw::velocitydetect_t::impl_t *i = (piw::velocitydetect_t::impl_t *)a;
    float n = *(float *)b;
    i->ctl_.curve_ = n;
    return 0;
}

static int __set_scale(void *a, void *b)
{
    piw::velocitydetect_t::impl_t *i = (piw::velocitydetect_t::impl_t *)a;
    float n = *(float *)b;
    i->ctl_.scale_ = n;
    return 0;
}

void piw::velocitydetect_t::set_samples(unsigned n) { piw::tsd_fastcall(__set_samples,impl_,&n); }
void piw::velocitydetect_t::set_curve(float n) { piw::tsd_fastcall(__set_curve,impl_,&n); }
void piw::velocitydetect_t::set_scale(float n) { piw::tsd_fastcall(__set_scale,impl_,&n); }

