
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

#ifndef __PIW_OUTPUT__
#define __PIW_OUTPUT__

#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_server.h>
#include <piw/piw_fastdata.h>
#include <map>

namespace piw
{
    class PIW_DECLSPEC_CLASS splitter_node_t: public server_t
    {
        public:
            splitter_node_t();
            ~splitter_node_t();
            void add_signal(unsigned v, server_t *);
            void remove_signal(unsigned v);
            void server_opened();
            void close_server();
        private:
            std::map<unsigned, pic::weak_t<server_t> > signals_;
            splitter_node_t *extension_;
    };

    class PIW_DECLSPEC_CLASS splitter_t
    {
        public:
            splitter_t();
            virtual ~splitter_t();
            cookie_t cookie();
            void add_signal(unsigned signal,splitter_node_t *root);
            void remove_signal(unsigned signal);
            unsigned get_latency();
            bct_clocksink_t *get_clock();
            virtual void set_latency(unsigned latency) {}
            virtual void set_clock(bct_clocksink_t *clock) {}
        public:
            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif
