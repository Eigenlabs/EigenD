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

#include <picross/pic_error.h>
#include <picross/pic_resources.h>

#include "aiff_format.h"

cdtr::aiff_format_t::aiff_format_t() : audio_output_(0), audio_writer_(0), xml_document_(0), xml_meta_(0), zerocross_(false), at_start_(true), sample_rate_(0), sample_offset_(0), sample_length_(0)
{
}

cdtr::aiff_format_t::~aiff_format_t()
{
    teardown();
}

void cdtr::aiff_format_t::set_zerocross_start(bool zerocross)
{
    zerocross_ = zerocross;
}

void cdtr::aiff_format_t::prepare(const juce::File &recording_dir, const juce::File &clips_dir, const juce::String &uid, unsigned long samplerate)
{
    PIC_ASSERT(audio_output_ == 0);
    PIC_ASSERT(audio_writer_ == 0);
    PIC_ASSERT(xml_document_ == 0);
    PIC_ASSERT(xml_meta_ == 0);
    PIC_ASSERT(labels_.size() == 0);
    PIC_ASSERT(samplerate != 0);

    at_start_ = true;
    start_sign_ = 0;
    sample_rate_ = samplerate;
    sample_offset_ = 0;
    sample_length_ = 0;

    clips_dir_ = clips_dir;

    juce::String filename("audio_");
    filename += uid;

    audio_file_ = recording_dir.getChildFile(filename+".aiff");
    juce::String audio_fullpath = audio_file_.getFullPathName();
    audio_output_ = new juce::FileOutputStream(audio_file_);
    if(audio_output_->failedToOpen())
    {
        delete audio_output_;
        audio_output_ = 0;

        PIC_THROW((juce::String("Unable to open output stream for writing aiff format: ") + audio_fullpath).toUTF8());
    }

    juce::String now(juce::Time::getCurrentTime().toMilliseconds());
    juce::StringPairArray meta;
    meta.set("NumCueNotes", "2");
    meta.set("CueNote0TimeStamp", now);
    meta.set("CueNote0Text", juce::String(pic::release().c_str()));
    meta.set("CueNote1TimeStamp", now);
    meta.set("CueNote1Text", filename);
    audio_writer_ = format_.createWriterFor(audio_output_, samplerate, 1, 24, meta, 0);
    if(audio_writer_ == 0)
    {
        PIC_THROW((juce::String("Unable to create writer for aiff format: ") + audio_fullpath).toUTF8());
    }

    xml_file_ = juce::File(recording_dir.getChildFile(filename+".xml"));
    if(!xml_file_.hasWriteAccess())
    {
        juce::String xml_fullpath = xml_file_.getFullPathName();
        xml_file_ = juce::File();

        PIC_THROW((juce::String("Unable to open file for writing XML information: ") + xml_fullpath).toUTF8());
    }

    xml_document_ = new juce::XmlElement("clip");
    xml_document_->setAttribute("uuid", filename);
    xml_document_->setAttribute("type", "audio");

    xml_meta_ = xml_document_->createNewChildElement("meta");
    add_xml_meta_entry("samplerate", juce::String((juce::uint64)samplerate));
    add_xml_meta_entry("bitdepth", juce::String(24));
    add_xml_meta_entry("channels", juce::String(1));
}

int cdtr::aiff_format_t::sign(float value)
{
    return (value > 0) - (value < 0);
}

bool cdtr::aiff_format_t::add_audio(cdtr::recorded_data_t &data)
{
    const piw::data_t &d = data.get_data();
    const float *samplesToWrite = (const float *)d.as_array();
    int numSamples =  d.as_arraylen();

    PIC_ASSERT(audio_writer_ != 0);
    if(at_start_)
    {
        if(zerocross_)
        {
            if(0 == start_sign_)
            {
                start_sign_ = sign(samplesToWrite[0]);
            }

            for(int i = 0; i < numSamples; ++i)
            {
                int sgn = sign(samplesToWrite[i]);
                if(0 == sgn || start_sign_ != sgn)
                {
                    at_start_ = false;
                    sample_offset_ += i;
                    if(add_audio_zero(sample_offset_))
                    {
                        add_xml_meta_entry("sampleoffset", juce::String((juce::uint64)sample_offset_));
                        add_xml_clock(data);
                        sample_length_ += (numSamples-i);
                        return add_audio_raw(samplesToWrite+i, numSamples-i);
                    }
                    else
                    {
                        return false;
                    }
                }
            }
             sample_offset_ += numSamples;
        }
        else
        {
            at_start_ = false;
            add_xml_meta_entry("sampleoffset", juce::String(0));
            add_xml_clock(data);
            return add_audio_raw(samplesToWrite, numSamples);
        }

        return true;
    }
    else
    {
        at_start_ = false;
        sample_length_ += numSamples;
        return add_audio_raw(samplesToWrite, numSamples);
    }
}

bool cdtr::aiff_format_t::add_labels(cdtr::recorded_data_t &data)
{
    const piw::data_t &d = data.get_data();
    if(!d.is_tuple())
    {
        return false;
    }

    for(unsigned i = 0; i < d.as_tuplelen(); ++i)
    {
        const piw::data_t &e = d.as_tuple_value(i);
        if(e.is_tuple() && e.as_tuplelen() == 2 && e.as_tuple_value(0).is_string() && e.as_tuple_value(1).is_string())
        {
            labels_.add(std::make_pair(juce::String(e.as_tuple_value(0).as_string()),juce::String(e.as_tuple_value(1).as_string())));
        }
    }

    return true;
}

void cdtr::aiff_format_t::add_xml_meta_entry(const juce::String &key, const juce::String &value)
{
    XmlElement *xml_entry = xml_meta_->createNewChildElement("entry");
    xml_entry->setAttribute("key", key);
    xml_entry->addTextElement(value);
}

void cdtr::aiff_format_t::add_xml_clock(const cdtr::recorded_data_t &data)
{
    XmlElement *xml_clock = xml_document_->createNewChildElement("clock");
    xml_clock->setAttribute("songbeat", data.get_songbeat());
    xml_clock->setAttribute("barbeat", data.get_barbeat());
}

bool cdtr::aiff_format_t::add_audio_raw(const float *samplesToWrite, int numSamples)
{
    HeapBlock<int> tempBuffer;
    tempBuffer.malloc(numSamples);

    typedef AudioData::Pointer<AudioData::Int32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> DestSampleType;
    typedef AudioData::Pointer<AudioData::Float32, AudioData::LittleEndian, AudioData::NonInterleaved, AudioData::Const> SourceSampleType;

    DestSampleType destData(tempBuffer);
    SourceSampleType sourceData(samplesToWrite);
    destData.convertSamples(sourceData, numSamples);

    const int* channels[] = {(const int*)tempBuffer.getData()};
    return audio_writer_->write(channels, numSamples);
}

bool cdtr::aiff_format_t::add_audio_zero(int numSamples)
{
    if(numSamples <= 0)
    {
        return true;
    }
    
    float *samples = new float[numSamples];
    memset(samples, 0, numSamples*sizeof(float));
    bool result = add_audio_raw(samples, numSamples);
    delete [] samples;
    return result;
}

void cdtr::aiff_format_t::teardown()
{
    if(audio_writer_)
    {
        if(audio_output_)
        {
            audio_output_->flush();
        }

        delete audio_writer_; // will delete outputstream too
        audio_writer_ = 0;
        audio_output_ = 0;
    }

    if(xml_document_)
    {
        add_xml_meta_entry("samplelength", juce::String((juce::uint64)sample_length_));
        add_xml_meta_entry("duration", juce::String((juce::uint64)((sample_length_+sample_offset_)/(((double)sample_rate_)/1000))));

        juce::XmlElement *xml_labels = xml_document_->createNewChildElement("labels");
        for(int i = 0; i < labels_.size(); ++i)
        {
            XmlElement *xml_label = xml_labels->createNewChildElement("label");
            xml_label->setAttribute("category", labels_[i].first);
            xml_label->addTextElement(labels_[i].second);
        }
        xml_labels = 0;

        if(xml_file_ != juce::File::nonexistent)
        {
            xml_document_->writeToFile(xml_file_, "");
        }

        delete xml_document_;
        xml_document_ = 0;
        xml_meta_ = 0;
        labels_.clear();
    }

    if(audio_file_ != juce::File::nonexistent)
    {
        bool moved = false;
        do
        {
            juce::File final_audio_file = clips_dir_.getNonexistentChildFile(audio_file_.getFileNameWithoutExtension(), audio_file_.getFileExtension());
            if(audio_file_.copyFileTo(final_audio_file))
            {
                audio_file_.deleteFile();
                moved = true;
            }
        }
        while(!moved);
        audio_file_ = juce::File();
    }

    if(xml_file_ != juce::File::nonexistent)
    {
        bool moved = false;
        do
        {
            juce::File final_xml_file = clips_dir_.getNonexistentChildFile(xml_file_.getFileNameWithoutExtension(), xml_file_.getFileExtension());
            if(xml_file_.copyFileTo(final_xml_file))
            {
                xml_file_.deleteFile();
                moved = true;
            }
        }
        while(!moved);
        xml_file_ = juce::File();
    }

    zerocross_ = false;
}
