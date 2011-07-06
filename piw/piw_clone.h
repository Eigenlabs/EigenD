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

#ifndef __PIW_CLONE__
#define __PIW_CLONE__
#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"
#include "piw_data.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS clone_t
    {
        public:
            class impl_t;
        public:
            clone_t(bool init);
            ~clone_t();
            void set_policy(bool activate_on_enable);
            void set_output(unsigned name, const cookie_t &);
            void set_filtered_output(unsigned name, const cookie_t &, const piw::d2d_nb_t &);
            void set_filtered_data_output(unsigned name, const cookie_t &, const piw::d2d_nb_t &, unsigned filtered_signal);
            void clear_output(unsigned name);
            change_nb_t gate(unsigned name);
            void enable(unsigned name,bool enabled);
            cookie_t cookie();
            int gc_traverse(void *, void *) const;
            int gc_clear();
        private:
            impl_t *root_;
    };
};

#endif
