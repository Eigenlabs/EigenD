
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

#include <piw/piw_window.h>
#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include "piw_catchkill.h"

int piw::window_t::gc_traverse(void *v, void *a) const
{
    int r;
    if((r=_fstatus.gc_traverse(v,a))!=0) return r;
    return 0;
}

int piw::window_t::gc_clear()
{
    _fstatus.gc_clear();
    return 0;
}
void piw::window_t::window_attached_thunk(bct_window_t *t_)
{
}

void piw::window_t::window_state_thunk(bct_window_t *t_, bct_entity_t x, int o)
{
    tsd_setcontext(x);
    window_t *k = (window_t *)t_;

    try
    {
        k->window_state(o?true:false);
    }
    CATCHLOG()
}

void piw::window_t::window_closed_thunk(bct_window_t *t_, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<window_t> k = (window_t *)t_;

    try
    {
        k->window_closed();
    }
    CATCHLOG()

    if(k.isvalid() && k->open())
    {
        try
        {
            k->close_window();
        }
        CATCHLOG()
    }
}

bct_window_plug_ops_t piw::window_t::_dispatch = 
{
    window_attached_thunk,
    window_state_thunk,
    window_closed_thunk,
};

piw::window_t::window_t()
{
    host_ops=0;
    plg_state = PLG_STATE_CLOSED;
    plug_ops=&_dispatch;
}
