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

#include "conductor_impl.h"

#include <math.h>

#include "audio_channel.h"
#include "data_channel.h"

cdtr::conductor_t::impl_t::impl_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain, const std::string &library_path):
    cdtr::conductor_library_t(library_path), metronome_input_(this),
    consumer_(this), clock_(2), data_factory_(10000)
{
    domain->sink(this,"conductor");
    set_clock(this);
    connect(output);

    tick_enable(true);

    consumer_.run();
}

cdtr::conductor_t::impl_t::~impl_t()
{
    tick_disable();
    invalidate();
    close_sink();
    consumer_.shutdown();
    stop_recordings();
}

void cdtr::conductor_t::impl_t::invalidate()
{
    tick_disable();

    std::map<unsigned,cdtr::audio_channel_t *>::iterator it;
    while((it=audio_channels_.alternate().begin())!=audio_channels_.alternate().end())
    {
        it->second->invalidate();
        delete it->second;
    }
}

piw::cookie_t cdtr::conductor_t::impl_t::metronome_cookie()
{
    return piw::cookie_t(&metronome_input_);
}

piw::cookie_t cdtr::conductor_t::impl_t::create_audio_channel(const unsigned index)
{
    std::map<unsigned,cdtr::audio_channel_t *>::iterator it;
    if((it=audio_channels_.alternate().find(index))!=audio_channels_.alternate().end())
    {
        it->second->invalidate();
        delete it->second;
    }
    return piw::cookie_t(new cdtr::audio_channel_t(this, index));
}

void cdtr::conductor_t::impl_t::destroy_audio_channel(const unsigned index)
{
    std::map<unsigned,cdtr::audio_channel_t *>::iterator it;
    if((it=audio_channels_.alternate().find(index))!=audio_channels_.alternate().end())
    {
        it->second->invalidate();
        delete it->second;
    }
}

void cdtr::conductor_t::impl_t::clear_controller_input(const unsigned index)
{
    std::map<unsigned,cdtr::audio_channel_t *>::iterator it;
    if((it=audio_channels_.alternate().find(index))!=audio_channels_.alternate().end())
    {
        it->second->clear_controller();
    }
}

void cdtr::conductor_t::impl_t::add_audio_channel(const unsigned index, cdtr::audio_channel_t *channel)
{
    audio_channels_.alternate().insert(std::make_pair(index, channel));
    audio_channels_.exchange();
}

void cdtr::conductor_t::impl_t::remove_audio_channel(const unsigned index)
{
    audio_channels_.alternate().erase(index);
    audio_channels_.exchange();
}

void cdtr::conductor_t::impl_t::activate_channel(cdtr::data_channel_t *channel)
{
    if(!active_channels_.head())
    {
        tick_suppress(false);
    }

    active_channels_.append(channel);
}

void cdtr::conductor_t::impl_t::deactivate_channel(cdtr::data_channel_t *channel)
{
    active_channels_.remove(channel);

    if(!active_channels_.head())
    {
        tick_suppress(true);
    }
}

void cdtr::conductor_t::impl_t::activate_recording(cdtr::recording_t *recording)
{
    active_recordings_.append(recording);
}

void cdtr::conductor_t::impl_t::deactivate_recording(cdtr::recording_t *recording)
{
    active_recordings_.remove(recording);
}

void cdtr::conductor_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
    metronome_input_.ticked(t);

    cdtr::data_channel_t *channel = active_channels_.head();
    cdtr::data_channel_t *channel_next;
    while(channel != 0)
    {
        channel_next = active_channels_.next(channel);
        channel->ticked(f,t);
        channel = channel_next;
    }
}

void cdtr::conductor_t::impl_t::beat_receive(unsigned clk, const piw::data_nb_t &d)
{
    if(!d.as_arraylen()) return;
    clock_.recv_clock(clk, d);
}

void cdtr::conductor_t::impl_t::consume_recordings(bool *activity)
{
    cdtr::recording_t *rec = active_recordings_.head();
    cdtr::recording_t *rec_next;
    while(rec != 0)
    {
        rec_next = active_recordings_.next(rec);
        bool a = false;
        rec->consume(&a);
        *activity = *activity | a;
        rec = rec_next;
    }
}

void cdtr::conductor_t::impl_t::shutdown()
{
    stop_recordings();
}

void cdtr::conductor_t::impl_t::start_recording()
{
    unsigned long long t = piw::tsd_time();

    pic::flipflop_t<std::map<unsigned,cdtr::audio_channel_t *> >::guard_t g(audio_channels_);
    std::map<unsigned,cdtr::audio_channel_t *>::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        it->second->start_recording(t);
    }
}

void cdtr::conductor_t::impl_t::stop_recordings()
{
    cdtr::recording_t *rec = active_recordings_.head();
    cdtr::recording_t *rec_next;
    while(rec != 0)
    {
        rec_next = active_recordings_.next(rec);
        rec->stop();
        rec = rec_next;
    }
}

float cdtr::conductor_t::impl_t::interpolate_songbeat(unsigned long long t)
{
    if(!metronome_input_.is_running())
    {
        return 0.f;
    }

    return clock_.interpolate_clock(CLK_SONGBEAT, t);
}

float cdtr::conductor_t::impl_t::interpolate_barbeat(unsigned long long t)
{
    if(!metronome_input_.is_running())
    {
        return 0.f;
    }

    return clock_.interpolate_clock(CLK_BARBEAT, t);
}

cdtr::recorded_data_t *cdtr::conductor_t::impl_t::new_recorded_data(unsigned long long time, float songbeat, float barbeat, const cdtr::recorded_data_type_t type, const piw::data_t &data)
{
    return data_factory_.get_instance(time, songbeat, barbeat, type, data);
}

void cdtr::conductor_t::impl_t::delete_recorded_data(cdtr::recorded_data_t *data)
{
    data_factory_.delete_instance(data);
}
