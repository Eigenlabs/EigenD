
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

#include <piw/piw_server.h>
#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include "piw_catchkill.h"

std::string piw::server_t::id()
{
    std::ostringstream s;
    piw::data_t p = path();

    s << servername().as_string();

    unsigned pl = p.as_pathlen();

    if(pl>0)
    {
        const unsigned char *pd = p.as_path();
        bool f = true;

        while(pl>0)
        {
            s << (f?'#':'.') << (int)(pd[0]);
            f=false;
            pd++;
            pl--;
        }
    }

    return s.str();
}

piw::server_t::~server_t()
{
    tracked_invalidate();
    close_server();
}

int piw::server_t::gc_traverse(void *v, void *a) const { return fchanged_.gc_traverse(v,a); }
int piw::server_t::gc_clear() { return fchanged_.gc_clear(); }

void piw::server_t::close_server()
{
    if(host_ops)
    {
        bct_server_host_close(this);
        host_ops=0;
    }
}

void piw::server_t::set_source(fastdata_t *f)
{
    data_source_=f;

    if(f)
    {
        set_flags(get_flags()|PLG_SERVER_FAST);
    }
    else
    {
        set_flags(get_flags()&~PLG_SERVER_FAST);
    }

    if(open())
    {
        bct_server_host_set_source(this,f);
    }
}

void piw::server_t::set_flags(unsigned f)
{
    if(flags_!=f)
    {
        flags_=f;
        if(open()) bct_server_host_setflags(this,f);
    }
}

void piw::server_t::set_data(const piw::data_t &d)
{
    data_=d;
    if(open()) bct_server_host_changed(this,d.lend()); 
}

bct_data_t piw::server_t::attached_thunk(bct_server_t *s, unsigned *flags)
{
    server_t *r = (server_t *)s;
    *flags = r->flags_;
    return r->data_.give_copy();
}

void piw::server_t::opened_thunk(bct_server_t *s, bct_entity_t x)
{
    tsd_setcontext(x);
    server_t *r = (server_t *)s;
    fastdata_t *f;

    try
    {
        f = r->data_source_.ptr();

        if(f)
        {
            bct_server_host_set_source(r,f);
        }

        if(r->clock_)
        {
            bct_server_host_setclock(r,r->clock_);
        }

        r->server_opened();
    }
    EVENT_CATCHER(x)
}

void piw::server_t::closed_thunk(bct_server_t *s, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<server_t> r = (server_t *)s;

    try
    {
        r->server_closed();
    }
    CATCHLOG()

    if(r.isvalid() && r->open())
    {
        try
        {
            r->close_server();
        }
        CATCHLOG()
    }
}

bct_server_plug_ops_t piw::server_t::_dispatch = 
{
    attached_thunk,
    opened_thunk,
    closed_thunk
};

void piw::server_t::set_clock(bct_clocksink_t *clock)
{
    clock_ = clock;

    if(open())
    {
        bct_server_host_setclock(this,clock_);
    }
}

piw::server_t::server_t(unsigned f)
{
    host_ops=0;
    plug_ops=&_dispatch;
    plg_state = PLG_STATE_CLOSED;
    flags_=f;
    clock_=0;
    data_source_=0;
}
