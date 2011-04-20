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

#ifndef __PIA_SRC_RPC__
#define __PIA_SRC_RPC__

#include <pibelcanto/plugin.h>
#include "pia_manager.h"

struct pia_ctx_t;
struct pia_data_t;

struct pia_rpclist_t
{
    public:
        struct impl_t;

    public:
        pia_rpclist_t(pia::manager_t::impl_t *);
        ~pia_rpclist_t();
        void kill(const pia_ctx_t &e);
        void dump(const pia_ctx_t &e);
        void server(const pia_ctx_t &e, bct_rpcserver_t *s, const pia_data_t &id);
        void client(const pia_ctx_t &e, bct_rpcclient_t *s, const pia_data_t &id, const pia_data_t &path, const pia_data_t &n, const pia_data_t &val, unsigned long);

    private:
        impl_t *impl_;

};

#endif
