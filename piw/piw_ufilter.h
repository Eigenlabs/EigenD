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

#ifndef __PIW_UFILTER__
#define __PIW_UFILTER__
#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    struct PIW_DECLSPEC_CLASS ufilterenv_t
    {
        virtual ~ufilterenv_t() {}
        virtual void ufilterenv_output(unsigned output_index, const piw::data_nb_t &data) = 0;
        virtual void ufilterenv_start(unsigned long long) = 0;
        virtual bool ufilterenv_latest(unsigned, piw::data_nb_t &, unsigned long long) = 0;
        virtual void ufilterenv_dump(bool) = 0;
    };

    struct PIW_DECLSPEC_CLASS ufilterfunc_t: virtual public pic::lckobject_t
    {
        virtual ~ufilterfunc_t() {}
        virtual void ufilterfunc_changed(ufilterenv_t *, void *) {}
        virtual void ufilterfunc_start(ufilterenv_t *,const piw::data_nb_t &) = 0;
        virtual void ufilterfunc_data(ufilterenv_t *,unsigned,const piw::data_nb_t &) = 0;
        virtual void ufilterfunc_end(ufilterenv_t *, unsigned long long) {}

    };

    struct PIW_DECLSPEC_CLASS ufilterctl_t
    {
        virtual ufilterfunc_t *ufilterctl_create(const piw::data_t &path) = 0;
        virtual void ufilterctl_delete(ufilterfunc_t *f) { delete f; }
        virtual ~ufilterctl_t() {}
        // additional latency contributed by this ufilter
        virtual unsigned ufilterctl_latency() { return 0; }
        virtual void ufilterctl_clock(bct_clocksink_t *) {}
        virtual void ufilterctl_latency_in(unsigned) {}
        virtual unsigned long long ufilterctl_thru() = 0;
        virtual unsigned long long ufilterctl_inputs() = 0;
        virtual unsigned long long ufilterctl_outputs() = 0;
    };

    class PIW_DECLSPEC_CLASS ufilter_t
    {
        public:
            class impl_t;

        public:
            ufilter_t(ufilterctl_t *ctl, const cookie_t &o);
            cookie_t cookie();
            void changed(void *);
            virtual ~ufilter_t();

        private:
            impl_t *root_;
    };
};

#endif
