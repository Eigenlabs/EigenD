
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

#include "lng_wordrec.h"

#include <piw/piw_keys.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_thing.h>

#include <map>
#include <cmath>

struct wordrec_wire_t;

struct wordrec_wire_t: piw::wire_t, piw::event_data_sink_t, piw::fastdata_t, virtual public pic::lckobject_t
{
    wordrec_wire_t(language::wordrec_t::impl_t *i,const piw::event_data_source_t &es): fastdata_t(PLG_FASTDATA_SENDER), impl_(i)
    {
        subscribe(es);
        piw::tsd_fastdata(this);
        enable(true,false,false);
    }

    ~wordrec_wire_t() { invalidate(); }

    void event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei)
    {
        send_fast(d,ei.signal(1));
    }

    void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
    {
        if(s==1)
            send_fast(current_id(),n);
    }

    bool event_end(unsigned long long t)
    {
        return true;
    }

    bool fastdata_receive_event(const piw::data_nb_t &d,const piw::dataqueue_t &q)
    {
        ping(d.time(),q);
        return true;
    }

    bool fastdata_receive_data(const piw::data_nb_t &d);

    unsigned get_key(const piw::data_nb_t &d,bool &hard);
    void wire_closed() { delete this; }
    void invalidate() { unsubscribe(); disconnect(); }

    language::wordrec_t::impl_t *impl_;
};

struct language::wordrec_t::impl_t: piw::thing_t, piw::decoder_t, piw::decode_ctl_t, virtual pic::lckobject_t
{
    impl_t(const piw::change_t &c): decoder_t(this), functor_(c) { piw::tsd_thing(this); }
    ~impl_t() { tracked_invalidate(); }
    piw::wire_t *wire_create(const piw::event_data_source_t &es) { return new wordrec_wire_t(this,es); }
    void set_clock(bct_clocksink_t *s) { }
    void set_latency(unsigned l) { }

    void key_pressed(unsigned id, bool hard);
    void thing_dequeue_slow(const piw::data_t &d) { functor_(d); }

    piw::change_t functor_;
};

unsigned wordrec_wire_t::get_key(const piw::data_nb_t &d, bool &hard)
{
    float mc,mr;
    piw::hardness_t h;

    if(!piw::decode_key(d,0,0,&mr,&mc,&h) || h==piw::KEY_LIGHT)
    {
        return 0;
    }

    hard = (h==piw::KEY_HARD);
    pic::logmsg() << "k " << mr << ' ' << mc << ' ' << hard;

    if(mr==1 && mc>=1 && mc<=8)
    {
        return mc;
    }

    if(mr==2 && mc>=1 && mc<=2)
    {
        return 9+mc-1;
    }

    return 0;
}

bool wordrec_wire_t::fastdata_receive_data(const piw::data_nb_t &d)
{
    bool hard;
    int key;

    if((key=get_key(d,hard))>0)
    {
        impl_->key_pressed(key,hard);
        return false;
    }

    return true;
}

void language::wordrec_t::impl_t::key_pressed(unsigned id, bool hard)
{
    if(id<=10)
    {
        if(hard)
        {
            id+=1000;
        }

        enqueue_slow_nb(piw::makelong_nb(id,0));
    }
}

language::wordrec_t::wordrec_t(const piw::change_t &change): impl_(new impl_t(change))
{
}

language::wordrec_t::~wordrec_t()
{
    delete impl_;
}

piw::cookie_t language::wordrec_t::cookie()
{
    return impl_->cookie();
}

int language::wordrec_t::gc_clear()
{
    impl_->functor_.gc_clear();
    return 0;
}

int language::wordrec_t::gc_traverse(void *v, void *a) const
{
    return impl_->functor_.gc_traverse(v,a);
}
