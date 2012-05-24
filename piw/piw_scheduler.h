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

#ifndef __PIW_SCHEDULER__
#define __PIW_SCHEDULER__
#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_controller2.h>

namespace piw
{
    class event_t;
    class scheduler_t;

    class PIW_DECLSPEC_CLASS scheduler_t: public pic::nocopy_t
    {
        public:
            friend class event_t;
            class impl_t;

        public:
            scheduler_t(unsigned signals);
            ~scheduler_t();

            cookie_t cookie();
            float current(unsigned);

        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS event_t: public controlled2_t
    {
        public:
            friend class scheduler_t;
            class impl_t;

        public:
            event_t(scheduler_t *, bool enabled, const change_t &enable_changed);
            ~event_t();

            virtual void control_enable() { event_enable(); }
            virtual void control_disable() { event_disable(); }

            int gc_traverse(void *v, void *a) const;
            int gc_clear();

            void event_enable();
            void event_disable();
            void event_clear();
            void upper_bound(unsigned signal, float value);
            void lower_bound(unsigned signal, float value);
            void modulo(unsigned signal, unsigned divisor, float remainder);
            void zone(unsigned signal, unsigned divisor, float remainder1, float remainder2);

        private:
            impl_t *impl_;
    };
};

#endif
