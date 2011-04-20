
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

#ifndef __LOOP_FILE__
#define __LOOP_FILE__

#include <picross/pic_error.h>
#include <picross/pic_ref.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_endian.h>
#include <vector>

#include <plg_loop/src/loop_exports.h>

namespace loop
{
    class PILOOP_DECLSPEC_CLASS loopraw_t : public pic::counted_t, public pic::nocopy_t
    {
        public:
            loopraw_t();
            ~loopraw_t() { delete[] transients_; delete[] samples_; }

            unsigned long samples() const { return samplecount_; }
            unsigned long sample_rate() const { return srate_; }
            unsigned long beats() const { return beats_; }
            unsigned short note() const { return note_; }
            unsigned short scale() const { return scale_; }
            unsigned short timesig_numerator() const { return numer_; }
            unsigned short timesig_denominator() const { return denom_; }
            unsigned short looping() const { return looping_; }
            const float *frame(unsigned long n) const { return samples_ + n*numchannels_; }
            unsigned short num_channels() const { return numchannels_; }
            unsigned short num_transients() const { return numtransients_; }
            unsigned long transient(unsigned n) const { return transients_[n]; }
            unsigned ntags() const { return tags_.size(); }
            std::string tag(unsigned n) { return tags_[n]; }

        private:
            friend class aiff_reader_t;

            void decode_samples(const char *raw);
            void read(const char *raw, float (*reader)(const char **));

            uint16_t width_;
            uint32_t samplecount_;
            uint32_t srate_;
            uint32_t beats_;
            uint16_t note_;
            uint16_t scale_;
            uint16_t numer_;
            uint16_t denom_;
            uint16_t looping_;
            float *samples_;
            uint16_t numchannels_;
            uint32_t numtransients_;
            uint32_t *transients_;
            std::vector<std::string> tags_;
    };

    typedef pic::ref_t<loopraw_t> loopref_t;
    PILOOP_DECLSPEC_FUNC(loopref_t) read_aiff(const char *name,bool justmeta);
}

#endif
