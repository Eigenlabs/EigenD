
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

#include <piw/piw_rpc.h>
#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include "piw_catchkill.h"

void piw::rpcserver_t::rpcserver_attached_thunk(bct_rpcserver_t *t_)
{
}

void piw::rpcserver_t::rpcserver_invoke_thunk(bct_rpcserver_t *t_, bct_entity_t x, void *ctx, bct_data_t path, bct_data_t name, bct_data_t val)
{
    tsd_setcontext(x);
    rpcserver_t *k = (rpcserver_t *)t_;

    try
    {
        k->rpcserver_invoke(rpctoken_t(ctx), piw::data_t::from_lent(path), piw::data_t::from_lent(name), piw::data_t::from_lent(val));
    }
    EVENT_CATCHER(x)
}

void piw::rpcserver_t::rpcserver_closed_thunk(bct_rpcserver_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<rpcserver_t> k = (rpcserver_t *)t_;

    try
    {
        k->rpcserver_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_rpcserver();
        }
        CATCHLOG()
    }
}

bct_rpcserver_plug_ops_t piw::rpcserver_t::dispatch__ = 
{
    rpcserver_attached_thunk,
    rpcserver_invoke_thunk,
    rpcserver_closed_thunk,
};

piw::rpcserver_t::rpcserver_t()
{
    host_ops=0;
    plug_ops=&dispatch__;
    plg_state = PLG_STATE_CLOSED;
}

void piw::rpcclient_t::rpcclient_attached_thunk(bct_rpcclient_t *t_)
{
}

void piw::rpcclient_t::rpcclient_complete_thunk(bct_rpcclient_t *t_, bct_entity_t x, int status, bct_data_t val)
{
    tsd_setcontext(x);
    pic::weak_t<rpcclient_t> k = (rpcclient_t *)t_;

    try
    {
        k->rpcclient_complete(status, piw::data_t::from_lent(val));
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_rpcclient();
        }
        CATCHLOG()
    }
}

bct_rpcclient_plug_ops_t piw::rpcclient_t::dispatch__ = 
{
    rpcclient_attached_thunk,
    rpcclient_complete_thunk,
};

piw::rpcclient_t::rpcclient_t()
{
    host_ops=0;
    plug_ops=&dispatch__;
}
