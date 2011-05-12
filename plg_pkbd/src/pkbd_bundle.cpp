
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

// move strip/breath handling logic to pico_active

#include <lib_pico/pico_active.h>
#include <lib_pico/pico_usb.h>

#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_table.h>
#include <piw/piw_clock.h>
#include <piw/piw_status.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_safeq.h>

#include "pkbd_bundle.h"

#include <math.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include <memory>

#define KEYS 22

#define INITIAL_VELOCITY 0.00001
#define ESTIMATION_END 10
#define KBD_LATENCY 5000
#define KEY_SCALE 100.0
#define THRESHOLD_GATE 100
#define THRESHOLD_HARD 800
#define AXIS_WINDOW 1.0f
#define STRIP_THRESH 50
#define STRIP_MIN 110
#define STRIP_MAX 3050
#define TEMP_INTERVAL 30000
#define DEBOUNCE_US 20000
#define BREATH_GAIN_ADJUST 1.4f

#define BPMSG_KLASS "Breath Pipe Blocked"
#define BPMSG_TITLE "Breath Pipe Blocked"
#define BPMSG_MSG   "Your Breath Pipe appears to be blocked\n\n" \
                    "Please see the FAQ at the Eigenlabs support " \
                    "site for instructions on how to clear your breath pipe:\n\n" \
                    "http://www.eigenlabs.com/faq/pico/im-having-breath-pipe-problems"

namespace
{
    struct ledsink_t: pic::sink_t<void(const piw::data_nb_t &)>
    {
        ledsink_t(pkbd::bundle_t::impl_t *i): i_(i)  {}
        void invoke(const piw::data_nb_t &p1) const;
        bool iscallable() const { return i_.isvalid(); }
        bool compare(const pic::sink_t<void(const piw::data_nb_t &)> *s) const
        {
            const ledsink_t *c = dynamic_cast<const ledsink_t *>(s);
            if(c && c->i_==i_) return true;
            return false;
        }

        pic::weak_t<pkbd::bundle_t::impl_t> i_;
    };

    struct logblob_t
    {
        unsigned k;
        float p,r,y;
    };

    struct keyboard_t;

    struct kwire_t: piw::wire_ctl_t, piw::event_data_source_real_t
    {
        kwire_t(unsigned i,const piw::data_t &path,keyboard_t *k);
        ~kwire_t() { source_shutdown(); keyboard = 0; }

        void activate(unsigned long long);

        void source_ended(unsigned seq)
        {
        }

        void key(unsigned long long t, bool a, unsigned p, int r, int y);
        void mode(unsigned long long t,unsigned p);

        unsigned counter;
        float maxpressure;
        keyboard_t *keyboard;
        unsigned index;

        bool active;
        unsigned long long last;
        bool gated;
        piw::data_nb_t id_;
        piw::xevent_data_buffer_t output_;
        unsigned long long ts_;
        unsigned long long start_;
    };

    struct cwire_t: piw::wire_ctl_t, piw::event_data_source_real_t
    {
        cwire_t(const piw::data_t &,keyboard_t *);
        ~cwire_t() { source_shutdown(); }

        void source_ended(unsigned seq)
        {
        }

        void id_enable(unsigned long long t)
        {
            output_=piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_NORM);
            output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,t));
            source_start(0,id_.restamp(t),output_);
        }

        void id_disable(unsigned long long t)
        {
            source_end(t);
        }

        piw::data_nb_t id_;
        piw::xevent_data_buffer_t output_;
    };

    struct strip_t: cwire_t
    {
        strip_t(keyboard_t *);

        void update(unsigned long long t, unsigned s);

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
    };

    struct breath_t: cwire_t, piw::thing_t
    {
        breath_t(keyboard_t *);
        ~breath_t();

        void update(unsigned long long t,unsigned b);
        void thing_trigger_slow();

        int tick;
        unsigned warmup_count_;
        int zero_;
        float pos_threshold_;
        float pos_gain_;
        float neg_threshold_;
        float neg_gain_;
        int breath_raw_;
        unsigned stuck_count_;
        int stuck_min_;
        int stuck_max_;
    };

    void strip_t::update(unsigned long long t,unsigned s)
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

                //pic::logmsg() << "strip starting: " << s;
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
                    //pic::logmsg() << "strip origin: " << s;
                    state_ = 2;
                    origin_ = s;
                    id_enable(t);
                }
                break;

            case 2:
                if(abs((double)s-last_)<200 && s>threshold_)
                {
                    int o = origin_;
                    o -= s;
                    float f = (float)o/(float)range_;
                    float abs = (float)(s-min_)/(float)(range_/2.0f)-1.0f;
                    if(abs>1.0f)
                        abs=1.0f;
                    if(abs<-1.0f)
                        abs=-1.0f;
                    //pic::logmsg() << "strip " << s << " -> " << f << "  range=" << range_ << "  abs=" << abs;
                    output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,f,t));
                    output_.add_value(2,piw::makefloat_bounded_nb(1,-1,0,abs,t));
                    break;
                }

                state_ = 3;
                count_ = 80;
                break;

            case 3:
                if(s<threshold_)
                {
                    //pic::logmsg() << "strip ending";
                    output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,t));
                    id_disable(t+1);
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

    void breath_t::thing_trigger_slow()
    {
        pic::logmsg() << "breath pipe stuck: min=" << stuck_min_ << " max=" << stuck_max_;
        piw::tsd_alert(BPMSG_KLASS,BPMSG_TITLE,BPMSG_MSG);
    }

    void breath_t::update(unsigned long long t, unsigned b)
    {
        if(warmup_count_<1000)
        {
            ++warmup_count_;
            if(warmup_count_==1000)
            {
                ++warmup_count_;
                zero_=b;

                pos_threshold_=std::min((float)zero_+100.0,4000.0);
                pos_gain_=BREATH_GAIN_ADJUST/(4095.0-pos_threshold_);

                neg_threshold_=std::max((float)zero_-100.0,100.0);
                neg_gain_=BREATH_GAIN_ADJUST/neg_threshold_;

                pic::logmsg() << "breath controller zero point set to " << zero_;
            }
            return;
        }

        int d = abs(zero_-(int)b);
        if(d>20 && d<800)
        {
            stuck_min_=std::min(stuck_min_,(int)b);
            stuck_max_=std::max(stuck_max_,(int)b);
            breath_raw_ = b;
            ++stuck_count_;
            if(stuck_count_%1000==0)
                pic::logmsg() << "count " << stuck_count_ << " diff " << (stuck_max_-stuck_min_) << " d " << d;

            if(stuck_count_>(10*2000))
            {
                if((stuck_max_-stuck_min_)<50)
                {
                    trigger_slow();
                }
                stuck_count_=0;
                stuck_min_=4095;
                stuck_max_=0;
            }
        }
        else
        {
            stuck_count_=0;
            stuck_min_=4095;
            stuck_max_=0;
        }

        if(b>pos_threshold_ || b<neg_threshold_)
        {
            if(tick==0)
            {
                id_enable(t);
            }

            tick = 100;
        }

        if(tick>0)
        {
            --tick;
            if(tick>0)
            {
                float f = 0.0;
                if(b>pos_threshold_)
                {
                    f = std::min(1.0f,((float)b-pos_threshold_)*pos_gain_);
                }
                if(b<neg_threshold_)
                {
                    f = std::max(-1.0f,((float)b-neg_threshold_)*neg_gain_);
                }
                output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,f,t));
            }
            else
            {
                output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,t));
                id_disable(t+1);
            }
        }
    }

    struct keyboard_t: piw::root_ctl_t, piw::wire_ctl_t, piw::thing_t, pico::active_t::delegate_t
    {
        keyboard_t(const piw::cookie_t &c, const pic::notify_t &d, const piw::change_t &a): dead(d), key_logging_(false)
        {
            threshold1 = THRESHOLD_GATE;
            threshold2 = THRESHOLD_HARD;
            roll_axis_window_ = AXIS_WINDOW;
            yaw_axis_window_ = AXIS_WINDOW;
            piw::tsd_thing(this);

            set_slow_dequeue_handler(a);

            set_latency(KBD_LATENCY);

            connect(c);

            queue = 0;
            drops = 0;
            counter = 0;

            for(unsigned k=0; k<KEYS; ++k)
            {
                kwires_[k] = std::auto_ptr<kwire_t>(new kwire_t(k+1,piw::pathtwo(1,k+1,0),this));
            }

            swire_ = std::auto_ptr<strip_t>(new strip_t(this));
            bwire_ = std::auto_ptr<breath_t>(new breath_t(this));

            tick_bound_low_ = 0;
            tick_bound_hi_ = 0;

        }

        ~keyboard_t()
        {
            tracked_invalidate();
            close_thing();
        }

        void thing_dequeue_slow(const piw::data_t &d)
        {
            if(key_logging_ && key_logfile_ && d.is_blob())
            {
                logblob_t *b = (logblob_t *)d.as_blob();
                fprintf(key_logfile_,"%llu %u %f %f %f\n",d.time(),b->k,b->p,b->r,b->y);
            }
        }

        void log_keypress(unsigned k,float p,float r,float y)
        {
            if(key_logging_)
            {
                logblob_t *b;
                piw::data_nb_t d = piw::makeblob_nb(piw::tsd_time(),sizeof(logblob_t),(unsigned char **)&b);

                b->p = p;
                b->r = r;
                b->y = y;
                b->k = k;

                enqueue_slow_nb(d);
            }
        }

        void enable_key_logging(bool e)
        {
            if(e!=key_logging_)
            {
                key_logging_ = e;

                if(e)
                {
                    key_logfile_ = fopen("/tmp/key_log","w");
                }
                else
                {
                    fclose(key_logfile_);
                    key_logfile_ = 0;
                }
            }
        }

        void activate(unsigned index, unsigned long long time)
        {
            piw::data_nb_t data = piw::makelong_nb(index,time);
            enqueue_slow_nb(data);
        }

        void kbd_dead()
        {
            trigger_slow();
        }

        void kbd_strip(unsigned long long t, unsigned s)
        {
            swire_->update(t,s);
        }

        void kbd_breath(unsigned long long t, unsigned b)
        {
            bwire_->update(t,b);
        }

        void kbd_mode(unsigned long long t, unsigned key, unsigned b)
        {
            kwires_[key]->mode(t,b);
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

        std::auto_ptr<kwire_t> kwires_[KEYS];
        std::auto_ptr<strip_t> swire_;
        std::auto_ptr<breath_t> bwire_;
        pic::notify_t dead;
        float threshold1,threshold2,roll_axis_window_,yaw_axis_window_;
        bool key_logging_;
        FILE *key_logfile_;

        pic_atomic_t queue;
        unsigned drops;
        unsigned counter;

        unsigned long long tick_bound_low_;
        unsigned long long tick_bound_hi_;

    };

    kwire_t::kwire_t(unsigned i,const piw::data_t &path,keyboard_t *k): piw::event_data_source_real_t(path), counter(0), maxpressure(0), keyboard(k), index(i), active(false), last(0), gated(false),id_(piw::pathone_nb(i,0)),output_(15,PIW_DATAQUEUE_SIZE_NORM),ts_(0ULL)
    {
        keyboard->connect_wire(this,source());
    }

    cwire_t::cwire_t(const piw::data_t &path, keyboard_t *k): piw::event_data_source_real_t(path), id_(piw::pathnull_nb(0))
    {
        k->connect_wire(this,source());
    }

    strip_t::strip_t(keyboard_t *k): cwire_t(piw::pathone(2,0),k), count_(100), last_(0), state_(0)
    {
        set_range(STRIP_THRESH,STRIP_MIN,STRIP_MAX);
    }

    breath_t::breath_t(keyboard_t *k): cwire_t(piw::pathone(3,0),k), tick(0), warmup_count_(0), zero_(1900), pos_threshold_(2000), pos_gain_(1.0), neg_threshold_(1800), neg_gain_(1.0), breath_raw_(0), stuck_count_(0), stuck_min_(4095), stuck_max_(0)
    {
        piw::tsd_thing(this);
    }

    breath_t::~breath_t()
    {
        close_thing();
    }

    static float __clip(float x,float l)
    {
        if(l<0.0001)
        {
            return 0;
        }

        x = std::max(x,-l);
        x = std::min(x,l);
        x = x*1.0/l;
        x = std::max(x,-1.0f);
        x = std::min(x,1.0f);

        return x;
    }

    void kwire_t::mode(unsigned long long t,unsigned p)
    {
       bool bactive = false;

        if(p)
        {
            bactive = true;
        }

        if( bactive != active)
        {
            active = bactive;
            printf("mode change %d %llu\n",active,t);

            if(active)
            {
                output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_NORM);
                source_start(0,id_.restamp(t),output_);
            }
            else
            {

                output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,t));
                source_end(t);
                return;
            }

        }

        if( !active )
        {
            return;
        }

        output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,float(p/1024.0),t));
    }

    void kwire_t::key(unsigned long long t, bool a, unsigned p, int r, int y)
    {
        // adjust time if out of clock tick bounds
        t = std::max(t,keyboard->tick_bound_low_ + 1);

        if(t < last)
        {
            long long dt = t-last;
            pic::msg() << "pkbd key " << index << " sent out of order last=" << last <<  " now=" << t << " diff=" << dt << pic::log;
        }

        last=t;

        if(!a)
        {
            counter=0;
            output_.add_value(2,piw::makefloat_bounded_nb(1.0,0.0,0.0,0.0,t));

            source_end(t+1);
            ts_ = pic_microtime();
            if(t+1-start_<10000) pic::logmsg() << "k press lasted " << (t+1-start_);

            return;
        }

        output_.add_value(2,piw::makefloat_bounded_nb(1.0,0.0,0.0,p/4096.0,t));
        float r2 = __clip(((float)r)/2048.0,keyboard->roll_axis_window_);
        float y2 = __clip(((float)y)/2048.0,keyboard->roll_axis_window_);
        output_.add_value(3,piw::makefloat_bounded_nb(1.0,-1.0,0.0,r2,t));
        output_.add_value(4,piw::makefloat_bounded_nb(1.0,-1.0,0.0,y2,t));

        keyboard->log_keypress(index,p/4096.0,r2,y2);

        if(counter==0)
        {
            unsigned long long ts = pic_microtime();

            if(ts_ && ts<ts_+DEBOUNCE_US)
            {
                //pic::logmsg() << "debounce " << ts-ts_ << " us";
                //ts_=ts;
                return;
            }

            output_ = piw::xevent_data_buffer_t(15,PIW_DATAQUEUE_SIZE_NORM);
            output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,0,t));
            output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,0,t));
            output_.add_value(3,piw::makefloat_bounded_nb(1,-1,0,0,t));
            output_.add_value(4,piw::makefloat_bounded_nb(1,-1,0,0,t));

            start_=t;
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
                output_.add_value(1,piw::makefloat_bounded_nb(3.0,0.0,0.0,3.0,t));
                gated = true;
            }
        }

        if(!gated && maxpressure > keyboard->threshold1)
        {
            output_.add_value(1,piw::makefloat_bounded_nb(3.0,0.0,0.0,2.0,t));
            gated=true;
        }
    }

    void kwire_t::activate(unsigned long long time)
    {
        if(keyboard)
        {
            keyboard->activate(index,time);
        }
    }

}

struct pkbd::bundle_t::impl_t : virtual pic::tracked_t, piw::thing_t, piw::clockdomain_ctl_t, piw::clocksink_t, pic::safe_worker_t
{
    impl_t(const char *n, const piw::cookie_t &c, const pic::notify_t &d, const piw::change_t &a): pic::safe_worker_t(10,PIC_THREAD_PRIORITY_NORMAL), loop_(n,&keyboard_), keyboard_(c,d,a), leds_(KEYS)
    {
        set_source(piw::makestring("*",0));

        piw::tsd_thing(this);
        sink(this,"pkbd");

        keyboard_.set_clock(this);

        loop_.load_calibration_from_device();

        //run();
        loop_.start();
        tick_enable(false);

        timer_slow(TEMP_INTERVAL);
    }

    ~impl_t()
    {
        cancel_timer_slow();
        tracked_invalidate();
        tick_disable();
        loop_.stop();
        quit();
    }

    void thing_timer_slow()
    {
        pic::logmsg() << "instrument internal temperature: " << loop_.get_temperature();
        pic::logmsg() << "breath sensor value: " << keyboard_.bwire_->breath_raw_;

    }

    void thing_dequeue_slow(const piw::data_t &d)
    //void lightenq(const piw::data_nb_t &d)
    {
        if(getenv("PI_NOLEDS")!=0)
            return;

        //queue_.add(__lightdeq,this,(void *)(d.give()),0,0);
        __lightdeq(this,(void *)(d.give_copy()),0,0);
    }

    static void __set_led(void *self, unsigned key, unsigned color)
    {
        impl_t *i = (impl_t *)self;
        i->loop_.set_led(key,color);
    }

    static void __lightdeq(void *a, void *b, void *c, void *d)
    {
        impl_t *i = (impl_t *)a;
        piw::data_nb_t data = piw::data_nb_t::from_given((bct_data_t)b);
        i->leds_.update_leds(data, i, __set_led);
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        keyboard_.tick_bound_low_ = f;
        keyboard_.tick_bound_hi_ = t;
        loop_.poll(t);
    }

    pico::active_t loop_;
    keyboard_t keyboard_;
    piw::statusledconvertor_t leds_;
};

void ledsink_t::invoke(const piw::data_nb_t &d) const
{
    i_->enqueue_slow(piw::data_t::from_given(d.give()));
}

pkbd::bundle_t::bundle_t(const char *n, const piw::cookie_t &c, const pic::notify_t &d, const piw::change_t &a): _root(new impl_t(n,c,d,a))
{
}

pkbd::bundle_t::~bundle_t()
{
    if(_root)
        delete _root;
}

void pkbd::bundle_t::close()
{
    impl_t *r = _root;
    _root = 0;

    if(r)
    {
        delete r;
    }
}

void pkbd::bundle_t::set_roll_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.roll_axis_window_ = t;
}

float pkbd::bundle_t::get_roll_axis_window()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.roll_axis_window_;
}

void pkbd::bundle_t::set_yaw_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.yaw_axis_window_ = t;
}

float pkbd::bundle_t::get_yaw_axis_window()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.yaw_axis_window_;
}

void pkbd::bundle_t::enable_key_logging(bool e)
{
    PIC_ASSERT(_root);
    return _root->keyboard_.enable_key_logging(e);
}

void pkbd::bundle_t::set_threshold1(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.threshold1=t*4096.0;
}

float pkbd::bundle_t::get_threshold1()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.threshold1/4096.0;
}

void pkbd::bundle_t::set_threshold2(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.threshold2=t*4096.0;
}

float pkbd::bundle_t::get_threshold2()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.threshold2/4096.0;
}

std::string pkbd::bundle_t::name()
{
    PIC_ASSERT(_root);
    return _root->loop_.get_name();
}

piw::change_nb_t pkbd::bundle_t::led_functor()
{
    PIC_ASSERT(_root);
    return piw::change_nb_t(pic::ref(new ledsink_t(_root)));
}

int pkbd::bundle_t::gc_traverse(void *v, void *a) const
{
    if(_root)
    {
        int r;
        if((r=_root->keyboard_.dead.gc_traverse(v,a))!=0) return r;
        if((r=_root->keyboard_.gc_traverse(v,a))!=0) return r;
    }

    return 0;
}

int pkbd::bundle_t::gc_clear()
{
    if(_root)
    {
        _root->keyboard_.dead.gc_clear();
        _root->keyboard_.gc_clear();
    }

    return 0;
}
