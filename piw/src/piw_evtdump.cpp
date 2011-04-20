
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

#include <piw/piw_evtdump.h>
#include <piw/piw_cfilter.h>

namespace
{
    struct dumper_t: piw::cfilterfunc_t
    {
        dumper_t(const std::string &n): prime_(false), running_(false), name_(n)
        {
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            prime_=true;
            running_=true;
            return true;
        }

        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
        {
            running_=false;
            return false;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
            if(prime_)
            {
                prime_=false;
                pic::logmsg() << name_ << " started in frame " << t;
            }

            return running_;
        }

        bool prime_;
        bool running_;
        std::string name_;
    };
}

struct piw::evtdump_t::impl_t : piw::cfilterctl_t, piw::cfilter_t
{
    impl_t(const std::string &n,const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d), name_(n) {}
    piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &) { return new dumper_t(name_); }
    unsigned long long cfilterctl_inputs() { return 0ULL; }
    unsigned long long cfilterctl_outputs() { return 0ULL; }
    unsigned long long cfilterctl_thru() { return ~0ULL; }

    std::string name_;
};

piw::evtdump_t::evtdump_t(const std::string &n,const piw::cookie_t &o, piw::clockdomain_ctl_t *d): impl_(new impl_t(n,o,d)) {}
piw::cookie_t piw::evtdump_t::cookie() { return impl_->cookie(); }
piw::evtdump_t::~evtdump_t() { delete impl_; }
