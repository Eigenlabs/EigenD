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

#ifndef __PIW_CLIENT__
#define __PIW_CLIENT__
#include "piw_exports.h"
#include <pibelcanto/plugin.h>

#include <string>
#include <ostream>

#include "piw_data.h"
#include "piw_fastdata.h"

#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS client_t: public pic::nocopy_t, public bct_client_t, virtual public pic::tracked_t
    {
        public:
            client_t(unsigned flags = 0);
            virtual ~client_t();

        public:
            virtual void close_client();
            bool open() { return host_ops!=0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

            const data_t &get_data();

            unsigned get_host_flags() { return host_flags; }
            unsigned get_plug_flags() { return plug_flags; }

            void set_change_handler(const change_t &f);
            void clear_change_handler();

            int gc_traverse(void *v, void *a) const { return _fchanged.gc_traverse(v,a); }
            int gc_clear() { return _fchanged.gc_clear(); }

            piw::data_t servername();
            piw::data_t servername_fq();
            piw::data_t path();
            void child_add(const unsigned char *o, unsigned l, bct_client_t *c);
            void child_add(unsigned char n, bct_client_t *c);
            void child_remove(const unsigned char *o, unsigned l, bct_client_t *c);
            unsigned char enum_child(unsigned char n);
            bct_client_t *child_get(const unsigned char *o, unsigned l);
            void clone(bct_client_t *c);

            void child_add_str(const unsigned char *o, unsigned l, bct_client_t *c) { child_add(o,l,c); }
            unsigned char child_enum_child_str(const unsigned char *o, unsigned l, unsigned char ch);
            piw::data_t child_data_str(const unsigned char *o, unsigned ol);
            bool child_exists_str(const unsigned char *o, unsigned ol);
            unsigned child_flags_str(const unsigned char *o, unsigned ol);
            unsigned long child_dcrc(const unsigned char *o, unsigned ol);
            unsigned long child_tcrc(const unsigned char *o, unsigned ol);
            unsigned long child_ncrc(const unsigned char *o, unsigned ol);

            unsigned short cookie();
            void shutdown();
            void set_sink(fastdata_t *f);
            void clear_sink() { set_sink(0); }

            bool set_downstream(bct_clocksink_t *);
            void clear_downstream() { set_downstream(0); }

            void sync();

            unsigned long tcrc();
            unsigned long dcrc();
            unsigned long ncrc();

            std::string id();

        protected:
            virtual void client_tree();
            virtual void client_child();
            virtual void client_data(const piw::data_t &);
            virtual void client_opened();
            virtual void client_closed();
            virtual void client_sync();
            virtual void client_clock();

        private:
            static void attached_thunk(bct_client_t *, bct_data_t);
            static void tree_thunk(bct_client_t *, bct_entity_t);
            static void data_thunk(bct_client_t *, bct_entity_t, bct_data_t);
            static void opened_thunk(bct_client_t *, bct_entity_t);
            static void closed_thunk(bct_client_t *, bct_entity_t);
            static void sync_thunk(bct_client_t *, bct_entity_t);
            static void clock_thunk(bct_client_t *, bct_entity_t);
            static void child_thunk(bct_client_t *, bct_entity_t);

            static bct_client_plug_ops_t _dispatch;

        private:
            change_t _fchanged;
            piw::data_t _data;
            pic::weak_t<fastdata_t> sink_;
    };
};

PIW_DECLSPEC_FUNC(std::ostream) &operator<<(std::ostream &, piw::client_t &);

#endif
