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

#ifndef __MOOG_MOOG__
#define __MOOG_MOOG__

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <picross/pic_functor.h>
#include <pisynth_exports.h>

/**
 *  Components for modelling analogue synths.
 */
namespace piw
{
    class cookie_t;
    class clockdomain_ctl_t;
}

namespace synth
{
    class PISYNTH_DECLSPEC_CLASS sine_t
    {
        public:
            sine_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~sine_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS sawtooth_t
    {
        public:
            sawtooth_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~sawtooth_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS rect_t
    {
        public:
            rect_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~rect_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS triangle_t
    {
        public:
            triangle_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~triangle_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS synthfilter_t
    {
        public:
            synthfilter_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~synthfilter_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS synthfilter2_t
    {
        public:
            synthfilter2_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~synthfilter2_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS adsr_t
    {
        public:
            adsr_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~adsr_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS adsr2_t
    {
        public:
            adsr2_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~adsr2_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PISYNTH_DECLSPEC_CLASS polysummer_t
    {
        public:
            polysummer_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~polysummer_t();
            piw::cookie_t cookie();

            class impl_t;
        private:
            impl_t *impl_;
    };

    PISYNTH_DECLSPEC_FUNC(piw::dd2d_nb_t) sharpener(float sharpness);
    PISYNTH_DECLSPEC_FUNC(piw::d2d_nb_t) compressor(float compression);
};

#endif
