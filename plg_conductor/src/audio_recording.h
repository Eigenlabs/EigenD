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

#ifndef __CDTR_AUDIO_RECORDING_H__
#define __CDTR_AUDIO_RECORDING_H__

#include "recording.h"
#include "recorded_data.h"
#include "aiff_format.h"

namespace cdtr
{
    class data_wire_t;

    class audio_recording_t: public cdtr::recording_t 
    {
        public:
            audio_recording_t(cdtr::data_wire_t *wire);
            void prepare(cdtr::recorded_data_t &first);
            bool consumed(cdtr::recorded_data_t &data);
            void teardown();
    
        private:
            cdtr::aiff_format_t format_;
    };
};

#endif
