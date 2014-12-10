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
#include "clip_manager_impl.h"

cdtr::clip_manager_t::clip_manager_t(const std::string &library_path) : impl_(new impl_t(library_path))
{
}

cdtr::clip_manager_t::~clip_manager_t()
{
    delete impl_;
}

void cdtr::clip_manager_t::initialise_widget(widget_t *w)
{
    impl_->initialise_widget(w);
}
