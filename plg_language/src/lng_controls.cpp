
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

#include "lng_controls.h"
#include <picross/pic_flipflop.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_address.h>
#include <piw/piw_keys.h>
#include <map>
#include <math.h>

#define THRESHOLD_ONE 0.2f
#define THRESHOLD_MANY 0.5f
#define THRESHOLD_OFF 0.1f
#define FACTOR 0.05f

namespace
{
    struct swire_t : piw::wire_t, piw::event_data_sink_t, virtual pic::tracked_t
    {
        swire_t(language::xselector_t::simpl_t *s, const piw::event_data_source_t &es);
        ~swire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        virtual void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b);
        virtual void event_data(const piw::xevent_data_buffer_t &data);
        virtual bool event_end(unsigned long long);
        virtual void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);

        void activate(bool b);
        void id_receive(const piw::data_nb_t &d);

        language::xselector_t::simpl_t *xselector_;
        bool on_;
        int id_;
        piw::data_t path_;
    };
}

struct language::xselector_t::simpl_t: piw::decoder_t, piw::decode_ctl_t
{
    simpl_t(const piw::change_nb_t g,piw::data_t v,piw::xcontrolled_t *l);

    static int init__(void *self_, void *arg_);
    static int set_choice__(void *self_, void *arg1_, void *arg2_);

    piw::wire_t *wire_create(const piw::event_data_source_t &es);
    void set_clock(bct_clocksink_t *) {}
    void set_latency(unsigned) {}
    void control_receive(unsigned,const piw::data_nb_t &);
    void control_term(unsigned long long);
    void activate(bool,unsigned long long);

    void set_choice(unsigned i, const piw::data_t &v);

    int gc_traverse(void *v, void *a) const
    {
        int r;
        if((r=output_.gc_traverse(v,a))!=0) return r;
        if((r=gate_.gc_traverse(v,a))!=0) return r;
        return 0;
    }

    int gc_clear()
    {
        output_.gc_clear();
        gate_.gc_clear();
        return 0;
    }

    pic::flipflop_functor_t<piw::change_nb_t> output_;
    pic::flipflop_functor_t<piw::change_nb_t> gate_;
    std::map<piw::data_t, piw::wire_t *> swires_;
    bool selecting_;
    bool on_;
    pic::flipflop_t<std::map<unsigned,piw::data_nb_t> > choices_;
    piw::xcontrolled_t *controlled_;
};

struct language::updown_t::uimpl_t
{
    uimpl_t(language::updown_t *host,piw::data_t v,float coarse,float fine);

    static int init__(void *self_, void *arg_);

    void control_init();
    void control_receive(unsigned, const piw::data_nb_t &);
    void control_term(unsigned long long t);
    static int reset(void *i, void *d);
    piw::data_nb_t adjusted(unsigned long long t);
    void activate(bool,unsigned long long);

    void adjust(const piw::data_nb_t &v)
    {
        float f = fabsf(v.as_norm());
        int dir = (v.as_norm()>0)?1:-1;

        if(f<THRESHOLD_OFF)
        {
            adjusting_ = false;
            return;
        }

        if(f<THRESHOLD_ONE)
        {
            return;
        }

        if(!adjusting_)
        {
            integrate_ = 0.0;
            adjusting_ = true;
            adjusted_ += (inc_*dir);
            adjusted_ = std::min(adjusted_,current_.get().as_array_ubound());
            adjusted_ = std::max(adjusted_,current_.get().as_array_lbound());
            return;
        }

        if(f>THRESHOLD_MANY)
        {
            integrate_ += FACTOR*(f-THRESHOLD_MANY);
            if(integrate_ > 1.f)
            {
                integrate_ = 0.f;
                adjusted_ += (inc_*dir);
                adjusted_ = std::min(adjusted_,current_.get().as_array_ubound());
                adjusted_ = std::max(adjusted_,current_.get().as_array_lbound());
            }
            return;
        }
    }


    int gc_traverse(void *v, void *a) const
    {
        int r;
        if((r=output_.gc_traverse(v,a))!=0) return r;
        return 0;
    }

    int gc_clear()
    {
        output_.gc_clear();
        return 0;
    }

    language::updown_t *host_;
    pic::flipflop_functor_t<piw::change_nb_t> output_;
    piw::dataholder_nb_t current_;
    piw::dataholder_nb_t reset_;
    float integrate_;
    float adjusted_;
    bool adjusting_;
    float inc_;
    bool on_;
};

struct language::toggle_t::timpl_t
{
    timpl_t(language::toggle_t *host,piw::data_t v);

    void control_init();
    void control_receive(unsigned, const piw::data_nb_t &);
    static int reset(void *i, void *d);

    int gc_traverse(void *v, void *a) const
    {
        int r;
        if((r=output_.gc_traverse(v,a))!=0) return r;
        return 0;
    }

    int gc_clear()
    {
        output_.gc_clear();
        return 0;
    }

    language::toggle_t *host_;
    pic::flipflop_functor_t<piw::change_nb_t> output_;
    piw::dataholder_nb_t current_;
    bool on_;
};

language::updown_t::uimpl_t::uimpl_t(language::updown_t *host,piw::data_t v,float coarse,float fine): host_(host),output_(host->sender()),current_(v),integrate_(0),adjusted_(0),adjusting_(false),inc_(fine),on_(false)
{
    piw::tsd_fastcall(init__,this,0);
}

int language::updown_t::uimpl_t::init__(void *self_, void *arg_)
{
    uimpl_t *self = (uimpl_t *)self_;

    self->output_(self->current_);

    return 1;
}

void language::updown_t::uimpl_t::control_init()
{
    host_->set_light(host_->ordinal(),3);
}

int language::updown_t::uimpl_t::reset(void *i_,void *d_)
{
    uimpl_t *i = (uimpl_t *)i_;
    const piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)d_);

    if(i->on_)
    {
        i->reset_.set_nb(d);
    }
    else
    {
        i->current_.set_nb(d);
    }
    return 0;
}

void language::updown_t::uimpl_t::activate(bool on, unsigned long long t)
{
    if(on==on_)
    {
        return;
    }

    on_=on;
    host_->set_light(host_->ordinal(),on_?1:3);

    if(!on_)
    {
        if(reset_.get().is_null())
        {
            current_.set_nb(adjusted(t));
        }
        else
        {
            current_.set_nb(reset_);
            reset_.clear_nb();
        }
    }
    else
    {
        reset_.clear_nb();
    }

    integrate_=0.f;
    adjusting_=false;
    adjusted_=current_.get().as_denorm_float();
}

void language::updown_t::uimpl_t::control_receive(unsigned s, const piw::data_nb_t &d)
{
    float a=adjusted_;

    switch(s)
    {
        case 1:
            activate(piw::decode_key(d), d.time());
            break;

        case 4:
            if(d.as_arraylen()==0) return;
            if(!on_) return;
            adjust(d);
            break;
    }

    if(a!=adjusted_)
    {
        piw::data_nb_t v = adjusted(d.time());
        //pic::logmsg() << "adjusting " << current_ << " -> " << v;
        output_(v);
    }
}

void language::updown_t::uimpl_t::control_term(unsigned long long t)
{
    activate(false,t);
}

piw::data_nb_t language::updown_t::uimpl_t::adjusted(unsigned long long t)
{
    float l = current_.get().as_array_lbound();
    float u = current_.get().as_array_ubound();
    float r = current_.get().as_array_rest();
    unsigned un = current_.get().units();
    return piw::makefloat_bounded_units_nb(un,u,l,r,adjusted_,t);
}

language::xselector_t::simpl_t::simpl_t(const piw::change_nb_t g,piw::data_t v,piw::xcontrolled_t *l) : decoder_t(this), output_(l->sender()), gate_(g), selecting_(false), on_(false), controlled_(l)
{
    piw::tsd_fastcall(init__,this,v.give_copy(PIC_ALLOC_NB));
}

int language::xselector_t::simpl_t::init__(void *self_, void *arg_)
{
    simpl_t *self = (simpl_t *)self_;
    piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)arg_);

    self->output_(d);

    return 1;
}

piw::wire_t *language::xselector_t::simpl_t::wire_create(const piw::event_data_source_t &es)
{
    std::map<piw::data_t, piw::wire_t *>::iterator i;

    if((i=swires_.find(es.path()))!=swires_.end())
        delete i->second;

    return new swire_t(this,es);
}

void language::xselector_t::simpl_t::control_receive(unsigned s, const piw::data_nb_t &value)
{
    if(s!=1)
        return;
    if(!piw::decode_key(value))
        return;
    if(selecting_)
        return;

    activate(true,value.time());
}

void language::xselector_t::simpl_t::control_term(unsigned long long t)
{
    activate(false,t);
}

void language::xselector_t::simpl_t::activate(bool b, unsigned long long t)
{
    if(b==on_)
        return;

    on_=b;
    if(!on_)
    {
        controlled_->save_lights();

        pic::flipflop_t<std::map<unsigned,piw::data_nb_t> >::guard_t g(choices_);
        std::map<unsigned,piw::data_nb_t>::const_iterator ci;
        for(ci=g.value().begin(); ci!=g.value().end(); ++ci)
            controlled_->set_light(ci->first-1,2);

        gate_(piw::makefloat_bounded_nb(1,0,0,0,0));
        selecting_=true;
    }
}

void language::xselector_t::simpl_t::set_choice(unsigned i, const piw::data_t &v)
{
    tsd_fastcall3(set_choice__,this,&i,v.give_copy(PIC_ALLOC_NB));
}

int language::xselector_t::simpl_t::set_choice__(void *self_, void *arg1_, void *arg2_)
{
    simpl_t *self = (simpl_t *)self_;
    unsigned i = *(unsigned *)arg1_;
    piw::data_nb_t v = piw::data_nb_t::from_given((bct_data_t)arg2_);

    self->choices_.alternate().insert(std::make_pair(i,v));
    self->choices_.exchange();

    return 1;
}

swire_t::swire_t(language::xselector_t::simpl_t *s, const piw::event_data_source_t &es): xselector_(s), on_(false), id_(-1), path_(es.path())
{
    s->swires_.insert(std::make_pair(path_,this));
    subscribe(es);
}

void swire_t::invalidate()
{
    if(xselector_)
    {
        unsubscribe();
        xselector_->swires_.erase(path_);
        xselector_=0;
    }
}

void swire_t::event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei)
{
    id_=d.as_path()[d.as_pathlen()-1];
    activate(true);
}

void swire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
}

void swire_t::event_data(const piw::xevent_data_buffer_t &eb)
{
}

bool swire_t::event_end(unsigned long long t)
{
    activate(false);
    id_=-1;
    return true;
}

void swire_t::activate(bool b)
{
    if(id_<0)
        return;

    if(!xselector_->selecting_)
        return;

    if(b==on_)
        return;

    on_=b;
    if(!on_)
    {
        pic::flipflop_t<std::map<unsigned,piw::data_nb_t> >::guard_t g(xselector_->choices_);
        std::map<unsigned,piw::data_nb_t>::const_iterator ci;

        if((ci=g.value().find(id_))!=g.value().end())
        {
            //pic::logmsg() << "selection set to " << id_;
            xselector_->output_(ci->second);
        }

        xselector_->selecting_ = false;
        xselector_->gate_(piw::makefloat_bounded_nb(1,0,0,1,0));
        xselector_->controlled_->restore_lights();
    }
}

language::toggle_t::timpl_t::timpl_t(language::toggle_t *host,piw::data_t v): host_(host),output_(host->sender()),current_(v),on_(false)
{
    on_=false;

    if(!current_.get().is_null())
    {
        on_=(current_.get().as_norm()!=0.0);
    }

    output_(piw::makebool_nb(on_,current_.get().time()));
}

void language::toggle_t::timpl_t::control_init()
{
    host_->set_light(host_->ordinal(),on_?1:3);
}

int language::toggle_t::timpl_t::reset(void *i_, void *d_)
{
    timpl_t *i = (timpl_t *)i_;
    const piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)d_);

    i->current_.set_nb(d);

    if(!i->current_.get().is_null())
    {
        i->on_=(i->current_.get().as_norm()!=0.0);
    }

    i->host_->set_light(i->host_->ordinal(),i->on_?1:3);

    return 0;
}

void language::toggle_t::timpl_t::control_receive(unsigned s, const piw::data_nb_t &d)
{
    if(!d.is_null())
    {
        if(s==1 && piw::decode_key(d))
        {
            output_(piw::makebool_nb(!on_,d.time()));
        }
    }
}

language::updown_t::updown_t(const piw::data_t &v,float coarse,float fine): uimpl_(new uimpl_t(this,v,coarse,fine)) { }
language::updown_t::~updown_t() { delete uimpl_; }
void language::updown_t::control_init() { uimpl_->control_init(); }
void language::updown_t::reset(const piw::data_t &v) { piw::tsd_fastcall(&uimpl_t::reset,(void *)uimpl_,(void *)v.give_copy(PIC_ALLOC_NB)); } 
void language::updown_t::control_term(unsigned long long t) { xcontrolled_t::control_term(t); uimpl_->control_term(t); }
void language::updown_t::control_receive(unsigned s, const piw::data_nb_t &d) { uimpl_->control_receive(s,d); }
int language::updown_t::gc_traverse(void *v,void *a) const { return uimpl_->gc_traverse(v,a); }
int language::updown_t::gc_clear() { return uimpl_->gc_clear(); }

language::xselector_t::xselector_t(const piw::change_nb_t &g,const piw::data_t &v): simpl_(new simpl_t(g,v,this)) {}
language::xselector_t::~xselector_t() { delete simpl_; }
void language::xselector_t::control_receive(unsigned s, const piw::data_nb_t &value) { simpl_->control_receive(s,value); }
void language::xselector_t::control_term(unsigned long long t) { xcontrolled_t::control_term(t); simpl_->control_term(t); }
void language::xselector_t::set_choice(unsigned i, const piw::data_t &v) { simpl_->set_choice(i,v); }
piw::cookie_t language::xselector_t::cookie() { return simpl_->cookie(); }
void language::xselector_t::control_init() { set_light(ordinal(),3); }
void language::xselector_t::reset(const piw::data_t &d) {}
int language::xselector_t::gc_traverse(void *v,void *a) const { return simpl_->gc_traverse(v,a); }
int language::xselector_t::gc_clear() { return simpl_->gc_clear(); }

language::toggle_t::toggle_t(const piw::data_t &v): timpl_(new timpl_t(this,v)) { }
language::toggle_t::~toggle_t() { delete timpl_; }
void language::toggle_t::control_init() { timpl_->control_init(); }
void language::toggle_t::reset(const piw::data_t &v) { piw::tsd_fastcall(&timpl_t::reset,(void *)timpl_,(void *)v.give_copy(PIC_ALLOC_NB)); }
void language::toggle_t::control_receive(unsigned s, const piw::data_nb_t &d) { timpl_->control_receive(s,d); }
int language::toggle_t::gc_traverse(void *v,void *a) const { return timpl_->gc_traverse(v,a); }
int language::toggle_t::gc_clear() { return timpl_->gc_clear(); }

