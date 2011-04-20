
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

#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include "piw_catchkill.h"

int piw::thing_t::gc_traverse(void *v, void *a) const
{
    int r;
    if((r=_fftimer.gc_traverse(v,a))!=0) return r;
    if((r=_fstimer.gc_traverse(v,a))!=0) return r;
    if((r=_fftrigger.gc_traverse(v,a))!=0) return r;
    if((r=_fstrigger.gc_traverse(v,a))!=0) return r;
    if((r=_ffdequeue.gc_traverse(v,a))!=0) return r;
    if((r=_fsdequeue.gc_traverse(v,a))!=0) return r;
    return 0;
}

int piw::thing_t::gc_clear()
{
    _fftimer.gc_clear();
    _fstimer.gc_clear();
    _fftrigger.gc_clear();
    _fstrigger.gc_clear();
    _ffdequeue.gc_clear();
    _fsdequeue.gc_clear();
    return 0;
}

void piw::thing_t::thing_attached_thunk(bct_thing_t *t_)
{
}

void piw::thing_t::thing_dequeue_fast_thunk(bct_thing_t *t_, bct_entity_t x, bct_data_t d)
{
    tsd_setcontext(x);
    thing_t *k = (thing_t *)t_;

    try
    {
        k->thing_dequeue_fast(piw::data_nb_t::from_lent(d));
    }
    CATCHLOG()
}

void piw::thing_t::thing_dequeue_slow_thunk(bct_thing_t *t_, bct_entity_t x, bct_data_t d)
{
    tsd_setcontext(x);
    thing_t *k = (thing_t *)t_;

    try
    {
        k->thing_dequeue_slow(piw::data_t::from_lent(d));
    }
    EVENT_CATCHER(x)
}

void piw::thing_t::thing_triggered_fast_thunk(bct_thing_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    thing_t *k = (thing_t *)t_;

    try
    {
        k->thing_trigger_fast();
    }
    CATCHLOG()
}

void piw::thing_t::thing_triggered_slow_thunk(bct_thing_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    thing_t *k = (thing_t *)t_;

    try
    {
        k->thing_trigger_slow();
    }
    EVENT_CATCHER(x)
}

void piw::thing_t::thing_timer_fast_thunk(bct_thing_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    thing_t *k = (thing_t *)t_;

    try
    {
        k->thing_timer_fast();
    }
    CATCHLOG()
}

void piw::thing_t::thing_timer_slow_thunk(bct_thing_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    thing_t *k = (thing_t *)t_;

    try
    {
        k->thing_timer_slow();
    }
    EVENT_CATCHER(x)
}

void piw::thing_t::thing_closed_thunk(bct_thing_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<thing_t> k = (thing_t *)t_;

    try
    {
        k->thing_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_thing();
        }
        CATCHLOG()
    }
}

bct_thing_plug_ops_t piw::thing_t::_dispatch = 
{
    thing_attached_thunk,
    thing_triggered_fast_thunk,
    thing_triggered_slow_thunk,
    thing_dequeue_fast_thunk,
    thing_dequeue_slow_thunk,
    thing_timer_fast_thunk,
    thing_timer_slow_thunk,
    thing_closed_thunk,
};

piw::thing_t::thing_t()
{
    host_ops=0;
    plug_ops=&_dispatch;
}
