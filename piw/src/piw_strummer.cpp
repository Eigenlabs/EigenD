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

#include <piw/piw_strummer.h>
#include <picross/pic_ilist.h>
#include <cmath>

namespace
{
    struct ctl_input_t;
    struct correlator_t;
    struct producer_t;
    struct consumer_t;

    struct producer_t
    {
        public:
            producer_t(correlator_t *);
            ~producer_t();

            void start_event(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer);
            void end_event(unsigned long long t);
            void reset_event(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
            const piw::xevent_data_buffer_t buffer() { return buffer_; }
            const piw::data_nb_t id() { return id_; }

        private:
            friend class correlator_t;
            friend class consumer_t;

            static int ender__(void *this_, void *time_) { ((producer_t *)this_)->end_event(*(unsigned long long *)time_); return 0; }

            correlator_t *correlator_;
            piw::data_nb_t id_;
            pic::ilist_t<consumer_t> consumers_;
            piw::xevent_data_buffer_t buffer_;
            unsigned long long last_event_start_;
    };

    struct consumer_t: pic::element_t<>
    {
        public:
            consumer_t(correlator_t *);
            virtual ~consumer_t();

            void start_event(const piw::data_nb_t &id);
            void end_event();
            const piw::data_nb_t id() { return id_; }
            producer_t *producer() { return producer_; }

            virtual void producer_start(unsigned long long t,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer) {}
            virtual void producer_join(unsigned long long t,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer) {}
            virtual void producer_end(unsigned long long t) {}
            virtual void producer_reset(unsigned s, unsigned long long t,  const piw::dataqueue_t &o, const piw::dataqueue_t &n) {}

        private:
            friend class correlator_t;
            friend class producer_t;

            static int ender__(void *this_, void *) { ((consumer_t *)this_)->end_event(); return 0; }

            correlator_t *correlator_;
            producer_t *producer_;
            piw::data_nb_t id_;
    };

    struct correlator_t
    {
        public:
            correlator_t();
            ~correlator_t();

        private:
            friend class consumer_t;
            friend class producer_t;

            void add_producer(producer_t *p,unsigned long long t);
            void del_producer(producer_t *p,unsigned long long t);
            void add_consumer(consumer_t *c);
            void del_consumer(consumer_t *c);

            pic::lckmultimap_t<piw::data_nb_t, producer_t *,piw::path_less>::nbtype producers_;
            pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype consumers_;
    };

    struct ctl_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<2>
    {
        ctl_wire_t(ctl_input_t *i, const piw::event_data_source_t &);
        ~ctl_wire_t() { invalidate(); }
        void wire_closed() { delete this; }

        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void wire_ticked(unsigned long long f, unsigned long long t);

        producer_t producer_;
        ctl_input_t *input_;
        piw::data_t path_;
        piw::data_nb_t id_;
        piw::xevent_data_buffer_t buffer_;

        bool running_;
        float high_ctl_, low_ctl_;
    };

    struct data_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_sink_t, piw::event_data_source_real_t, consumer_t, pic::element_t<1>
    {
        data_wire_t(piw::strummer_t::impl_t *i, const piw::event_data_source_t &);
        ~data_wire_t() { invalidate(); }
        void wire_closed() { delete this; }

        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void source_ended(unsigned);
        void producer_start(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer);
        void producer_join(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer);
        void producer_end(unsigned long long t);
        void producer_reset(unsigned s, unsigned long long t,  const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void wire_ticked(unsigned long long f, unsigned long long t);
        void send_value(unsigned long long);

        piw::strummer_t::impl_t *input_;
        piw::data_t path_;
        unsigned seq_;

        bool running_;
        unsigned long long from_;
        piw::dataqueue_t ctl_queue_;
        unsigned long long ctl_queue_index_;
        piw::dataqueue_t data_queue_;
        float last_data_, last_ctl_;

        piw::xevent_data_buffer_t data_buffer_;
    };

    struct ctl_input_t: piw::root_t
    {
        ctl_input_t(piw::strummer_t::impl_t *i);
        ~ctl_input_t() { invalidate(); }
        void invalidate();

        piw::wire_t *root_wire(const piw::event_data_source_t &);

        void root_closed() { invalidate(); }
        void root_opened() { root_clock(); root_latency(); }
        void root_clock();
        void root_latency() { }

        piw::strummer_t::impl_t *impl_;
        bct_clocksink_t *clock_;
        std::map<piw::data_t,ctl_wire_t *> children_;
    };

};

struct piw::strummer_t::impl_t: correlator_t, piw::clocksink_t, piw::root_t, piw::root_ctl_t, virtual pic::lckobject_t
{
    impl_t(const piw::cookie_t &, piw::clockdomain_ctl_t *);
    ~impl_t();

    void invalidate();

    piw::wire_t *root_wire(const piw::event_data_source_t &);

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }
    void root_clock();
    void root_latency() {}
    void activate_wire(data_wire_t *w);
    void deactivate_wire(data_wire_t *w);
    void activate_wire(ctl_wire_t *w);
    void deactivate_wire(ctl_wire_t *w);
    void clocksink_ticked(unsigned long long f, unsigned long long t);
    void set_enable(bool b) { piw::tsd_fastcall(__set_enable,this,&b); }
    static int __set_enable(void *, void *);
    void set_key_mix(float km) { piw::tsd_fastcall(__set_key_mix,this,&km); }
    static int __set_key_mix(void *, void *);
    void set_on_threshold(float t) { piw::tsd_fastcall(__set_on_threshold,this,&t); }
    static int __set_on_threshold(void *, void *);
    void set_off_threshold(float t) { piw::tsd_fastcall(__set_off_threshold,this,&t); }
    static int __set_off_threshold(void *, void *);
    void set_trigger_window(unsigned tw) { piw::tsd_fastcall(__set_trigger_window,this,&tw); }
    static int __set_trigger_window(void *, void *);

    bct_clocksink_t *clock_;
    std::map<piw::data_t,data_wire_t *> children_;
    ctl_input_t ctl_input_;
    pic::ilist_t<data_wire_t,1> active_data_wires_;
    pic::ilist_t<ctl_wire_t,2> active_ctl_wires_;
    bool enabled_;
    float key_mix_;
    float threshold_on_, threshold_off_;
    unsigned trigger_window_;
};

int piw::strummer_t::impl_t::__set_enable(void *r_, void *b_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    bool b = *(bool *)b_;
    r->enabled_ = b;

    return 0;
}

int piw::strummer_t::impl_t::__set_key_mix(void *r_, void *km_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    float km = *(float *)km_;
    r->key_mix_ = km;

    return 0;
}

int piw::strummer_t::impl_t::__set_on_threshold(void *r_, void *t_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    float t = *(float *)t_;
    r->threshold_on_ = t;

    return 0;
}

int piw::strummer_t::impl_t::__set_off_threshold(void *r_, void *t_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    float t = *(float *)t_;
    r->threshold_off_ = t;

    return 0;
}

int piw::strummer_t::impl_t::__set_trigger_window(void *r_, void *tw_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    unsigned tw = *(unsigned *)tw_;
    r->trigger_window_ = tw * 1000;

    return 0;
}

piw::wire_t *piw::strummer_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    std::map<piw::data_t,data_wire_t *>::iterator ci;

    if((ci=children_.find(es.path()))!=children_.end())
    {
        delete ci->second;
    }

    data_wire_t *c = new data_wire_t(this,es);
    return c;
}

data_wire_t::data_wire_t(piw::strummer_t::impl_t *i, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), consumer_t(i), input_(i)
{
    path_ = es.path();
    input_->children_.insert(std::make_pair(path_,this));
    subscribe_and_ping(es);
    input_->connect_wire(this,source());
}

void data_wire_t::wire_ticked(unsigned long long f, unsigned long long t)
{
    piw::data_nb_t cd,dd;

    for(;;)
    {
        unsigned long long ct = t;
        bool cok = false;
        
        if(ctl_queue_.read(cd,&ctl_queue_index_,t))
        {
            cok = true;
            ct = cd.time();
        }

        if(data_queue_.latest(dd,0,ct))
        {
            last_data_ = dd.as_norm();
        }

        if(cok)
        {
            last_ctl_ = cd.as_norm();
            ctl_queue_index_++;
            send_value(ct);
        }
        else
        {
            break;
        }

    }

    from_ = t;
}

void data_wire_t::send_value(unsigned long long t)
{
    if(running_)
    {
        float av = fabs(last_ctl_);
        float v = av*(1.0-input_->key_mix_*(1.0-last_data_));
        data_buffer_.signal(1).write_fast(piw::makefloat_bounded_nb(1,-1,0,v,t));
    }
}

void data_wire_t::source_ended(unsigned seq)
{
    if(!input_->enabled_)
    {
        source_ended(seq);
        return;
    }
}

void data_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    ctl_queue_ = piw::dataqueue_t();
    from_ = id.time();
    running_ = false;
    seq_ = seq;

    if(!input_->enabled_)
    {
        source_start(seq,id,b);
        return;
    }

    data_queue_ = b.signal(1);
    piw::data_nb_t d;
    data_queue_.latest(d,0,from_);
    last_data_ = d.as_norm();

    data_buffer_ = piw::xevent_data_buffer_t();
    data_buffer_.set_signal(1,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM));
    for(unsigned i=2;i<MAX_SIGNALS;i++)
    {
        data_buffer_.set_signal(i,b.signal(i));
    }

    last_ctl_ = 0.0;

    input_->activate_wire(this);

    start_event(id);
}

void data_wire_t::producer_start(unsigned long long t, const piw::data_nb_t &data_id, const piw::xevent_data_buffer_t &buffer)
{
    from_ = t;
    producer_reset(1,t,ctl_queue_,buffer.signal(1));
    source_start(seq_,id().restamp(t),data_buffer_);
    running_ = true;
}

void data_wire_t::producer_join(unsigned long long t, const piw::data_nb_t &data_id, const piw::xevent_data_buffer_t &buffer)
{
    unsigned long long now = piw::tsd_time();
    if(now - t < input_->trigger_window_)
    {
        from_ = t;
        producer_reset(1,now,ctl_queue_,buffer.signal(1));
        source_start(seq_,id().restamp(t),data_buffer_);
        running_ = true;
    }
}

void data_wire_t::producer_end(unsigned long long t)
{
    from_ = t;
    producer_reset(1,t,ctl_queue_,piw::dataqueue_t());
    if(running_)
    {
        source_end(t);
        running_ = false;
    }
}

void data_wire_t::producer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s==1)
    {
        ctl_queue_ = n;
        last_ctl_ = 0.0;

        if(ctl_queue_.isvalid())
        {
            piw::data_nb_t d;
            if(ctl_queue_.latest(d,&ctl_queue_index_,from_))
            {
                last_ctl_ = d.as_norm();
            }
        }

        send_value(t);
    }
}

void data_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(!input_->enabled_)
    {
        source_buffer_reset(s,t,o,n);
        return;
    }

    if(s==1)
    {
        data_queue_ = n;
        if(data_queue_.isvalid())
        {
            piw::data_nb_t d;
            data_queue_.latest(d,0,from_);
            last_data_ = d.as_norm();
        }
    }
    else
    {
        data_buffer_.set_signal(s,n);
        source_buffer_reset(s,t,o,n);
    }
}

bool data_wire_t::event_end(unsigned long long t)
{
    if(!input_->enabled_)
    {
        return source_end(t);
    }

    end_event();

    source_end(t);

    input_->deactivate_wire(this);
    ctl_queue_.clear();
    data_queue_.clear();

    return true;
}

void data_wire_t::invalidate()
{
    input_->deactivate_wire(this);
    unsubscribe();
    piw::wire_t::disconnect();
    piw::wire_ctl_t::disconnect();
    source_shutdown();
    input_->children_.erase(path_);
}

void piw::strummer_t::impl_t::invalidate()
{
    tick_disable();

    std::map<piw::data_t,data_wire_t *>::iterator ci;

    while((ci=children_.begin())!=children_.end())
    {
        delete ci->second;
    }

    piw::root_t::disconnect();

    if(clock_)
    {
        remove_upstream(clock_);
        clock_ = 0;
    }
}

ctl_wire_t::ctl_wire_t(ctl_input_t *i, const piw::event_data_source_t &es): producer_(i->impl_), input_(i)
{
    path_ = es.path();
    input_->children_.insert(std::make_pair(path_,this));
    subscribe_and_ping(es);
}

void ctl_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
    input_->children_.erase(path_);
}

void ctl_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    if(!input_->impl_->enabled_)
    {
        return;
    }

    high_ctl_ = 0.0;
    low_ctl_ = 0.0;
    running_ = false;

    id_ = id;
    buffer_ = b;

    input_->impl_->activate_wire(this);
}

void ctl_wire_t::wire_ticked(unsigned long long f, unsigned long long t)
{
    if(!buffer_.isvalid(1))
        return;

    piw::dataqueue_t queue = buffer_.signal(1);

    unsigned long long i;
    piw::data_nb_t d;

    if(queue.earliest(d,&i,f))
    {
        do
        {
            float av = fabs(d.as_norm());

            if(av<low_ctl_)
            {
                low_ctl_ = av;
            }

            if(av>high_ctl_)
            {
                high_ctl_ = av;
            }

            if(running_)
            {
                bool off = false;

                if(input_->impl_->threshold_off_ > input_->impl_->threshold_on_)
                {
                    if(av <= input_->impl_->threshold_off_ && high_ctl_ >= input_->impl_->threshold_off_)
                    {
                        off = true;
                    }

                    if(av <= input_->impl_->threshold_on_)
                    {
                        off = true;
                    }
                }
                else
                {
                    if(av <= input_->impl_->threshold_off_)
                    {
                        off = true;
                    }
                }


                if(off)
                {
                    producer_.end_event(d.time());
                    running_ = false;
                    low_ctl_ = av;
                }
            }
            else
            {
                if(av > input_->impl_->threshold_on_ && low_ctl_ <= input_->impl_->threshold_on_)
                {
                    producer_.start_event(d.time(), id_, buffer_);
                    running_ = true;
                    high_ctl_ = 0.0;
                }
            }
            i++;
        }
        while(queue.read(d,&i,t));
    }
}

void ctl_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    producer_.reset_event(s,t,o,n);
}

bool ctl_wire_t::event_end(unsigned long long t)
{
    if(!input_->impl_->enabled_)
    {
        return true;
    }

    producer_.end_event(t);

    id_ = piw::data_nb_t();
    input_->impl_->deactivate_wire(this);
    return true;
}

piw::wire_t *ctl_input_t::root_wire(const piw::event_data_source_t &es)
{
    std::map<piw::data_t,ctl_wire_t *>::iterator ci;

    if((ci=children_.find(es.path()))!=children_.end())
    {
        delete ci->second;
    }

    ctl_wire_t *c = new ctl_wire_t(this,es);
    return c;
}

ctl_input_t::ctl_input_t(piw::strummer_t::impl_t *i): piw::root_t(0), impl_(i), clock_(0)
{
}

void ctl_input_t::invalidate()
{
    std::map<piw::data_t,ctl_wire_t *>::iterator ci;

    while((ci=children_.begin())!=children_.end())
    {
        delete ci->second;
    }

    piw::root_t::disconnect();

    if(clock_)
    {
        impl_->remove_upstream(clock_);
        clock_ = 0;
    }
}

void ctl_input_t::root_clock()
{
    bct_clocksink_t *c = get_clock();

    if(c!=clock_)
    {
        if(clock_)
        {
            impl_->remove_upstream(clock_);
        }

        clock_ = c;

        if(clock_)
        {
            impl_->add_upstream(clock_);
        }
    }
}

void piw::strummer_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
    ctl_wire_t *cw;

    for(cw=active_ctl_wires_.head(); cw!=0; cw=active_ctl_wires_.next(cw))
    {
        cw->wire_ticked(f,t);
    }

    data_wire_t *dw;

    for(dw=active_data_wires_.head(); dw!=0; dw=active_data_wires_.next(dw))
    {
        dw->wire_ticked(f,t);
    }
}

void piw::strummer_t::impl_t::root_clock()
{
    bct_clocksink_t *c = get_clock();

    if(c!=clock_)
    {
        if(clock_)
        {
            remove_upstream(clock_);
        }

        clock_ = c;

        if(clock_)
        {
            add_upstream(clock_);
        }
    }
}

piw::strummer_t::impl_t::impl_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d): piw::root_t(0), clock_(0), ctl_input_(this)
{
    d->sink(this,"ucorrelator");
    set_clock(this);
    connect(c);
    tick_enable(true);
    enabled_ = true;
    key_mix_ = 0.0;
    threshold_on_ = 0.01;
    threshold_off_ = 0.01;
    trigger_window_= 0;
}

piw::strummer_t::impl_t::~impl_t()
{
    tick_disable();
    invalidate();
    close_sink();
}

void piw::strummer_t::impl_t::activate_wire(data_wire_t *w)
{
    if(!active_data_wires_.head())
    {
        tick_suppress(false);
    }

    active_data_wires_.append(w);
}

void piw::strummer_t::impl_t::activate_wire(ctl_wire_t *w)
{
    if(!active_ctl_wires_.head())
    {
        tick_suppress(false);
    }

    active_ctl_wires_.append(w);
}

void piw::strummer_t::impl_t::deactivate_wire(data_wire_t *w)
{
    active_data_wires_.remove(w);

    if(!active_data_wires_.head())
    {
        tick_suppress(true);
    }
}

void piw::strummer_t::impl_t::deactivate_wire(ctl_wire_t *w)
{
    active_ctl_wires_.remove(w);

    if(!active_ctl_wires_.head())
    {
        tick_suppress(true);
    }
}

piw::strummer_t::strummer_t(const piw::cookie_t &c,piw::clockdomain_ctl_t *d): impl_(new impl_t(c,d))
{
}

piw::strummer_t::~strummer_t()
{
    delete impl_;
}

piw::cookie_t piw::strummer_t::data_cookie()
{
    return piw::cookie_t(impl_);
}

piw::cookie_t piw::strummer_t::ctl_cookie()
{
    return piw::cookie_t(&impl_->ctl_input_);
}

void piw::strummer_t::enable(bool b)
{
    impl_->set_enable(b);
}

void piw::strummer_t::set_on_threshold(float t)
{
    impl_->set_on_threshold(t);
}

void piw::strummer_t::set_off_threshold(float t)
{
    impl_->set_off_threshold(t);
}

void piw::strummer_t::set_key_mix(float km)
{
    impl_->set_key_mix(km);
}

void piw::strummer_t::set_trigger_window(unsigned tw)
{
    impl_->set_trigger_window(tw);
}

producer_t::producer_t(correlator_t *c): correlator_(c), last_event_start_(0)
{
}

producer_t::~producer_t()
{
    unsigned long long t = piw::tsd_time();
    piw::tsd_fastcall(ender__,this,&t);
}

void producer_t::start_event(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer)
{
    end_event(id.time());

    id_ = id;
    buffer_ = buffer;
    last_event_start_ = t;
    correlator_->add_producer(this,t);
}

void producer_t::end_event(unsigned long long t)
{
    if(!id_.is_null())
    {
        correlator_->del_producer(this,t);
        id_ = piw::data_nb_t();
    }
}

void producer_t::reset_event(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    for(consumer_t *c = consumers_.head(); c!=0; c=consumers_.next(c))
    {
        c->producer_reset(s,t,o,n);
    }
}

consumer_t::consumer_t(correlator_t *c): correlator_(c), producer_(0)
{
}

consumer_t::~consumer_t()
{
    piw::tsd_fastcall(ender__,this,0);
}

void consumer_t::start_event(const piw::data_nb_t &id)
{
    end_event();
    id_ = id;
    correlator_->add_consumer(this);
}

void consumer_t::end_event()
{
    if(!id_.is_null())
    {
        correlator_->del_consumer(this);
        id_ = piw::data_nb_t();
    }
}

void correlator_t::add_producer(producer_t *p, unsigned long long t)
{
    pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype::iterator i,e;

    piw::data_nb_t pid = p->id_;
    unsigned pidlen = pid.as_pathlen();
    const unsigned char *pidpath = pid.as_path();

    i = consumers_.begin();
    e = consumers_.end();

    producers_.insert(std::make_pair(pid,p));

    for(;i!=e;i++)
    {
        consumer_t *c = i->second;
        piw::data_nb_t cid = c->id_;

        if(cid.is_null())
        {
            continue;
        }

        unsigned cidlen = cid.as_pathlen();
        const unsigned char *cidpath = cid.as_path();

        if(pidlen>cidlen || memcmp(cidpath,pidpath,pidlen)!=0)
        {
            continue;
        }

        if(c->producer_)
        {
            if(c->producer_->id_.as_pathlen()>=pidlen)
            {
                continue;
            }

            c->producer_->consumers_.remove(c);
            c->producer_ = 0;
        }

        c->producer_ = p;
        p->consumers_.append(c);
        c->producer_start(t, p->id_, p->buffer_);
    }
}

void correlator_t::del_producer(producer_t *p,unsigned long long t)
{
    producers_.erase(p->id_);

    consumer_t *c;

    while((c=p->consumers_.pop_front())!=0)
    {
        c->producer_ = 0;
        c->producer_end(t);
    }
}

void correlator_t::add_consumer(consumer_t *c)
{
    piw::data_nb_t cid = c->id_;
    unsigned cidlen = cid.as_pathlen();
    const unsigned char *cidpath = cid.as_path();

    consumers_.insert(std::make_pair(cid,c));

    for(int l=cidlen;l>=0;l--)
    {
        std::pair<
            pic::lckmultimap_t<piw::data_nb_t,producer_t *,piw::path_less>::nbtype::const_iterator,
            pic::lckmultimap_t<piw::data_nb_t,producer_t *,piw::path_less>::nbtype::const_iterator> range =
                producers_.equal_range(piw::makepath_nb(cidpath,l));

        for(; range.first != range.second; range.first++)
        {
            producer_t *p = range.first->second;
            c->producer_ = p;
            p->consumers_.append(c);
            c->producer_join(p->last_event_start_, p->id_, p->buffer_);
            return;
        }
    }
}

void correlator_t::del_consumer(consumer_t *c)
{
    consumers_.erase(c->id_);

    if(c->producer_)
    {
        c->producer_->consumers_.remove(c);
        c->producer_ = 0;
    }
}


correlator_t::correlator_t()
{
}

correlator_t::~correlator_t()
{
}
