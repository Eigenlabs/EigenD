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

#include "recording_consumer.h"

#include "conductor_impl.h"

cdtr::recording_consumer_t::recording_consumer_t(cdtr::conductor_t::impl_t *conductor): pic::thread_t(PIC_THREAD_PRIORITY_HIGH), conductor_(conductor), shutdown_(false)
{
}

cdtr::recording_consumer_t::~recording_consumer_t()
{
    shutdown();
}

void cdtr::recording_consumer_t::shutdown()
{
    if(!isrunning()) return;

    shutdown_ = true;
    gate_.open();
    wait();
}

void cdtr::recording_consumer_t::thread_main()
{
    bool activity = false;
    bool shutdown = false;
    bool shutting_down = false;
    while(!shutdown)
    {
        activity = false;

        if(shutdown_) shutting_down = true;
        conductor_->consume_recordings(&activity);
        if(shutting_down && !activity) shutdown = true;

        gate_.pass_and_shut_timed(500000ULL);
    }
}
