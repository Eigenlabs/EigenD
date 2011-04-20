
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

#ifndef __PIW_BACKEND__
#define __PIW_BACKEND__
#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS functor_backend_t
    {
        public:
            functor_backend_t(unsigned signal,bool end_null);
            ~functor_backend_t();
            void set_functor(const piw::data_t &, const change_nb_t &);
            void clear_functor(const piw::data_t &);
            void set_efunctor(const change_nb_t &);
            void clear_efunctor();
            void set_gfunctor(const change_nb_t &);
            void clear_gfunctor();
            cookie_t cookie();
            bct_clocksink_t *get_clock();
            void send_duplicates(bool);
            int gc_traverse(void *,void *) const;
            int gc_clear();
        public:
            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif
