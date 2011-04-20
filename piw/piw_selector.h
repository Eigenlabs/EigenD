
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

#ifndef __PIW_SELECTOR__
#define __PIW_SELECTOR__

#include "piw_exports.h"
#include "piw_data.h"
#include "piw_bundle.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS selector_t
    {
        public:
            selector_t(const cookie_t &light_output, const change_nb_t &lights_selector, unsigned name, bool initial);
            ~selector_t();

            change_nb_t mode_input();
            change_nb_t gate_input(unsigned);
            void gate_output(unsigned, const change_nb_t &gate, const change_nb_t &selected);
            void clear_output(unsigned);
            void choose(bool);
            void activate(unsigned);
            void select(unsigned,bool);

            int gc_traverse(void *,void *) const;
            int gc_clear();

        public:
            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif
