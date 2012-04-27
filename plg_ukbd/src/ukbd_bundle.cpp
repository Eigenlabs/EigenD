
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

#include <lib_micro/micro_active.h>
#include <lib_micro/micro_usb.h>

#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_table.h>
#include <piw/piw_clock.h>
#include <piw/piw_status.h>
#include <piw/piw_keys.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>

#include "ukbd_bundle.h"

#include <math.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <memory>

#define KEYS 17

#define ESTIMATION_END 10
#define KBD_LATENCY 5000
#define THRESHOLD_GATE 100
#define THRESHOLD_HARD 800

#define STRIP_THRESH 50
#define STRIP_MIN 400
#define STRIP_MAX 3000
#define STRIP_SIG 1
#define BREATH_SIG 2

#define TEMP_INTERVAL 30000

namespace
{
    struct ledsink_t: pic::sink_t<void(const piw::data_nb_t &)>
    {
        ledsink_t(ukbd::bundle_t::impl_t *i): i_(i) {}
        void invoke(const piw::data_nb_t &p1) const;
        bool iscallable() const { return i_.isvalid(); }
        bool compare(const pic::sink_t<void(const piw::data_nb_t &)> *s) const
        {
            const ledsink_t *c = dynamic_cast<const ledsink_t *>(s);
            if(c && c->i_==i_) return true;
            return false;
        }

        pic::weak_t<ukbd::bundle_t::impl_t> i_;
    };

    struct keyboard_t;

    struct kwire_t: piw::wire_ctl_t, piw::event_data_source_real_t
    {
        kwire_t(unsigned i,unsigned c,unsigned r,const piw::data_t &path,keyboard_t *k);
        ~kwire_t() { source_shutdown(); keyboard = 0; }

        void activate(unsigned long long);

        void source_ended(unsigned seq)
        {
        }

        void key(unsigned long long t, bool a, unsigned p, int r, int y);

        unsigned counter;
        float maxpressure;
        keyboard_t *keyboard;
        unsigned index_,column_,row_;

        bool active;
        unsigned long long last;
        bool gated;
        piw::data_nb_t id_;
        piw::xevent_data_buffer_t output_;
    };

    struct cwire_t;

    struct strip_t
    {
        strip_t(cwire_t *w): count_(100), last_(0), state_(0), wire_(w)
        {
            set_range(STRIP_THRESH,STRIP_MIN,STRIP_MAX);
        }

        void update(unsigned short s, unsigned long long t);

        void set_range(unsigned short thresh, unsigned short min, unsigned short max)
        {
            min_ = min;
            range_ = max-min;
            threshold_ = thresh+((min-thresh)/4);
            pic::logmsg() << "threshold " << threshold_ << " range " << range_ << " min " << min_;
        }

        unsigned count_;
        unsigned short last_, origin_;
        unsigned state_;
        unsigned short min_, range_, threshold_;
        cwire_t *wire_;
    };

    struct breath_t
    {
        breath_t(cwire_t *w) : tick(0), wire_(w) {}
        void update(unsigned short b, unsigned long long t);

        int tick;
        cwire_t *wire_;
    };

    struct cwire_t : piw::wire_ctl_t, piw::event_data_source_real_t
    {
        cwire_t(const piw::data_t &,keyboard_t *);
        ~cwire_t() { source_shutdown(); keyboard_ = 0; }

        void source_ended(unsigned seq)
        {
        }

        void id_enable(unsigned long long t)
        {
            if(enabled_++==0)
            {
                output_=piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_NORM);
                output_.add_value(STRIP_SIG,piw::makefloat_bounded_nb(1,-1,0,0,t));
                output_.add_value(BREATH_SIG,piw::makefloat_bounded_nb(1,-1,0,0,t));
                source_start(0,id_.restamp(t),output_);
            }
        }

        void id_disable(unsigned long long t)
        {
            if(enabled_==0)
                return;
            if(--enabled_==0)
            {
                source_end(t);
            }
        }

        piw::data_nb_t id_;
        unsigned enabled_;
        keyboard_t *keyboard_;
        piw::xevent_data_buffer_t output_;
        strip_t strip_;
        breath_t breath_;
    };

    void strip_t::update(unsigned short s, unsigned long long t)
    {
        if(--count_ != 0)
        {
            return;
        }

        count_ = 20;

        switch(state_)
        {
            case 0:
                if(s<threshold_)
                    break;

                state_ = 1;
                count_ = 100;
                break;

            case 1:
                if(s<threshold_)
                {
                    state_ = 0;
                }
                else
                {
                    state_ = 2;
                    origin_ = s;
                    wire_->id_enable(t);
                }
                break;
            
            case 2:
                if(abs(s-last_)<200 && s>threshold_)
                {
                    float f = 2.0*(float)(origin_-s)/((float)range_);
                    wire_->output_.add_value(STRIP_SIG,piw::makefloat_bounded_nb(1,-1,0,f,t));
                    break;
                }

                state_ = 3;
                count_ = 80;
                break;

            case 3:
                if(s<threshold_)
                {
                    wire_->output_.add_value(STRIP_SIG,piw::makefloat_bounded_nb(1,-1,0,0,t));
                    wire_->id_disable(t+1);
                    state_ = 0;
                }
                else
                {
                    state_ = 2;
                }
                break;
        }

        last_ = s;
    }

    void breath_t::update(unsigned short b, unsigned long long t)
    {
        int b0 = 2048-b;

        if(abs(b0)>200)
        {
            if(tick==0)
            {
                wire_->id_enable(t);
            }

            tick = 100;
        }

        if(tick>0)
        {
            --tick;
            if(tick>0)
            {
                wire_->output_.add_value(BREATH_SIG,piw::makefloat_bounded_nb(1,-1,0,float(b0)/2048.0,t));
            }
            else
            {
                wire_->output_.add_value(BREATH_SIG,piw::makefloat_bounded_nb(1,-1,0,0,t));
                wire_->id_disable(t+1);
            }
        }
    }
    
    static const unsigned int MICRO_COLUMNCOUNT = 5;
    static const unsigned int MICRO_ROWS[MICRO_COLUMNCOUNT] = {4,4,4,4,1};

    static const unsigned int MICRO_COURSEKEYS = 17;

    struct keyboard_t: piw::root_ctl_t, piw::wire_ctl_t, piw::thing_t, micro::active_t::delegate_t
    {
        keyboard_t(const piw::cookie_t &c, const pic::notify_t &d, const piw::change_t &a): dead(d)
        {
            threshold1 = THRESHOLD_GATE;
            threshold2 = THRESHOLD_HARD;
            piw::tsd_thing(this);

            set_slow_dequeue_handler(a);

            set_latency(KBD_LATENCY);

            connect(c);

#ifdef UKBD_LOG
            log = fopen("/tmp/key.out","w");
            fprintf(log,"key pressure slow-lpf p2 fast-lpf p3 active\n"); fflush(log);
#endif
            queue = 0;
            drops = 0;
            counter = 0;

            create_kwires();

            cwire_ = std::auto_ptr<cwire_t>(new cwire_t(piw::pathone(2,0),this));
        }

        ~keyboard_t()
        {
            tracked_invalidate();
        }

        void activate(unsigned index, unsigned long long time)
        {
            piw::data_t data = piw::makelong(index,time);
            enqueue_slow(data);
        }

        void kbd_dead(unsigned reason)
        {
            trigger_slow();
        }

        void kbd_strip(unsigned long long t, unsigned s)
        {
            cwire_->strip_.update(t,s);
        }

        void kbd_breath(unsigned long long t, unsigned b)
        {
            cwire_->breath_.update(t,b);
        }

        void kbd_key(unsigned long long t, unsigned key, bool a, unsigned p, int r, int y)
        {
            kwires_[key]->key(t,a,p,r,y);
        }

        void thing_trigger_slow()
        {
            root_ctl_t::disconnect();
            dead();
        }

        void create_kwires()
        {
            unsigned previous_rowkeys = 0;
            unsigned column = 1;
            unsigned row = 1;

            unsigned columncount = get_columncount();
            for(unsigned k=1; k <= KEYS && column <= columncount; k++)
            {
                unsigned rowkeys = get_columnlen_array()[column-1];
                if((k-previous_rowkeys) <= rowkeys)
                {
                    row = k-previous_rowkeys; 
                }
                else
                {
                    column++;
                    previous_rowkeys += rowkeys;
                    row = k-previous_rowkeys; 
                }
                kwires_[k-1] = std::auto_ptr<kwire_t>(new kwire_t(k,column,row,piw::pathtwo(1,k,0),this));
            }
        }

        const unsigned int get_columncount()
        {
            return MICRO_COLUMNCOUNT;
        }

        const unsigned int* get_columnlen_array()
        {
            return MICRO_ROWS;
        }

        const piw::data_t get_columnlen_tuple()
        {
            if(!columnlen_.is_null())
            {
                return columnlen_;
            }

            piw::data_t rows = piw::tuplenull(0);
            for(unsigned i = 0; i < get_columncount(); ++i)
            {
                rows = piw::tupleadd(rows, piw::makelong(get_columnlen_array()[i],0));
            }

            columnlen_ = rows;

            return columnlen_;
        }

        const piw::data_t get_columnoffset_tuple()
        {
            if(!columnoffset_.is_null())
            {
                return columnoffset_;
            }

            piw::data_t rows = piw::tuplenull(0);
            for(unsigned i = 0; i < get_columncount(); ++i)
            {
                rows = piw::tupleadd(rows, piw::makelong(0,0));
            }

            columnoffset_ = rows;

            return columnoffset_;
        }

        const unsigned int get_coursecount()
        {
            return 1;
        }

        const unsigned int get_courselen()
        {
            return MICRO_COURSEKEYS;
        }

        const piw::data_t get_courselen_tuple()
        {
            if(!courselen_.is_null())
            {
                return courselen_;
            }

            piw::data_t courses = piw::tuplenull(0);
            courses = piw::tupleadd(courses, piw::makelong(get_courselen(),0));

            courselen_ = courses;

            return courselen_;
        }

        const piw::data_t get_courseoffset_tuple()
        {
            if(!courseoffset_.is_null())
            {
                return courseoffset_;
            }

            piw::data_t courses = piw::tuplenull(0);
            courses = piw::tupleadd(courses, piw::makefloat(0.0,0));

            courseoffset_ = courses;

            return courseoffset_;
        }

        std::auto_ptr<kwire_t> kwires_[KEYS];
        std::auto_ptr<cwire_t> cwire_;
        pic::notify_t dead;
        float threshold1,threshold2;
        piw::data_t columnlen_;
        piw::data_t columnoffset_;
        piw::data_t courselen_;
        piw::data_t courseoffset_;

        pic_atomic_t queue;
        unsigned drops;
        unsigned counter;
    };

    kwire_t::kwire_t(unsigned i, unsigned c, unsigned r, const piw::data_t &path, keyboard_t *k): piw::event_data_source_real_t(path), counter(0), maxpressure(0), keyboard(k), index_(i), column_(c), row_(r), active(false), last(0), gated(false),id_(piw::pathone_nb(i,0)),output_(15,PIW_DATAQUEUE_SIZE_NORM)
    {
        keyboard->connect_wire(this,source());
    }

    cwire_t::cwire_t(const piw::data_t &path, keyboard_t *k): piw::event_data_source_real_t(path), id_(piw::pathnull_nb(0)), enabled_(0), keyboard_(0), strip_(this),breath_(this)
    {
        k->connect_wire(this,source());
    }

    static float __clip(float x)
    {
        x = std::max(x,-1.f);
        x = std::min(x,1.f);
        return x;
    }

    void kwire_t::key(unsigned long long t, bool a, unsigned p, int r, int y)
    {
        if(t < last)
        {
            long long dt = t-last;
            pic::msg() << "ukbd key " << index_ << " sent out of order last=" << last <<  " now=" << t << " diff=" << dt << pic::log;
        }

        last=t;

        if(!a)
        {
            counter=0;

            output_.add_value(2,piw::makefloat_bounded_nb(1.0,0.0,0.0,0.0,t));
            source_end(t+1);

            return;
        }

        output_.add_value(2,piw::makefloat_bounded_nb(1.0,0.0,0.0,p/4096.0,t));
        output_.add_value(3,piw::makefloat_bounded_nb(1.0,-1.0,0.0,__clip(r/2048.0),t));
        output_.add_value(4,piw::makefloat_bounded_nb(1.0,-1.0,0.0,__clip(y/2048.0),t));

        if(counter==0)
        {
            output_ = piw::xevent_data_buffer_t(15,PIW_DATAQUEUE_SIZE_NORM);
            output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,0,0));
            output_.add_value(3,piw::makefloat_bounded_nb(1,-1,0,0,0));
            output_.add_value(4,piw::makefloat_bounded_nb(1,-1,0,0,0));
            output_.add_value(5,piw::makekey(index_,column_,row_,index_,1,index_,piw::KEY_LIGHT,t));

            source_start(0,id_.restamp(t), output_);

            maxpressure=0;
            counter=1;
            gated=false;
            return;
        }

        maxpressure=std::max(maxpressure,(float)p);

        if(counter<ESTIMATION_END)
        {
            counter++;
            return;
        }

        if(counter==ESTIMATION_END)
        {
            counter=ESTIMATION_END+1;

            if(maxpressure > keyboard->threshold2)
            {
                output_.add_value(5,piw::makekey(index_,row_,column_,index_,1,index_,piw::KEY_HARD,t));

                gated = true;
            }
        }

        if(!gated && maxpressure > keyboard->threshold1)
        {
            output_.add_value(5,piw::makekey(index_,row_,column_,index_,1,index_,piw::KEY_SOFT,t));

            gated = true;
        }
    }

    void kwire_t::activate(unsigned long long time)
    {
        if(keyboard)
        {
            keyboard->activate(index_,time);
        }
    }

}

struct ukbd::bundle_t::impl_t : virtual pic::tracked_t, piw::thing_t, piw::clockdomain_ctl_t, piw::clocksink_t
{
    impl_t(const char *n, const piw::cookie_t &c, const pic::notify_t &d, const piw::change_t &a): loop_(n,&keyboard_), keyboard_(c,d,a), leds_(MICRO_COLUMNCOUNT,MICRO_ROWS)
    {
        set_source(piw::makestring("*",0));

        piw::tsd_thing(this);
        sink(this,"ukbd");

        keyboard_.set_clock(this);

        loop_.load_calibration_from_device();

        loop_.start();
        tick_enable(false);

        timer_slow(TEMP_INTERVAL);
    }

    void thing_timer_slow()
    {
        pic::logmsg() << "instrument internal temperature: " << loop_.get_temperature();
        timer_slow(TEMP_INTERVAL);
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        loop_.poll(t);
    }

    ~impl_t()
    {
        tracked_invalidate();
        tick_disable();
        loop_.stop();
    }

    void thing_dequeue_slow(const piw::data_t &d)
    {
        __lightdeq(this,(void *)(d.give()),0,0);
    }

    piw::data_t get_columnlen()
    {
        return keyboard_.get_columnlen_tuple();
    }

    piw::data_t get_columnoffset()
    {
        return keyboard_.get_columnoffset_tuple();
    }

    piw::data_t get_courselen()
    {
        return keyboard_.get_courselen_tuple();
    }

    piw::data_t get_courseoffset()
    {
        return keyboard_.get_courseoffset_tuple();
    }

    static void __set_led(void *self, unsigned key, unsigned color)
    {
        impl_t *i = (impl_t *)self;
        unsigned mode=color;
        i->loop_.set_led(key,((mode&1)<<2) + ((mode&2)<<4));
    }

    static void __lightdeq(void *a, void *b, void *c, void *d)
    {
        impl_t *i = (impl_t *)a;
        piw::data_nb_t data = piw::data_nb_t::from_given((bct_data_t)b);
        i->leds_.update_leds(data, i, __set_led);
    }

    micro::active_t loop_;
    keyboard_t keyboard_;
    piw::statusledconvertor_t leds_;
};

void ledsink_t::invoke(const piw::data_nb_t &d) const
{
    i_->enqueue_slow(piw::data_t::from_given(d.give()));
}

ukbd::bundle_t::bundle_t(const char *n, const piw::cookie_t &c, const pic::notify_t &d, const piw::change_t &a): _root(new impl_t(n,c,d,a))
{
}

ukbd::bundle_t::~bundle_t()
{
    if(_root)
        delete _root;
}

void ukbd::bundle_t::close()
{
    impl_t *r = _root;
    _root = 0;

    if(r)
    {
        delete r;
    }
}

void ukbd::bundle_t::set_threshold1(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.threshold1=t*4096.0;
}

float ukbd::bundle_t::get_threshold1()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.threshold1/4096.0;
}

void ukbd::bundle_t::set_threshold2(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.threshold2=t*4096.0;
}

float ukbd::bundle_t::get_threshold2()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.threshold2/4096.0;
}

std::string ukbd::bundle_t::name()
{
    PIC_ASSERT(_root);
    return _root->loop_.get_name();
}

piw::data_t ukbd::bundle_t::get_columnlen()
{
    PIC_ASSERT(_root);
    return _root->get_columnlen();
}

piw::data_t ukbd::bundle_t::get_columnoffset()
{
    PIC_ASSERT(_root);
    return _root->get_columnoffset();
}

piw::data_t ukbd::bundle_t::get_courselen()
{
    PIC_ASSERT(_root);
    return _root->get_courselen();
}

piw::data_t ukbd::bundle_t::get_courseoffset()
{
    PIC_ASSERT(_root);
    return _root->get_courseoffset();
}

piw::change_nb_t ukbd::bundle_t::led_functor()
{
    PIC_ASSERT(_root);
    return piw::change_nb_t(pic::ref(new ledsink_t(_root))); 
}

int ukbd::bundle_t::gc_traverse(void *v, void *a) const
{
    if(_root)
    {
        int r;
        if((r=_root->keyboard_.dead.gc_traverse(v,a))!=0) return r;
        if((r=_root->keyboard_.gc_traverse(v,a))!=0) return r;
    }

    return 0;
}

int ukbd::bundle_t::gc_clear()
{
    if(_root)
    {
        _root->keyboard_.dead.gc_clear();
        _root->keyboard_.gc_clear();
    }

    return 0;
}
