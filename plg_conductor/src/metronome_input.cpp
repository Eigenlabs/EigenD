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

#include "metronome_input.h"

#include "conductor.h"
#include "conductor_impl.h"

#define SIG_SONGBEAT 1
#define SIG_BARBEAT 2
#define SIG_RUNNING 3
#define SIG_TEMPO 4

cdtr::metronome_input_t::metronome_input_t(cdtr::conductor_t::impl_t *root): piw::root_t(0), root_(root), clock_(0), running_(false)
{
}

cdtr::metronome_input_t::~metronome_input_t()
{
    invalidate();
}

void cdtr::metronome_input_t::invalidate()
{
    piw::root_t::disconnect();

    if(clock_)
    {
        root_->remove_upstream(clock_);
        clock_ = 0;
        running_ = false;
    }
}

piw::wire_t *cdtr::metronome_input_t::root_wire(const piw::event_data_source_t &es)
{
    subscribe(es);

    return this;
}

void cdtr::metronome_input_t::root_closed()
{
    invalidate();
}

void cdtr::metronome_input_t::root_opened()
{
    root_clock();
    root_latency();
}

void cdtr::metronome_input_t::root_clock()
{
    bct_clocksink_t *c = get_clock();

    if(c!=clock_)
    {
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

void cdtr::metronome_input_t::root_latency()
{
}

void cdtr::metronome_input_t::wire_closed()
{
    piw::wire_t::disconnect();
    unsubscribe();
    running_ = false;
}

void cdtr::metronome_input_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    iterator_ = b.iterator();
    ticked(id.time());
}

void cdtr::metronome_input_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    iterator_->reset(sig,t);
}

bool cdtr::metronome_input_t::event_end(unsigned long long t)
{
    ticked(t);
    iterator_.clear();
    running_ = false;
    return true;
}

void cdtr::metronome_input_t::ticked(unsigned long long t)
{
    if(!iterator_.isvalid())
    {
        return;
    }

    piw::data_nb_t d;
    if(iterator_->latest(SIG_RUNNING,d,t))
    {
        running_ = d.as_float();
    }
    if(running_)
    {
        if(iterator_->latest(SIG_SONGBEAT,d,t))
        {
            root_->beat_receive(CLK_SONGBEAT,d);
        }
        if(iterator_->latest(SIG_BARBEAT,d,t))
        {
            root_->beat_receive(CLK_BARBEAT,d);
        }
    }
}

bool cdtr::metronome_input_t::is_running()
{
    return running_;
}
