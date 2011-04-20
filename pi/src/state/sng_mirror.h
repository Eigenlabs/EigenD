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

#ifndef __PI_STATE_MIRROR__
#define __PI_STATE_MIRROR__

#include "sng_database.h"
#include "sng_mapping.h"

#include <piw/piw_data.h>
#include <piw/piw_client.h>
#include <piw/piw_thing.h>
#include <piw/piw_state.h>
#include <picross/pic_log.h>
#include <picross/pic_functor.h>

#include <map>
#include <set>

namespace pi
{
    namespace state
    {
        class worker_t: public piw::client_t
        {
            public:
                worker_t(worker_t *p, unsigned char name, const noderef_t &sink, unsigned cflags);
                ~worker_t();

            protected:
                void save_template(noderef_t sink, const mapref_t &mapping);
                void start_save();
                void save();
                piw::term_t add_diff(int ord,noderef_t snap,const mapref_t &map);
                piw::term_t full_state(int n, noderef_t snap, const mapref_t &map);

            protected:

                unsigned hflags();
                bool rtransient();
                bool transient();
                bool writeable();
                bool dynlist();

            private:
                void populate();

            protected:
                void client_tree();
                void close_client();
                void client_data(const piw::data_t &d);
                void client_opened();
                std::string myid();

            private:
                std::map<unsigned char,worker_t *> _clients;
                std::string id_;
                bool idset_;
                worker_t *parent_;
                unsigned char name_;
                noderef_t sink_;
        };

        class manager_t: public worker_t
        {
            public:
                manager_t(const noderef_t &sink);
                ~manager_t();
                void save_template(const noderef_t &sink, const mapref_t &mapping);
                virtual void manager_checkpoint();
                piw::term_t get_diff(const noderef_t &snap, const mapref_t &mapping);
                virtual void client_sync();
                int gc_clear();
                int gc_traverse(void *v, void *a);
            private:
                bool saving_;
        };
    };
};

#endif
