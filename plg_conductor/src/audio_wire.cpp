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

#include "audio_wire.h"

#include "audio_recording.h"
#include "data_channel.h"

cdtr::audio_wire_t::audio_wire_t(data_channel_t *channel, const unsigned index, const piw::data_t &path, const piw::event_data_source_t &es): data_wire_t(channel, index, path, es)
{
    set_recording(new cdtr::audio_recording_t(this));
    initialize(piw::tsd_time());
    cdtr::data_wire_t::activate();
};

cdtr::audio_wire_t::~audio_wire_t()
{
    cdtr::data_wire_t::deactivate();
};

void cdtr::audio_wire_t::activate()
{
    // an audio wire should always be active while it exists and compensate when there's no event active
};

void cdtr::audio_wire_t::deactivate()
{
};

void cdtr::audio_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    if(has_event())
    {
        cdtr::data_wire_t::ticked(f,t);
    }
    else
    {
        unsigned buffer_size = get_channel()->get_conductor()->get_buffer_size();
        float *f;
        float *fs;
        piw::data_nb_t d = piw::makenorm_nb(t,buffer_size,&f,&fs);
        memset(f,0,buffer_size*sizeof(float));
        *fs = f[buffer_size-1];
        handle_data(d);
    }
};
