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

#ifndef __CDTR_CONDUCTOR_H__
#define __CDTR_CONDUCTOR_H__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include "widget.h"

// noindex directory suffix prevents spotlight on OSX from indexing the directory while recording
#define CONDUCTOR_RECORDINGS "Recordings.noindex"
#define CONDUCTOR_CLIPS "Clips"
#define CONDUCTOR_CLIPS_DB "Clips.db"

namespace cdtr
{
    class conductor_t
    {
        public:
            conductor_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain, const std::string &library_path);
            ~conductor_t();
            piw::cookie_t audio_input_cookie(unsigned index);
            piw::cookie_t metronome_cookie();
            void clear_controller_input(unsigned index);
            void remove_audio_input(unsigned index);
            void start_recording();
            void stop_recording();
            void shutdown();

            class impl_t;
        private:
            impl_t * const impl_;
    };

    class clip_manager_t
    {
        public:
            clip_manager_t(const std::string &library_path);
            ~clip_manager_t();

            void initialise_widget(widget_t *);

            class impl_t;
        private:
            impl_t * const impl_;
    };
};

#endif
