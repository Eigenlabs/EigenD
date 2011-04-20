
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

#ifndef __PICA_AUDIOCTL__
#define __PICA_AUDIOCTL__

#include "audio_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>

namespace pi_audio
{
    class PIAUDIO_DECLSPEC_CLASS audioctl_t
    {
        public:
            class impl_t;
        public:
            audioctl_t(piw::clockdomain_ctl_t *d,const std::string &domain_name);
            virtual ~audioctl_t();
            piw::cookie_t cookie();
            bool open_device(const std::string &uid,unsigned long sr, unsigned size, bool callback);
            void close_device();
            std::string get_uid();
            void mute();
            void unmute();
            unsigned num_devices();
            std::string device_name(unsigned);
            std::string device_uid(unsigned);
            void shutdown();
            void reset_dropout_count();
            unsigned long get_dropout_count();
            virtual void device_changed(const char *uid,unsigned long,unsigned) {}
            virtual void device_list_changed() {}
            void enable_callbacks(bool);
            void show_gui(bool);

        private:
            impl_t *impl_;
    };
};

#endif
