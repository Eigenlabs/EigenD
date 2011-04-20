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

#ifndef __PIW_MIXER__
#define __PIW_MIXER__

#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_state.h>
#include <picross/pic_functor.h>

namespace piw
{
    class cookie_t;
    class clockdomain_ctl_t;

    class PIW_DECLSPEC_CLASS stereomixer_t
    {
        public:
            class impl_t;
        public:
            stereomixer_t(const pic::f2f_t &vol, const pic::f2f_t &pan, piw::clockdomain_ctl_t *d, const piw::cookie_t &c);
            ~stereomixer_t();
            piw::cookie_t cookie();
            int gc_traverse(void *,void *) const;
            int gc_clear();
        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS monomixer_t
    {
        public:
            class impl_t;
        public:
            monomixer_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c);
            ~monomixer_t();
            piw::cookie_t cookie();
        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS stereosummer_t
    {
        public:
            class impl_t;
        public:
            stereosummer_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &c,unsigned n);
            ~stereosummer_t();
            piw::cookie_t cookie();
            void set_channels(unsigned n);
        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS consolemixer_t
    {
    public:
        class impl_t;
    public:
        consolemixer_t(const pic::f2f_t &vol, const pic::f2f_t &pan, piw::clockdomain_ctl_t *d, const piw::cookie_t &c);
        ~consolemixer_t();
        piw::cookie_t create_channel(unsigned num);
        void remove_channel(unsigned num);
        piw::cookie_t create_fx_channel(unsigned num, const piw::cookie_t &output_cookie);
        void remove_fx_channel(unsigned num);
        void set_fx_send_enable(bool enable, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan);
        void set_fx_send_prefader(bool prefader, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan);
        void set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan);
        piw::cookie_t master_controls_cookie();
    private:
        impl_t *impl_;
    };

};

#endif
