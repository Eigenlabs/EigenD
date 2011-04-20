
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

#include <piw/piw_aggregator.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_policy.h>

#include <map>

namespace
{
    struct agg_wire_t;
    struct agg_root_t;

    struct agg_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_sink_t, piw::event_data_source_real_t
    {
        agg_wire_t(agg_root_t *p, const piw::d2d_nb_t &filter, const piw::event_data_source_t &s);
        ~agg_wire_t() { invalidate(); }
        void wire_closed();
        void remove_child(unsigned name);
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
        {
            piw::data_nb_t nid = filter_(id);

            if(nid.is_path())
            {
                active_ = true;
                source_start(seq,nid,b);
            }
        }

        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
        {
            source_buffer_reset(sig,t,o,n);
        }

        void source_ended(unsigned seq)
        {
            event_ended(seq);
        }

        bool event_end(unsigned long long t)
        {
            if(active_)
            {
                active_ = false;
                return source_end(t);
            }

            return true;
        }

        agg_root_t *parent_;
        pic::flipflop_functor_t<piw::d2d_nb_t> filter_;
        bool active_;
    };

    struct agg_root_t: piw::root_t
    {
        agg_root_t(piw::aggregator_t::impl_t *ctlr, unsigned bndl, const piw::d2d_nb_t &filt);
        void destroy();

        piw::wire_t *root_wire(const piw::event_data_source_t &);
        void root_closed();
        void root_clock();
        void root_opened();
        void root_latency();

        int gc_traverse(void *, void *) const;
        int gc_clear();

        piw::aggregator_t::impl_t *ctlr_;
        unsigned bndl_;
        piw::d2d_nb_t filt_;
        bct_clocksink_t *up_;
        std::map<piw::data_t, agg_wire_t *> children_;
    };
};

struct piw::aggregator_t::impl_t: root_ctl_t, clocksink_t
{
    impl_t(const cookie_t &c, clockdomain_ctl_t *);
    ~impl_t();
    cookie_t get_output(unsigned name,const piw::d2d_nb_t &filter);
    void clear_output(unsigned name);
    void invalidate();
    void update_latency();
    unsigned find_unused();

    int gc_traverse(void *, void *) const;
    int gc_clear();

    std::map<unsigned, agg_root_t *> outputs_;
};

piw::aggregator_t::aggregator_t(const cookie_t &c, clockdomain_ctl_t *d): root_(new impl_t(c, d))
{
}

piw::aggregator_t::~aggregator_t()
{
    delete root_;
}

piw::cookie_t piw::aggregator_t::get_filtered_output(unsigned name, const piw::d2d_nb_t &filter)
{
    return root_->get_output(name,filter);
}

piw::cookie_t piw::aggregator_t::get_output(unsigned name)
{
    return root_->get_output(name,piw::aggregation_filter(name));
}

void piw::aggregator_t::clear_output(unsigned name)
{
    root_->clear_output(name);
}

unsigned piw::aggregator_t::find_unused()
{
    return root_->find_unused();
}

agg_root_t::agg_root_t(piw::aggregator_t::impl_t *ctlr, unsigned bndl,const piw::d2d_nb_t &filt): root_t(0), ctlr_(ctlr), bndl_(bndl), filt_(filt), up_(0)
{
    ctlr_->outputs_.insert(std::make_pair(bndl_,this));
}

piw::cookie_t piw::aggregator_t::impl_t::get_output(unsigned name, const piw::d2d_nb_t &filter)
{
    std::map<unsigned, agg_root_t *>::iterator i;

    if((i=outputs_.find(name))!=outputs_.end())
    {
        PIC_THROW("output in use");
    }

    agg_root_t *r = new agg_root_t(this,name,filter);
    return cookie_t(r);
}

int piw::aggregator_t::impl_t::gc_traverse(void *v, void *a) const
{
    std::map<unsigned, agg_root_t *>::const_iterator i;
    int r;

    for(i=outputs_.begin(); i!=outputs_.end(); i++)
    {
        if((r=i->second->gc_traverse(v,a))!=0)
        {
            return r;
        }
    }

    return 0;
}

int piw::aggregator_t::impl_t::gc_clear()
{
    std::map<unsigned, agg_root_t *>::iterator i;

    for(i=outputs_.begin(); i!=outputs_.end(); i++)
    {
        i->second->gc_clear();
    }

    return 0;
}

unsigned piw::aggregator_t::impl_t::find_unused()
{
    std::map<unsigned, agg_root_t *>::iterator i;

    for(unsigned j=1;j<=255;j++)
    {
        if((i=outputs_.find(j)) == outputs_.end())
        {
            return j;
        }
    }

    return 0;
}

void piw::aggregator_t::impl_t::clear_output(unsigned name)
{
    std::map<unsigned, agg_root_t *>::iterator i;

    if((i=outputs_.find(name))!=outputs_.end())
    {
        i->second->destroy();
    }
}

void piw::aggregator_t::impl_t::invalidate()
{
    std::map<unsigned, agg_root_t *>::iterator i;

    while((i=outputs_.begin())!=outputs_.end())
    {
        i->second->destroy();
    }
}

void piw::aggregator_t::impl_t::update_latency()
{
    unsigned l=0;
    std::map<unsigned, agg_root_t *>::iterator i=outputs_.begin(), e=outputs_.end();
    for(; i!=e; ++i)
    {
        l=std::max(l,i->second->get_latency());
    }
    set_latency(l);
}

piw::aggregator_t::impl_t::~impl_t()
{
    invalidate();
}

piw::aggregator_t::impl_t::impl_t(const cookie_t &c, clockdomain_ctl_t *d)
{
    set_clock(this);
    d->sink(this,"piw::aggregator");
    connect(c);
}

void agg_root_t::root_opened()
{
    root_clock();
    root_latency();
}

void agg_root_t::root_clock()
{
    bct_clocksink_t *c = get_clock();

    if(c!=up_)
    {
        if(up_)
        {
            ctlr_->remove_upstream(up_);
        }

        up_=c;

        if(up_)
        {
            ctlr_->add_upstream(up_);
        }
    }
}

int agg_root_t::gc_traverse(void *v, void *a) const
{
    std::map<piw::data_t,agg_wire_t *>::const_iterator ci;
    int r;

    for(ci=children_.begin(); ci!=children_.end(); ci++)
    {
        if((r=ci->second->filter_.gc_traverse(v,a))!=0)
        {
            return r;
        }
    }

    return 0;
}

int agg_root_t::gc_clear()
{
    std::map<piw::data_t,agg_wire_t *>::iterator ci;

    for(ci=children_.begin(); ci!=children_.end(); ci++)
    {
        ci->second->filter_.gc_clear();
    }

    return 0;
}

void agg_root_t::destroy()
{
    std::map<piw::data_t,agg_wire_t *>::iterator ci;

    disconnect();

    while((ci=children_.begin())!=children_.end())
    {
        delete ci->second;
    }

    ctlr_->outputs_.erase(bndl_);
    ctlr_->update_latency();

    if(up_)
    {
        ctlr_->remove_upstream(up_);
        up_=0;
    }

    delete this;
}

void agg_root_t::root_closed()
{
    destroy();
}

piw::wire_t *agg_root_t::root_wire(const piw::event_data_source_t &es)
{
    std::map<piw::data_t,agg_wire_t *>::iterator ci;

    if((ci=children_.find(es.path()))!=children_.end())
    {
        delete ci->second;
    }

    agg_wire_t *c = new agg_wire_t(this,filt_,es);
    ctlr_->connect_wire(c,c->source());
    return c;
}

agg_wire_t::agg_wire_t(agg_root_t *p, const piw::d2d_nb_t &filter, const piw::event_data_source_t &es): piw::event_data_source_real_t(piw::pathprepend(es.path(),p->bndl_)), parent_(p), filter_(filter), active_(false)
{
    parent_->children_.insert(std::make_pair(path(),this));
    subscribe_and_ping(es);
}

void agg_wire_t::wire_closed()
{
    delete this;
}

void agg_root_t::root_latency()
{
    ctlr_->update_latency();
}

void agg_wire_t::invalidate()
{
    source_shutdown();
    unsubscribe();
    piw::wire_t::disconnect();
    wire_ctl_t::disconnect();

    parent_->children_.erase(path());
    parent_=0;
}

unsigned piw::aggregator_t::size() 
{
    return root_->outputs_.size();
}

int piw::aggregator_t::gc_traverse(void *v, void *a) const
{
    return root_->gc_traverse(v,a);
}

int piw::aggregator_t::gc_clear()
{
    return root_->gc_clear();
}

