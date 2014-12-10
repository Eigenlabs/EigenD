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

#ifndef __CDTR_CONDUCTOR_IMPL_H__
#define __CDTR_CONDUCTOR_IMPL_H__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_clockclient.h>

#include "conductor.h"
#include "conductor_library.h"
#include "metronome_input.h"
#include "recorded_data.h"
#include "recording.h"
#include "recording_consumer.h"

namespace cdtr
{
    class audio_channel_t;
    class data_channel_t;

    class cdtr::conductor_t::impl_t: public piw::clocksink_t, public piw::root_ctl_t, public cdtr::conductor_library_t, public virtual pic::lckobject_t
    {
        public:
            impl_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain, const std::string &library_path);
            ~impl_t();
    
            void invalidate();
    
            piw::cookie_t metronome_cookie();
    
            piw::cookie_t create_audio_channel(const unsigned index);
            void destroy_audio_channel(const unsigned index);
            void add_audio_channel(const unsigned index, cdtr::audio_channel_t *channel);
            void remove_audio_channel(const unsigned index);
            void clear_controller_input(unsigned index);
    
            void activate_channel(cdtr::data_channel_t *channel);
            void deactivate_channel(cdtr::data_channel_t *channel);
    
            void activate_recording(cdtr::recording_t *recording);
            void deactivate_recording(cdtr::recording_t *recording);
    
            void clocksink_ticked(unsigned long long f, unsigned long long t);
            void beat_receive(unsigned clk, const piw::data_nb_t &d);
    
            void shutdown();
            void consume_recordings(bool *activity);
            void start_recording();
            void stop_recordings();

            float interpolate_songbeat(unsigned long long t);
            float interpolate_barbeat(unsigned long long t);

            cdtr::recorded_data_t *new_recorded_data(unsigned long long time, float songbeat, float barbeat, const cdtr::recorded_data_type_t type, const piw::data_t &data);
            void delete_recorded_data(cdtr::recorded_data_t *data);

    
        private:
            metronome_input_t metronome_input_;
            pic::flipflop_t<std::map<unsigned,cdtr::audio_channel_t *> > audio_channels_;
            pic::ilist_t<data_channel_t,3> active_channels_;
    
            pic::ilist_t<recording_t,2> active_recordings_;
    
            cdtr::recording_consumer_t consumer_;
            piw::clockinterp_t clock_;
            cdtr::recorded_data_factory_t data_factory_;
    };
};

#endif
