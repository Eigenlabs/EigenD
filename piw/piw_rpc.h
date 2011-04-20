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

#ifndef __PIW_RPC__
#define __PIW_RPC__

#include "piw_exports.h"
#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>

#include "piw_data.h"
#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS rpctoken_t
    {
        public:
            rpctoken_t(): token_(0) {}
            rpctoken_t(const rpctoken_t &t): token_(t.token_) {}
            rpctoken_t(void *vp): token_(vp) {}
            ~rpctoken_t() {}
            rpctoken_t &operator=(const rpctoken_t &r) { token_=r.token_; return *this; }
            rpctoken_t &operator=(void *vp) { token_=vp; return *this; }
            void *as_voidptr() const { return token_; }
            bool operator==(const rpctoken_t &r) const { return token_==r.token_; }
            bool operator<(const rpctoken_t &r) const { return token_<r.token_; }

        private:
            void *token_;
    };

    class PIW_DECLSPEC_CLASS rpcserver_t: public pic::nocopy_t, public bct_rpcserver_t, virtual public pic::tracked_t
    {
        public:
            rpcserver_t();
            virtual ~rpcserver_t() { tracked_invalidate(); close_rpcserver(); }

            void completed(const rpctoken_t &ctx, bool status, const piw::data_t &val) { PIC_ASSERT(open()); bct_rpcserver_host_complete(this,ctx.as_voidptr(),status?1:0,val.lend()); }
            virtual void close_rpcserver() { if(host_ops) { bct_rpcserver_host_close(this); host_ops=0; } }
            inline bool open() { return host_ops!=0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

        protected:
            virtual void rpcserver_invoke(const rpctoken_t &ctx, const piw::data_t &name, const piw::data_t &path, const piw::data_t &val) {}
            virtual void rpcserver_closed() {}

        private:
            static void rpcserver_attached_thunk(bct_rpcserver_t *);
            static void rpcserver_invoke_thunk(bct_rpcserver_t *, bct_entity_t, void *, bct_data_t, bct_data_t, bct_data_t);
            static void rpcserver_closed_thunk(bct_rpcserver_t *, bct_entity_t);

            static bct_rpcserver_plug_ops_t dispatch__;
    };

    class PIW_DECLSPEC_CLASS rpcclient_t: public pic::nocopy_t, public bct_rpcclient_t, virtual public pic::tracked_t
    {
        public:
            rpcclient_t();
            virtual ~rpcclient_t() { tracked_invalidate(); close_rpcclient(); }

            virtual void close_rpcclient() { if(host_ops) { bct_rpcclient_host_close(this); host_ops=0; } }
            inline bool open() { return host_ops!=0; }

        protected:
            virtual void rpcclient_complete(int status, const piw::data_t &val) {}

        private:
            static void rpcclient_attached_thunk(bct_rpcclient_t *);
            static void rpcclient_complete_thunk(bct_rpcclient_t *, bct_entity_t, int status, bct_data_t val);

            static bct_rpcclient_plug_ops_t dispatch__;
    };
};

#endif
