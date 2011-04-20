
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

#include <piw/piw_fastdata.h>
#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include <piw/piw_tsd.h>
#include "piw_catchkill.h"

bool piw::fastdata_t::fastdata_receive_event(const piw::data_nb_t &d,const dataqueue_t &q)
{
    return ichanged_(d,q);
}

bool piw::fastdata_t::fastdata_receive_data(const piw::data_nb_t &d)
{
    fchanged_(d);
    return true;
}

int piw::fastdata_t::gc_traverse(void *v, void *a) const
{
    int r;
    if((r=ichanged_.gc_traverse(v,a))!=0) return r;
    if((r=fchanged_.gc_traverse(v,a))!=0) return r;
    return 0;
}

int piw::fastdata_t::gc_clear()
{
    ichanged_.gc_clear();
    fchanged_.gc_clear();
    return 0;
}

void piw::fastdata_t::fastdata_attached_thunk(bct_fastdata_t *t_)
{
}

int piw::fastdata_t::fastdata_receive_event_thunk(void *t_, bct_entity_t x, bct_data_t d, bct_dataqueue_t q)
{
    tsd_setcontext(x);
    fastdata_t *k = (fastdata_t *)t_;

    try
    {
        return k->fastdata_receive_event(piw::data_nb_t::from_lent(d),dataqueue_t::from_lent(q))?1:0;
    }
    CATCHLOG()
    return false;
}

int piw::fastdata_t::fastdata_receive_data_thunk(void *t_, bct_entity_t x, bct_data_t d)
{
    tsd_setcontext(x);
    fastdata_t *k = (fastdata_t *)t_;

    try
    {
        return k->fastdata_receive_data(piw::data_nb_t::from_lent(d))?1:0;
    }
    CATCHLOG()
    return false;
}

void piw::fastdata_t::fastdata_closed_thunk(bct_fastdata_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<fastdata_t> k = (fastdata_t *)t_;

    try
    {
        k->fastdata_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_fastdata();
        }
        CATCHLOG()
    }
}

piw::fastdata_t::fastdata_t(int (*rcv)(void *,bct_entity_t,bct_data_t,bct_dataqueue_t), int (*rcv2)(void *,bct_entity_t,bct_data_t),void *rcvctx, unsigned short flags)
{
    host_ops=0;
    plug_ops=&dispatch_;
    dispatch_.fastdata_attached = fastdata_attached_thunk;
    dispatch_.fastdata_receive_event = rcv;
    dispatch_.fastdata_receive_data = rcv2;
    dispatch_.fastdata_closed = fastdata_closed_thunk;
    dispatch_.fastdata_receive_context = rcvctx;
    plug_flags=flags;
    sender_=piw::f_event_sender_t::method(this,&piw::fastdata_t::send_fast);
}

piw::fastdata_t::fastdata_t(unsigned short flags)
{
    host_ops=0;
    plug_ops=&dispatch_;
    dispatch_.fastdata_attached = fastdata_attached_thunk;
    dispatch_.fastdata_receive_event = fastdata_receive_event_thunk;
    dispatch_.fastdata_receive_data = fastdata_receive_data_thunk;
    dispatch_.fastdata_closed = fastdata_closed_thunk;
    dispatch_.fastdata_receive_context = this;
    plug_flags=flags;
    sender_=piw::f_event_sender_t::method(this,&piw::fastdata_t::send_fast);
}

void piw::fastdata_t::ping(unsigned long long time,const piw::dataqueue_t &q)
{
    piw::data_nb_t d;
    unsigned long long i=0;

    if(q.latest(d,&i,time))
    {
        //fastdata_receive_data(d.restamp(time));
        fastdata_receive_data(d);
        i++;
    }
    else
    {
        if(!q.earliest(d,&i,time))
        {
            return;
        }

        fastdata_receive_data(d);
        i++;
    }

    while(q.read(d,&i,~0ULL))
    {
        fastdata_receive_data(d);
        i++;
    }
}
