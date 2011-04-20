
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

#include <piw/piw_index.h>
#include <picross/pic_log.h>
#include <piw/piw_tsd.h>
#include "piw_catchkill.h"

void piw::index_t::attached_thunk(bct_index_t *g)
{
}

void piw::index_t::opened_thunk(bct_index_t *g, bct_entity_t x)
{
    tsd_setcontext(x);
    index_t *r = (index_t *)g;

    try
    {
        r->index_opened();
    }
    EVENT_CATCHER(x)
}

void piw::index_t::index_closed()
{
    close_index();
}

void piw::index_t::closed_thunk(bct_index_t *g, bct_entity_t x)
{
    tsd_setcontext(x);
    pic::weak_t<index_t> r = (index_t *)g;

    try
    {
        r->index_closed();
    }
    CATCHLOG()

    if(r.isvalid() && r->open())
    {
        try
        {
            r->close_index();
        }
        CATCHLOG()
    }
}

void piw::index_t::changed_thunk(bct_index_t *g, bct_entity_t x)
{
    tsd_setcontext(x);
    index_t *r = (index_t *)g;

    try
    {
        r->index_changed();
    }
    EVENT_CATCHER(x)
}

bct_index_plug_ops_t piw::index_t::_dispatch = 
{
    attached_thunk,
    opened_thunk,
    closed_thunk,
    changed_thunk
};

piw::index_t::index_t()
{
    host_ops=0;
    plug_ops=&_dispatch;
    plg_state = PLG_STATE_CLOSED;
}
