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

#include "strummer.h"

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
    struct correlator_t;
    struct producer_t;
    struct consumer_t;

    typedef pic::functor_t<bool(consumer_t *, bool)> valid_consumer_check_t;
    typedef pic::functor_t<bool(consumer_t *, void *)> similar_consumer_check_t;

    struct producer_t: virtual pic::tracked_t
    {
        public:
            producer_t(correlator_t *, valid_consumer_check_t, similar_consumer_check_t);
            ~producer_t();

            void start(consumer_t *c, unsigned long long);
            void join(consumer_t *c);
            void start_event(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer);
            void muting_event(const piw::data_nb_t &id);
            void end_event(unsigned long long t);
            void reset_event(unsigned s, unsigned long long t, const piw::dataqueue_t &n);
            const piw::xevent_data_buffer_t buffer() { return buffer_; }
            const piw::data_nb_t id() { return id_; }
            unsigned long long last_event_start() { return last_event_start_; }

            bool applies_to_consumer(consumer_t *c, bool allow_similar);
            bool has_similar_consumers(void *data);

        private:
            friend class strm::strummer_t::impl_t;
            friend class consumer_t;

            static int ender__(void *this_, void *time_) { ((producer_t *)this_)->end_event(*(unsigned long long *)time_); return 0; }

            correlator_t *correlator_;
            valid_consumer_check_t valid_consumer_check_;
            similar_consumer_check_t similar_consumer_check_;
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

            virtual void start_event(const piw::data_nb_t &id);
            virtual void muting_event(const piw::data_nb_t &id);
            virtual void end_event(unsigned long long t);
            const piw::data_nb_t id() { return id_; }
            producer_t *producer() { return producer_; }

            virtual void *get_context() { return 0; }
            virtual void producer_start(unsigned long long t, const piw::xevent_data_buffer_t &buffer) {}
            virtual void producer_join(unsigned long long t, const piw::xevent_data_buffer_t &buffer) {}
            virtual void producer_end(unsigned long long t) {}
            virtual void producer_mute(unsigned long long t) {}
            virtual void producer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &n) {}

        private:
            friend class strm::strummer_t::impl_t;
            friend class producer_t;

            static int ender__(void *this_, void *time_) { ((consumer_t *)this_)->end_event(*(unsigned long long *)time_); return 0; }

            correlator_t *correlator_;
            producer_t *producer_;
            piw::data_nb_t id_;
    };

    class correlator_t
    {
        public:
            virtual void associate(producer_t *p, consumer_t *c) = 0;
            virtual void disassociate(consumer_t *c) = 0;
            virtual void add_producer(producer_t *p, unsigned long long t) = 0;
            virtual void muting_producer(producer_t *p, unsigned long long t) = 0;
            virtual void del_producer(producer_t *p, unsigned long long t) = 0;
            virtual void add_consumer(consumer_t *c, unsigned long long t) = 0;
            virtual void muting_consumer(consumer_t *c, unsigned long long t) = 0;
            virtual void del_consumer(consumer_t *c, unsigned long long t) = 0;
            virtual void inherit(consumer_t *from, consumer_t *to, unsigned long long t) = 0;
    };

    struct strum_wire_t: piw::wire_t, piw::event_data_sink_t, virtual pic::tracked_t, pic::element_t<1>
    {
        strum_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &, const piw::event_data_source_t &);
        ~strum_wire_t() { invalidate(); }
        void wire_closed() { delete this; }

        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void wire_ticked(unsigned long long f, unsigned long long t);
        bool applies_to_consumer(consumer_t *, bool);
        bool is_similar_consumer(consumer_t *, void *);

        producer_t producer_;
        strm::strummer_t::impl_t *input_;
        piw::data_t path_;
        piw::data_nb_t id_;
        piw::xevent_data_buffer_t buffer_;
        piw::coordinate_t last_strum_key_;
        bool breath_started_;
        bool key_started_;
        float highest_signal_data_;
        unsigned long long signal_index;
        unsigned long long last_signal_time_;
    };
    
    struct output_wire_t: virtual pic::lckobject_t, piw::wire_ctl_t, piw::event_data_source_real_t
    {
        output_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &);
        ~output_wire_t();

        void startup(const piw::data_nb_t &id);
        void shutdown(unsigned long long t);
        void add_value(unsigned, const piw::data_nb_t &);
        bool is_active();

        strm::strummer_t::impl_t *input_;
        piw::xevent_data_buffer_t buffer_;
        bool active_;
    };

    struct data_wire_t: piw::wire_t, piw::event_data_sink_t, consumer_t, pic::element_t<2>
    {
        data_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &, const float);
        data_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &, const piw::event_data_source_t &);
        ~data_wire_t();
        void wire_closed() { delete this; }
        bool is_active() { return pic::element_t<2>::ison(); }
        bool is_output_active() { return output_->is_active(); }

        void initialize(unsigned long long t);
        void event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void end_event(unsigned long long t);
        void source_ended(unsigned);
        void *get_context();
        void producer_start(unsigned long long t, const piw::xevent_data_buffer_t &buffer);
        void producer_join(unsigned long long t, const piw::xevent_data_buffer_t &buffer);
        void producer_end(unsigned long long t);
        void producer_mute(unsigned long long t);
        void producer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &n);
        void inherit(data_wire_t *from, unsigned long long t);
        void wire_ticked(unsigned long long f, unsigned long long t);
        void handle_data(int signal, piw::data_nb_t &d);
        void send_value(unsigned long long);

        strm::strummer_t::impl_t *input_;
        output_wire_t *output_;
        piw::data_t path_;
        unsigned seq_;
        piw::data_nb_t event_id_;

        bool data_running_;
        bool strum_running_;
        bool generate_key_;
        bool is_key_generated_;
        unsigned long long from_;
        int course_;
        int key_;

        piw::dataqueue_t strum_queue_[SIG_STRUM_LAST];
        unsigned long long strum_index_[SIG_STRUM_LAST];
        float last_strum_data_[SIG_STRUM_LAST];

        piw::dataqueue_t data_queue_[SIG_DATA_LAST];
        unsigned long long data_index_[SIG_DATA_LAST];
        float last_data_[SIG_DATA_LAST];
        float highest_pressure_;
        unsigned long long last_pulloff_;

        piw::data_nb_t last_key_;
        unsigned long long last_key_time_;

        unsigned long long last_time_;
    };

    struct open_courses_info_t
    {
        open_courses_info_t(int key, int column, int row): key_(key), column_(column), row_(row) {}
        open_courses_info_t(): key_(0), column_(0), row_(0) {}

        int key_;
        int column_;
        int row_;
    };
};

struct strm::strumconfig_t::impl_t
{
    impl_t()
    {
        enabled_ = true;
        trigger_window_= 0;
        strum_note_end_ = true;
        poly_courses_enable_ = true;
        course_mute_threshold_ = 0.1f;
        course_mute_interval_ = 20000;
        course_mute_enable_ = true;
        strum_mute_threshold_ = 0.1f;
        strum_mute_interval_ = 20000;
        strum_mute_enable_ = true;
        pulloff_threshold_ = 0.6f;
        pulloff_interval_ = 10000;
        pulloff_enable_ = true;
        open_course_enable_ = false;
        max_course_ = 0;
        for(int i=0; i<SIG_STRUM_LAST; ++i)
        {
            strum_scale_[i] = 0.f;
            strum_pressure_mix_[i] = 0.f;
        }
        for(int i=0; i<SIG_DATA_LAST; ++i)
        {
            data_scale_[i] = 0.f;
            open_course_default_[i] = 0.f;
        }
        strum_scale_[SIG_STRUM_BREATH-1] = 1.f;
        strum_scale_[SIG_STRUM_PRESSURE-1] = 1.f;
        data_scale_[SIG_ROLL-1] = 1.f;
        data_scale_[SIG_YAW-1] = 1.f;
    }

    impl_t &operator=(const impl_t &o)
    {
        enabled_ = o.enabled_;
        trigger_window_= o.trigger_window_;
        strum_note_end_ = o.strum_note_end_;
        poly_courses_enable_ = o.poly_courses_enable_;
        course_mute_threshold_ = o.course_mute_threshold_;
        course_mute_interval_ = o.course_mute_interval_;
        course_mute_enable_ = o.course_mute_enable_;
        strum_mute_threshold_ = o.strum_mute_threshold_;
        strum_mute_interval_ = o.strum_mute_interval_;
        strum_mute_enable_ = o.strum_mute_enable_;
        open_course_enable_ = o.open_course_enable_;
        max_course_ = o.max_course_;
        for(int i=0; i<SIG_STRUM_LAST; ++i)
        {
            strum_scale_[i] = o.strum_scale_[i];
            strum_pressure_mix_[i] = o.strum_pressure_mix_[i];
        }
        for(int i=0; i<SIG_DATA_LAST; ++i)
        {
            data_scale_[i] = o.data_scale_[i];
            open_course_default_[i] = o.open_course_default_[i];
        }

        breath_courses_.clear();
        std::set<int>::const_iterator breath_it;
        for(breath_it = o.breath_courses_.begin(); breath_it != o.breath_courses_.end(); ++breath_it)
        {
            breath_courses_.insert(*breath_it);
        }

        key_courses_.clear();
        std::multimap<piw::coordinate_t,int>::const_iterator key_it;
        for(key_it = o.key_courses_.begin(); key_it != o.key_courses_.end(); ++key_it)
        {
            key_courses_.insert(std::make_pair(key_it->first,key_it->second));
        }

        open_courses_info_.clear();
        std::map<int,open_courses_info_t>::const_iterator opencourse_it;
        for(opencourse_it = o.open_courses_info_.begin(); opencourse_it != o.open_courses_info_.end(); ++opencourse_it)
        {
            open_courses_info_.insert(std::make_pair(opencourse_it->first,opencourse_it->second));
        }

        return *this;
    }

    void enable(bool b) { enabled_ = b; }

    void set_trigger_window(unsigned tw) { trigger_window_ = tw; }

    void set_course_pressure_scale(float scale) { data_scale_[SIG_PRESSURE-1] = scale; }
    void set_course_roll_scale(float scale) { data_scale_[SIG_ROLL-1] = scale; }
    void set_course_yaw_scale(float scale) { data_scale_[SIG_YAW-1] = scale; }

    void set_strum_breath_scale(float scale) { strum_scale_[SIG_STRUM_BREATH-1] = scale; }
    void set_strum_pressure_scale(float scale) { strum_scale_[SIG_STRUM_PRESSURE-1] = scale; }
    void set_strum_roll_scale(float scale) { strum_scale_[SIG_STRUM_ROLL-1] = scale; }
    void set_strum_roll_pressure_mix(float mix) { strum_pressure_mix_[SIG_STRUM_ROLL-1] = mix; }
    void set_strum_yaw_scale(float scale) { strum_scale_[SIG_STRUM_YAW-1] = scale; }
    void set_strum_yaw_pressure_mix(float mix) { strum_pressure_mix_[SIG_STRUM_YAW-1] = mix; }

    void set_strum_note_end(bool b) { strum_note_end_ = b; }
    void set_poly_courses_enable(bool p) { poly_courses_enable_ = p; }

    void set_course_mute_threshold(float t) { course_mute_threshold_ = t; }
    void set_course_mute_interval(unsigned i) { course_mute_interval_ = i*1000; }
    void set_course_mute_enable(bool b) { course_mute_enable_ = b; }
    void set_strum_mute_threshold(float t) { strum_mute_threshold_ = t; }
    void set_strum_mute_interval(unsigned i) { strum_mute_interval_ = i*1000; }
    void set_strum_mute_enable(bool b) { strum_mute_enable_ = b; }
    void set_pulloff_threshold(float t) { pulloff_threshold_ = t; }
    void set_pulloff_interval(unsigned i) { pulloff_interval_ = i*1000; }
    void set_pulloff_enable(bool b) { pulloff_enable_ = b; }

    void open_course_enable(bool b) { open_course_enable_ = b; }
    void open_course_pressure_default(float v) { open_course_default_[SIG_PRESSURE-1] = v; }
    void open_course_roll_default(float v) { open_course_default_[SIG_ROLL-1] = v; }
    void open_course_yaw_default(float v) { open_course_default_[SIG_YAW-1] = v; }

    void clear_breath_courses()
    {
        breath_courses_.clear();
    }

    void add_breath_course(int course)
    {
        breath_courses_.insert(course);
        if(course > int(max_course_))
        {
            max_course_ = course;
        }
    }

    void remove_breath_course(int course)
    {
        breath_courses_.erase(course);
    }

    void clear_key_courses()
    {
        key_courses_.clear();
    }

    void add_key_course(int course, int column, int row)
    {
        piw::coordinate_t coordinate(column,row);

        // don't add it if it's already there
        std::multimap<piw::coordinate_t,int>::iterator it;
        std::pair<std::multimap<piw::coordinate_t,int>::iterator,std::multimap<piw::coordinate_t,int>::iterator> ret;
        ret = key_courses_.equal_range(coordinate);
        for(it=ret.first; it!=ret.second; ++it)
        {
            if((*it).second == course)
            {
                return;
            }
        }

        key_courses_.insert(std::make_pair(coordinate,course));
        if(course > int(max_course_))
        {
            max_course_ = course;
        }
    }

    void clear_key_course(int course)
    {
        std::multimap<piw::coordinate_t,int> key_courses;
        std::multimap<piw::coordinate_t,int>::const_iterator key_it;
        for(key_it = key_courses_.begin(); key_it != key_courses_.end(); ++key_it)
        {
            if(key_it->second != course)
            {
                key_courses.insert(std::make_pair(key_it->first,key_it->second));
            }
        }
        key_courses_ = key_courses;
    }

    void clear_open_courses_info()
    {
        open_courses_info_.clear();
    }

    void set_open_course_info(int course, open_courses_info_t &info)
    {
        open_courses_info_.erase(course);
        open_courses_info_.insert(std::make_pair(course, info));
        if(course > int(max_course_))
        {
            max_course_ = course;
        }
    }

    void set_open_course_info(int course, int key, int column, int row)
    {
        open_courses_info_t info(key, column, row);
        set_open_course_info(course, info);
    }

    void set_open_course_key(int course, int key)
    {
        open_courses_info_t info;

        std::map<int,open_courses_info_t>::iterator it;
        it = open_courses_info_.find(course);
        if(it != open_courses_info_.end())
        {
            info = it->second;
        }
        info.key_ = key;

        set_open_course_info(course, info);
    }

    void set_open_course_physical(int course, int column, int row)
    {
        open_courses_info_t info;

        std::map<int,open_courses_info_t>::iterator it;
        it = open_courses_info_.find(course);
        if(it != open_courses_info_.end())
        {
            info = it->second;
        }
        info.column_ = column;
        info.row_ = row;

        set_open_course_info(course, info);
    }

    void clear_open_course_info(int course)
    {
        open_courses_info_.erase(course);
    }

    std::set<int> get_defined_courses() const
    {
        std::set<int> result;

        result.insert(breath_courses_.begin(), breath_courses_.end());

        std::multimap<piw::coordinate_t,int>::const_iterator key_course_it;
        for(key_course_it = key_courses_.begin(); key_course_it != key_courses_.end(); ++key_course_it)
        {
            result.insert(key_course_it->second);
        }

        std::map<int,open_courses_info_t>::const_iterator info_it;
        for(info_it = open_courses_info_.begin(); info_it != open_courses_info_.end(); ++info_it)
        {
            result.insert(info_it->first);
        }

        return result;
    }

    std::string encode_courses() const
    {
        std::ostringstream oss;

        oss << "[";
        bool content = false;

        std::set<int> defined_courses = get_defined_courses();
        std::set<int>::const_iterator course_it;
        for(course_it = defined_courses.begin(); course_it != defined_courses.end(); ++course_it)
        {
            int number = *course_it;

            bool breath = (breath_courses_.find(number) != breath_courses_.end());

            open_courses_info_t info;
            bool found_info = false;
            std::map<int,open_courses_info_t>::const_iterator info_it;
            info_it = open_courses_info_.find(number);
            if(info_it != open_courses_info_.end())
            {
                found_info = true;
                info = info_it->second;
            }

            std::set<piw::coordinate_t> keys;
            std::multimap<piw::coordinate_t,int>::const_iterator key_course_it;
            for(key_course_it = key_courses_.begin(); key_course_it != key_courses_.end(); ++key_course_it)
            {
                if(key_course_it->second == number)
                {
                    keys.insert(key_course_it->first);
                }
            }

            if(breath || found_info || !keys.empty())
            {
                if(content)
                {
                    oss << ",";
                }
                else
                {
                    content = true;
                }
                oss << "[" << number << "," << (breath?"true":"false") << "," << info.key_ << "," << info.column_ << "," << info.row_ << ",[";
                std::set<piw::coordinate_t>::const_iterator key_it;
                for(key_it = keys.begin(); key_it != keys.end(); ++key_it)
                {
                    if(key_it != keys.begin())
                    {
                        oss << ",";
                    }
                    oss << "[" << key_it->x_ << "," << key_it->y_ << "]";
                }
                oss << "]]";
            }
        }
        oss << "]";

        return oss.str();
    }

    bool enabled_;
    unsigned trigger_window_;
    float data_scale_[SIG_DATA_LAST];
    float strum_scale_[SIG_STRUM_LAST];
    float strum_pressure_mix_[SIG_STRUM_LAST];
    bool strum_note_end_;
    bool poly_courses_enable_;
    float course_mute_threshold_;
    unsigned course_mute_interval_;
    bool course_mute_enable_;
    float strum_mute_threshold_;
    unsigned strum_mute_interval_;
    bool strum_mute_enable_;
    float pulloff_threshold_;
    unsigned pulloff_interval_;
    bool pulloff_enable_;
    bool open_course_enable_;
    float open_course_default_[SIG_DATA_LAST];
    unsigned max_course_;
    std::set<int> breath_courses_;
    std::multimap<piw::coordinate_t,int> key_courses_;
    std::map<int,open_courses_info_t> open_courses_info_;
};

struct strm::strummer_t::impl_t: correlator_t, piw::clocksink_t, piw::root_t, piw::root_ctl_t, virtual pic::lckobject_t
{
    impl_t(const piw::cookie_t &, piw::clockdomain_ctl_t *);
    ~impl_t();

    void invalidate();

    virtual void associate(producer_t *p, consumer_t *c);
    virtual void disassociate(consumer_t *c);
    virtual void add_producer(producer_t *p, unsigned long long t);
    virtual void muting_producer(producer_t *p, unsigned long long t);
    virtual void del_producer(producer_t *p, unsigned long long t);
    virtual void add_consumer(consumer_t *c, unsigned long long t);
    virtual void muting_consumer(consumer_t *c, unsigned long long t);
    virtual void del_consumer(consumer_t *c, unsigned long long t);
    virtual void inherit(consumer_t *from, consumer_t *to, unsigned long long t);
    void find_del_beneficiary(data_wire_t *w, unsigned long long t);
    void activate_open_courses(producer_t *p, unsigned long long t);
    void deactivate_open_courses(producer_t *p, unsigned long long t);
    void deactivate_open_course(data_wire_t *w, unsigned long long t);
    void disassociate_open_courses(producer_t *p);
    bool is_open_course(consumer_t *w);

    piw::wire_t *root_wire(const piw::event_data_source_t &);

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }
    void root_clock();
    void root_latency() {}
    void activate_wire(strum_wire_t *w);
    void deactivate_wire(strum_wire_t *w);
    void activate_wire(data_wire_t *w);
    void deactivate_wire(data_wire_t *w);
    void clocksink_ticked(unsigned long long f, unsigned long long t);
    void set_strumconfig(strumconfig_t::impl_t &c) { piw::tsd_fastcall(__set_strumconfig,this,&c); update_course_wires(); }
    static int __set_strumconfig(void *, void *);
    void update_course_wires();
    void update_opencourse_data();

    bct_clocksink_t *clock_;
    strm::strumconfig_t::impl_t config_;

    pic::lckmultimap_t<piw::data_nb_t, producer_t *>::nbtype producers_;
    pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype consumers_;

    std::map<piw::data_t,strum_wire_t *> strum_children_;
    std::map<piw::data_t,data_wire_t *> data_children_;
    pic::ilist_t<strum_wire_t,1> active_strum_wires_;
    pic::ilist_t<data_wire_t,2> active_data_wires_;
    pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype> open_course_wires_;
};


/**
 * output_wire_t
 */

output_wire_t::output_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &path): piw::event_data_source_real_t(path), input_(i), active_(false)
{
    buffer_ = piw::xevent_data_buffer_t();
    for(int i=SIG_DATA_FIRST; i<=SIG_DATA_LAST; ++i)
    {
        buffer_.set_signal(i,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM));
    }
    input_->connect_wire(this,source());
}

output_wire_t::~output_wire_t()
{
    piw::wire_ctl_t::disconnect();
    source_shutdown();
}

void output_wire_t::startup(const piw::data_nb_t &id)
{
    source_start(0,id,buffer_);
    active_ = true;
}

void output_wire_t::shutdown(unsigned long long t)
{
    //if(!active_) pic::logmsg() << "shutdown when not active";
    active_ = false;
    source_end(t);
}

void output_wire_t::add_value(unsigned sig, const piw::data_nb_t &d)
{
    buffer_.signal(sig).write_fast(d);
}

bool output_wire_t::is_active()
{
    return active_;
}


/**
 * data_wire_t
 */

data_wire_t::data_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &path, const float course): consumer_t(i), input_(i), last_time_(0)
{
    for(int it=SIG_DATA_FIRST; it<=SIG_DATA_LAST; ++it)
    {
        int index = it-1;
        data_queue_[index] = piw::dataqueue_t();
        last_data_[index] = input_->config_.open_course_default_[index];
    }

    unsigned long long t = path.time();
    initialize(t);

    path_ = path;
    from_ = piw::tsd_time();
    seq_ = 0;
    generate_key_ = true;

    open_courses_info_t info;
    std::map<int,open_courses_info_t>::iterator it = input_->config_.open_courses_info_.find(course);
    if(it != input_->config_.open_courses_info_.end())
    {
        info = it->second;
    }
    last_key_ = piw::makekey(info.column_,info.row_,course,info.key_,piw::KEY_HARD,t);
    course_ = course;
    key_ = info.key_;

    input_->data_children_.insert(std::make_pair(path_,this));
    output_ = new output_wire_t(input_, piw::pathone(input_->data_children_.size(), t));
}

data_wire_t::data_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &path, const piw::event_data_source_t &es): consumer_t(i), input_(i), last_time_(0)
{
    path_ = path;
    generate_key_ = false;

    input_->data_children_.insert(std::make_pair(path_,this));
    subscribe_and_ping(es);
    output_ = new output_wire_t(input_, piw::pathone(input_->data_children_.size(), path.time()));
}

data_wire_t::~data_wire_t()
{
    input_->deactivate_wire(this);
    unsubscribe();
    piw::wire_t::disconnect();
    delete output_;
    input_->data_children_.erase(path_);
}

void data_wire_t::initialize(unsigned long long t)
{
    event_id_ = piw::makenull_nb(t);
    last_time_ = t;
    highest_pressure_ = 0.f;
    last_pulloff_ = 0;
    course_ = piw::MAX_KEY+1;
    key_ = piw::MAX_KEY+1;
    data_running_ = false;
    strum_running_= false;
    is_key_generated_ = false;
    last_key_time_ = 0;

    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        int index = i-1;
        strum_queue_[index] = piw::dataqueue_t();
        strum_index_[index] = 0;
        last_strum_data_[index] = 0.f;
    }

    for(int i=SIG_DATA_FIRST; i<=SIG_DATA_LAST; ++i)
    {
        int index = i-1;
        data_index_[index] = 0;
        piw::data_nb_t d;
        data_queue_[index].latest(d,&data_index_[index],t);
    }
}

void data_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    initialize(id.time());

    event_id_ = piw::pathprepend_event_nb(id, 1);
    from_ = id.time();
    seq_ = seq;

    if(!input_->config_.enabled_)
    {
        output_->source_start(seq,id,b);
        return;
    }

    piw::data_nb_t d;
    for(int i=SIG_DATA_FIRST; i<=SIG_DATA_LAST; ++i)
    {
        int index = i-1;
        data_queue_[index] = b.signal(i);

        if(data_queue_[index].latest(d,&data_index_[index],from_))
        {
            handle_data(i, d);
            last_time_ = std::max(last_time_,d.time());
        }
    }

    input_->activate_wire(this);

    wire_ticked(from_, piw::tsd_time());
}

void data_wire_t::handle_data(int signal, piw::data_nb_t &d)
{
    int index = signal-1;
    if(SIG_KEY == signal)
    {
        last_key_ = d;
        float course, key;
        if(piw::decode_key(d,0,0,&course,&key))
        {
            course_ = course;
            key_ = key;
        }
    }
    else
    {
        if(SIG_PRESSURE == signal)
        {
            highest_pressure_ = std::max(highest_pressure_,d.as_norm());
        }
        if(SIG_YAW == signal &&
           input_->config_.pulloff_enable_ &&
           d.as_norm() >= input_->config_.pulloff_threshold_)
        {
            last_pulloff_ = d.time();
        }

        last_data_[index] = input_->config_.data_scale_[index]*d.as_norm();
    }
}

void *data_wire_t::get_context()
{
    return (void *)course_;
}

void data_wire_t::wire_ticked(unsigned long long f, unsigned long long t)
{
    piw::data_nb_t d;

    last_time_ = std::max(last_time_,f);

    while(true)
    {
        if(!data_running_ && !event_id_.is_null() && id().is_null())
        {
            if(input_->config_.course_mute_enable_ &&
               last_time_-event_id_.time() >= input_->config_.course_mute_interval_ &&
               highest_pressure_ < input_->config_.course_mute_threshold_)
            {
                muting_event(event_id_);
            }
            if(!input_->config_.course_mute_enable_ ||
               highest_pressure_ >= input_->config_.course_mute_threshold_)
            {
                start_event(event_id_);
            }
        }

        bool strum_valid = false;
        bool strum_read = false;

        for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
        {
            int index = i-1;
            if(strum_queue_[index].isvalid())
            {
                strum_valid = true;
                if(strum_queue_[index].read(d,&strum_index_[index],t))
                {
                    strum_read = true;
                    last_time_ = std::max(last_time_+1,d.time());
                    last_strum_data_[index] = input_->config_.strum_scale_[index]*d.as_norm();
                    strum_index_[index]++;
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
                if(data_queue_[index].read(d,&data_index_[index],t))
                {
                    data_read = true;
                    last_time_ = std::max(last_time_+1,d.time());
                    handle_data(i, d);
                    data_index_[index]++;
                }
            }
        }
        
        if((!strum_valid || !strum_read) && (!data_valid || !data_read))
        {
            break;
        }

        send_value(last_time_);
    }

    from_ = t;
}

void data_wire_t::send_value(unsigned long long t)
{
    if(data_running_)
    {
        if(generate_key_)
        {
            if(!is_key_generated_)
            {
                output_->add_value(SIG_KEY, last_key_.restamp(t));
                is_key_generated_ = true;
            }
        }
        else
        {
            if(last_key_time_ != last_key_.time())
            {
                last_key_time_ = last_key_.time();
                output_->add_value(SIG_KEY, last_key_.restamp(t));
            }
        }

        float pressure = fabs(last_strum_data_[SIG_STRUM_BREATH-1])
            + last_strum_data_[SIG_STRUM_PRESSURE-1]
            + last_data_[SIG_PRESSURE-1]
            + fabs(last_strum_data_[SIG_STRUM_ROLL-1])*input_->config_.strum_pressure_mix_[SIG_STRUM_ROLL-1]
            + fabs(last_strum_data_[SIG_STRUM_YAW-1])*input_->config_.strum_pressure_mix_[SIG_STRUM_YAW-1];
        if(pressure>1.0) pressure=1.0;
        output_->add_value(SIG_PRESSURE, piw::makefloat_bounded_nb(1,0,0,pressure,t));

        float roll = last_strum_data_[SIG_STRUM_ROLL-1]+last_data_[SIG_ROLL-1];
        if(roll>1.0) roll=1.0;
        output_->add_value(SIG_ROLL, piw::makefloat_bounded_nb(1,-1,0,roll,t));

        float yaw = last_strum_data_[SIG_STRUM_YAW-1]+last_data_[SIG_YAW-1];
        if(yaw>1.0) yaw=1.0;
        output_->add_value(SIG_YAW, piw::makefloat_bounded_nb(1,-1,0,yaw,t));
    }
}

void data_wire_t::source_ended(unsigned seq)
{
    if(!input_->config_.enabled_)
    {
        source_ended(seq);
        return;
    }
}

void data_wire_t::producer_start(unsigned long long t, const piw::xevent_data_buffer_t &buffer)
{
    if(data_running_)
    {
        output_->shutdown(t);
        data_running_ = false;
    }

    from_ = t;
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        producer_reset(i,t,buffer.signal(i));
    }
    output_->startup(id().restamp(t));
    data_running_ = true;
    strum_running_ = true;
}

void data_wire_t::producer_join(unsigned long long t, const piw::xevent_data_buffer_t &buffer)
{
    if(data_running_)
    {
        output_->shutdown(t);
        data_running_ = false;
    }

    from_ = t;
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        producer_reset(i,piw::tsd_time(),buffer.signal(i));
    }
    output_->startup(id().restamp(t));
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
    if((generate_key_ || input_->config_.strum_note_end_) && data_running_)
    {
        output_->shutdown(t);
        data_running_ = false;
    }
}

void data_wire_t::producer_mute(unsigned long long t)
{
    from_ = t;
    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        producer_reset(i,t,piw::dataqueue_t());
    }
    strum_running_ = false;
    if(data_running_)
    {
        output_->shutdown(t);
        data_running_ = false;
    }
}

void data_wire_t::inherit(data_wire_t *from, unsigned long long t)
{
    if(data_running_)
    {
        output_->shutdown(t);
        data_running_ = false;
    }

    output_wire_t *from_output = from->output_;
    from->output_ = output_;
    output_ = from_output;

    for(int i=SIG_STRUM_FIRST; i<=SIG_STRUM_LAST; ++i)
    {
        int index = i-1;
        strum_queue_[index] = from->strum_queue_[index];
        last_strum_data_[index] = from->last_strum_data_[index];
        strum_index_[index] = from->strum_index_[index];
    }

    from_ = t;
    is_key_generated_ = false;
    last_key_time_ = 0;
    last_time_ = std::max(from->last_time_,t);

    from->is_key_generated_ = false;
    from->last_key_time_ = 0;
    from->last_time_ = std::max(from->last_time_,t);

    bool r;

    r = from->data_running_;
    from->data_running_ = data_running_;
    data_running_ = r;

    r = from->strum_running_;
    from->strum_running_ = strum_running_;
    strum_running_ = r;

    send_value(last_time_);
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
            last_strum_data_[index] = input_->config_.strum_scale_[index]*d.as_norm();
            strum_index_[index]++;
        }
    }
}

void data_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(!input_->config_.enabled_)
    {
        output_->source_buffer_reset(s,t,o,n);
        return;
    }

    int index = s-1;
    data_queue_[index] = n;
    data_index_[index] = 0;
    last_data_[index] = 0.f;

    piw::data_nb_t d;
    if(data_queue_[index].isvalid() &&
       data_queue_[index].latest(d,0,from_))
    {
        handle_data(s, d);
    }
}

void data_wire_t::end_event(unsigned long long t)
{
    consumer_t::end_event(t);
    is_key_generated_ = false;
}

bool data_wire_t::event_end(unsigned long long t)
{
    if(!input_->config_.enabled_)
    {
        return output_->source_end(t);
    }

    wire_ticked(from_, t);

    end_event(t);

    output_->shutdown(t);
    course_ = piw::MAX_KEY+1;
    key_ = piw::MAX_KEY+1;
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


/**
 * strum_wire_t
 */

strum_wire_t::strum_wire_t(strm::strummer_t::impl_t *i, const piw::data_t &path, const piw::event_data_source_t &es):
    producer_(i, valid_consumer_check_t::method(this,&strum_wire_t::applies_to_consumer),
                 similar_consumer_check_t::method(this,&strum_wire_t::is_similar_consumer)),
    input_(i)
{
    path_ = path;
    input_->strum_children_.insert(std::make_pair(path_,this));
    subscribe_and_ping(es);
}

void strum_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
    input_->strum_children_.erase(path_);
}

void strum_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    if(!input_->config_.enabled_)
    {
        return;
    }

    id_ = piw::pathprepend_event_nb(id, 1);
    buffer_ = b;
    last_strum_key_ = piw::coordinate_t();
    breath_started_ = false;
    key_started_ = false;
    highest_signal_data_ = 0.f;
    signal_index = 0;
    last_signal_time_ = 0;

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

    buffer_.signal(SIG_STRUM_PRESSURE).latest(d,&signal_index,id.time());

    if(id.time()-producer_.last_event_start() > input_->config_.trigger_window_)
    {
        input_->activate_wire(this);

        wire_ticked(id_.time(), piw::tsd_time());
    }
}

void strum_wire_t::wire_ticked(unsigned long long f, unsigned long long t)
{
    int signal = 0;
    if(breath_started_)
    {
        signal = SIG_STRUM_BREATH;
    }
    else if(key_started_)
    {
        signal = SIG_STRUM_PRESSURE;
    }
    else
    {
        return;
    }

    if(!id_.is_null() && buffer_.signal(signal).isvalid())
    {
        piw::data_nb_t d;
        while(buffer_.signal(signal).read(d,&signal_index,t))
        {
            signal_index++;

            highest_signal_data_ = std::max(highest_signal_data_,d.as_norm());
            last_signal_time_ = d.time();

            if(input_->config_.strum_mute_enable_ &&
               last_signal_time_-id_.time() >= input_->config_.strum_mute_interval_ &&
               highest_signal_data_ < input_->config_.strum_mute_threshold_)
            {
                producer_.muting_event(id_);
                continue;
            }
            if(!input_->config_.strum_mute_enable_ ||
               highest_signal_data_ >= input_->config_.strum_mute_threshold_)
            {
                input_->deactivate_wire(this);
                producer_.start_event(id_.time(), id_, buffer_);
                return;
            }
        }
    }
}

void strum_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    producer_.reset_event(s,t,n);
}

bool strum_wire_t::event_end(unsigned long long t)
{
    if(!input_->config_.enabled_)
    {
        return true;
    }

    producer_.end_event(t);

    input_->deactivate_wire(this);

    id_ = piw::data_nb_t();
    return true;
}

bool strum_wire_t::applies_to_consumer(consumer_t *c, bool allow_similar)
{
    int consumer_course = int(c->get_context());
    bool consumer_applies = false;
    if(key_started_)
    {
        std::multimap<piw::coordinate_t,int>::iterator it;
        std::pair<std::multimap<piw::coordinate_t,int>::iterator,std::multimap<piw::coordinate_t,int>::iterator> ret;
        ret = input_->config_.key_courses_.equal_range(last_strum_key_);
        for(it=ret.first; it!=ret.second; ++it)
        {
            if((*it).second == consumer_course)
            {
                consumer_applies = true;
                break;
            }
        }
    }
    else if(breath_started_)
    {
        if(input_->config_.breath_courses_.find(consumer_course) != input_->config_.breath_courses_.end())
        {
            consumer_applies = true;
        }
    }

    if(consumer_applies && !allow_similar && producer_.has_similar_consumers((void *)consumer_course))
    {
        consumer_applies = false;
    }

    return consumer_applies;
}

bool strum_wire_t::is_similar_consumer(consumer_t *c, void *data)
{
    int course = int(data);
    return int(c->get_context()) == course;
}


/**
 * strm::strummer_t::impl_t
 */

strm::strummer_t::impl_t::impl_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d): piw::root_t(0), clock_(0)
{
    d->sink(this,"strummer");
    set_clock(this);
    connect(c);
    tick_enable(true);
}

strm::strummer_t::impl_t::~impl_t()
{
    tick_disable();
    invalidate();
    close_sink();
}

int strm::strummer_t::impl_t::__set_strumconfig(void *r_, void *c_)
{
    strm::strummer_t::impl_t *r = (strm::strummer_t::impl_t *)r_;
    r->config_ = *(strm::strumconfig_t::impl_t *)c_;

    return 0;
}

void strm::strummer_t::impl_t::invalidate()
{
    tick_disable();

    std::map<piw::data_t,strum_wire_t *>::iterator si;
    while((si=strum_children_.begin())!=strum_children_.end())
    {
        delete si->second;
    }

    std::map<piw::data_t,data_wire_t *>::iterator di;
    while((di=data_children_.begin())!=data_children_.end())
    {
        delete di->second;
    }

    piw::root_t::disconnect();

    if(clock_)
    {
        remove_upstream(clock_);
        clock_ = 0;
    }
}

void strm::strummer_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
    strum_wire_t *sw = active_strum_wires_.head();
    strum_wire_t *swn;
    while(sw != 0)
    {
        swn = active_strum_wires_.next(sw);
        sw->wire_ticked(f,t);
        sw = swn;
    }

    data_wire_t *dw = active_data_wires_.head();
    data_wire_t *dwn;
    while(dw != 0)
    {
        dwn = active_data_wires_.next(dw);
        dw->wire_ticked(f,t);
        dw = dwn;
    }
}

void strm::strummer_t::impl_t::root_clock()
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

void strm::strummer_t::impl_t::update_course_wires()
{
    std::set<int> defined_courses = config_.get_defined_courses();
    std::set<int>::const_iterator course_it;
    for(course_it = defined_courses.begin(); course_it != defined_courses.end(); ++course_it)
    {
        if(open_course_wires_.alternate().find(*course_it) == open_course_wires_.alternate().end())
        {
            int course = *course_it;
            piw::data_t path = piw::pathone(open_course_wires_.alternate().size()+1, piw::tsd_time());
            path = piw::pathprepend_event(path, 2);
            open_course_wires_.alternate().insert(std::make_pair(course, new data_wire_t(this,path,course)));
        }
    }

    open_course_wires_.exchange();

    update_opencourse_data();
}

void strm::strummer_t::impl_t::update_opencourse_data()
{
    pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t g(open_course_wires_);
    pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        for(int j=SIG_DATA_FIRST; j<=SIG_DATA_LAST; ++j)
        {
            int index = j-1;
            it->second->last_data_[index] = config_.open_course_default_[index];
        }
    }

}

piw::wire_t *strm::strummer_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    piw::data_t path = piw::pathprepend_event(piw::pathpretruncate(es.path()), 1);

    unsigned char type = es.path().as_path()[0];
    switch(type)
    {
        case 1:
            {
                std::map<piw::data_t,strum_wire_t *>::iterator ci;
                if((ci=strum_children_.find(path))!=strum_children_.end())
                {
                    delete ci->second;
                }

                return new strum_wire_t(this,path,es);
            }
            break;
        case 2:
            {
                std::map<piw::data_t,data_wire_t *>::iterator ci;
                if((ci=data_children_.find(path))!=data_children_.end())
                {
                    delete ci->second;
                }
                return new data_wire_t(this,path,es);
            }
            break;
    }
    return 0;
}

void strm::strummer_t::impl_t::activate_wire(strum_wire_t *w)
{
    if(!active_strum_wires_.head())
    {
        tick_suppress(false);
    }

    active_strum_wires_.append(w);
}

void strm::strummer_t::impl_t::deactivate_wire(strum_wire_t *w)
{
    active_strum_wires_.remove(w);

    if(!active_data_wires_.head() &&
       !active_strum_wires_.head())
    {
        tick_suppress(true);
    }
}

void strm::strummer_t::impl_t::activate_wire(data_wire_t *w)
{
    if(!active_data_wires_.head())
    {
        tick_suppress(false);
    }

    active_data_wires_.append(w);
}

void strm::strummer_t::impl_t::deactivate_wire(data_wire_t *w)
{
    active_data_wires_.remove(w);

    if(!active_data_wires_.head() &&
       !active_strum_wires_.head())
    {
        tick_suppress(true);
    }
}

void strm::strummer_t::impl_t::associate(producer_t *p, consumer_t *c)
{
    if(!p || !c) return;

    disassociate(c);

    c->producer_ = p;
    p->consumers_.append(c);
}

void strm::strummer_t::impl_t::disassociate(consumer_t *c)
{
    if(c->producer_)
    {
        c->producer_ = 0;
        c->producer_->consumers_.remove(c);
    }
}

void strm::strummer_t::impl_t::activate_open_courses(producer_t *p, unsigned long long t)
{
    if(config_.open_course_enable_ && p)
    {
        pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t g(open_course_wires_);
        pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it;
        for(it=g.value().begin(); it!=g.value().end(); ++it)
        {
            data_wire_t *c = it->second;
            if(p->applies_to_consumer(c,false))
            {
                activate_wire(c);
                c->start_event(c->path_.make_nb().restamp(t));

                associate(p,c);
                p->start(c,t);
            }
        }
    }
}

void strm::strummer_t::impl_t::deactivate_open_courses(producer_t *p, unsigned long long t)
{
    if(!p) return;

    pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t g(open_course_wires_);
    pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        if(p->applies_to_consumer(it->second, true))
        {
            deactivate_open_course(it->second,t);
        }
    }
}

void strm::strummer_t::impl_t::deactivate_open_course(data_wire_t *w, unsigned long long t)
{
    if(!w) return;

    w->end_event(t);
    deactivate_wire(w);

    w->producer_end(t);
    disassociate(w);
}

void strm::strummer_t::impl_t::disassociate_open_courses(producer_t *p)
{
    if(!p) return;

    pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t g(open_course_wires_);
    pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        if(it->second->producer() == p)
        {
            consumers_.erase(it->second->id_);
            disassociate(it->second);
        }
    }
}

bool strm::strummer_t::impl_t::is_open_course(consumer_t *c)
{
    pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t g(open_course_wires_);
    pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it;
    for(it=g.value().begin(); it!=g.value().end(); ++it)
    {
        if(c==it->second)
        {
            return true;
        }
    }
    return false;
}

void strm::strummer_t::impl_t::add_producer(producer_t *p, unsigned long long t)
{
    if(!config_.strum_note_end_)
    {
        deactivate_open_courses(p,t);
    }

    producers_.insert(std::make_pair(p->id_,p));

    pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype::iterator i,e;
    i = consumers_.begin();
    e = consumers_.end();

    // when courses aren't polyphonic, find the consumer that's the furthest
    // down the course
    if(!config_.poly_courses_enable_)
    {
        consumer_t *c = 0;
        int key = 0;
        for(;i!=e;i++)
        {
            data_wire_t *w = (data_wire_t *)i->second;
            if(w->id_.is_null()) continue;

            if(p->applies_to_consumer(w, true) && w->key_ > key)
            {
                c = w;
                key = w->key_;
            }
        }
        if(c)
        {
            associate(p,c);
            p->start(c, t);
        }
    }
    // when courses are polyphonic, just activate all consumers that apply
    else
    {
        for(;i!=e;i++)
        {
            consumer_t *c = i->second;
            if(c->id_.is_null()) continue;

            if(p->applies_to_consumer(c, true))
            {
                associate(p,c);
                p->start(c, t);
            }
        }
    }

    activate_open_courses(p,t);
}

void strm::strummer_t::impl_t::muting_producer(producer_t *p, unsigned long long t)
{
    deactivate_open_courses(p,t);

    pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype::iterator ic,ec;
    ic = consumers_.begin();
    ec = consumers_.end();
    while(ic != ec)
    {
        consumer_t *c = ic->second;
        ++ic;

        if(p->applies_to_consumer(c, true))
        {
            c->producer_mute(t);
        }
    }
}

void strm::strummer_t::impl_t::del_producer(producer_t *p, unsigned long long t)
{
    if(config_.strum_note_end_)
    {
        deactivate_open_courses(p,t);
    }
    else
    {
        disassociate_open_courses(p);
    }

    producers_.erase(p->id_);

    consumer_t *c = p->consumers_.head();
    while(c!=0)
    {
        consumer_t *n = p->consumers_.next(c);
        c->producer_end(t);
        disassociate(c);
        c = n;
    }
}

void strm::strummer_t::impl_t::add_consumer(consumer_t *c, unsigned long long t)
{
    consumers_.insert(std::make_pair(c->id_,c));

    if(!is_open_course(c))
    {
        int course = int(c->get_context());

        data_wire_t *wire = (data_wire_t *)c;

        // take over the output from an open course if that was actively sending out
        // data over its output
        pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t goc(open_course_wires_);
        pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it = goc.value().find(course);
        if(it != goc.value().end())
        {
            if(it->second->is_output_active())
            {
                inherit(it->second, wire, t);
                return;
            }
        }

        if(!config_.poly_courses_enable_)
        {
            // take over the output from another key on the same course if it's active
            // if other keys on the same cours exist, but are not further down the course,
            // this consumer is simply not activated
            pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype::iterator ic,ec;
            ic = consumers_.begin();
            ec = consumers_.end();
            for(;ic!=ec;ic++)
            {
                data_wire_t *other = (data_wire_t *)ic->second;
                if(other == wire) continue;

                if(other->is_output_active() && int(other->get_context()) == course)
                {
                    if(other->key_ < wire->key_)
                    {
                        inherit(other, wire, t);
                    }

                    return;
                }
            }
        }

        // look for active producers that this consumer could apply to
        pic::lckmultimap_t<piw::data_nb_t, producer_t *>::nbtype::iterator ip,ep;
        ip = producers_.begin();
        ep = producers_.end();
        for(;ip!=ep;ip++)
        {
            producer_t *p = ip->second;
            if(p->id_.is_null())
            {
                continue;
            }

            if(p->applies_to_consumer(c, true))
            {
                associate(p,c);
                p->join(c);
                deactivate_open_courses(p,t);
                return;
            }
        }
    }
}

void strm::strummer_t::impl_t::muting_consumer(consumer_t *c, unsigned long long t)
{
    if(!is_open_course(c))
    {
        int course = int(c->get_context());

        // mute open courses
        pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t goc(open_course_wires_);
        pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it = goc.value().find(course);
        if(it != goc.value().end())
        {
            deactivate_open_course(it->second, t);
        }
    }
}

void strm::strummer_t::impl_t::del_consumer(consumer_t *c, unsigned long long t)
{
    consumers_.erase(c->id_);

    if(!is_open_course(c))
    {
        find_del_beneficiary((data_wire_t *)c, t);
    }

    disassociate(c);
}

void strm::strummer_t::impl_t::find_del_beneficiary(data_wire_t *w, unsigned long long t)
{
    // only do further processing if the deleted consumer is
    // actively sending output data
    if(config_.pulloff_enable_ &&
       w->is_output_active() &&
       t-w->last_pulloff_ <= config_.pulloff_interval_)
    {
        int course = int(w->get_context());

        if(!config_.poly_courses_enable_)
        {
            // hand over the output to another consumer on the same course if
            // it's not actively sending output data, find the consumer that's
            // furthest down the course
            pic::lckmultimap_t<piw::data_nb_t, consumer_t *>::nbtype::iterator ic,ec;
            ic = consumers_.begin();
            ec = consumers_.end();
            data_wire_t *other = 0;
            int other_key_ = 0;
            for(;ic!=ec;ic++)
            {
                data_wire_t *o = (data_wire_t *)ic->second;
                int other_course = int(o->get_context());

                if(other_course == course)
                {
                    if(o->key_ >= w->key_)
                    {
                        return;
                    }

                    if(!o->is_output_active() && o->key_ > other_key_)
                    {
                        other = o;
                        other_key_ = o->key_;
                    }
                }
            }

            if(other)
            {
                inherit(w, other, t);
                return;
            }
        }

        // hand over the output to an open course for the same course if that option
        // is enabled
        if(config_.open_course_enable_)
        {
            pic::flipflop_t<pic::lckmap_t<int,data_wire_t *>::lcktype>::guard_t goc(open_course_wires_);
            pic::lckmap_t<int,data_wire_t *>::lcktype::const_iterator it = goc.value().find(course);
            if(it != goc.value().end())
            {
                inherit(w, it->second, t);
                return;
            }
        }
    }
}

void strm::strummer_t::impl_t::inherit(consumer_t *from, consumer_t *to, unsigned long long t)
{
    data_wire_t *from_wire = (data_wire_t *)from;
    data_wire_t *to_wire = (data_wire_t *)to;

    if(is_open_course(to))
    {
        activate_wire(to_wire);
        to_wire->start_event(to_wire->path_.make_nb().restamp(t));
    }

    to_wire->inherit(from_wire, t);
    associate(from->producer(), to);

    if(is_open_course(from))
    {
        from_wire->end_event(t);
        deactivate_wire(from_wire);
        disassociate(from);
    }
}

/**
 * producer_t
 */

producer_t::producer_t(correlator_t *c, valid_consumer_check_t valid_check, similar_consumer_check_t similar_check):
    correlator_(c), valid_consumer_check_(valid_check), similar_consumer_check_(similar_check), last_event_start_(0)
{
}

producer_t::~producer_t()
{
    unsigned long long t = piw::tsd_time();
    piw::tsd_fastcall(ender__,this,&t);
}

void producer_t::start(consumer_t *c, unsigned long long t)
{
    c->producer_start(t, buffer_);
}

void producer_t::join(consumer_t *c)
{
    c->producer_join(last_event_start_, buffer_);
}

void producer_t::start_event(unsigned long long t, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &buffer)
{
    end_event(id.time());

    id_ = id;
    buffer_ = buffer;
    last_event_start_ = t;
    correlator_->add_producer(this,t);
}

void producer_t::muting_event(const piw::data_nb_t &id)
{
    if(id_.is_null())
    {
        correlator_->muting_producer(this, id.time());
    }
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

bool producer_t::applies_to_consumer(consumer_t *c, bool allow_similar)
{
    return valid_consumer_check_(c, allow_similar);
}

bool producer_t::has_similar_consumers(void *data)
{
    for(consumer_t *c = consumers_.head(); c!=0; c=consumers_.next(c))
    {
        if(similar_consumer_check_(c, data))
        {
            return true;
        }
    }
    return false;
}


/**
 * consumer_t
 */

consumer_t::consumer_t(correlator_t *c): correlator_(c), producer_(0)
{
}

consumer_t::~consumer_t()
{
    unsigned long long t = piw::tsd_time();
    piw::tsd_fastcall(ender__,this,&t);
}

void consumer_t::start_event(const piw::data_nb_t &id)
{
    end_event(id.time());

    id_ = id;
    correlator_->add_consumer(this, id.time());
}

void consumer_t::muting_event(const piw::data_nb_t &id)
{
    if(id_.is_null())
    {
        correlator_->muting_consumer(this, id.time());
    }
}

void consumer_t::end_event(unsigned long long t)
{
    if(!id_.is_null())
    {
        correlator_->del_consumer(this, t);
        id_ = piw::data_nb_t();
    }
}


/**
 * strm::strumconfig_t
 */

strm::strumconfig_t::strumconfig_t(): impl_(new impl_t())
{
}

strm::strumconfig_t::~strumconfig_t()
{
    delete impl_;
}

strm::strumconfig_t::strumconfig_t(const strumconfig_t &o)
{
    *impl_ = *(o.impl_);
}

strm::strumconfig_t &strm::strumconfig_t::operator=(const strumconfig_t &o)
{
    *impl_ = *(o.impl_);
    return *this;
}

void strm::strumconfig_t::enable(bool b) { impl_->enable(b); }
void strm::strumconfig_t::set_trigger_window(unsigned tw) { impl_->set_trigger_window(tw); }
void strm::strumconfig_t::set_course_pressure_scale(float scale) { impl_->set_course_pressure_scale(scale); }
void strm::strumconfig_t::set_course_roll_scale(float scale) { impl_->set_course_roll_scale(scale); }
void strm::strumconfig_t::set_course_yaw_scale(float scale) { impl_->set_course_yaw_scale(scale); }
void strm::strumconfig_t::set_strum_breath_scale(float scale) { impl_->set_strum_breath_scale(scale); }
void strm::strumconfig_t::set_strum_pressure_scale(float scale) { impl_->set_strum_pressure_scale(scale); }
void strm::strumconfig_t::set_strum_roll_scale(float scale) { impl_->set_strum_roll_scale(scale); }
void strm::strumconfig_t::set_strum_roll_pressure_mix(float mix) { impl_->set_strum_roll_pressure_mix(mix); }
void strm::strumconfig_t::set_strum_yaw_scale(float scale) { impl_->set_strum_yaw_scale(scale); }
void strm::strumconfig_t::set_strum_yaw_pressure_mix(float mix) { impl_->set_strum_yaw_pressure_mix(mix); }
void strm::strumconfig_t::set_strum_note_end(bool b) { impl_->set_strum_note_end(b); }
void strm::strumconfig_t::set_poly_courses_enable(bool p) { impl_->set_poly_courses_enable(p); }
void strm::strumconfig_t::set_course_mute_threshold(float t) { impl_->set_course_mute_threshold(t); }
void strm::strumconfig_t::set_course_mute_interval(unsigned i) { impl_->set_course_mute_interval(i); }
void strm::strumconfig_t::set_course_mute_enable(bool p) { impl_->set_course_mute_enable(p); }
void strm::strumconfig_t::set_strum_mute_threshold(float t) { impl_->set_strum_mute_threshold(t); }
void strm::strumconfig_t::set_strum_mute_interval(unsigned i) { impl_->set_strum_mute_interval(i); }
void strm::strumconfig_t::set_strum_mute_enable(bool p) { impl_->set_strum_mute_enable(p); }
void strm::strumconfig_t::set_pulloff_threshold(float t) { impl_->set_pulloff_threshold(t); }
void strm::strumconfig_t::set_pulloff_interval(unsigned i) { impl_->set_pulloff_interval(i); }
void strm::strumconfig_t::set_pulloff_enable(bool p) { impl_->set_pulloff_enable(p); }
void strm::strumconfig_t::open_course_enable(bool b) { impl_->open_course_enable(b); }
void strm::strumconfig_t::open_course_pressure_default(float v) { impl_->open_course_pressure_default(v); }
void strm::strumconfig_t::open_course_roll_default(float v) { impl_->open_course_roll_default(v); }
void strm::strumconfig_t::open_course_yaw_default(float v) { impl_->open_course_yaw_default(v); }
void strm::strumconfig_t::clear_breath_courses() { impl_->clear_breath_courses(); }
void strm::strumconfig_t::add_breath_course(int course) { impl_->add_breath_course(course); }
void strm::strumconfig_t::remove_breath_course(int course) { impl_->remove_breath_course(course); }
void strm::strumconfig_t::clear_key_courses() { impl_->clear_key_courses(); }
void strm::strumconfig_t::add_key_course(int course, int column, int row) { impl_->add_key_course(course, column, row); }
void strm::strumconfig_t::clear_key_course(int course) { impl_->clear_key_course(course); }
void strm::strumconfig_t::clear_open_courses_info() { impl_->clear_open_courses_info(); }
void strm::strumconfig_t::set_open_course_info(int course, int key, int column, int row) { impl_->set_open_course_info(course, key, column, row); }
void strm::strumconfig_t::set_open_course_key(int course, int key) { impl_->set_open_course_key(course, key); }
void strm::strumconfig_t::set_open_course_physical(int course, int column, int row) { impl_->set_open_course_physical(course, column, row); }
void strm::strumconfig_t::clear_open_course_info(int course) { impl_->clear_open_course_info(course); }
std::string strm::strumconfig_t::encode_courses() const { return impl_->encode_courses(); }


/**
 * strm::strummer_t
 */

strm::strummer_t::strummer_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d): impl_(new impl_t(c,d))
{
}

strm::strummer_t::~strummer_t()
{
    delete impl_;
}

void strm::strummer_t::set_strumconfig(const strm::strumconfig_t &c)
{
    impl_->set_strumconfig(*c.impl_);
}

piw::cookie_t strm::strummer_t::cookie()
{
    return piw::cookie_t(impl_);
}
