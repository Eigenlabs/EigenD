
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

#include <piw/piw_client.h>
#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <piw/piw_tsd.h>
#include "piw_catchkill.h"

piw::client_t::~client_t()
{
    tracked_invalidate();
    close_client();
}

std::string piw::client_t::id()
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

void piw::client_t::close_client() 
{
    if(host_ops) 
    {
        bct_client_host_close(this);
        host_ops=0; 
    }
}

const piw::data_t &piw::client_t::get_data() 
{
    return _data; 
}

void piw::client_t::set_tree(const piw::data_t &d)
{
    PIC_ASSERT(open());
    PIC_ASSERT(bct_client_host_tree(this,d.lend()) >= 0);
}

void piw::client_t::set_data(const piw::data_t &d) 
{
    PIC_ASSERT(open());
    PIC_ASSERT(bct_client_host_change(this,d.lend()) >= 0);
}

void piw::client_t::clear_change_handler()
{
    _fchanged.clear();
}

void piw::client_t::set_change_handler(const change_t &f) 
{
    _fchanged=f; 
}

unsigned long piw::client_t::ncrc() 
{
    if(!open()) return 0;
    long tcrc=bct_client_host_child_ncrc(this,0,0); 
    PIC_ASSERT(tcrc>=0);
    return (unsigned long)tcrc;
}

unsigned long piw::client_t::tcrc() 
{
    if(!open()) return 0;
    long tcrc=bct_client_host_child_tcrc(this,0,0); 
    PIC_ASSERT(tcrc>=0);
    return (unsigned long)tcrc;
}

unsigned long piw::client_t::dcrc() 
{
    if(!open()) return 0;
    long dcrc=bct_client_host_child_dcrc(this,0,0); 
    PIC_ASSERT(dcrc>=0);
    return (unsigned long)dcrc;
}

unsigned short piw::client_t::cookie() 
{
    PIC_ASSERT(open());
    int cookie=bct_client_host_cookie(this); 
    PIC_ASSERT(cookie>=0);
    return (unsigned short)cookie;
}

piw::data_t piw::client_t::servername() 
{
    PIC_ASSERT(open());
    bct_data_t d = bct_client_host_server_name(this);
    PIC_ASSERT(d);
    return piw::data_t::from_given(d); 
}

piw::data_t piw::client_t::path() 
{
    PIC_ASSERT(open());
    bct_data_t d = bct_client_host_path(this);
    PIC_ASSERT(d);
    return piw::data_t::from_given(d); 
}

void piw::client_t::child_add(const unsigned char *o, unsigned l, bct_client_t *c) 
{
    PIC_ASSERT(open() && c);
    PIC_ASSERT(bct_client_host_child_add(this,o,l,c)>=0);
}

void piw::client_t::child_add(unsigned char n, bct_client_t *c)
{
    PIC_ASSERT(open() && c);
    PIC_ASSERT(bct_client_host_child_add(this,&n,1,c)>=0);
}

void piw::client_t::child_remove(const unsigned char *o, unsigned l, bct_client_t *c) 
{
    PIC_ASSERT(open() && c);
    PIC_ASSERT(bct_client_host_child_remove(this,o,l,c)>=0);
}

unsigned char piw::client_t::child_enum_child_str(const unsigned char *o, unsigned l, unsigned char ch) 
{
    PIC_ASSERT(open());
    return bct_client_host_child_enum(this,o,l,ch); 
}

unsigned char piw::client_t::enum_child(unsigned char n)
{
    PIC_ASSERT(open());
    return bct_client_host_child_enum(this,0,0,n); 
}

bct_client_t *piw::client_t::child_get(const unsigned char *o, unsigned l) 
{
    PIC_ASSERT(open());
    return bct_client_host_child_get(this,o,l); 
}

void piw::client_t::clone(bct_client_t *c) 
{
    PIC_ASSERT(open());
    PIC_ASSERT(bct_client_host_clone(this,c)>=0);
}

unsigned piw::client_t::child_flags_str(const unsigned char *o, unsigned ol) 
{
    unsigned flags;
    PIC_ASSERT(open());
    piw::data_t d = piw::data_t::from_given(bct_client_host_child_data(this,o,ol,&flags));
    return flags;
}

piw::data_t piw::client_t::child_data_str(const unsigned char *o, unsigned ol) 
{
    PIC_ASSERT(open());
    bct_data_t d = bct_client_host_child_data(this,o,ol,0);
    return piw::data_t::from_given(d); 
}

unsigned long piw::client_t::child_dcrc(const unsigned char *o, unsigned ol) 
{
    if(!open()) return 0;
    long dcrc=bct_client_host_child_dcrc(this,o,ol); 
    PIC_ASSERT(dcrc>=0);
    return (unsigned long)dcrc;
}

unsigned long piw::client_t::child_ncrc(const unsigned char *o, unsigned ol) 
{
    if(!open()) return 0;
    long ncrc=bct_client_host_child_ncrc(this,o,ol); 
    PIC_ASSERT(ncrc>=0);
    return (unsigned long)ncrc;
}

unsigned long piw::client_t::child_tcrc(const unsigned char *o, unsigned ol) 
{
    if(!open()) return 0;
    long dcrc=bct_client_host_child_tcrc(this,o,ol); 
    PIC_ASSERT(dcrc>=0);
    return (unsigned long)dcrc;
}

bool piw::client_t::child_exists_str(const unsigned char *o, unsigned ol) 
{
    PIC_ASSERT(open());
    int e=bct_client_host_child_exists(this,o,ol);
    PIC_ASSERT(e>=0);
    return e>0;
}

void piw::client_t::child_change_str(const unsigned char *o, unsigned ol, const piw::data_t &d) 
{
    PIC_ASSERT(open());
    PIC_ASSERT(bct_client_host_child_change(this,o,ol,d.lend())>=0);
}

void piw::client_t::shutdown() 
{
    if(open())
    {
        bct_client_host_shutdown(this); 
    }
}

void piw::client_t::sync() 
{
    PIC_ASSERT(open());
    bct_client_host_sync(this); 
}

bool piw::client_t::set_downstream(bct_clocksink_t *c)
{
    if(!c && !open())
    {
        return true;
    }

    PIC_ASSERT(open());
    return bct_client_host_set_downstream(this,c) >= 0;
}

void piw::client_t::set_sink(fastdata_t *f)
{
    sink_=f;

    if(open())
    {
        PIC_ASSERT(bct_client_host_set_sink(this,f)>=0);
    }
}

void piw::client_t::client_tree() 
{
}

void piw::client_t::client_child() 
{
}

void piw::client_t::client_data(const piw::data_t &d) 
{
    _fchanged(d); 
}

void piw::client_t::client_sync() 
{
}

void piw::client_t::client_clock() 
{
}

void piw::client_t::attached_thunk(bct_client_t *c, bct_data_t d)
{
    client_t *r = (client_t *)c;

    try
    {
        r->_data = piw::data_t::from_given(d);
    }
    CATCHLOG()
}

void piw::client_t::data_thunk(bct_client_t *c, bct_entity_t x, bct_data_t d)
{
    tsd_setcontext(x);
    client_t *r = (client_t *)c;

    try
    {
        r->_data = piw::data_t::from_lent(d);
        r->client_data(r->_data);
    }
    EVENT_CATCHER(x)
}

void piw::client_t::tree_thunk(bct_client_t *c, bct_entity_t x)
{
    tsd_setcontext(x);
    client_t *r = (client_t *)c;

    try
    {
        r->client_tree();
    }
    EVENT_CATCHER(x)
}

void piw::client_t::child_thunk(bct_client_t *c, bct_entity_t x)
{
    tsd_setcontext(x);
    client_t *r = (client_t *)c;

    try
    {
        r->client_child();
    }
    EVENT_CATCHER(x)
}

void piw::client_t::clock_thunk(bct_client_t *c, bct_entity_t x)
{
    tsd_setcontext(x);
    client_t *r = (client_t *)c;

    try
    {
        r->client_clock();
    }
    EVENT_CATCHER(x)
}

void piw::client_t::sync_thunk(bct_client_t *c, bct_entity_t x)
{
    tsd_setcontext(x);
    client_t *r = (client_t *)c;

    try
    {
        r->client_sync();
    }
    EVENT_CATCHER(x)
}

void piw::client_t::client_opened()
{
}

void piw::client_t::opened_thunk(bct_client_t *c, bct_entity_t x)
{
    tsd_setcontext(x);
    client_t *r = (client_t *)c;

    try
    {
        r->_data=piw::data_t::from_given(bct_client_host_current_slow(r));
    }
    EVENT_CATCHER(x)

    try
    {
        fastdata_t *f = r->sink_.ptr();

        if(f)
        {
            bct_client_host_set_sink(r,f);
        }
    }
    EVENT_CATCHER(x)

    try
    {
        r->client_opened();
    }
    EVENT_CATCHER(x)
}

void piw::client_t::client_closed()
{
    close_client();
}

void piw::client_t::closed_thunk(bct_client_t *c, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<client_t> r = (client_t *)c;

    try
    {
        r->client_closed();
    }
    CATCHLOG()

    if(r.isvalid() && r->open())
    {
        try
        {
            r->close_client();
        }
        CATCHLOG()
    }
}

bct_client_plug_ops_t piw::client_t::_dispatch = 
{
    attached_thunk,
    data_thunk,
    tree_thunk,
    opened_thunk,
    closed_thunk,
    sync_thunk,
    clock_thunk,
    child_thunk
};

piw::client_t::client_t(unsigned flags)
{
    plug_ops = &_dispatch;
    host_ops=0;
    plg_state = PLG_STATE_CLOSED;
    plug_flags=flags;
}
