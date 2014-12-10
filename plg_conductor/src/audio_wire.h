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

#ifndef __CDTR_AUDIO_WIRE_H__
#define __CDTR_AUDIO_WIRE_H__

#include <piw/piw_bundle.h>

#include "data_wire.h"

namespace cdtr
{
    class data_channel_t;
    
    class audio_wire_t: public cdtr::data_wire_t
    {
        public:
            audio_wire_t(cdtr::data_channel_t *channel, const unsigned index, const piw::data_t &path, const piw::event_data_source_t &es);
            ~audio_wire_t();
    
            void activate();
            void deactivate();
            void ticked(unsigned long long f, unsigned long long t);
    };
};

#endif
