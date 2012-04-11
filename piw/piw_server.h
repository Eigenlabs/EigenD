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

#ifndef __PIW_SERVER__
#define __PIW_SERVER__
#include "piw_exports.h"

#include <string>
#include <cstdlib>

#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>

#include "piw_data.h"
#include "piw_fastdata.h"
#include "piw_clock.h"
#include "piw_state.h"

#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_log.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS server_t: public pic::nocopy_t, public bct_server_t, virtual public pic::tracked_t
    {
        public:
            server_t(unsigned flags = PLG_SERVER_RO);
            virtual ~server_t();

        public:
            void set_data(const piw::data_t &d);
            const piw::data_t &get_data() { return data_; }
            bool has_value() const { return !data_.is_null(); }
            void set_change_handler(change_t f) { fchanged_=f; }
            void clear_change_handler() { fchanged_.clear(); }
            void set_flags(unsigned flags);
            unsigned get_flags() { return flags_; }
            void set_readwrite() { set_flags(get_flags()&~PLG_SERVER_RO); }
            void set_readonly() { set_flags(get_flags()|PLG_SERVER_RO); }

            int gc_traverse(void *v, void *a) const;
            int gc_clear();

        public:
            virtual void close_server();
            bool open() { return host_ops!=0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

        public:
            piw::data_t servername() { PIC_ASSERT(running()); return piw::data_t::from_given(bct_server_host_servername(this,0)); }
            piw::data_t servername_fq() { PIC_ASSERT(running()); return piw::data_t::from_given(bct_server_host_servername(this,1)); }
            piw::data_t path() { PIC_ASSERT(running()); return piw::data_t::from_given(bct_server_host_path(this)); }
            void child_add(unsigned char n, bct_server_t *c) { PIC_ASSERT(open()); PIC_ASSERT(bct_server_host_child_add(this,n,c)>=0); }
            void shutdown() { if(open()) bct_server_host_shutdown(this); }
            void advertise(const char *i) { PIC_ASSERT(open()); bct_server_host_advertise(this,i); }
            void unadvertise(const char *i) { PIC_ASSERT(open()); bct_server_host_unadvertise(this,i); }
            void cancel() { PIC_ASSERT(open()); bct_server_host_cancel(this); }
            void set_clock(bct_clocksink_t *c);
            void clear_clock() { set_clock(0); }
            void set_source(fastdata_t *f);
            void clear_source() { set_source(0); }

            std::string id();

        protected:
            virtual void server_opened() {}
            virtual void server_closed() {}
            virtual void server_change(const data_t &d) { fchanged_(d); }

        private:
            static bct_data_t attached_thunk(bct_server_t *, unsigned *);
            static void closed_thunk(bct_server_t *s, bct_entity_t);
            static void opened_thunk(bct_server_t *s, bct_entity_t);
            static bct_server_plug_ops_t _dispatch;

        private:
            piw::data_t data_;
            change_t fchanged_;
            unsigned flags_;
            pic::weak_t<fastdata_t> data_source_;
            bct_clocksink_t *clock_;
    };
};

#endif
