
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

        bool started_;

        velctl_t *ctl_;
        piw::velocitydetector_t detector_;
    };

    struct velctl_t : piw::ufilterctl_t, virtual public pic::lckobject_t
    {
        velctl_t(unsigned p,unsigned v) : pressure_(p), velocity_(v) {}
        piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &) { return new velfunc_t(this); }
        unsigned long long ufilterctl_inputs();
        unsigned long long ufilterctl_thru();
        unsigned long long ufilterctl_outputs();

        unsigned pressure_, velocity_;
        piw::velocityconfig_t config_;
    };
}

namespace piw
{
    struct velocityconfig_t::impl_t
    {
        impl_t() : samples_(4), curve_(NON_LINEARITY), scale_(4.0) {}

        unsigned samples_;
        float curve_;
        float scale_;
    };

    struct velocitydetector_t::impl_t
    {
        impl_t(const piw::velocityconfig_t &config) : config_(config), vcount_(0), started_(false) {}

        void init()
        {
            started_=false;

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

        bool detect(const piw::data_nb_t &d, double *velocity)
        {
            unsigned s = config_.impl_->samples_;
            float v = d.as_norm();

            if(vcount_<s)
            {
                sumx_+=x_;
                sumy_+=v;
                sumxy_+=(x_*v);
                sumxsq_+=(x_*x_);
                vcount_++;
                x_++;
                return false;
            }

            started_ = true;

            double vel_raw = config_.impl_->scale_*(x_*sumxy_-(sumx_*sumy_))/(x_*sumxsq_-(sumx_*sumx_));
            double vel = 1-pow((double)(1-vel_raw),(double)(config_.impl_->curve_));
            *velocity = vel;
            return true;
        }

        const piw::velocityconfig_t &config_;
        unsigned vcount_;
        float sumx_,sumy_,sumxy_,sumxsq_,x_;
        bool started_;
    };

    struct velocitydetect_t::impl_t
    {
        impl_t(const piw::cookie_t &c, unsigned p, unsigned v) : ctl_(p,v), filter_(&ctl_,c) {}
        piw::cookie_t cookie() { return filter_.cookie(); }
        velctl_t ctl_;
        piw::ufilter_t filter_;
    };
}


/*
 * velfunc_t
 */

velfunc_t::velfunc_t(velctl_t *c) : ctl_(c), detector_(c->config_)
{
}

void velfunc_t::ufilterfunc_end(piw::ufilterenv_t *env,unsigned long long t)
{
    if(!detector_.is_started())
    {
        pic::logmsg() << "velocity aborted";
        env->ufilterenv_start(t-1);
    }
}

void velfunc_t::ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
{
    detector_.init();
}

void velfunc_t::ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t &d)
{
    if(!detector_.is_started() && sig==ctl_->pressure_)
    {
        double velocity;
        if(detector_.detect(d, &velocity))
        {
            //pic::logmsg() << "velocity: " << vel_raw << "->" << vel;

            env->ufilterenv_output(ctl_->velocity_,piw::makefloat_bounded_nb(1,0,0,std::min(1.0,velocity),d.time()));
            env->ufilterenv_start(d.time());
        }
    }
}


/*
 * velctl_t
 */

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


/*
 * velocityconfig_t
 */

piw::velocityconfig_t::velocityconfig_t() : impl_(new impl_t()) {}
piw::velocityconfig_t::~velocityconfig_t() { delete impl_; }

static int __set_samples(void *a, void *b)
{
    piw::velocityconfig_t::impl_t *i = (piw::velocityconfig_t::impl_t *)a;
    unsigned n = *(unsigned *)b;
    i->samples_ = n;
    return 0;
}

static int __set_curve(void *a, void *b)
{
    piw::velocityconfig_t::impl_t *i = (piw::velocityconfig_t::impl_t *)a;
    float n = *(float *)b;
    i->curve_ = n;
    return 0;
}

static int __set_scale(void *a, void *b)
{
    piw::velocityconfig_t::impl_t *i = (piw::velocityconfig_t::impl_t *)a;
    float n = *(float *)b;
    i->scale_ = n;
    return 0;
}

void piw::velocityconfig_t::set_samples(unsigned s) { piw::tsd_fastcall(__set_samples,impl_,&s); }
void piw::velocityconfig_t::set_curve(float c) { piw::tsd_fastcall(__set_curve,impl_,&c); }
void piw::velocityconfig_t::set_scale(float s) { piw::tsd_fastcall(__set_scale,impl_,&s); }


/*
 * velocitydetector_t
 */

piw::velocitydetector_t::velocitydetector_t(const piw::velocityconfig_t &config) : impl_(new impl_t(config)) {}
piw::velocitydetector_t::~velocitydetector_t() { delete impl_; }
void piw::velocitydetector_t::init() { impl_->init(); }
bool piw::velocitydetector_t::detect(const piw::data_nb_t &d, double *velocity) { return impl_->detect(d, velocity); }
bool piw::velocitydetector_t::is_started() { return impl_->started_; }


/*
 * velocitydetect_t
 */

piw::velocitydetect_t::velocitydetect_t(const piw::cookie_t &c,unsigned p,unsigned v) : impl_(new impl_t(c,p,v)) {}
piw::velocitydetect_t::~velocitydetect_t() { delete impl_; }
piw::cookie_t piw::velocitydetect_t::cookie() { return impl_->cookie(); }

void piw::velocitydetect_t::set_samples(unsigned n) { impl_->ctl_.config_.set_samples(n); }
void piw::velocitydetect_t::set_curve(float n) { impl_->ctl_.config_.set_curve(n); }
void piw::velocitydetect_t::set_scale(float n) { impl_->ctl_.config_.set_scale(n); }

