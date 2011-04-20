
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

#ifndef __PIW_SAMPLER__
#define __PIW_SAMPLER__
#include "piw_exports.h"
#include <picross/pic_ref.h>
#include <piw/piw_sample.h>

namespace piw
{
    struct PIW_DECLSPEC_CLASS sampler_voice_t : pic::atomic_counted_t
    {
        virtual bool write(float *out0, float *out1, unsigned from, unsigned to)=0;
        virtual bool fade(float *out0, float *out1, unsigned from, unsigned to)=0;
        virtual void recalc_freq(float sr, float freq)=0;
        virtual void disable_fadein()=0;
    };

    typedef pic::ref_t<sampler_voice_t> sampler_voiceref_t;

    PIW_DECLSPEC_FUNC(sampler_voiceref_t) create_player(const piw::voiceref_t &zone);

    class PIW_DECLSPEC_CLASS sampler_t : public pic::atomic_counted_t
    {
        public:
            sampler_t(const piw::presetref_t &preset);
            ~sampler_t();
            sampler_voiceref_t create_voice(float velocity, float freq) const;

            class impl_t;
        private:
            impl_t *impl_;
    };

    typedef pic::ref_t<sampler_t> samplerref_t;
}

#endif
