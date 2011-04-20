
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

#include <piw/piw_gate.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>

#include <map>

namespace {
    struct gate_wire_t;

    struct gate_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_sink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        gate_wire_t(piw::gate_t::impl_t *p, const piw::event_data_source_t &);
        ~gate_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);

        void source_ended(unsigned seq);
        void activate(bool b, unsigned long long t);

        piw::gate_t::impl_t *parent_;
        unsigned seq_;
    };
};

struct piw::gate_t::impl_t: root_t, root_ctl_t, virtual pic::lckobject_t, virtual pic::tracked_t
{
    impl_t(const piw::cookie_t &c, bool init): root_t(0), on_(init)
    {
        connect(c);
    }

    ~impl_t() { tracked_invalidate(); invalidate(); }

    void invalidate()
    {
        pic::lckmap_t<piw::data_t,gate_wire_t *>::lcktype::iterator ci;

        while((ci=children_.alternate().begin())!=children_.alternate().end())
        {
            delete ci->second;
        }

    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
        pic::lckmap_t<piw::data_t,gate_wire_t *>::lcktype::iterator ci;

        if((ci=children_.alternate().find(es.path()))!=children_.alternate().end())
        {
            delete ci->second;
        }

        return new gate_wire_t(this,es);
    }

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }

    void root_clock()
    {
        set_clock(get_clock());
    }

    void root_latency()
    {
        set_latency(get_latency());
    }

    void gate(const piw::data_nb_t &d)
    {
        if(d.as_arraylen() == 0) return;

        bool on = (d.as_norm()!=0.0);

        gate__(on,d.time());
    }

    void gate__(bool on, unsigned long long t)
    {
        if(on == on_) return;

        on_ = on;

        if(!t) t=piw::tsd_time();

        pic::flipflop_t<pic::lckmap_t<piw::data_t, gate_wire_t *>::lcktype>::guard_t wg(children_);
        pic::lckmap_t<piw::data_t,gate_wire_t *>::lcktype::const_iterator wi;

        for(wi=wg.value().begin(); wi!=wg.value().end(); wi++)
        {
            wi->second->activate(on_,t);
        }
    }

    static int __enabler(void *r, void *e)
    {
        impl_t *root = (impl_t *)r;
        bool on = *(bool *)e;
        root->gate__(on,0);
        return 0;
    }

    void enable(bool on)
    {
        piw::tsd_fastcall(__enabler,this,&on);
    }

    pic::flipflop_t<pic::lckmap_t<piw::data_t, gate_wire_t *>::lcktype> children_;
    bool on_;
};

gate_wire_t::gate_wire_t(piw::gate_t::impl_t *p, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), parent_(p)
{
    parent_->children_.alternate().insert(std::make_pair(path(),this));
    parent_->children_.exchange();
    parent_->connect_wire(this,source());
    subscribe_and_ping(es);
}

void gate_wire_t::invalidate()
{
    source_shutdown();
    unsubscribe();

    if(parent_)
    {
        parent_->children_.alternate().erase(path());
        parent_->children_.exchange();
        parent_=0;
    }
}

void gate_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{

    seq_ = seq;

    if(parent_->on_)
    {
        source_start(seq,id,b);
    }
}

void gate_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    if(parent_->on_)
    {
        source_buffer_reset(s,t,o,n);
    }
}

bool gate_wire_t::event_end(unsigned long long t)
{
    //pic::logmsg() << "gate end at " << t;

    seq_ = 0;

    if(parent_->on_)
    {
        //pic::logmsg() << "gate ending source";
        return source_end(t);
    }

    return true;
}

void gate_wire_t::source_ended(unsigned seq)
{
    if(!parent_->on_)
    {
        event_ended(seq);
    }
}

void gate_wire_t::activate(bool b, unsigned long long t)
{
    if(!current_id().is_path())
    {
        return;
    }

    if(b)
    {
        source_start(seq_,current_id().restamp(t),current_data());
    }
    else
    {
        source_end(t);
    }
}

void piw::gate_t::enable(bool enabled)
{
    root_->enable(enabled);
}

piw::change_nb_t piw::gate_t::gate()
{
    return piw::change_nb_t::method(root_,&impl_t::gate);
}

piw::cookie_t piw::gate_t::cookie()
{
    return piw::cookie_t(root_);
}

piw::gate_t::gate_t(const piw::cookie_t &cookie, bool init): root_(new impl_t(cookie,init))
{
}

piw::gate_t::~gate_t()
{
    delete root_;
}

