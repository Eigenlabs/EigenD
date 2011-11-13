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

#ifndef __PIW_INDEX__
#define __PIW_INDEX__

#include "piw_exports.h"
#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>

#include <picross/pic_weak.h>
#include "piw_data.h"
#include <picross/pic_nocopy.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS index_t: public pic::nocopy_t, public bct_index_t, virtual public pic::tracked_t
    {
        public:
            index_t();
            virtual ~index_t() { close_index(); }

            void set_change_handler(const pic::notify_t &f) { _fchange=f; }
            void clear_change_handler() { _fchange.clear(); }

            inline piw::data_t index_name() { PIC_ASSERT(open()); return piw::data_t::from_given(bct_index_host_name(this,0)); }
            inline piw::data_t member_name(int i) { PIC_ASSERT(open()); return piw::data_t::from_given(bct_index_host_member_name(this,i,0)); }
            inline piw::data_t index_name_fq() { PIC_ASSERT(open()); return piw::data_t::from_given(bct_index_host_name(this,1)); }
            inline piw::data_t member_name_fq(int i) { PIC_ASSERT(open()); return piw::data_t::from_given(bct_index_host_member_name(this,i,1)); }
            inline int member_count() { PIC_ASSERT(open()); return bct_index_host_member_count(this); }
            inline unsigned short member_cookie(int i) { PIC_ASSERT(open()); return bct_index_host_member_cookie(this,i); }
            inline void member_died(const piw::data_t &n, unsigned short c) { PIC_ASSERT(open()); bct_index_host_member_died(this,n.lend(),c); }

            virtual void close_index() { if(host_ops) { bct_index_host_close(this); host_ops=0; } }
            bool open() { return host_ops!=0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

            int gc_traverse(void *v, void *a) const { return _fchange.gc_traverse(v,a); }
            int gc_clear() { return _fchange.gc_clear(); }

        protected:
            virtual void index_opened() {}
            virtual void index_closed();
            virtual void index_changed() { _fchange(); }

        private:
            static bct_index_plug_ops_t _dispatch;
            static void attached_thunk(bct_index_t *);
            static void changed_thunk(bct_index_t *, bct_entity_t);
            static void opened_thunk(bct_index_t *, bct_entity_t);
            static void closed_thunk(bct_index_t *, bct_entity_t);

            pic::notify_t _fchange;
    };
};

#endif
