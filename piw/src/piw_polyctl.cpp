
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

#include <piw/piw_polyctl.h>
#include <piw/piw_clock.h>
#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>

#include <picross/pic_ilist.h>
#include <picross/pic_ref.h>

#include <vector>

namespace
{
    struct polyctl_wire_t;

    struct polyctl_voice_t: piw::wire_ctl_t, piw::event_data_source_real_t, pic::element_t<0>, pic::element_t<1>, virtual pic::counted_t, virtual pic::lckobject_t
    {
        polyctl_voice_t(piw::polyctl_t::impl_t *i, const piw::data_t &path): piw::event_data_source_real_t(path), impl_(i), owner_(0) {}
        ~polyctl_voice_t() { source_shutdown(); }
        void source_ended(unsigned seq);

        piw::polyctl_t::impl_t *impl_;
        polyctl_wire_t *owner_;
    };

    struct polyctl_wire_t: piw::wire_t, piw::event_data_sink_t
    {
        polyctl_wire_t(piw::polyctl_t::impl_t *i, const piw::event_data_source_t &);
        ~polyctl_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);

        piw::polyctl_t::impl_t *impl_;
        polyctl_voice_t *voice_;
    };

}

struct piw::polyctl_t::impl_t: piw::thing_t, piw::decode_ctl_t, virtual pic::lckobject_t
{
    impl_t(unsigned poly,const piw::cookie_t &, bool, unsigned);

    piw::wire_t *wire_create(const piw::event_data_source_t &es) { return new polyctl_wire_t(this,es); } 
    void set_clock(bct_clocksink_t *c) { encoder_.set_clock(c); }
    void set_latency(unsigned l) { encoder_.set_latency(l); }
    void set_poly(unsigned poly);
    void add_voice();
    void set_cycle(bool cycle) { id_=cycle?1:0; }
    void thing_trigger_slow();
    polyctl_voice_t *get_voice()
    {
        polyctl_voice_t *v = free_queue_.pop_front();
        if(v)
        {
            --freecount_;
            busy_queue_.append(v); 
            if(freecount_<headroom_)
                trigger_slow();
        }
        return v;
    }

    void return_voice(polyctl_voice_t *v)
    {
        busy_queue_.remove(v);
        free_queue_.append(v);
        ++freecount_;
    }

    piw::data_nb_t nextid(const piw::data_nb_t &);

    unsigned poly_;

    pic::ilist_t<polyctl_voice_t,0> free_queue_;
    pic::ilist_t<polyctl_voice_t,1> busy_queue_;

    std::vector<pic::ref_t<polyctl_voice_t> > voices_;
    piw::decoder_t decoder_;
    piw::root_ctl_t encoder_;
    unsigned id_;
    unsigned headroom_;
    unsigned freecount_;
};

polyctl_wire_t::polyctl_wire_t(piw::polyctl_t::impl_t *impl, const piw::event_data_source_t &es) : impl_(impl), voice_(0)
{
    subscribe(es);
}

void polyctl_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
}

piw::polyctl_t::impl_t::impl_t(unsigned poly, const piw::cookie_t &c, bool cyc, unsigned hr): poly_(poly), voices_(poly), decoder_(this), id_(cyc?1:0), headroom_(hr), freecount_(0)
{
    piw::tsd_thing(this);
    encoder_.connect(c);

    for(unsigned i=0; i<poly; ++i)
    {
        voices_[i] = pic::ref(new polyctl_voice_t(this,piw::pathone(i+1,0)));
        encoder_.connect_wire(voices_[i].ptr(),voices_[i]->source());
        free_queue_.append(voices_[i].ptr());
        ++freecount_;
    }
}

static int setup_voice__(void *impl_, void *voice_)
{
    piw::polyctl_t::impl_t *impl = (piw::polyctl_t::impl_t *)impl_;
    polyctl_voice_t *voice = (polyctl_voice_t *)voice_;
    impl->free_queue_.append(voice);
    ++impl->freecount_;
    return 0;
}
        
void piw::polyctl_t::impl_t::add_voice()
{
    voices_.resize(poly_+1);
    voices_[poly_]=pic::ref(new polyctl_voice_t(this,piw::pathone(poly_+1,0)));
    encoder_.connect_wire(voices_[poly_].ptr(),voices_[poly_]->source());
    piw::tsd_fastcall(setup_voice__,this,voices_[poly_].ptr());
    poly_++;
}

void piw::polyctl_t::impl_t::set_poly(unsigned poly)
{
    while(poly_<poly)
    {
        add_voice();
    }
}

void piw::polyctl_t::impl_t::thing_trigger_slow()
{
    while(freecount_<headroom_)
    {
        pic::logmsg() << "adding voice free=" << freecount_ << " headroom=" << headroom_;
        add_voice();
    }
}

piw::data_nb_t piw::polyctl_t::impl_t::nextid(const piw::data_nb_t &d_)
{
    piw::data_nb_t d(d_);

    if(id_)
    {
        d = piw::pathappend_channel_nb(d,id_++);
        if(id_>255)
            id_=1;
    }

    return d;
}

void polyctl_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(voice_)
    {
        voice_->source_buffer_reset(s,t,o,n);
    }
}

void polyctl_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    if(!voice_)
    {
        voice_=impl_->get_voice();

        if(!voice_)
        {
            return;
        }

        voice_->owner_=this;
    }

    voice_->source_start(seq,impl_->nextid(id),b);
}

bool polyctl_wire_t::event_end(unsigned long long t)
{
    if(!voice_)
    {
        return true;
    }

    if(voice_->source_end(t))
    {
        impl_->return_voice(voice_);
        voice_=0;
        return true;
    }

    return false;
}

void polyctl_voice_t::source_ended(unsigned seq)
{
    if(owner_)
    {
        owner_->event_ended(seq);
        owner_->voice_=0;
        owner_=0;
        impl_->return_voice(this);
    }
}

piw::polyctl_t::polyctl_t(unsigned poly, const piw::cookie_t &c, bool cycle, unsigned hr): impl_(new impl_t(poly,c,cycle,hr))
{
}

piw::polyctl_t::~polyctl_t()
{
    delete impl_;
}

piw::cookie_t piw::polyctl_t::cookie()
{
    return impl_->decoder_.cookie();
}

void piw::polyctl_t::set_poly(unsigned poly)
{
    impl_->set_poly(poly);
}

void piw::polyctl_t::set_cycle(bool cycle)
{
    impl_->set_cycle(cycle);
}
