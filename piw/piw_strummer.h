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

#ifndef __PIW_STRUMMER__
#define __PIW_STRUMMER__

#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"

namespace piw
{
    class  PIW_DECLSPEC_CLASS strummer_t
    {
        public:
            class impl_t;
        public:
            strummer_t(const cookie_t &, piw::clockdomain_ctl_t *);
            ~strummer_t();
            cookie_t data_cookie();
            cookie_t strum_cookie();
            void enable(bool);
            void set_trigger_window(unsigned);
            void set_strum_breath_scale(float);
            void set_pressure_scale(float);
            void set_strum_pressure_scale(float);
            void set_roll_scale(float);
            void set_strum_roll_scale(float);
            void set_yaw_scale(float);
            void set_strum_yaw_scale(float);
            void clear_breath_courses();
            void add_breath_course(int);
            void clear_key_courses();
            void add_key_course(int,int,int);
            void set_strum_note_end(bool);

        private:
            impl_t *impl_;
    };
};

#endif
