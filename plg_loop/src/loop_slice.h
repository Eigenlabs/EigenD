
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

#ifndef __LOOP_SLICE__
#define __LOOP_SLICE__

#include <piw/piw_bundle.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_phase.h>
#include <picross/pic_ilist.h>
#include "loop_file.h"
#include <plg_loop/src/loop_exports.h>

#define FADELENGTH_GRAIN 128UL
#define FADETIME_GRAIN  (128.f/48000.f)
#define FADELENGTH_SLICE 1024UL
#define FADETIME_SLICE  (1024.f/48000.f)

namespace loop
{
    class PILOOP_DECLSPEC_CLASS slice_t : public pic::nocopy_t, public pic::element_t<>, virtual public pic::lckobject_t
    {
        public:
            slice_t(const loopref_t &l, unsigned long start, unsigned long end);
            ~slice_t();

            float beat_start() const { return beat_start_; }
            float beat_end() const { return beat_end_; }
            float beat_len() const { return beat_len_; }

            void reset(float chop);

            void render(float **output, unsigned o, unsigned n, float sr, bool initial);
            unsigned fadeout(float **output, unsigned o, unsigned n, float sr);
            unsigned fadein(float **output, unsigned o, unsigned n, float sr);

        private:
            void add_xfade();
            float interpolate(unsigned c);
            float interpolate_fade(const float *fdata, piw::phase_t &fphase);
            unsigned fade(float **output, unsigned o, unsigned n, float sr, const float *fdata, piw::phase_t &fphase);
            bool mono();

            float *data_;
            unsigned gsize_, channels_, frames_;
            float samplerate_;
            piw::phase_t phase_;
            piw::phase_t fadeinphase_;
            piw::phase_t fadeoutphase_;

            float beat_start_;
            float beat_end_;
            float beat_len_;
            float beat_mod_;

            float gain_;
            float decay_;
            float decay0_;
    };

    typedef pic::ilist_t<slice_t> slicelist_t;
}

#endif
