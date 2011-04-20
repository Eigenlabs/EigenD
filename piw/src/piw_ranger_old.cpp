
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

#include <piw/piw_ranger.h>
#include <piw/piw_ufilter.h>
#include <piw/piw_cfilter.h>

namespace
{
    struct ctl_filter_t: piw::ufilterctl_t, piw::ufilterfunc_t, piw::ufilter_t
    {
        ctl_filter_t(const piw::cookie_t &output): piw::ufilter_t(this,output), env_(0)
        {
        }

        virtual piw::ufilterfunc_t *ufilterctl_create(const piw::data_nb_t &) { return this; }
        virtual void ufilterctl_delete(piw::ufilterfunc_t *f) { }
        virtual unsigned ufilterctl_latency() { return 0; }
        virtual unsigned long long ufilterctl_thru() { return 0; }
        virtual unsigned long long ufilterctl_inputs() { return SIG4(1,2,3,4); }
        virtual unsigned long long ufilterctl_outputs() { return SIG1(1); }

        virtual void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
        {
            env_=0;
        }

        virtual void ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
        {
            unsigned long long t=id.time();
            piw::data_nb_t d;
            min_ = -1; max_ = 1; rst_ = 0; val_ = 0;
            env_ = e;

            if(e->ufilterenv_latest(1,d,t)) min_ = d.as_renorm_float(-10000000,10000000,0);
            if(e->ufilterenv_latest(2,d,t)) max_ = d.as_renorm_float(-10000000,10000000,0);
            if(e->ufilterenv_latest(3,d,t)) rst_ = d.as_renorm_float(-10000000,10000000,0);

            output(e,val_,t);
            e->ufilterenv_start(t);
        }

        virtual void ufilterfunc_data(piw::ufilterenv_t *e,unsigned s,const piw::data_nb_t &d)
        {
            switch(s)
            {
                case 1: min_ = d.as_renorm_float(-10000000,10000000,0); break;
                case 2: max_ = d.as_renorm_float(-10000000,10000000,0); break;
                case 3: rst_ = d.as_renorm_float(-10000000,10000000,0); break;
            }

            output(e,val_,d.time());
        }

        void setval(float d, unsigned long long t)
        {
            val_ = d;
            if(env_) output(env_,val_,t);
        }

        void output(piw::ufilterenv_t *e, float d, unsigned long long t)
        {
            float v, min, max, rst;

            if(min_<max_)
            {
                rst = rst_;
                min = min_;
                max = max_;

                if(rst<min) rst=min;
                if(rst>max) rst=max;

                v=piw::denormalise(max,min,rst,d);
            }
            else
            {
                rst = rst_;
                min = max_;
                max = min_;

                if(rst<min) rst=min;
                if(rst>max) rst=max;

                v=piw::denormalise(max,min,rst,-d);
            }

            piw::data_nb_t d2 = piw::makefloat_bounded_units_nb(BCTUNIT_GLOBAL,max,min,rst,v,t);
            //pic::logmsg() << "in=" << d << " min=" << min_ << " max=" << max_ << " rst=" << rst_ << " o=" << v << " d2=" << d2 << ':' << d2.units();
            e->ufilterenv_output(1,d2);
        }

        piw::ufilterenv_t *env_;
        float min_;
        float max_;
        float rst_;
        float val_;
    };

    struct data_filter_t: piw::cfilterctl_t, piw::cfilterfunc_t, piw::cfilter_t
    {
        data_filter_t(ctl_filter_t *ctl, piw::clockdomain_ctl_t *clock_domain): piw::cfilter_t(this,piw::cookie_t(0),clock_domain), ctl_(ctl), active_(false)
        {
        }

        virtual ~data_filter_t() {}

        virtual cfilterfunc_t *cfilterctl_create(const piw::data_nb_t &path) { return this; }
        virtual void cfilterctl_delete(cfilterfunc_t *f) { }
        virtual unsigned cfilterctl_latency() { return 0; }
        virtual unsigned long long cfilterctl_thru() { return 0; }
        virtual unsigned long long cfilterctl_inputs() { return SIG1(1); }
        virtual unsigned long long cfilterctl_outputs() { return 0; }

        virtual bool cfilterfunc_start(piw::cfilterenv_t *e, const piw::data_nb_t &id)
        {
            piw::data_nb_t d;
            unsigned long long t = id.time();
            active_=true;

            if(e->cfilterenv_latest(1,d,t)) { ctl_->setval(d.as_norm(),d.time()); }

            return true;
        }

        virtual bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
            if(active_)
            {
                piw::data_nb_t d;
                if(e->cfilterenv_latest(1,d,t)) { ctl_->setval(d.as_norm(),d.time()); }
            }
            return true;
        }

        virtual bool cfilterfunc_end(piw::cfilterenv_t *e, unsigned long long time)
        {
            ctl_->setval(0,time);
            active_=false;
            return false;
        }

        ctl_filter_t *ctl_;
        bool active_;
    };
};

struct piw::ranger_t::impl_t: pic::lckobject_t
{
    impl_t(piw::clockdomain_ctl_t *clock_domain, const piw::cookie_t &output): ctl_(output), data_(&ctl_,clock_domain)
    {
    }

    virtual ~impl_t()
    {
    }

    ctl_filter_t ctl_;
    data_filter_t data_;
};

piw::ranger_t::ranger_t(piw::clockdomain_ctl_t *clock_domain, const piw::cookie_t &output): impl_(new impl_t(clock_domain,output))
{
}

piw::ranger_t::~ranger_t()
{
    delete impl_;
}

piw::cookie_t piw::ranger_t::ctl_cookie()
{
    return impl_->ctl_.cookie();
}

piw::cookie_t piw::ranger_t::data_cookie()
{
    return impl_->data_.cookie();
}
