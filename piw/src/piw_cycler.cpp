
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

#include <piw/piw_cycler.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>

#include <picross/pic_ilist.h>
#include <picross/pic_ref.h>
#include <vector>

#define VSTATE_IDLE      0
#define VSTATE_ACTIVE    1
#define VOICE_COUNTDOWN  1

namespace
{
    struct main_wire_t;
    struct voice_t;

    struct damp_source_t: piw::event_data_source_real_t
    {
        inline damp_source_t(voice_t *v,const piw::data_t &path): piw::event_data_source_real_t(path), voice_(v) {}
        inline void source_ended(unsigned seq);
        voice_t *voice_;
        piw::xevent_data_buffer_t buffer_;
    };

    struct main_source_t: piw::event_data_source_real_t
    {
        inline main_source_t(voice_t *v,const piw::data_t &path): piw::event_data_source_real_t(path), voice_(v) {}
        inline void source_ended(unsigned seq);
        voice_t *voice_;
    };

    struct voice_t: pic::element_t<0>, pic::element_t<1>, pic::element_t<2>, virtual pic::counted_t, virtual pic::lckobject_t
    {
        voice_t(piw::cycler_t::impl_t *i, const piw::data_t &path): impl_(i), state_(VSTATE_IDLE), main_source_(this,path), damp_source_(this,path), refcount_(0), owner_(0) {}
        ~voice_t() { main_source_.source_shutdown(); }

        void startup(main_wire_t *owner,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b,float);
        bool shutdown(unsigned long long time, float damp);
        void detach(unsigned long long time, float damp);
        void damp(unsigned long long time, float damp);
        void data(const piw::xevent_data_buffer_t &b);

        void main_source_ended(unsigned seq);
        void damp_source_ended(unsigned seq);

        bool decref(unsigned long long t, bool now);

        piw::cycler_t::impl_t *impl_;
        piw::dataholder_nb_t srcid_;
        piw::dataholder_nb_t dstid_;
        unsigned state_;

        main_source_t main_source_;
        piw::wire_ctl_t main_wire_;

        damp_source_t damp_source_;
        piw::wire_ctl_t damp_wire_;
        unsigned refcount_;
        unsigned long long etime_;
        main_wire_t *owner_;
    };

    struct feedback_wire_t: piw::wire_t, piw::event_data_sink_t
    {
        feedback_wire_t(piw::cycler_t::impl_t *i, const piw::event_data_source_t &);
        ~feedback_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n) {}

        piw::cycler_t::impl_t *impl_;
        voice_t *voice_;
    };

    struct main_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<>
    {
        main_wire_t(piw::cycler_t::impl_t *i, const piw::event_data_source_t &);
        ~main_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();

        void voice_ended(voice_t *);
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void ticked(unsigned long long t);
        bool event_end(unsigned long long t);
        void startup(const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b, float);

        piw::cycler_t::impl_t *impl_;
        voice_t *primary_voice_;
        voice_t *secondary_voice_;
        unsigned seq_;
        float damper_;
        piw::dataqueue_t queue_;
        unsigned long long qindex_;
        unsigned countdown_;
    };

    struct feedback_ctl_t: piw::decoder_t, piw::decode_ctl_t
    {
        feedback_ctl_t(piw::cycler_t::impl_t *i): piw::decoder_t(this), impl_(i) {}

        piw::wire_t *wire_create(const piw::event_data_source_t &es);
        void set_clock(bct_clocksink_t *c) {}
        void set_latency(unsigned l) {}

        piw::cycler_t::impl_t *impl_;
    };
}

struct piw::cycler_t::impl_t: piw::decode_ctl_t, virtual pic::lckobject_t, piw::clocksink_t
{
    impl_t(piw::clockdomain_ctl_t *,int poly,const piw::cookie_t &, const piw::cookie_t &, bool);
    ~impl_t();

    piw::wire_t *wire_create(const piw::event_data_source_t &es);
    piw::wire_t *feedback_wire_create(const piw::event_data_source_t &es);
    
    void set_clock(bct_clocksink_t *c) { if(up_) remove_upstream(up_); up_=c; if(up_) add_upstream(up_); }
    void set_latency(unsigned l) { damp_encoder_.set_latency(l); main_encoder_.set_latency(l); }
    void set_poly(unsigned poly);

    void clocksink_ticked(unsigned long long f,unsigned long long t)
    {
        main_wire_t *w=active_.head();
        if(!w)
        {
            tick_suppress(true);
            return;
        }

        while(w)
        {
            w->ticked(t);
            w=active_.next(w);
        }
    }

    piw::data_nb_t nextid(const piw::data_nb_t &);

    void set_invert(bool invert) { invert_=invert; }
    void set_cycle(bool cycle) { id_=cycle?1:0; }
    void set_curve(float curve) { curve_=curve; }
    bool cycling() { return id_!=0; }

    void set_maxdamp(float maxdamp) { maxdamp_ = maxdamp; }

    unsigned poly_;

    pic::ilist_t<voice_t,0> free_queue_;
    pic::ilist_t<voice_t,1> busy_queue_;

    std::vector<pic::ref_t<voice_t> > voices_;
    pic::lckmap_t<piw::data_nb_t,voice_t *,piw::path_less>::nbtype dst2voice_;

    feedback_ctl_t feedback_;
    piw::decoder_t main_decoder_;
    piw::root_ctl_t main_encoder_;
    piw::root_ctl_t damp_encoder_;

    unsigned id_;
    float maxdamp_;
    bool invert_;
    float curve_;

    pic::ilist_t<main_wire_t> active_;
    bct_clocksink_t *up_;
};

feedback_wire_t::feedback_wire_t(piw::cycler_t::impl_t *impl, const piw::event_data_source_t &es) : impl_(impl), voice_(0)
{
    subscribe(es);
}

void feedback_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
}

piw::wire_t *feedback_ctl_t::wire_create(const piw::event_data_source_t &es)
{
    return impl_->feedback_wire_create(es);
}

main_wire_t::main_wire_t(piw::cycler_t::impl_t *impl, const piw::event_data_source_t &es) : impl_(impl), primary_voice_(0), secondary_voice_(0), damper_(0.5)
{
    subscribe(es);
}

void main_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
}

piw::cycler_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd,int poly, const piw::cookie_t &c, const piw::cookie_t &d, bool cyc): poly_(poly), voices_(poly), feedback_(this), main_decoder_(this), id_(1), maxdamp_(1.0), invert_(0), curve_(1.0), up_(0)
{
    main_encoder_.connect(c);
    damp_encoder_.connect(d);

    cd->sink(this,"cycler");
    tick_enable(true);

    main_encoder_.set_clock(this);
    damp_encoder_.set_clock(this);

    for(int i=0; i<poly; ++i)
    {
        voices_[i] = pic::ref(new voice_t(this,piw::pathone(i+1,0)));
        main_encoder_.connect_wire(&voices_[i]->main_wire_,voices_[i]->main_source_.source());
        damp_encoder_.connect_wire(&voices_[i]->damp_wire_,voices_[i]->damp_source_.source());
        free_queue_.append(voices_[i].ptr());
    }
}

piw::cycler_t::impl_t::~impl_t()
{
}

piw::wire_t *piw::cycler_t::impl_t::wire_create(const piw::event_data_source_t &es)
{
    return new main_wire_t(this,es);
} 

piw::wire_t *piw::cycler_t::impl_t::feedback_wire_create(const piw::event_data_source_t &es)
{
    return new feedback_wire_t(this,es);
} 

static int setup_voice__(void *impl_, void *voice_)
{
    piw::cycler_t::impl_t *impl = (piw::cycler_t::impl_t *)impl_;
    voice_t *voice = (voice_t *)voice_;
    impl->free_queue_.append(voice);
    return 0;
}

void piw::cycler_t::impl_t::set_poly(unsigned poly)
{
    while(poly_<poly)
    {
        voices_.resize(poly_+1);
        voices_[poly_]=pic::ref(new voice_t(this,piw::pathone(poly_+1,0)));
        main_encoder_.connect_wire(&voices_[poly_]->main_wire_,voices_[poly_]->main_source_.source());
        damp_encoder_.connect_wire(&voices_[poly_]->damp_wire_,voices_[poly_]->damp_source_.source());
        piw::tsd_fastcall(setup_voice__,this,voices_[poly_].ptr());
        poly_++;
    }
}

piw::data_nb_t piw::cycler_t::impl_t::nextid(const piw::data_nb_t &d_)
{
    piw::data_nb_t d(d_);

    if(id_)
    {
        d = piw::pathappend_chaff_nb(d,id_++);
        if(id_>255)
            id_=1;
    }

    return d;
}

void feedback_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    pic::lckmap_t<piw::data_nb_t,voice_t *,piw::path_less>::nbtype::iterator i;

    //pic::logmsg() << "feedback: " << id << " started " << (void *)voice_;

    if((i=impl_->dst2voice_.find(id))!=impl_->dst2voice_.end())
    {
        if(voice_)
        {
            if(voice_==i->second)
            {
                return;
            }

            voice_->decref(id.time(),false);
            voice_=0;
        }

        //pic::logmsg() << "attached to voice " << (void *)(i->second);
        voice_=i->second;
        voice_->refcount_++;
    }
}

bool feedback_wire_t::event_end(unsigned long long t)
{
    //pic::logmsg() << "feedback: ended " << (void *)voice_;

    if(voice_)
    {
        voice_->decref(t,false);
        voice_=0;
    }

    return true;
}

void voice_t::damp_source_ended(unsigned seq)
{
    //pic::logmsg() << "voice " << (void *)this << " freed";
    impl_->busy_queue_.remove(this);
    impl_->free_queue_.append(this);
    impl_->dst2voice_.erase(dstid_.get());

    if(owner_)
    {
        owner_->voice_ended(this);
        owner_=0;
    }
}

bool voice_t::decref(unsigned long long t, bool now)
{
    if(--refcount_==0)
    {
        if(damp_source_.source_end(t))
        {
            if(now)
            {
                //pic::logmsg() << "voice " << (void *)this << " freed " << t;
                impl_->busy_queue_.remove(this);
                impl_->free_queue_.append(this);
                impl_->dst2voice_.erase(dstid_.get());
                owner_=0;
                return true;
            }

            damp_source_ended(0);
            return true;
        }
    }

    return false;
}

void voice_t::startup(main_wire_t *owner,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b, float damp)
{
    if(owner_)
    {
        PIC_ASSERT(owner_==owner);
    }

    owner_=owner;
    srcid_.set_nb(id);
    dstid_.set_nb(impl_->nextid(id));

    damp_source_.buffer_=piw::xevent_data_buffer_t(SIG1(16),PIW_DATAQUEUE_SIZE_TINY);
    damp_source_.buffer_.add_value(16,piw::makefloat_bounded_nb(1.0,0.0,0.0,damp,id.time()));
    damp_source_.source_start(0,dstid_,damp_source_.buffer_);

    main_source_.source_start(0,dstid_,b);

    if(state_==VSTATE_IDLE)
    {
        state_=VSTATE_ACTIVE;
        refcount_++;
        impl_->free_queue_.remove(this);
        impl_->busy_queue_.append(this);
        impl_->dst2voice_.insert(std::make_pair(dstid_.get(),this));
    }

    //pic::logmsg() << "voice " << (void *)this << " starting " << srcid_ << " -> " << dstid_ << " refc " << refcount_ << " damp " << damp;
}

void voice_t::detach(unsigned long long time, float damp)
{
    //pic::logmsg() << "detach " << (void *)this << " " << state_ << ' ' << damp << ' ' << srcid_ << ' ' << dstid_ << ' ' << time;

    PIC_ASSERT(state_==VSTATE_IDLE);

    owner_=0;
    damp_source_.buffer_.add_value(16,piw::makefloat_bounded_nb(1.0,0.0,0.0,damp,time));
}

bool voice_t::shutdown(unsigned long long time, float damp)
{
    //pic::logmsg() << "shutdown " << (void *)this << " " << state_ << " damp " << damp << ' ' << srcid_ << ' ' << dstid_ << ' ' << time;

    PIC_ASSERT(state_==VSTATE_ACTIVE);

    state_=VSTATE_IDLE;
    damp_source_.buffer_.add_value(16,piw::makefloat_bounded_nb(1.0,0.0,0.0,damp,time));
    etime_=time+1;

    if(main_source_.source_end(etime_))
    {
        return decref(etime_,true);
    }

    return false;
}

void voice_t::damp(unsigned long long time, float damp)
{
    if(state_==VSTATE_ACTIVE)
    {
        return;
    }

    //pic::logmsg() << "send damp: " << damp;
    damp_source_.buffer_.add_value(16,piw::makefloat_bounded_nb(1.0,0.0,0.0,damp,time));
}

void damp_source_t::source_ended(unsigned seq)
{
    voice_->damp_source_ended(seq);
}

void main_source_t::source_ended(unsigned seq)
{
    voice_->main_source_ended(seq);
}

void voice_t::main_source_ended(unsigned seq)
{
    decref(etime_,false);
}

void main_wire_t::ticked(unsigned long long t)
{
    if(primary_voice_ || secondary_voice_)
    {
        piw::data_nb_t d;
        bool ok = false;
        while(queue_.read(d,&qindex_,t))
        {
            qindex_++;
            ok = true;
        }

        if(ok)
        {
            float s=d.as_renorm(0,1,0);
            if(impl_->invert_) s=1.0-s;
            s=std::min(1.0f,s*impl_->curve_);
            s=0.5*s;
            damper_ = s;
            //pic::logmsg() << "damp: " << d << " -> " << s << ' ' << qindex_;
            if(primary_voice_) primary_voice_->damp(t,damper_);
            if(secondary_voice_) secondary_voice_->damp(t,damper_);
        }
    }

    if(secondary_voice_)
    {
        if(countdown_>0)
        {
            countdown_--;
            return;
        }

        //pic::logmsg() << "detaching secondary voice";
        secondary_voice_->detach(t,impl_->maxdamp_);
        secondary_voice_ = 0;
    }
}

void main_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s==16)
    {
        //pic::logmsg() << "buffer reset on 16";
        piw::data_nb_t d2;
        queue_=n;
        queue_.latest(d2,&qindex_,t);
    }

    if(primary_voice_) primary_voice_->main_source_.source_buffer_reset(s,t,o,n);
    if(secondary_voice_) secondary_voice_->main_source_.source_buffer_reset(s,t,o,n);
}

void main_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    queue_=b.signal(16);

    piw::data_nb_t d;

    seq_=seq;
    damper_=0.5;
    qindex_=0;

    if(queue_.latest(d,&qindex_,id.time()))
    {
        qindex_++;
        float s=d.as_renorm(0,1,0);
        if(impl_->invert_) s=1.0-s;
        s=std::min(1.0f,s*impl_->curve_);
        s=0.5*s;
        damper_ = s;
        //pic::logmsg() << "initial damping: " << s << ' ' << d << ' ' << damper_;
    }

    if(!impl_->cycling())
    {
        if(!primary_voice_)
        {
            primary_voice_=impl_->free_queue_.pop_front();
        }

        if(primary_voice_)
        {
            startup(id,b,0);
        }
        else
        {
            pic::logmsg() << "cycler (not cycling) out of voices";
        }

        return;
    }

    if(primary_voice_)
    {
        if(!secondary_voice_)
        {
            secondary_voice_ = primary_voice_;
            countdown_ = VOICE_COUNTDOWN;
        }
        else
        {
            primary_voice_->detach(id.time(),impl_->maxdamp_);
        }

        primary_voice_=0;
    }

    primary_voice_=impl_->free_queue_.pop_front();

    if(primary_voice_)
    {
        startup(id,b,0);
    }
    else
    {
        pic::logmsg() << "cycler out of voices";
    }
}

void main_wire_t::startup(const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b, float)
{
    primary_voice_->startup(this,id,b,0);
    impl_->tick_suppress(false);
    impl_->active_.append(this);
}

void main_wire_t::voice_ended(voice_t *v)
{
    if(v==primary_voice_)
    {
        primary_voice_=0;
    }

    if(v==secondary_voice_)
    {
        secondary_voice_=0;
    }

    if(secondary_voice_ && !primary_voice_)
    {
        //pic::logmsg() << "resurrecting secondary voice";
        primary_voice_ = secondary_voice_;
        secondary_voice_ = 0;
        return;
    }

    if(!primary_voice_ && !secondary_voice_)
    {
        event_ended(seq_);
        remove();
        //pic::logmsg() << "deferred voice shutdown complete";
    }
}

bool main_wire_t::event_end(unsigned long long t)
{
    ticked(t);

    if(primary_voice_)
    {
        float s = damper_;
        //pic::logmsg() << "shutdown, damper=" << s;
        if(primary_voice_->shutdown(t,s))
        {
            primary_voice_=0;
            //pic::logmsg() << "voice shutdown";
            if(!secondary_voice_)
            {
                remove();
                return true;
            }

            //pic::logmsg() << "resurrecting secondary voice";
            primary_voice_ = secondary_voice_;
            secondary_voice_ = 0;
        }

        //pic::logmsg() << "voice shutdown deferred";
        return false;
    }

    remove();
    return true;
}

piw::cycler_t::cycler_t(piw::clockdomain_ctl_t *cd,int poly, const piw::cookie_t &c, const piw::cookie_t &d, bool cycle): impl_(new impl_t(cd,poly,c,d,cycle))
{
}

piw::cycler_t::~cycler_t()
{
    delete impl_;
}

piw::cookie_t piw::cycler_t::feedback_cookie()
{
    return impl_->feedback_.cookie();
}

piw::cookie_t piw::cycler_t::main_cookie()
{
    return impl_->main_decoder_.cookie();
}

void piw::cycler_t::set_maxdamp(float damp)
{
    impl_->set_maxdamp(damp);
}

void piw::cycler_t::set_poly(unsigned poly)
{
    impl_->set_poly(poly);
}

void piw::cycler_t::set_cycle(bool cycle)
{
    impl_->set_cycle(cycle);
}

void piw::cycler_t::set_invert(bool invert)
{
    impl_->set_invert(invert);
}

void piw::cycler_t::set_curve(float curve)
{
    impl_->set_curve(curve);
}
