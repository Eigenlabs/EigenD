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

#ifndef __PIW_WINDOW__
#define __PIW_WINDOW__

#include "piw_exports.h"
#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>

#include "piw_data.h"
#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS window_t: public pic::nocopy_t, public bct_window_t, virtual public pic::tracked_t
    {
        public:
            window_t();
            virtual ~window_t() { tracked_invalidate(); close_window(); }

            void set_window_state(bool o) { PIC_ASSERT(open()); bct_window_host_state(this,o?1:0); }
            void set_window_title(const char *t) { PIC_ASSERT(open()); bct_window_host_title(this,t); }
            virtual void close_window() { if(host_ops) { bct_window_host_close(this); host_ops=0; } }
            inline bool open() { return host_ops!=0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

            void set_state_handler(pic::status_t f) { _fstatus=f; }
            void clear_state_handler() { _fstatus.clear(); }

            int gc_traverse(void *v, void *a) const;
            int gc_clear();

        protected:
            virtual void window_closed() {}
            virtual void window_state(bool o) { _fstatus(o); }

        private:
            static void window_attached_thunk(bct_window_t *);
            static void window_closed_thunk(bct_window_t *, bct_entity_t);
            static void window_state_thunk(bct_window_t *, bct_entity_t, int o);
            static bct_window_plug_ops_t _dispatch;

            pic::status_t _fstatus;
    };
};

#endif
