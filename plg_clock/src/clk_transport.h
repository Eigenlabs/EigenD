
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

#ifndef __CLK_TRANSPORT__
#define __CLK_TRANSPORT__

#include <piw/piw_bundle.h>

namespace clocks
{
    class transport_t : public pic::nocopy_t
    {
        public:
            class impl_t;
        public:
            transport_t(const piw::cookie_t &c);
            ~transport_t();
            void start();
            void stop();
            void rewind();
        private:
            impl_t *impl_;
    };
}
#endif
