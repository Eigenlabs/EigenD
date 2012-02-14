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

#ifndef __PIW_KEYGROUP__
#define __PIW_KEYGROUP__

#include "piw_exports.h"
#include "piw_data.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS modekey_handler_t
    {
        public:
            class impl_t;

        public:
            modekey_handler_t();
            ~modekey_handler_t();

            piw::d2b_nb_t key_filter();
            void set_modekey(int row, int column);
            void set_upstream_rowlength(const piw::data_t &rowlen);

        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS keygroup_mapper_t
    {
        public:
            class impl_t;

        public:
            keygroup_mapper_t();
            ~keygroup_mapper_t();

            piw::d2d_nb_t key_filter();
            piw::d2d_nb_t light_filter();

            void clear_mapping();
            void activate_mapping();

            void clear_physical_mapping();
            void set_physical_mapping(int row_in, int column_in, int rel_row_in, int rel_column_in, unsigned sequential_in, int row_out, int column_out, int rel_row_out, int rel_column_out, unsigned sequential_out);
            void activate_physical_mapping();

            void clear_musical_mapping();
            void set_musical_mapping(int course_in, int key_in, int rel_course_in, int rel_key_in, unsigned sequential_in, int course_out, int key_out, int rel_course_out, int rel_key_out, unsigned sequential_out);
            void activate_musical_mapping();

        private:
            impl_t *impl_;
    };
};

#endif
