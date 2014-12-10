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

#include "conductor.h"
#include "conductor_impl.h"

cdtr::conductor_t::conductor_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain, const std::string &library_path) : impl_(new impl_t(output, domain, library_path))
{
}

cdtr::conductor_t::~conductor_t()
{
    delete impl_;
}

piw::cookie_t cdtr::conductor_t::audio_input_cookie(unsigned index)
{
    return impl_->create_audio_channel(index);
}

piw::cookie_t cdtr::conductor_t::metronome_cookie()
{
    return impl_->metronome_cookie();
}

void cdtr::conductor_t::clear_controller_input(unsigned index)
{
    impl_->clear_controller_input(index);
}

void cdtr::conductor_t::remove_audio_input(unsigned index)
{
    impl_->destroy_audio_channel(index);
}

void cdtr::conductor_t::start_recording()
{
    impl_->start_recording();
}

void cdtr::conductor_t::stop_recording()
{
    impl_->stop_recordings();
}

void cdtr::conductor_t::shutdown()
{
    return impl_->shutdown();
}

