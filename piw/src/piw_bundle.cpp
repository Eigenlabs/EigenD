
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

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_address.h>
#include <piw/piw_tsd.h>
#include <map>
#include <sstream>

namespace
{
    struct decoder_wire_t : piw::wire_t
    {
        decoder_wire_t(piw::decoder_t::impl_t *parent, const piw::event_data_source_t &es);
        void invalidate();
        void wire_closed();

        piw::decoder_t::impl_t *parent_;
        piw::event_data_source_t source_;
        piw::wire_t *wire_;
        piw::wire_ctl_t wirectl_;
    };
}

struct piw::decoder_t::impl_t
{
    impl_t(decode_ctl_t *ctl);
    ~impl_t();
    piw::wire_t *wire(const piw::event_data_source_t &es);
    void invalidate();

    decode_ctl_t *ctl_;
    std::map<piw::data_t,decoder_wire_t *> wires_;
};

decoder_wire_t::decoder_wire_t(piw::decoder_t::impl_t *parent, const piw::event_data_source_t &es) : parent_(parent), source_(es)
{
    wire_ = parent_->ctl_->wire_create(es);

    if(wire_)
    {
        wirectl_.set_wire(wire_);
    }

    std::map<piw::data_t,decoder_wire_t *>::iterator i = parent_->wires_.find(source_.path());

    if(i != parent_->wires_.end())
    {
        i->second->invalidate();
    }

    parent_->wires_.insert(std::make_pair(source_.path(),this));
}

void decoder_wire_t::invalidate()
{
    wirectl_.disconnect();
    disconnect();
    parent_->wires_.erase(source_.path());
}

piw::decoder_t::decoder_t(piw::decode_ctl_t *ctl) : root_t(0), impl_(new impl_t(ctl))
{
}

piw::decoder_t::~decoder_t()
{
    delete impl_;
}

void piw::decoder_t::shutdown()
{
    impl_->invalidate();
}

void piw::decoder_t::root_closed()
{
    impl_->invalidate();
}

void piw::decoder_t::root_clock()
{
    impl_->ctl_->set_clock(get_clock());
}

void piw::decoder_t::root_latency()
{
    impl_->ctl_->set_latency(get_latency());
}

void piw::decoder_t::root_opened()
{
    root_clock();
    root_latency();
}

unsigned piw::decoder_t::wire_count()
{
    return impl_->wires_.size();
}

piw::wire_t *piw::decoder_t::root_wire(const piw::event_data_source_t &es)
{
    return impl_->wire(es);
}

void piw::wire_ctl_t::set_wire(wire_t *w)
{
    disconnect();
    _wire=w;
    if(w)
    {
        w->_ctl=this;
    }
}

piw::root_t::root_t(unsigned l): _ctl(0)
{
}

piw::root_t::~root_t()
{
    disconnect();
}

bct_clocksink_t *piw::root_t::get_clock()
{
    return _ctl?_ctl->_clock:0;
}

piw::wire_ctl_t::wire_ctl_t(): _wire(0)
{
}

piw::wire_ctl_t::~wire_ctl_t()
{
    disconnect();
}

void piw::wire_ctl_t::disconnect()
{
    if (_wire)
    {
        try
        {
            _wire->_ctl=0;
            _wire->wire_closed();
        }
        CATCHLOG();
        _wire=0;
    }
}

void piw::wire_t::disconnect()
{
    if(_ctl)
    {
        _ctl->_wire=0;
        _ctl=0;
    }
}

piw::root_ctl_t::~root_ctl_t()
{
    disconnect();
}

void piw::root_ctl_t::set_clock(bct_clocksink_t *c)
{
    _clock=c;

    if(_root)
    {
        try
        {
            _root->root_clock();
        }
        CATCHLOG()
    }
}

piw::root_ctl_t::root_ctl_t(): _root(0), _clock(0), latency_(0)
{
}

void piw::root_ctl_t::disconnect()
{
    if (_root)
    {
        _root->_ctl = 0;

        try
        {
            _root->root_clock();
            _root->root_closed();
        }
        CATCHLOG();

        _root=0;
    }
}

void piw::root_t::disconnect()
{
    if(_ctl)
    {
        _ctl->_root=0;
        _ctl=0;
    }
}

unsigned piw::root_t::get_latency()
{
    return _ctl?_ctl->latency_:0;
}

void piw::root_ctl_t::connect_wire(wire_ctl_t *w, const piw::event_data_source_t &es)
{
    w->disconnect();

    if(_root)
    {
        piw::wire_t *rw = 0;

        try
        {
            rw = _root->root_wire(es);
        }
        CATCHLOG();

        w->set_wire(rw);
    }
}

void piw::root_ctl_t::connect(const cookie_t &c)
{
    disconnect();

    root_t *r = c._root.ptr();

    if(r)
    {
        PIC_ASSERT(!r->_ctl);

        _root=r;
        _root->_ctl=this;
        set_latency(latency_);
        _root->root_opened();
    }
}

void piw::root_ctl_t::set_latency(unsigned l)
{
    latency_=l;

    if(_root)
    {
        try
        {
            _root->root_latency();
        }
        CATCHLOG()
    }
}

piw::wire_t::wire_t(): _ctl(0)
{
}

piw::wire_t::~wire_t()
{
    disconnect();
}

piw::cookie_t::cookie_t(root_t *r): _root(r)
{
}

piw::xevent_data_buffer_t::xevent_data_buffer_t(unsigned long long mask,unsigned size) : event_(pic::ref(new event_data_t))
{
    for(unsigned i=0; i<MAX_SIGNALS; ++i)
    {
        if(mask&(1ULL<<i))
        {
            event_->signals_[i] = tsd_dataqueue(size);
        }
    }
}

void piw::xevent_data_buffer_t::merge(const piw::xevent_data_buffer_t &b,unsigned long long m)
{
    for(unsigned i=0; i<MAX_SIGNALS; ++i)
    {
        if(m&(1ULL<<i))
            event_->signals_[i] = b.event_->signals_[i];
    }
}

void piw::xevent_data_buffer_t::dump(bool full) const
{
    pic::logmsg() << "--- event data dump";
    for(unsigned i=0; i<MAX_SIGNALS; ++i)
    {
        dataqueue_t q = event_->signals_[i];
        if(q.isvalid())
        {
            pic::logmsg() << " + signal " << (i+1) << ":";
            q.dump(full);
        }
    }
    pic::logmsg() << "--- end dump";
}

piw::evtiterator_t::evtiterator_t(const pic::ref_t<piw::event_data_t> &e): event_(e)
{
    memset(index_,0,sizeof(index_));
}

bool piw::evtiterator_t::next(unsigned long long m,unsigned &sig,piw::data_nb_t &data,unsigned long long t)
{
    int x=-1;
    piw::data_nb_t d;

    for(unsigned i=0; i<MAX_SIGNALS; ++i)
    {
        if(!(m&(1ULL<<i)))
        {
            continue;
        }

        piw::data_nb_t d2;
        if(!event_->signals_[i].read(d2,&index_[i],t))
        {
            continue;
        }

        t=d2.time();
        d=d2;
        x=i;
    }

    if(x<0)
    {
        return false;
    }

    index_[x]++;
    data=d;
    sig=x+1;

    return true;
}

bool piw::evtiterator_t::latest(unsigned sig,piw::data_nb_t &data,unsigned long long t)
{
    unsigned i=sig-1;
    if(event_->signals_[i].latest(data,&index_[i],t))
    {
        index_[i]++;
        return true;
    }
    return false;
}

bool piw::evtiterator_t::nextsig(unsigned sig,piw::data_nb_t &data,unsigned long long t)
{
    unsigned i=sig-1;
    if(event_->signals_[i].read(data,&index_[i],t))
    {
        index_[i]++;
        return true;
    }
    return false;
}

void piw::evtiterator_t::reset(unsigned sig,unsigned long long t)
{
    unsigned i=sig-1;
    piw::data_nb_t d;
    event_->signals_[i].earliest(d,&index_[i],t);
}

void piw::evtiterator_t::reset_all(unsigned long long t)
{
    for(unsigned i=0; i<MAX_SIGNALS; ++i)
    {
        if(event_->signals_[i].isvalid())
        {
            piw::data_nb_t d;
            event_->signals_[i].earliest(d,&index_[i],t);
        }
    }
}

void piw::evtiterator_t::dump(unsigned sig,bool full)
{
    unsigned i=sig-1;
    event_->signals_[i].dump(full);
}

void piw::evtiterator_t::dump_index()
{
    for(unsigned i=0; i<MAX_SIGNALS; ++i)
    {
        pic::logmsg() << "signal " << (i+1) << " index " << index_[i];
    }
}

void piw::evtsource_data_t::event_start(unsigned seq, const piw::data_nb_t &id,const xevent_data_buffer_t &init) // fast
{
//    pic::logmsg() << (void *)this << " starting event on " << id; init.dump(false);

    PIC_ASSERT(id.is_path());
    PIC_WARN(id.time()!=0ULL);
    if(event_.get().is_path() && id.compare_path(event_.get())!=0)
    {
        pic::logmsg() << "bad evt:" << event_ << " id:" << id;
        return;
    }

    if(!seq)
    {
        seq=seq_internal_;
        if(++seq_internal_==0) seq_internal_=1;
    }

    seq_current_=seq;
    lingering_count_=0;

    event_.set_nb(id);
    buffer_ = init;

    pic::lcklist_t<event_data_sink_t *>::nbtype::const_iterator i=clients_.begin(),e=clients_.end();

    for(; i!=e; ++i)
    {
        (*i)->started_=true;
        (*i)->lingering_=false;
        (*i)->event_start(seq,id,init);
        (*i)->seq_outstanding_=0;
    }
}

void piw::evtsource_data_t::event_ended(unsigned seq) // fast
{
    if(seq==seq_current_)
    {

        if(lingering_count_>0)
        {
            --lingering_count_;
        }
        else
        {
            pic::logmsg() << "warning, stray event_ended received";
            return;
        }

        if(lingering_count_==0)
        {
            event_.clear();
            seq_current_=0;
            buffer_ = xevent_data_buffer_t();

            pic::flipflop_t<event_data_source_real_t *>::guard_t g(source_);

            if(g.value())
            {
                g.value()->source_ended(seq);
            }
        }
    }
}

bool piw::evtsource_data_t::event_end(unsigned long long t) // fast
{
    pic::lcklist_t<event_data_sink_t *>::nbtype::const_iterator i;

    //pic::printmsg() << "event end: " << clients_.size();
    //for(i=clients_.begin(); i!=clients_.end(); ++i) pic::printmsg() << "client " << *i;

    for(i=clients_.begin(); i!=clients_.end(); ++i)
    {
        event_data_sink_t *es(*i);
        if(!es->started_)
        {
            //pic::logmsg() << "event end skipped (not started)";
            continue;
        }

        if(!es->event_end(t))
        {
            es->lingering_=true;
            ++lingering_count_;
            es->seq_outstanding_=seq_current_;
        }
        else
        {
            es->lingering_=false;
            (*i)->seq_outstanding_=0;
        }
    }

    if(lingering_count_==0)
    {
        event_.clear();
        seq_current_=0;
        return true;
    }

    return false;
}

void piw::evtsource_data_t::event_buffer_reset(unsigned s,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n) // fast
{
    if(!n.isvalid())
    {
        return;
    }

    pic::lcklist_t<event_data_sink_t *>::nbtype::const_iterator i=clients_.begin(),e=clients_.end();

    for(; i!=e; ++i)
    {
        if(!(*i)->started_)
            continue;

        (*i)->event_buffer_reset(s,t,o,n);
    }
}

int piw::event_data_sink_t::subscribe__(void *self_, void *source_)
{
    unsubscribe__(self_,source_);
    event_data_sink_t *self = (event_data_sink_t *)self_;
    event_data_source_t *source = (event_data_source_t *)source_;
    self->seq_outstanding_=0;
    self->started_=false;
    self->subject_ = source->list_;
    self->subject_->clients_.push_back(self);
    return 1;
}

int piw::event_data_sink_t::subscribe_and_ping__(void *sink_, void *source_)
{
    subscribe__(sink_,source_);
    event_data_sink_t *self = (event_data_sink_t *)sink_;
    if(!self->subject_.isvalid())
    {
        return 0;
    }

    if(!self->subject_->event_.get().is_path())
    {
        return 0;
    }

    //unsigned long long t = self->subject_->event_.time();
    //self->subject_->buffer_.restamp_latest(t);
    self->started_ = true;
    self->event_start(self->subject_->seq_current_,self->subject_->event_,self->subject_->buffer_);
    return 1;
}

int piw::event_data_sink_t::unsubscribe__(void *sink_, void *_)
{
    event_data_sink_t *self = (event_data_sink_t *)sink_;

    if(self->seq_outstanding_)
    {
        if(self->subject_.isvalid())
        {
            self->subject_->event_ended(self->seq_outstanding_);
        }

        self->seq_outstanding_=0;
    }

    if(self->subject_.isvalid())
    {
        self->subject_->clients_.remove(self);
        self->subject_.clear();
    }

    self->started_=false;

    return 1;
}

void piw::event_data_sink_t::unsubscribe()
{
    tsd_fastcall(unsubscribe__,(void *)this,0);
}

void piw::event_data_sink_t::unsubscribe_fast()
{
    unsubscribe__((void *)this,0);
}

piw::data_nb_t piw::event_data_sink_t::current_id()
{
    return subject_->event_;
}

piw::xevent_data_buffer_t piw::event_data_sink_t::current_data()
{
    return subject_->buffer_;
}

void piw::event_data_sink_t::event_ended(unsigned seq)
{
    if(lingering_)
    {
        lingering_=false;
        subject_->event_ended(seq);
    }
}

void piw::event_data_sink_t::subscribe_and_ping(const piw::event_data_source_t &es)
{
    tsd_fastcall(subscribe_and_ping__,(void *)this,(void *)&es);
}

void piw::event_data_sink_t::subscribe_and_ping_fast(const piw::event_data_source_t &es)
{
    subscribe_and_ping__((void *)this,(void *)&es);
}

void piw::event_data_sink_t::subscribe(const piw::event_data_source_t &s)
{
    tsd_fastcall(subscribe__,(void *)this,(void *)&s);
}

void piw::event_data_sink_t::subscribe_fast(const piw::event_data_source_t &s)
{
    subscribe__((void *)this, (void *)&s);
}

static int start__(void *p1,void *p2,void *p3)
{
    piw::event_data_source_real_t *es = (piw::event_data_source_real_t *)p1;
    const piw::data_nb_t id = piw::data_nb_t::from_given((bct_data_t)p2);
    piw::xevent_data_buffer_t ei = *(const piw::xevent_data_buffer_t *)p3;
    es->source_start(0,id,ei);
    return 0;
}

static int end__(void *p1,void *p2)
{
    piw::event_data_source_real_t *es = (piw::event_data_source_real_t *)p1;
    unsigned long long t = *(unsigned long long *)p2;
    es->source_end(t);
    return 0;
}

void piw::event_data_source_real_t::start_slow(const piw::data_t &id,const xevent_data_buffer_t &init)
{
    PIC_WARN(id.time()!=0ULL);
    tsd_fastcall3(start__,this,(void*)id.give_copy(),(void*)&init);
}

void piw::event_data_source_real_t::end_slow(unsigned long long t)
{
    tsd_fastcall(end__,this,(void*)&t);
}

void piw::event_data_source_real_t::source_shutdown()
{
    list_->source_.set(0);
}

piw::decoder_t::impl_t::impl_t(decode_ctl_t *ctl) : ctl_(ctl)
{
}

piw::decoder_t::impl_t::~impl_t()
{
    invalidate();
}

void piw::decoder_t::impl_t::invalidate()
{
    std::map<piw::data_t,decoder_wire_t *>::iterator i,e;
    e = wires_.end();
    while((i=wires_.begin()) != e)
    {
        i->second->invalidate();
        delete i->second;
    }
}

piw::wire_t *piw::decoder_t::impl_t::wire(const piw::event_data_source_t &es)
{
    return new decoder_wire_t(this,es);
}

void decoder_wire_t::wire_closed()
{
    invalidate();
    delete this;
}

piw::event_data_source_real_t::event_data_source_real_t(const piw::data_t &path) : list_(pic::ref(new evtsource_data_t(this,path)))
{
}

piw::evtsource_data_t::evtsource_data_t(event_data_source_real_t *src, const piw::data_t &path): source_(src), path_(path), seq_internal_(1), seq_current_(0), lingering_count_(0)
{
}

piw::evtsource_data_t::~evtsource_data_t()
{
}
