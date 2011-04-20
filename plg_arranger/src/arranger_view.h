
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


#ifndef __ARRANGER_VIEW__
#define __ARRANGER_VIEW__
#include <plg_arranger/piarranger_exports.h>

#include "picross/pic_config.h"

#include <piw/piw_bundle.h>
#include <picross/pic_functor.h>
#include "arranger_colrow.h"
#include "arranger_model.h"

namespace arranger
{
    class PIARRANGER_DECLSPEC_CLASS view_t : public pic::nocopy_t
    {
        public:
            view_t(model_t *,const piw::cookie_t &lights_output);
            ~view_t();

            piw::cookie_t cookie();
            piw::change_nb_t control();
            piw::change_nb_t set_doubletap();
            void doubletap_set(const piw::change_nb_t &);
            void clear_events();

            class impl_t;

        private:
            impl_t *impl_;
    };
}

#endif

