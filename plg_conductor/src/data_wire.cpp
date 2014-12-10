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

#include "data_wire.h"

#include "data_channel.h"

cdtr::data_wire_t::data_wire_t(cdtr::data_channel_t *channel, const unsigned index, const piw::data_t &path, const piw::event_data_source_t &es): channel_(channel), index_(index), path_(path), from_(0)
{
    PIC_ASSERT(channel_ != 0);
    PIC_ASSERT(index_ != 0);
    PIC_ASSERT(!path_.is_null());

    recording_.set(0);

    channel_->add_wire(path_,this);
    subscribe_and_ping(es);
}

cdtr::data_wire_t::~data_wire_t()
{
    piw::tsd_fastcall(__invalidator, this, 0);
    unsubscribe();
    piw::wire_t::disconnect();
    channel_->remove_wire(path_);
    clear_recording();
}

int cdtr::data_wire_t::__invalidator(void *w_, void *)
{
    cdtr::data_wire_t *w = (cdtr::data_wire_t *)w_;
    w->remove();
    return 0;
}

void cdtr::data_wire_t::wire_closed()
{
    delete this;
}

cdtr::data_channel_t *cdtr::data_wire_t::get_channel()
{
    return channel_;
}

unsigned cdtr::data_wire_t::get_index()
{
    return index_;
}

void cdtr::data_wire_t::initialize(unsigned long long t)
{
    event_id_ = piw::makenull_nb(t);
    from_ = t;
    data_index_ = 0;
    controller_index_ = 0;
}

void cdtr::data_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    initialize(id.time());

    event_id_ = id;

    piw::data_nb_t d;
    data_queue_ = b.signal(1);
    if(data_queue_.latest(d,&data_index_,from_))
    {
        handle_data(d);
    }

    piw::data_nb_t c;
    controller_queue_ = b.signal(2);
    if(controller_queue_.latest(c,&controller_index_,from_))
    {
        handle_controller(c);
    }

    activate();

    ticked(from_, piw::tsd_time());
}

void cdtr::data_wire_t::ticked(unsigned long long f, unsigned long long t)
{

    while(true)
    {
        bool data_valid = false;
        bool data_read = false;
        bool controller_valid = false;
        bool controller_read = false;

        if(data_queue_.isvalid())
        {
            data_valid = true;
            piw::data_nb_t d;
            if(data_queue_.read(d, &data_index_,t))
            {
                data_read = true;
                handle_data(d);
                data_index_++;
            }
        }

        if(controller_queue_.isvalid())
        {
            controller_valid = true;
            piw::data_nb_t d;
            if(controller_queue_.read(d, &controller_index_,t))
            {
                controller_read = true;
                handle_controller(d);
                controller_index_++;
            }
        }
        
        if((!data_valid && !controller_valid) ||
           (!data_read && !controller_read))
        {
            break;
        }
    }

    from_ = t;
}

void cdtr::data_wire_t::handle_data(const piw::data_nb_t &d)
{
    pic::flipflop_t<cdtr::recording_t *>::guard_t g(recording_);
    if(g.value())
    {
        g.value()->add(d.time(), cdtr::AUDIO, d);
    }
}

void cdtr::data_wire_t::handle_controller(const piw::data_nb_t &d)
{
    pic::flipflop_t<cdtr::recording_t *>::guard_t g(recording_);
    if(g.value())
    {
        if(d.is_dict())
        {
            piw::data_nb_t labels = d.as_dict_lookup("labels");
            if(labels.is_tuple())
            {
                g.value()->add(d.time(), cdtr::LABELS, labels);
            }
        }
    }
}

void cdtr::data_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s == 1)
    {
        data_queue_ = n;
        data_index_ = 0;

        piw::data_nb_t d;
        if(data_queue_.isvalid() &&
           data_queue_.latest(d, &data_index_, from_))
        {
            handle_data(d);
        }
    }
    else if(s == 2)
    {
        controller_queue_ = n;
        controller_index_ = 0;

        piw::data_nb_t d;
        if(controller_queue_.isvalid() &&
           controller_queue_.latest(d, &controller_index_, from_))
        {
            handle_controller(d);
        }
    }
}

bool cdtr::data_wire_t::event_end(unsigned long long t)
{
    ticked(from_, t);

    deactivate();

    data_queue_.clear();
    controller_queue_.clear();
    initialize(0);

    return true;
}

bool cdtr::data_wire_t::has_event()
{
    return !event_id_.is_null();
}

void cdtr::data_wire_t::set_recording(cdtr::recording_t *rec)
{
    cdtr::recording_t *old = recording_.current();
    recording_.set(rec);
    if(old && old != rec)
    {
        delete old;
    }
};

void cdtr::data_wire_t::clear_recording()
{
    cdtr::recording_t *old = recording_.current();
    recording_.set(0);
    if(old)
    {
        delete old;
    }
};

cdtr::recording_t *cdtr::data_wire_t::get_recording()
{
    return recording_.current();
};

void cdtr::data_wire_t::activate()
{
    channel_->activate_wire(this);
};

void cdtr::data_wire_t::deactivate()
{
    channel_->deactivate_wire(this);
};
