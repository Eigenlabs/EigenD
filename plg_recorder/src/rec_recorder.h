
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

#ifndef __RECORDER__
#define __RECORDER__
#include <pirecorder_exports.h>
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>

#include "rec_recording.h"

// errors from record()
#define RECORD_OK           (0)
#define RECORD_ERR_NO_CLOCK (-1)
#define RECORD_ERR_IN_PROG  (-2)

namespace recorder
{
    class PIRECORDER_DECLSPEC_CLASS recorder_t
    {
        public:
            recorder_t(piw::clockdomain_ctl_t *,unsigned s);
            virtual ~recorder_t();
            piw::cookie_t cookie();

            int record(unsigned bars);
            virtual void record_done(const recording_t &record) = 0;
            virtual void record_started(const recording_t &record) = 0;
            virtual void record_aborted() = 0;
            void abort();

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif

