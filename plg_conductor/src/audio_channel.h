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

#ifndef __CDTR_AUDIO_CHANNEL_H__
#define __CDTR_AUDIO_CHANNEL_H__

#include <piw/piw_bundle.h>

#include "data_channel.h"

namespace cdtr
{
    class data_wire_t;
    
    class audio_channel_t: public cdtr::data_channel_t
    {
        public:
            audio_channel_t(cdtr::conductor_t::impl_t *root, const unsigned index);
    
            virtual data_wire_t *create_wire(unsigned index, const piw::data_t &path, const piw::event_data_source_t &es);
            virtual void delete_wire(cdtr::data_wire_t *wire);
            virtual void remove();
    };
};

#endif
