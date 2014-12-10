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

#include "audio_recording.h"

#include "data_channel.h"
#include "data_wire.h"

cdtr::audio_recording_t::audio_recording_t(cdtr::data_wire_t *wire) : cdtr::recording_t(wire)
{
}

void cdtr::audio_recording_t::prepare(cdtr::recorded_data_t &first)
{
    cdtr::data_channel_t *channel = wire_->get_channel();
    cdtr::conductor_t::impl_t *conductor = channel->get_conductor();

    juce::String uid = juce::String(channel->get_recording_start());
    uid += "_";
    uid += juce::String(channel->get_index());
    uid += "_";
    uid += juce::String(wire_->get_index());

    format_.set_zerocross_start(true);
    format_.prepare(conductor->get_conductor_dir(CONDUCTOR_RECORDINGS), conductor->get_conductor_dir(CONDUCTOR_CLIPS), uid, conductor->get_sample_rate());
}

bool cdtr::audio_recording_t::consumed(cdtr::recorded_data_t &data)
{
    switch(data.get_type())
    {
        case cdtr::AUDIO:
            return format_.add_audio(data);
        case cdtr::LABELS:
            return format_.add_labels(data);
    }
    return false;
}

void cdtr::audio_recording_t::teardown()
{
    format_.teardown();
}
