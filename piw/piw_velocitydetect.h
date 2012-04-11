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

#ifndef __VELOCITYDETECT__
#define __VELOCITYDETECT__
#include "piw_exports.h"
#include <piw/piw_bundle.h>

namespace piw
{
    class cookie_t;
    class velocitydetector_t;

    class PIW_DECLSPEC_CLASS velocityconfig_t
    {
        public:
            velocityconfig_t();
            ~velocityconfig_t();
            void set_samples(unsigned);
            void set_curve(float);
            void set_scale(float);

            class impl_t;

        private:
            friend class velocitydetector_t;
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS velocitydetector_t
    {
        public:
            velocitydetector_t(const piw::velocityconfig_t &config);
            ~velocitydetector_t();
            void init();
            bool detect(const piw::data_nb_t &d, double *velocity);
            bool is_started();

            class impl_t;

        private:
            impl_t *impl_;
    };
 
    class PIW_DECLSPEC_CLASS velocitydetect_t
    {
        public:
            velocitydetect_t(const piw::cookie_t &o, unsigned psig, unsigned vsig);
            ~velocitydetect_t();
            void set_samples(unsigned);
            void set_curve(float);
            void set_scale(float);
            piw::cookie_t cookie();

            class impl_t;

        private:
            impl_t *impl_;
    };
};

#endif
