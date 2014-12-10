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

#include "recorded_data.h"

cdtr::recorded_data_t::recorded_data_t(unsigned long long time, float songbeat, float barbeat, const cdtr::recorded_data_type_t type, const piw::data_t &data):
    time_(time), songbeat_(songbeat), barbeat_(barbeat), type_(type),
    data_(data)
{
}

cdtr::recorded_data_t::~recorded_data_t()
{
}

unsigned long long cdtr::recorded_data_t::get_time() const
{
    return time_;
}

float cdtr::recorded_data_t::get_songbeat() const
{
    return songbeat_;
}

float cdtr::recorded_data_t::get_barbeat() const
{
    return barbeat_;
}

const cdtr::recorded_data_type_t cdtr::recorded_data_t::get_type() const
{
    return type_;
}

const piw::data_t cdtr::recorded_data_t::get_data() const
{
    return data_;
}

cdtr::recorded_data_factory_t::recorded_data_factory_t(unsigned maximum):
    maximum_(maximum), allocation_count_(0)
{
}

cdtr::recorded_data_t *cdtr::recorded_data_factory_t::get_instance(unsigned long long time, float songbeat, float barbeat, const cdtr::recorded_data_type_t type, const piw::data_t &data)
{
    while(true)
    {
        if(allocation_count_ < maximum_)
        {
            pic_atomicinc(&allocation_count_);
            return new cdtr::recorded_data_t(time, songbeat, barbeat, type, data);
        }

        pic_thread_yield();
    }
}

void cdtr::recorded_data_factory_t::delete_instance(cdtr::recorded_data_t *data)
{
    delete data;
    pic_atomicdec(&allocation_count_);
}
