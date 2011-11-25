
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

#ifndef __LOOP_CLICKER__
#define __LOOP_CLICKER__

#include <piw/piw_clock.h>
#include <piw/piw_bundle.h>
#include <piloop_exports.h>

namespace loop
{
    struct PILOOP_DECLSPEC_CLASS samplearray2_t : pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        samplearray2_t(): size(0)
        {
        }

        samplearray2_t(unsigned s) : size(s)
        {
            data = (float *)malloc(sizeof(float)*size);
            PIC_ASSERT(data);
        }

        ~samplearray2_t()
        {
            free(data);
        }

        unsigned long size;
        float *data;
        float rate;
    };

    typedef pic::ref_t<samplearray2_t> samplearray2ref_t;

    PILOOP_DECLSPEC_FUNC(samplearray2ref_t) canonicalise_samples(const std::string &,float);

    class PILOOP_DECLSPEC_CLASS clicker_t
    {
        public:
            class impl_t;
        public:
            clicker_t(const piw::cookie_t &, piw::clockdomain_ctl_t *d, const loop::samplearray2ref_t &accent, const loop::samplearray2ref_t &beat);
            ~clicker_t();
            void play(bool);
            piw::cookie_t cookie();
        private:
            impl_t *impl_;
    };

    class PILOOP_DECLSPEC_CLASS xplayer_t
    {
        public:
            class impl_t;
        public:
            xplayer_t(const piw::cookie_t &, piw::clockdomain_ctl_t *d, const loop::samplearray2ref_t &audio);
            ~xplayer_t();
            void play(bool);
            void set_gain(float);
        private:
            impl_t *impl_;
    };
}

#endif
