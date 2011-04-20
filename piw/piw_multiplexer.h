
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

#ifndef __PIW_MULTIPLEXER__
#define __PIW_MULTIPLEXER__

#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    class clockdomain_ctl_t;

    class PIW_DECLSPEC_CLASS multiplexer_t
    {
        public:
            multiplexer_t(const cookie_t &, clockdomain_ctl_t *);
            ~multiplexer_t();

            cookie_t get_input(unsigned);
            void clear_input(unsigned);
            change_nb_t gate_input();

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif
