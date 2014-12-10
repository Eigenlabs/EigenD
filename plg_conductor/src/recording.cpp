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

#include "recording.h"

#include "data_channel.h"
#include "data_wire.h"

cdtr::recording_t::recording_t(data_wire_t *wire) : wire_(wire), consumer_lock_(true, false), consumer_phase_(CONSUMER_IDLE)
{
    recorded_data_[0] = new cdtr::recorded_data_list_t();
    recorded_data_[1] = new cdtr::recorded_data_list_t();
    switch_ = 0;
    producer_ = 0;
    is_recording_ = false;
    clear_labels();
}

cdtr::recording_t::~recording_t()
{
    stop();

    cdtr::conductor_t::impl_t *conductor = wire_->get_channel()->get_conductor();
    for (int i=0; i<2; ++i)
    {
        cdtr::recorded_data_list_t *list = recorded_data_[i];
        recorded_data_[i] = 0;
        if(list)
        {
            cdtr::recorded_data_list_t::const_iterator it;
            while((it=list->begin())!=list->end())
            {
                cdtr::recorded_data_t *data = *it;
                list->pop_front();
                conductor->delete_recorded_data(data);
            }
            delete list;
        }
    }
}

void cdtr::recording_t::start()
{
    if(is_recording_)
    {
        return;
    }

    is_recording_ = true;
    wire_->get_channel()->get_conductor()->activate_recording(this);

    pic::flipflop_t<piw::data_t>::guard_t g(last_labels_);
    piw::data_t l = g.value();
    if(!l.is_null())
    {
        __add(piw::tsd_time(), cdtr::LABELS, l);
    }
}

void cdtr::recording_t::stop()
{
    if(!is_recording_)
    {
        return;
    }

    is_recording_ = false;
    consumer_phase_ = CONSUMER_TEARDOWN;
}

void cdtr::recording_t::__stop()
{
    wire_->get_channel()->get_conductor()->deactivate_recording(this);
    teardown();
}

bool cdtr::recording_t::is_recording()
{
    return is_recording_;
}

void cdtr::recording_t::add(unsigned long long time, const cdtr::recorded_data_type_t type, const piw::data_nb_t &data)
{
    if(type == cdtr::LABELS)
    {
        last_labels_.set(piw::data_t::from_given(data.give_copy()));
    }

    if(!is_recording_ || !recorded_data_[producer_])
    {
        return;
    }

    __add(time, type, piw::data_t::from_given(data.give_copy()));
}

void cdtr::recording_t::__add(unsigned long long time, const cdtr::recorded_data_type_t type, const piw::data_t &data)
{
    // add the data to the back of the producer list
    cdtr::conductor_t::impl_t *conductor = wire_->get_channel()->get_conductor();
    recorded_data_t *recd = conductor->new_recorded_data(
            data.time(),
            conductor->interpolate_songbeat(data.time()),
            conductor->interpolate_barbeat(data.time()),
            type,
            data);
    recorded_data_[producer_]->push_back(recd);

    __switch();
}

void cdtr::recording_t::__switch()
{
    if(switch_)
    {
        pic_atomic_t producer = producer_;

        // switch around the lists
        pic_atomiccas(&producer_, producer, 1-producer);

        // indicate that the switch has been performed so that
        // the consumer thread can process the new data
        pic_atomiccas(&switch_, 1, 0);
    }
}

void cdtr::recording_t::consume(bool *activity)
{
    cdtr::conductor_t::impl_t *conductor = wire_->get_channel()->get_conductor();

    // ensure that there's only one consumer of the data in the list
    pic::mutex_t::guard_t g(consumer_lock_);

    // get the consumer list
    pic_atomic_t consumer = 1-producer_;
    cdtr::recorded_data_list_t *list = recorded_data_[consumer];
    if(!list)
    {
        *activity = false;
        return;
    }

    bool work = !list->empty();
    *activity = work;
    if(work)
    {
        // if the consumer is idle, start it up if a recording has started
        if(consumer_phase_ == CONSUMER_IDLE)
        {
            if(is_recording_)
            {
                prepare(*list->front());

                consumer_phase_ = CONSUMER_RUNNING;
            }
        }

        // consume all the data in the list and progressively empty it
        if(consumer_phase_ != CONSUMER_IDLE)
        {
            cdtr::recorded_data_list_t::const_iterator it;
            while((it=list->begin())!=list->end())
            {
                cdtr::recorded_data_t *data = *it;
                if(!consumed(*data))
                {
                    return;
                }
                list->pop_front();
                conductor->delete_recorded_data(data);
            }
        }
    }

    // indicate to the produce thread that the list is empty
    // and that it should be switched around
    pic_atomiccas(&switch_, 0, 1);

    // if the consumer is being tore down, wait until everything
    // is consumed and only fully stop when no work is outstanding
    if(consumer_phase_ == CONSUMER_TEARDOWN)
    {
        __switch();

        if(!work)
        {
            consumer_phase_ = CONSUMER_IDLE;
            __stop();
        }
    }
}

void cdtr::recording_t::clear_labels()
{
    last_labels_.set(piw::makenull(0));
}
