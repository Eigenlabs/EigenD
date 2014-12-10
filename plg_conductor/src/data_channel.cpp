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

#include "data_channel.h"

#include "data_wire.h"

cdtr::data_channel_t::data_channel_t(cdtr::conductor_t::impl_t *root, const unsigned index): piw::root_t(0), root_(root), index_(index), recording_start_(0), clock_(0), wire_index_(0)
{
}

cdtr::data_channel_t::~data_channel_t()
{
}

cdtr::conductor_t::impl_t *cdtr::data_channel_t::get_conductor()
{
    return root_;
}

unsigned cdtr::data_channel_t::get_index()
{
    return index_;
}

unsigned long long cdtr::data_channel_t::get_recording_start()
{
    return recording_start_;
}

void cdtr::data_channel_t::invalidate()
{
    std::map<piw::data_t,cdtr::data_wire_t *>::iterator it;
    while((it=wires_.alternate().begin())!=wires_.alternate().end())
    {
        delete_wire(it->second);
    }

    piw::root_t::disconnect();

    if(clock_)
    {
        root_->remove_upstream(clock_);
        clock_ = 0;
    }

    remove();
}

piw::wire_t *cdtr::data_channel_t::root_wire(const piw::event_data_source_t &es)
{
    piw::data_t path = es.path();

    std::map<piw::data_t,cdtr::data_wire_t *>::iterator it;
    if((it=wires_.alternate().find(path))!=wires_.alternate().end())
    {
        delete_wire(it->second);
    }
    return create_wire(++wire_index_, path, es);
}

void cdtr::data_channel_t::root_closed()
{
    invalidate();
}

void cdtr::data_channel_t::root_opened()
{
    root_clock();
    root_latency();
}

void cdtr::data_channel_t::root_clock()
{
    bct_clocksink_t *c = get_clock();

    if(c!=clock_)
    {
        stop_recordings();

        if(clock_)
        {
            root_->remove_upstream(clock_);
        }

        clock_ = c;

        if(clock_)
        {
            root_->add_upstream(clock_);
        }
    }
}

void cdtr::data_channel_t::root_latency()
{
}

void cdtr::data_channel_t::add_wire(const piw::data_t &path, cdtr::data_wire_t *wire)
{
    wires_.alternate().insert(std::make_pair(path, wire));
    wires_.exchange();
}

void cdtr::data_channel_t::remove_wire(const piw::data_t &path)
{
    wires_.alternate().erase(path);
    wires_.exchange();
}

void cdtr::data_channel_t::activate_wire(cdtr::data_wire_t *wire)
{
    if(!active_wires_.head())
    {
        root_->activate_channel(this);
    }

    active_wires_.append(wire);
}

void cdtr::data_channel_t::deactivate_wire(cdtr::data_wire_t *wire)
{
    active_wires_.remove(wire);

    if(!active_wires_.head())
    {
        root_->deactivate_channel(this);
    }
}

void cdtr::data_channel_t::ticked(unsigned long long f, unsigned long long t)
{
    cdtr::data_wire_t *wire = active_wires_.head();
    cdtr::data_wire_t *wire_next;
    while(wire != 0)
    {
        wire_next = active_wires_.next(wire);
        wire->ticked(f,t);
        wire = wire_next;
    }
}

void cdtr::data_channel_t::start_recording(unsigned long long t)
{
    recording_start_ = t;

    pic::flipflop_t<std::map<piw::data_t,cdtr::data_wire_t *> >::guard_t g(wires_);
    std::map<piw::data_t,cdtr::data_wire_t *>::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        cdtr::data_wire_t *wire = it->second;
        cdtr::recording_t *recording = wire->get_recording();
        if(recording)
        {
            recording->start();
        }
    }
}

void cdtr::data_channel_t::stop_recordings()
{
    pic::flipflop_t<std::map<piw::data_t,cdtr::data_wire_t *> >::guard_t g(wires_);
    std::map<piw::data_t,cdtr::data_wire_t *>::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        cdtr::data_wire_t *wire = it->second;
        cdtr::recording_t *recording = wire->get_recording();
        if(recording)
        {
            recording->stop();
        }
    }

    recording_start_ = 0;
}

void cdtr::data_channel_t::clear_controller()
{
    pic::flipflop_t<std::map<piw::data_t,cdtr::data_wire_t *> >::guard_t g(wires_);
    std::map<piw::data_t,cdtr::data_wire_t *>::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        cdtr::data_wire_t *wire = it->second;
        cdtr::recording_t *recording = wire->get_recording();
        if(recording)
        {
            recording->clear_labels();
        }
    }
}
