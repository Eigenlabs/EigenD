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
#include <piw/piw_keys.h>
#include <piw/piw_tsd.h>
#include <picross/pic_ilist.h>
#include <picross/pic_functor.h>
#include <cmath>

#define SIG_STRUM_BREATH 1 
#define SIG_STRUM_KEY 2 
#define SIG_STRUM_PRESSURE 3 
#define SIG_STRUM_ROLL 4
#define SIG_STRUM_YAW 5
#define SIG_STRUM_FIRST SIG_STRUM_BREATH
#define SIG_STRUM_LAST SIG_STRUM_YAW

#define SIG_KEY 1 
#define SIG_PRESSURE 2
#define SIG_ROLL 3
#define SIG_YAW 4
#define SIG_DATA_FIRST SIG_KEY
#define SIG_DATA_LAST SIG_YAW

namespace
{
    struct strum_wire_t;
    struct strum_input_t;
    struct correlator_t;
    struct producer_t;
    struct consumer_t;

    typedef pic::functor_t<bool(consumer_t *)> consumer_check_t;

    struct producer_t: virtual pic::tracked_t
    {
        public:
            producer_t(correlator_t *, consumer_check_t c);
            ~producer_t();

            void start_event(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer);
            void end_event(unsigned long long t);
            void reset_event(unsigned s, unsigned long long t, const piw::dataqueue_t &n);
            const piw::xevent_data_buffer_t buffer() { return buffer_; }
            const piw::data_nb_t id() { return id_; }

            virtual bool applies_to_consumer(consumer_t *c) { return consumer_check_(c); }

        private:
            friend class correlator_t;
            friend class consumer_t;
            friend class strum_wire_t;

            static int ender__(void *this_, void *time_) { ((producer_t *)this_)->end_event(*(unsigned long long *)time_); return 0; }

            correlator_t *correlator_;
            consumer_check_t consumer_check_;
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
            virtual void producer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &n) {}

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

            pic::lckmultimap_t<piw::data_nb_t, producer_t *>::nbtype producers_;
            pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype consumers_;
    };

    struct strum_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<2>, virtual pic::tracked_t
    {
        strum_wire_t(strum_input_t *i, const piw::event_data_source_t &);
        ~strum_wire_t() { invalidate(); }
        void wire_closed() { delete this; }

        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        bool applies_to_consumer(consumer_t *);

        producer_t producer_;
        strum_input_t *input_;
        piw::data_t path_;
        piw::data_nb_t id_;
        piw::xevent_data_buffer_t buffer_;
        piw::coordinate_t last_strum_key_;
        bool breath_started_;
        bool key_started_;
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
        void producer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &n);
        void wire_ticked(unsigned long long f, unsigned long long t);
        void send_value(unsigned long long);

        piw::strummer_t::impl_t *input_;
        piw::data_t path_;
        unsigned seq_;

        bool data_running_;
        bool strum_running_;
        unsigned long long from_;

        piw::dataqueue_t strum_queue_[SIG_STRUM_LAST];
        unsigned long long strum_index_[SIG_STRUM_LAST];
        float last_strum_data_[SIG_STRUM_LAST];

        piw::dataqueue_t data_queue_[SIG_DATA_LAST];
        unsigned long long data_index_[SIG_DATA_LAST];
        float last_data_[SIG_DATA_LAST];

        piw::xevent_data_buffer_t data_buffer_;
    };

    struct strum_input_t: piw::root_t
    {
        strum_input_t(piw::strummer_t::impl_t *i);
        ~strum_input_t() { invalidate(); }
        void invalidate();

        piw::wire_t *root_wire(const piw::event_data_source_t &);

        void root_closed() { invalidate(); }
        void root_opened() { root_clock(); root_latency(); }
        void root_clock();
        void root_latency() { }

        piw::strummer_t::impl_t *impl_;
        bct_clocksink_t *clock_;
        std::map<piw::data_t,strum_wire_t *> children_;
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
    void activate_wire(strum_wire_t *w);
    void deactivate_wire(strum_wire_t *w);
    void clocksink_ticked(unsigned long long f, unsigned long long t);
    void set_enable(bool b) { piw::tsd_fastcall(__set_enable,this,&b); }
    static int __set_enable(void *, void *);
    void set_trigger_window(unsigned tw) { piw::tsd_fastcall(__set_trigger_window,this,&tw); }
    static int __set_trigger_window(void *, void *);
    void set_strum_breath_scale(float scale) { piw::tsd_fastcall(__set_strum_breath_scale,this,&scale); }
    static int __set_strum_breath_scale(void *, void *);
    void set_pressure_scale(float scale) { piw::tsd_fastcall(__set_pressure_scale,this,&scale); }
    static int __set_pressure_scale(void *, void *);
    void set_strum_pressure_scale(float scale) { piw::tsd_fastcall(__set_strum_pressure_scale,this,&scale); }
    static int __set_strum_pressure_scale(void *, void *);
    void set_roll_scale(float scale) { piw::tsd_fastcall(__set_roll_scale,this,&scale); }
    static int __set_roll_scale(void *, void *);
    void set_strum_roll_scale(float scale) { piw::tsd_fastcall(__set_strum_roll_scale,this,&scale); }
    static int __set_strum_roll_scale(void *, void *);
    void set_yaw_scale(float scale) { piw::tsd_fastcall(__set_yaw_scale,this,&scale); }
    static int __set_yaw_scale(void *, void *);
    void set_strum_yaw_scale(float scale) { piw::tsd_fastcall(__set_strum_yaw_scale,this,&scale); }
    static int __set_strum_yaw_scale(void *, void *);
    void clear_breath_courses() { piw::tsd_fastcall(__clear_breath_courses,this,0); }
    static int __clear_breath_courses(void *, void *);
    void add_breath_course(int course) { piw::tsd_fastcall(__add_breath_course,this,&course); }
    static int __add_breath_course(void *, void *);
    void clear_key_courses() { piw::tsd_fastcall(__clear_key_courses,this,0); }
    static int __clear_key_courses(void *, void *);
    void add_key_course(int column, int row, int course) { piw::tsd_fastcall4(__add_key_course,this,&column,&row,&course); }
    static int __add_key_course(void *, void *, void *, void *);
    void set_strum_note_end(bool b) { piw::tsd_fastcall(__set_strum_note_end,this,&b); }
    static int __set_strum_note_end(void *, void *);

    bct_clocksink_t *clock_;
    std::map<piw::data_t,data_wire_t *> children_;
    strum_input_t strum_input_;
    pic::ilist_t<data_wire_t,1> active_data_wires_;
    std::set<int> breath_courses_;
    std::multimap<piw::coordinate_t,int> key_courses_;
    bool enabled_;
    unsigned trigger_window_;
    float strum_breath_scale_;
    float pressure_scale_;
    float strum_pressure_scale_;
    float roll_scale_;
    float strum_roll_scale_;
    float yaw_scale_;
    float strum_yaw_scale_;
    bool strum_note_end_;
};

int piw::strummer_t::impl_t::__set_enable(void *r_, void *b_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->enabled_ = *(bool *)b_;

    return 0;
}

int piw::strummer_t::impl_t::__set_trigger_window(void *r_, void *tw_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    unsigned tw = *(unsigned *)tw_;
    r->trigger_window_ = tw * 1000;

    return 0;
}

int piw::strummer_t::impl_t::__set_strum_breath_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->strum_breath_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__set_pressure_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->pressure_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__set_strum_pressure_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->strum_pressure_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__set_roll_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->roll_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__set_strum_roll_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->strum_roll_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__set_yaw_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->yaw_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__set_strum_yaw_scale(void *r_, void *sc_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->strum_yaw_scale_ = *(float *)sc_;

    return 0;
}

int piw::strummer_t::impl_t::__clear_breath_courses(void *r_, void *)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->breath_courses_.clear();

    return 0;
}

int piw::strummer_t::impl_t::__add_breath_course(void *r_, void *course_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    int course = *(int *)course_;
    r->breath_courses_.insert(course);

    return 0;
}

int piw::strummer_t::impl_t::__clear_key_courses(void *r_, void *)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->key_courses_.clear();

    return 0;
}

int piw::strummer_t::impl_t::__add_key_course(void *r_, void *col_, void *row_, void *course_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    int col = *(int *)col_;
    int row = *(int *)row_;
    int course = *(int *)course_;
    r->key_courses_.insert(std::make_pair(coordinate_t(col,row),course));

    return 0;
}

int piw::strummer_t::impl_t::__set_strum_note_end(void *r_, void *b_)
{
    piw::strummer_t::impl_t *r = (piw::strummer_t::impl_t *)r_;
    r->strum_note_end_ = *(bool *)b_;

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
    piw::data_nb_t dd;

    while(true)
    {
        unsigned long long tt = t;

        bool strum_valid = false;
        bool strum_read = false;

        for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
        {
            int index = i-1;
            if(strum_queue_[index].isvalid())
            {
                strum_valid = true;
                if(strum_queue_[index].read(dd,&strum_index_[index],t))
                {
                    strum_read = true;
                    tt = std::max(tt,dd.time());
                    strum_index_[index]++;
                    last_strum_data_[index] = dd.as_norm();
                }
            }
        }

        bool data_valid = false;
        bool data_read = false;

        for(int i=SIG_DATA_FIRST; i<=SIG_DATA_LAST; ++i)
        {
            int index = i-1;
            if(data_queue_[index].isvalid())
            {
                data_valid = true;
                if(data_queue_[index].read(dd,&data_index_[index],tt))
                {
                    data_read = true;
                    tt = std::max(tt,dd.time());
                    if(SIG_KEY == i)
                    {
                        piw::decode_key(dd,0,0,&last_data_[index]);
                    }
                    else
                    {
                        last_data_[index] = dd.as_norm();
                    }
                    data_index_[index]++;
                }
            }
        }
        
        if((!strum_valid || !strum_read) && (!data_valid || !data_read))
        {
            break;
        }

        send_value(tt);
    }

    from_ = t;
}

void data_wire_t::send_value(unsigned long long t)
{
    if(data_running_)
    {
        float strum_breath_scaled = input_->strum_breath_scale_*fabs(last_strum_data_[SIG_STRUM_BREATH-1]);
        float strum_pressure_scaled = input_->strum_pressure_scale_*last_strum_data_[SIG_STRUM_PRESSURE-1];
        float strum_roll_scaled = input_->strum_roll_scale_*last_strum_data_[SIG_STRUM_ROLL-1];
        float strum_yaw_scaled = input_->strum_yaw_scale_*last_strum_data_[SIG_STRUM_YAW-1];

        float pressure_scaled = input_->pressure_scale_*last_data_[SIG_PRESSURE-1];
        float pressure = strum_breath_scaled+strum_pressure_scaled+pressure_scaled;
        if(pressure>1.0) pressure=1.0;
        data_buffer_.signal(SIG_PRESSURE).write_fast(piw::makefloat_bounded_nb(1,0,0,pressure,t));

        float roll_scaled = input_->roll_scale_*last_data_[SIG_ROLL-1];
        float roll = strum_roll_scaled+roll_scaled;
        if(roll>1.0) roll=1.0;
        data_buffer_.signal(SIG_ROLL).write_fast(piw::makefloat_bounded_nb(1,-1,0,roll,t));

        float yaw_scaled = input_->yaw_scale_*last_data_[SIG_YAW-1];
        float yaw = strum_yaw_scaled+yaw_scaled;
        if(yaw>1.0) yaw=1.0;
        data_buffer_.signal(SIG_YAW).write_fast(piw::makefloat_bounded_nb(1,-1,0,yaw,t));
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
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        int index = i-1;
        strum_queue_[index] = piw::dataqueue_t();
        strum_index_[index] = 0;
        last_strum_data_[index] = 0.f;
    }

    from_ = id.time();
    data_running_ = false;
    strum_running_= false;
    seq_ = seq;

    if(!input_->enabled_)
    {
        source_start(seq,id,b);
        return;
    }

    piw::data_nb_t d;
    data_buffer_ = piw::xevent_data_buffer_t();
    for(int i=SIG_DATA_FIRST; i<=SIG_DATA_LAST; ++i)
    {
        int index = i-1;
        data_queue_[index] = b.signal(i);
        data_index_[index] = 0;
        last_data_[index] = 0.f;

        if(data_queue_[index].latest(d,0,from_))
        {
            if(SIG_KEY == i)
            {
                piw::decode_key(d,0,0,&last_data_[index]);
            }
            else
            {
                last_data_[index] = d.as_norm();
            }
        }

        if(SIG_KEY == i)
        {
            data_buffer_.set_signal(i,b.signal(i));
        }
        else
        {
            data_buffer_.set_signal(i,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM));
        }
    }

    input_->activate_wire(this);

    start_event(id);
}

void data_wire_t::producer_start(unsigned long long t, const piw::data_nb_t &data_id, const piw::xevent_data_buffer_t &buffer)
{
    if(data_running_ && !strum_running_)
    {
        source_end(t);
        data_running_ = false;
    }

    from_ = t;
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        producer_reset(i,t,buffer.signal(i));
    }
    source_start(seq_,id().restamp(t),data_buffer_);
    data_running_ = true;
    strum_running_ = true;
}

void data_wire_t::producer_join(unsigned long long t, const piw::data_nb_t &data_id, const piw::xevent_data_buffer_t &buffer)
{
    if(data_running_ && !strum_running_)
    {
        source_end(t);
        data_running_ = false;
    }

    from_ = t;
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        producer_reset(i,piw::tsd_time(),buffer.signal(i));
    }
    source_start(seq_,id().restamp(t),data_buffer_);
    data_running_ = true;
    strum_running_ = true;
}

void data_wire_t::producer_end(unsigned long long t)
{
    from_ = t;
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        producer_reset(i,t,piw::dataqueue_t());
    }
    strum_running_ = false;
    if(input_->strum_note_end_ && data_running_)
    {
        source_end(t);
        data_running_ = false;
    }
}

void data_wire_t::producer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &n)
{
    int index = s-1;

    strum_queue_[index] = n;
    last_strum_data_[index] = 0.f;
    strum_index_[index] = 0;

    if(strum_queue_[index].isvalid())
    {
        piw::data_nb_t d;
        if(strum_queue_[index].latest(d,&strum_index_[index],from_))
        {
            last_strum_data_[index] = d.as_norm();
            strum_index_[index]++;
        }
    }

    send_value(t);
}

void data_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(!input_->enabled_)
    {
        source_buffer_reset(s,t,o,n);
        return;
    }

    piw::data_nb_t d;
    int index = s-1;
    data_queue_[index] = n;
    data_index_[index] = 0;
    last_data_[index] = 0.f;
    switch(s)
    {
        case SIG_KEY:
            data_buffer_.set_signal(s,n);
            source_buffer_reset(s,t,o,n);
            if(data_queue_[index].isvalid() &&
               data_queue_[index].latest(d,0,from_))
            {
                piw::decode_key(d,0,0,&last_data_[index]);
            }
            break;
        case SIG_PRESSURE:
        case SIG_ROLL:
        case SIG_YAW:
            if(data_queue_[index].isvalid())
            {
                data_queue_[index].latest(d,0,from_);
                last_data_[index] = d.as_norm();
            }
            break;
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
    data_running_ = false;

    input_->deactivate_wire(this);

    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        strum_queue_[i-1].clear();
    }

    for(int i=SIG_DATA_FIRST; i<=SIG_DATA_LAST; ++i)
    {
        data_queue_[i-1].clear();
    }
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

strum_wire_t::strum_wire_t(strum_input_t *i, const piw::event_data_source_t &es): producer_(i->impl_, consumer_check_t::method(this,&strum_wire_t::applies_to_consumer)), input_(i)
{
    path_ = es.path();
    input_->children_.insert(std::make_pair(path_,this));
    subscribe_and_ping(es);
}

void strum_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
    input_->children_.erase(path_);
}

void strum_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    if(!input_->impl_->enabled_)
    {
        return;
    }

    id_ = id;
    buffer_ = b;
    last_strum_key_ = piw::coordinate_t();
    breath_started_ = false;
    key_started_ = false;

    piw::data_nb_t d;
    if(b.signal(SIG_STRUM_KEY).latest(d,0,id.time()))
    {
        float column, row;
        if(piw::decode_key(d,&column,&row))
        {
            last_strum_key_ = piw::coordinate_t(column,row);
            key_started_ = true;
        }
    }
    else if(b.signal(SIG_STRUM_BREATH).latest(d,0,id.time()))
    {
        if(!d.is_null())
        {
            breath_started_ = true;
        }
    }

    if(id.time()-producer_.last_event_start_ > input_->impl_->trigger_window_)
    {
        producer_.start_event(id.time(), id_, buffer_);
    }
}

void strum_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    producer_.reset_event(s,t,n);
}

bool strum_wire_t::event_end(unsigned long long t)
{
    if(!input_->impl_->enabled_)
    {
        return true;
    }

    producer_.end_event(t);

    id_ = piw::data_nb_t();
    return true;
}

bool strum_wire_t::applies_to_consumer(consumer_t *c)
{
    data_wire_t *dw = (data_wire_t*)(c);
    int consumer_course = dw->last_data_[SIG_KEY-1];
    if(key_started_)
    {
        std::multimap<piw::coordinate_t,int>::iterator it;
        std::pair<std::multimap<piw::coordinate_t,int>::iterator,std::multimap<piw::coordinate_t,int>::iterator> ret;
        ret = input_->impl_->key_courses_.equal_range(last_strum_key_);
        for(it=ret.first; it!=ret.second; ++it)
        {
            if((*it).second == consumer_course)
            {
                return true;
            }
        }
    }
    else if(breath_started_)
    {
        if(input_->impl_->breath_courses_.empty() ||
           input_->impl_->breath_courses_.find(consumer_course) != input_->impl_->breath_courses_.end())
        {
            return true;
        }
    }
    return false;
}

piw::wire_t *strum_input_t::root_wire(const piw::event_data_source_t &es)
{
    std::map<piw::data_t,strum_wire_t *>::iterator ci;

    if((ci=children_.find(es.path()))!=children_.end())
    {
        delete ci->second;
    }

    strum_wire_t *c = new strum_wire_t(this,es);
    return c;
}

strum_input_t::strum_input_t(piw::strummer_t::impl_t *i): piw::root_t(0), impl_(i), clock_(0)
{
}

void strum_input_t::invalidate()
{
    std::map<piw::data_t,strum_wire_t *>::iterator ci;

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

void strum_input_t::root_clock()
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

piw::strummer_t::impl_t::impl_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d): piw::root_t(0), clock_(0), strum_input_(this)
{
    d->sink(this,"strummer");
    set_clock(this);
    connect(c);
    tick_enable(true);
    enabled_ = true;
    trigger_window_= 0;
    strum_breath_scale_ = 1.f;
    pressure_scale_ = 0.f;
    strum_pressure_scale_ = 1.f;
    roll_scale_ = 1.f;
    strum_roll_scale_ = 0.f;
    yaw_scale_ = 1.f;
    strum_yaw_scale_ = 0.f;
    strum_note_end_ = true;
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

void piw::strummer_t::impl_t::deactivate_wire(data_wire_t *w)
{
    active_data_wires_.remove(w);

    if(!active_data_wires_.head())
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

piw::cookie_t piw::strummer_t::strum_cookie()
{
    return piw::cookie_t(&impl_->strum_input_);
}

void piw::strummer_t::enable(bool b)
{
    impl_->set_enable(b);
}

void piw::strummer_t::set_trigger_window(unsigned tw)
{
    impl_->set_trigger_window(tw);
}

void piw::strummer_t::set_strum_breath_scale(float scale)
{
    impl_->set_strum_breath_scale(scale);
}

void piw::strummer_t::set_pressure_scale(float scale)
{
    impl_->set_pressure_scale(scale);
}

void piw::strummer_t::set_strum_pressure_scale(float scale)
{
    impl_->set_strum_pressure_scale(scale);
}

void piw::strummer_t::set_roll_scale(float scale)
{
    impl_->set_roll_scale(scale);
}

void piw::strummer_t::set_strum_roll_scale(float scale)
{
    impl_->set_strum_roll_scale(scale);
}

void piw::strummer_t::set_yaw_scale(float scale)
{
    impl_->set_yaw_scale(scale);
}

void piw::strummer_t::set_strum_yaw_scale(float scale)
{
    impl_->set_strum_yaw_scale(scale);
}

void piw::strummer_t::clear_breath_courses()
{
    impl_->clear_breath_courses();
}

void piw::strummer_t::add_breath_course(int course)
{
    impl_->add_breath_course(course);
}

void piw::strummer_t::clear_key_courses()
{
    impl_->clear_key_courses();
}

void piw::strummer_t::add_key_course(int column, int row, int course)
{
    impl_->add_key_course(column,row,course);
}

void piw::strummer_t::set_strum_note_end(bool b)
{
    impl_->set_strum_note_end(b);
}

producer_t::producer_t(correlator_t *c, consumer_check_t check): correlator_(c), consumer_check_(check), last_event_start_(0)
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

void producer_t::reset_event(unsigned s, unsigned long long t, const piw::dataqueue_t &n)
{
    for(consumer_t *c = consumers_.head(); c!=0; c=consumers_.next(c))
    {
        c->producer_reset(s,t,n);
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
    unsigned pidlen = pid.as_pathchannellen();
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

        unsigned cidlen = cid.as_pathchannellen();
        const unsigned char *cidpath = cid.as_path();

        if(pidlen>cidlen || memcmp(cidpath,pidpath,pidlen)!=0)
        {
            continue;
        }

        if(p->applies_to_consumer(c))
        {
            if(c->producer_)
            {
                c->producer_->consumers_.remove(c);
                c->producer_ = 0;
            }

            c->producer_ = p;
            p->consumers_.append(c);
            c->producer_start(t, p->id_, p->buffer_);
        }
    }
}

void correlator_t::del_producer(producer_t *p,unsigned long long t)
{
    producers_.erase(p->id_);

    consumer_t *c = p->consumers_.head();
 
    while(c!=0)
    {
        consumer_t *n = p->consumers_.next(c);
        if(p->applies_to_consumer(c))
        {
            c->producer_ = 0;
            c->producer_end(t);
            p->consumers_.remove(c);
        }
        c = n;
    }
}

void correlator_t::add_consumer(consumer_t *c)
{
    pic::lckmultimap_t<piw::data_nb_t, producer_t *>::nbtype::iterator i,e;

    piw::data_nb_t cid = c->id_;
    unsigned cidlen = cid.as_pathchannellen();
    const unsigned char *cidpath = cid.as_path();

    i = producers_.begin();
    e = producers_.end();

    consumers_.insert(std::make_pair(cid,c));

    for(;i!=e;i++)
    {
        producer_t *p = i->second;
        piw::data_nb_t pid = p->id_;
        if(pid.is_null())
        {
            continue;
        }

        unsigned pidlen = pid.as_pathchannellen();
        const unsigned char *pidpath = pid.as_path();

        if(pidlen>cidlen || memcmp(cidpath,pidpath,pidlen)!=0)
        {
            continue;
        }

        if(p->applies_to_consumer(c))
        {
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
        if(c->producer_->applies_to_consumer(c))
        {
            c->producer_->consumers_.remove(c);
            c->producer_ = 0;
        }
    }
}


correlator_t::correlator_t()
{
}

correlator_t::~correlator_t()
{
}
