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

#ifndef __LNG_WORDREC__
#define __LNG_WORDREC__

#include <piw/piw_bundle.h>
#include <pilanguage_exports.h>

namespace language
{
    class PILANGUAGE_DECLSPEC_CLASS wordrec_t
    {
        public:
            class impl_t;
        public:
            wordrec_t(const piw::change_t &);
            ~wordrec_t();
            piw::cookie_t cookie();
            int gc_traverse(void *,void *) const;
            int gc_clear();
        private:
            impl_t *impl_;
    };
};

#endif
