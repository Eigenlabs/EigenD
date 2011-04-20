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

#ifndef __PIW_CFILTER__
#define __PIW_CFILTER__
#include "piw_exports.h"
#include "piw_bundle.h"
#include <picross/pic_stl.h>

namespace piw
{
    class clockdomain_ctl_t;


    struct PIW_DECLSPEC_CLASS cfilterenv_t
    {
        virtual ~cfilterenv_t() {}
        virtual unsigned cfilterenv_latency() = 0;
        virtual clockdomain_ctl_t *cfilterenv_clock() = 0;
        virtual clocksink_t *cfilterenv_clocksink() = 0;

        // enqueue output data for current event
        virtual void cfilterenv_output(unsigned output_signal, const piw::data_nb_t &data) = 0;

        // get next input value (up to and including time t)
        // updates the internal iterator
        // signal is filled in with 0 to indicate end of event
        virtual bool cfilterenv_next(unsigned &signal, piw::data_nb_t &value, unsigned long long t) = 0;
        virtual bool cfilterenv_nextsig(unsigned signal, piw::data_nb_t &value, unsigned long long t) = 0;

        // get most recent input value for signal (up to and including time t)
        // updates the internal iterator
        virtual bool cfilterenv_latest(unsigned signal, piw::data_nb_t &value, unsigned long long t) = 0;
        // reset (but not consume) iterator up to and including time t (so next read will get it)
        virtual void cfilterenv_reset(unsigned signal, unsigned long long t) = 0;

        virtual piw::data_t cfilterenv_path() = 0;
        virtual void cfilterenv_dump(bool) = 0;
    };

    struct PIW_DECLSPEC_CLASS cfilterfunc_t: virtual public pic::lckobject_t
    {
        virtual ~cfilterfunc_t() {}
        virtual void cfilterfunc_changed(cfilterenv_t *, void *) {}
        virtual bool cfilterfunc_start(cfilterenv_t *e, const piw::data_nb_t &id) = 0;
        virtual bool cfilterfunc_process(cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs) = 0;
        virtual bool cfilterfunc_end(cfilterenv_t *e, unsigned long long time) = 0;

        unsigned sample_offset(unsigned bs,unsigned long long time, unsigned long long from, unsigned long long to)
        {
            unsigned long long et = std::max(time,from+1);
            et = std::min(et,to);
            unsigned long long offset = (bs*(et-from))/(to-from);
            return (unsigned)offset;
        }
    };

    struct PIW_DECLSPEC_CLASS cfilterctl_t
    {
        virtual cfilterfunc_t *cfilterctl_create(const piw::data_t &path) = 0;
        virtual void cfilterctl_delete(cfilterfunc_t *f) { delete f; }
        virtual ~cfilterctl_t() {}
        // additional latency contributed by this filter
        virtual unsigned cfilterctl_latency() { return 0; }
        virtual unsigned long long cfilterctl_thru() { return 0; }
        virtual unsigned long long cfilterctl_inputs() { return ~0ULL; }
        virtual unsigned long long cfilterctl_outputs() { return ~0ULL; }
    };

    class PIW_DECLSPEC_CLASS cfilter_t
    {
        public:
            class impl_t;

        public:
            cfilter_t(cfilterctl_t *ctl, const cookie_t &o, clockdomain_ctl_t *d, bool linger_restart=true);
            piw::clocksink_t *sink();
            cookie_t cookie();
            void changed(void *);
            virtual ~cfilter_t();

        private:
            impl_t *root_;
    };
};

#endif
