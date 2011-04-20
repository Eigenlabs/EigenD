/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __PIW_SLOWCHANGE__
#define __PIW_SLOWCHANGE__
#include "piw_exports.h"
#include <picross/pic_functor.h>
#include <piw/piw_data.h>

namespace piw
{
     PIW_DECLSPEC_FUNC(piw::change_t) slowchange(const piw::change_t &);
     PIW_DECLSPEC_FUNC(piw::change_nb_t) slowchange_polled(const piw::change_t &,unsigned ms);
     PIW_DECLSPEC_FUNC(piw::change_t) slowtrigger(const piw::change_t &);
     PIW_DECLSPEC_FUNC(piw::change_t) fastchange(const piw::change_nb_t &);
     PIW_DECLSPEC_FUNC(piw::change_nb_t) deferred_sender(const piw::change_nb_t &, const piw::data_t &);
};

#endif
