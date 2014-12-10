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

#include "audio_channel.h"

#include "audio_wire.h"
#include "data_wire.h"

cdtr::audio_channel_t::audio_channel_t(cdtr::conductor_t::impl_t *root, const unsigned index): cdtr::data_channel_t(root, index)
{
    get_conductor()->add_audio_channel(index,this);
}

cdtr::data_wire_t *cdtr::audio_channel_t::create_wire(unsigned index, const piw::data_t &path, const piw::event_data_source_t &es)
{
    return new cdtr::audio_wire_t(this, index, path, es);
}

void cdtr::audio_channel_t::delete_wire(cdtr::data_wire_t *wire)
{
    delete (cdtr::audio_wire_t *)wire;
}

void cdtr::audio_channel_t::remove()
{
    get_conductor()->remove_audio_channel(get_index());
}
