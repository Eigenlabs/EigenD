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

#ifndef __PIW_CANCHOR__
#define __PIW_CANCHOR__
#include "piw_exports.h"
#include "piw_client.h"
#include "piw_thing.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS canchor_t
    {
        public:
            class impl_t;

        public:
            canchor_t();
            ~canchor_t();

            bool set_address(const piw::data_nb_t &a);
            bool set_address_str(const std::string &);
            std::string get_address();
            void set_client(client_t *);
            void set_slow_client(client_t *);

        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS subcanchor_t: public piw::client_t, private piw::thing_t
    {
        public:
            subcanchor_t();
            ~subcanchor_t();

            bool set_path(const std::string &);
            bool clear_path();
            std::string get_path() { return path_; }
            void set_client(client_t *);
            void clear_client() { set_client(0); }

            void client_sync();
            void close_client();

            int gc_clear()
            {
                piw::thing_t::gc_clear();
                piw::client_t::gc_clear();
                return 0;
            }

            int gc_traverse(void *v, void *a)
            {
                int r;
                if((r=piw::thing_t::gc_traverse(v,a))!=0) return r;
                if((r=piw::client_t::gc_traverse(v,a))!=0) return r;
                return 0;
            }

        private:
            void thing_trigger_slow();
            void synchronise();

            pic::weak_t<client_t> client_;
            std::string path_;
            int pathlen_;
    };
};

#endif
