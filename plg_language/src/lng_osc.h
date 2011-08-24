
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

#ifndef __LNG_OSC__
#define __LNG_OSC__

#include <plg_language/src/lng_exports.h>
#include <piw/piw_fastdata.h>

namespace language
{
    class PILANGUAGE_DECLSPEC_CLASS oscserver_t
    {
        public:
            class impl_t;

        public:
            oscserver_t(const char *server_port, const char *xmlrpc_server_port);
            ~oscserver_t();

            unsigned add_widget(const char *name, piw::fastdata_t *sender, piw::fastdata_t *receiver);
            void del_widget(unsigned);
            void startup();
            void shutdown();
            void send(const char *name,float v);
            void set_connected(unsigned index, bool is_connected);
        
        private:
            impl_t *impl_;
    };
};

#endif
