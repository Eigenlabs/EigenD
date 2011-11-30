
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

#ifndef __PIW_CONNECTOR__
#define __PIW_CONNECTOR__

#include <piw/piw_server.h>
#include <piw/piw_client.h>
#include <piw/piw_data.h>

namespace rig
{
    class output_t: public piw::server_t
    {
        public:
            output_t();
            virtual ~output_t();
    };

    class connector_t: public piw::client_t
    {
        public:
            connector_t(output_t *output,unsigned index, const piw::d2d_nb_t &filter);
            virtual ~connector_t();
            void set_clocked(bool c);

            int gc_traverse(void *, void *) const;
            int gc_clear();

        protected:
            void client_opened();
            void client_clock();
            void close_client();

        public:
            class impl_t;

        private:
            impl_t *impl_;
    };
}

#endif

