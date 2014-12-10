/*
 Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __CDTR_AIFF_FORMAT_H__
#define __CDTR_AIFF_FORMAT_H__

#include <lib_juce/juce.h>
#include <piw/piw_data.h>

#include "recording_format.h"
#include "recorded_data.h"

namespace cdtr
{
    class aiff_format_t: public cdtr::recording_format_t
    {
        public:
            aiff_format_t();
            ~aiff_format_t();

            void set_zerocross_start(bool zerocross);
    
            void prepare(const juce::File &recording_dir, const juce::File &clips_dir, const juce::String &uid, unsigned long samplerate);
            bool add_audio(cdtr::recorded_data_t &data);
            bool add_labels(cdtr::recorded_data_t &data);
            void teardown();
    
        private:
            static inline int sign(float value);
            void add_xml_meta_entry(const juce::String &key, const juce::String &value);
            void add_xml_clock(const cdtr::recorded_data_t &data);
            bool add_audio_raw(const float *samplesToWrite, int numSamples);
            bool add_audio_zero(int numSamples);

            juce::File clips_dir_;
            juce::AiffAudioFormat format_;
            juce::File audio_file_;
            juce::FileOutputStream *audio_output_;
            juce::AudioFormatWriter *audio_writer_;
            juce::File xml_file_;
            juce::XmlElement *xml_document_;
            juce::XmlElement *xml_meta_;
            bool zerocross_;
            bool at_start_;
            int start_sign_;
            unsigned long long sample_rate_;
            unsigned long long sample_offset_;
            unsigned long long sample_length_;
            juce::SortedSet<std::pair<juce::String,juce::String> > labels_;
    };
};

#endif
