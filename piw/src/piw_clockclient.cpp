
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

#include <piw/piw_clockclient.h>
#include <piw/piw_clock.h>
#include <piw/piw_fastdata.h>
#include <picross/pic_ref.h>
#include <picross/pic_log.h>

#include <math.h>
#include <vector>

struct piw::clockinterp_t::impl_t: public pic::lckobject_t
{
    impl_t(unsigned n) : clocklast_(n), clockspeed_(n, 0), clockvalue_(n), clockmod_(n), clockvalid_(n, false)
    {
    }

    void recv_clock(unsigned n, const piw::data_nb_t &d)
    {
        //pic::logmsg() << "recv_clock " << n << ":" << d << " @ " << d.time();

        if(d.is_null() || d.as_arraylen()==0)
        {
            clockvalid_[n] = false;
            clocklast_[n].set_nb(piw::data_nb_t());
            return;
        }

        if(clocklast_[n].get().is_null())
        {
            clocklast_[n].set_nb(d);
            return;
        }

        unsigned long long lt = clocklast_[n].get().time();
        unsigned long long ct = d.time();

        if(ct<=lt)
        {
            pic::logmsg() << "data too old " << ct << " last=" << lt;
            return;
        }

        double lower = clocklast_[n].get().as_renorm_double(BCTUNIT_BEATS,0,100000,0);
        double upper = d.as_renorm_double(BCTUNIT_BEATS,0,100000,0);
        double mod = d.as_array_ubound();
        double d1 = upper-lower;
        double distance = __mod(d1, mod);
        unsigned long long ivl = (ct-lt);

        //pic::logmsg() << "clock " << n << " " << d << " " << lower << " " << upper;


        clockspeed_[n] = distance/((double)ivl);
        clocklast_[n].set_nb(d);
        clockvalid_[n] = true;
        clockmod_[n] = mod;
        clockvalue_[n] = upper;
        //pic::logmsg() << "recv last=" << lower << " cur=" << upper << " db=" << d1; 
        //pic::logmsg() << "speed " << clockspeed_[n] << " last " << clocklast_[n] << " mod " << clockmod_[n] << " value " << clockvalue_[n];

    }

    unsigned long long interpolate_time_backward(unsigned n, float c)
    {
        PIC_ASSERT(clockvalid_[n] && clockspeed_[n]>0.0);

        double lower = clockvalue_[n];
        unsigned long long last = clocklast_[n].get().time();
        //PIC_ASSERT(c<=lower);
        if(c>lower)
        {
            //pic::logmsg( ) << "beat ahead of current  "  << c << " " << lower << " " << last;
            return last;
        }

        double mod = clockmod_[n];
        double dist = fmod(lower-c, mod);

        return (unsigned long long)(last-(dist/clockspeed_[n]));
    }

    unsigned long long interpolate_time(unsigned n, float c, unsigned long long lb)
    {
        if(!clockvalid_[n] || clockspeed_[n]<=0.0)
        {
            return ~0ULL;
        }


        double lower = clockvalue_[n];
        double mod = clockmod_[n];
        unsigned long long last = clocklast_[n].get().time();

        if(c < lower)
        {
            double dist = fmod(lower-c, mod);
            unsigned long long t = (unsigned long long)(last-(dist/clockspeed_[n]));

            if(t>=lb)
            {
                return t;
            }
        }

        double dist = __mod(c-lower, mod);
        unsigned long long t = (unsigned long long)(last+(dist/clockspeed_[n]));

        if(t>=lb)
        {
            return t;
        }

        return ~0ULL;
    }

    unsigned long long interpolate_time_noncyclic(unsigned n, float c, unsigned long long lb)
    {
        if(!clockvalid_[n] || clockspeed_[n]<=0.0)
        {
            return lb;
        }

        double lower = clockvalue_[n];
        double mod = clockmod_[n];
        unsigned long long last = clocklast_[n].get().time();

        if(c < lower)
        {
            return lb;
        }

        double dist = __mod(c-lower, mod);
        unsigned long long t = (unsigned long long)(last+(dist/clockspeed_[n]));

        return std::max(t,lb);
    }

    float interpolate_clock(unsigned n, unsigned long long t)
    {
        if(!clockvalid_[n])
        {
            return 0;
        }

        if(t > clocklast_[n].get().time())
        {
            unsigned long long ivl = t-clocklast_[n].get().time();
            double b = clockvalue_[n]+clockspeed_[n]*((double)ivl);
            double v = __mod(b, clockmod_[n]);
            return v;
        }
        else
        {
            unsigned long long ivl = clocklast_[n].get().time()-t;
            double b = clockvalue_[n]-clockspeed_[n]*((double)ivl);
            double v = __mod(b, clockmod_[n]);
            return v;
        }
    }

    bool clockvalid(unsigned n)
    {
        return clockvalid_[n];
    }

    float get_mod(unsigned n)
    {
        if(!clockvalid_[n])
        {
            return 100000;
        }

        return clockmod_[n];
    }

    double get_speed(unsigned n)
    {
        if(!clockvalid_[n])
        {
            return 0;
        }

        return clockspeed_[n];
    }


    double __mod(double x, double mod)
    {
        double m = fmod(x,mod);

        if(m<0.0)
            m+=mod;

        return m;
    }

    pic::lckvector_t<piw::dataholder_nb_t>::lcktype clocklast_;
    pic::lckvector_t<double>::lcktype clockspeed_;
    pic::lckvector_t<double>::lcktype clockvalue_;
    pic::lckvector_t<double>::lcktype clockmod_;
    pic::lckvector_t<bool>::lcktype clockvalid_;
};


piw::clockinterp_t::clockinterp_t(unsigned n) : impl_(new impl_t(n)) {}
piw::clockinterp_t::~clockinterp_t() { delete impl_; }

void piw::clockinterp_t::recv_clock(unsigned n, const piw::data_nb_t &d) { impl_->recv_clock(n,d); }

unsigned long long piw::clockinterp_t::interpolate_time(unsigned n, float c, unsigned long long lb) { return impl_->interpolate_time(n,c,lb); }
float piw::clockinterp_t::interpolate_clock(unsigned n, unsigned long long t) { return impl_->interpolate_clock(n,t); }
float piw::clockinterp_t::get_mod(unsigned n) { return impl_->get_mod(n); }
double piw::clockinterp_t::get_speed(unsigned n) { return impl_->get_speed(n); }
bool piw::clockinterp_t::clockvalid(unsigned n) { return impl_->clockvalid(n); }
double piw::clockinterp_t::get_clock(unsigned n) { return impl_->clockvalue_[n]; }
unsigned long long piw::clockinterp_t::get_time(unsigned n) { return impl_->clocklast_[n].get().time(); }
unsigned long long piw::clockinterp_t::interpolate_time_backward(unsigned n,float c) { return impl_->interpolate_time_backward(n,c); }
unsigned long long piw::clockinterp_t::interpolate_time_noncyclic(unsigned n,float c,unsigned long long lb) { return impl_->interpolate_time_noncyclic(n,c,lb); }
