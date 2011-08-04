
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

#include <picross/pic_config.h>

#include <lib_alpha1/alpha1_active.h>

#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_clock.h>
#include <piw/piw_status.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_safeq.h>

#include <math.h>
#include <memory>

#include "kbd_bundle.h"

#define ESTIMATION_END 10
#define KBD_LATENCY 5000
#define KEY_SCALE 100.0

#define THRESHOLD_GATE 100
#define THRESHOLD_HARD 800
#define AXIS_WINDOW 1.0f

#define TEMP_INTERVAL 30000
#define POLL_INTERVAL 50

#define DEBOUNCE_US 20000

#define BWMSG_KLASS "Insufficient USB Bandwidth"
#define BWMSG_TITLE "Insufficient USB Bandwidth"
#define BWMSG_MSG   "Your USB bus doesn't have enough USB bandwidth available " \
                    "for your Eigenharp to be able to function correctly.\n\n" \
                    "Your instrument or pedals might not be working until you " \
                    "plug your Eigenharp into another USB bus or remove other devices."

namespace
{
    struct ledsink_t: pic::sink_t<void(const piw::data_nb_t &)>
    {
        ledsink_t(kbd::bundle_t::impl_t *i): i_(i)  {}
        void invoke(const piw::data_nb_t &p1) const;
        bool iscallable() const { return i_.isvalid(); }
        bool compare(const pic::sink_t<void(const piw::data_nb_t &)> *s) const
        {
            const ledsink_t *c = dynamic_cast<const ledsink_t *>(s);
            if(c && c->i_==i_) return true;
            return false;
        }

        pic::weak_t<kbd::bundle_t::impl_t> i_;
    };

    struct blob_t
    {
        blob_t(unsigned k_, unsigned long long t_, unsigned a_, unsigned p_,int r_,int y_): k(k_),a(a_),p(p_),t(t_),r(r_),y(y_) {}
        unsigned k,a,p;
        unsigned long long t;
        int r,y;
    };

    struct keyboard_t;
    struct cwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        cwire_t(const piw::data_t &,keyboard_t *);
        ~cwire_t() { source_shutdown(); }

        void source_ended(unsigned seq) {}

        piw::data_nb_t id_;
        piw::xevent_data_buffer_t output_;
    };

    struct breath_t: cwire_t
    {
        breath_t(const piw::data_t &d,keyboard_t *k): cwire_t(d,k), active_(false) {}
        void update(const blob_t *b);

        bool active_;
        cwire_t *wire_;
    };

    struct strip_t: cwire_t
    {
        strip_t(const piw::data_t &d,keyboard_t *k): cwire_t(d,k),active_(false),count_(20) {}
        void update(const blob_t *b);

        bool active_;
        unsigned count_;
        unsigned short origin_;
    };

    struct kwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        kwire_t(unsigned,const piw::data_t &,keyboard_t *);
        ~kwire_t() { source_shutdown(); keyboard_ = 0; }

        void source_ended(unsigned seq) {}

        void update(const blob_t *b);

        unsigned index_;
        piw::data_nb_t id_;
        keyboard_t *keyboard_;
        piw::xevent_data_buffer_t output_;
        float maxpressure_;
        unsigned counter_;
        unsigned gated_;
        unsigned gated_count_;
        bool running_;
        bool skipping_;
        unsigned long long ts_;
    };

    void strip_t::update(const blob_t *b)
    {
        if(b->a != active_)
        {
            active_ = b->a;
            if(active_)
            {
                output_ = piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_NORM);
                source_start(0,id_.restamp(b->t),output_);
                count_ = 20;
                origin_ = b->p;
            }
            else
            {
                output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,b->t));
                source_end(b->t+1);
                return;
            }
        }

        if(--count_ != 0)
        {
            return;
        }

        count_ = 20;

        int dx = origin_-b->p;
        float f = (float)dx/4096.0;
        output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,f,b->t));
        //output_.add_value(2,piw::makefloat_bounded_nb(1,-1,0,((float)b->p/2048.0)-1.0,b->t));
        output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,((float)b->p/4096.0),b->t));
    }

    void breath_t::update(const blob_t *b)
    {
        if(b->a != active_)
        {
            active_ = b->a;
            if(active_)
            {
                output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_NORM);
                source_start(0,id_.restamp(b->t),output_);
            }
            else
            {
                output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,b->t));
                source_end(b->t);
                return;
            }
        }

        int b0 = b->p-2048;
        float bf = float(b0)/2048.0;
        output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,bf,b->t));
    }

    struct keyboard_t: piw::root_ctl_t, piw::thing_t, alpha1::active_t::delegate_t, virtual pic::lckobject_t
    {
        keyboard_t(const piw::cookie_t &c,const pic::notify_t &d, const char *n): dead_(d)
        {
            threshold1_ = THRESHOLD_GATE;
            threshold2_ = THRESHOLD_HARD;
            yaw_axis_window_ = AXIS_WINDOW;
            roll_axis_window_ = AXIS_WINDOW;
            piw::tsd_thing(this);
            memset(curmap_,0,sizeof(curmap_));
            memset(skpmap_,0,sizeof(skpmap_));
            set_latency(KBD_LATENCY);

            connect(c);

            for(unsigned k=0; k<KBD_KEYS; k++)
            {
                kwires_[k] = std::auto_ptr<kwire_t>(new kwire_t(k+1,piw::pathtwo(1,k+1,0),this));
            }

            strip1_ = std::auto_ptr<strip_t>(new strip_t(piw::pathone(2,0),this));
            strip2_ = std::auto_ptr<strip_t>(new strip_t(piw::pathone(4,0),this));
            breath_ = std::auto_ptr<breath_t>(new breath_t(piw::pathone(5,0),this));
        }

        ~keyboard_t()
        {
            tracked_invalidate();
        }

        void kbd_dead()
        {
            trigger_slow();
        }

        void insufficient_bandwidth()
        {
            pic::logmsg() << "insufficient USB bandwidth";
            piw::tsd_alert(BWMSG_KLASS,BWMSG_TITLE,BWMSG_MSG);
        }

        void shutdown()
        {
            root_ctl_t::disconnect();
            dead_();
        }

        void thing_trigger_slow()
        {
            shutdown();
        }

        void kbd_enqueue(unsigned long long t, unsigned key, unsigned a, unsigned p, int r, int y)
        {
            blob_t blob(key,t,a,p,r,y);

            switch(key)
            {
                case KBD_STRIP1:  strip1_->update(&blob); break;
                case KBD_STRIP2:  strip2_->update(&blob); break;
                case KBD_BREATH1: breath_->update(&blob); break;

                default:
                    if(key < KBD_KEYS)
                    {
                        kwires_[key]->update(&blob);
                    }
                    break;
            }
        }

        void kbd_key(unsigned long long t, unsigned key, unsigned p, int r, int y)
        {
            unsigned w = alpha1::active_t::key2word(key);
            unsigned short m = alpha1::active_t::key2mask(key);

            if(!(skpmap_[w]&m))
            {
                curmap_[w]|=m;
                kbd_enqueue(t,key,1,p,r,y);
            }
        }

        void dump_keydown(const unsigned short *map)
        {
            if(memcmp(map,curmap_,18)==0)
                return;

            pic::msg_t m = pic::logmsg();
            for(unsigned k=0; k<KBD_KEYS; ++k)
            {
                m << (alpha1::active_t::keydown(k,map)?"X":"-");
            }
        }

        void kbd_keydown(unsigned long long t, const unsigned short *newmap)
        {
            //dump_keydown(newmap);

            for(unsigned w=0; w<9; w++)
            {
                skpmap_[w] &= newmap[w];

                if(curmap_[w] == newmap[w]) continue;

                unsigned keybase = alpha1::active_t::word2key(w);

                for(unsigned k=0; k<16; k++)
                {
                    unsigned short mask = alpha1::active_t::key2mask(k);

                    if( (curmap_[w]&mask) && !(newmap[w]&mask) )
                    {
                        kbd_enqueue(t,keybase+k,0,0,0,0);
                    }
                }

                curmap_[w] = newmap[w];
            }
        }

        void kbd_off(unsigned long long t)
        {
            for(unsigned w=0; w<9; w++)
            {
                unsigned keybase = alpha1::active_t::word2key(w);

                for(unsigned k=0; k<16; k++)
                {
                    unsigned short mask = alpha1::active_t::key2mask(k);

                    if(!(skpmap_[w]&mask))
                    {
                        if((curmap_[w]&mask))
                        {
                            kbd_enqueue(t,keybase+k,0,0,0,0);
                        }
                    }
                }

                skpmap_[w] = 0xffff;
                curmap_[w] = 0x0000;
            }
        }

        std::auto_ptr<kwire_t> kwires_[KBD_KEYS];
        std::auto_ptr<strip_t> strip1_;
        std::auto_ptr<strip_t> strip2_;
        std::auto_ptr<breath_t> breath_;
        unsigned short curmap_[9],skpmap_[9];
        pic::notify_t dead_;
        float threshold1_,threshold2_,yaw_axis_window_,roll_axis_window_;
    };

    kwire_t::kwire_t(unsigned i,const piw::data_t &path,keyboard_t *k):piw::event_data_source_real_t(path), index_(i),id_(piw::pathone_nb(i,0)),keyboard_(k),maxpressure_(0),counter_(0),gated_(0),gated_count_(0), running_(false), ts_(0)
    {
        if(getenv("PI_JIMKBD"))
        {
            if(i>=121 && i<=132)
            {
                id_=piw::pathone_nb(i-108,0);
            }
        }

        k->connect_wire(this,source());
    }

    cwire_t::cwire_t(const piw::data_t &path, keyboard_t *k): piw::event_data_source_real_t(path), id_(piw::pathnull_nb(0))
    {
        k->connect_wire(this,source());
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


    void kwire_t::update(const blob_t *b)
    {
        unsigned a = b->a;
        unsigned long long t = b->t;
        float p = float(b->p);
        float r = 2047.0-float(b->r);
        float y = 2047.0-float(b->y);

        if(!a)
        {
            if(running_)
            {
                source_end(t);
                running_=false;
            }

            ts_ = pic_microtime();
            counter_=0;
            return;
        }

        if(counter_==0)
        {
            counter_=1;
            unsigned long long ts = pic_microtime();

            if(ts_ && ts<ts_+DEBOUNCE_US)
            {
                pic::logmsg() << "debounce " << index_ << ' ' << ts-ts_ << " us";
                skipping_=true;
                return;
            }

            skipping_=false;

            output_ = piw::xevent_data_buffer_t(15,PIW_DATAQUEUE_SIZE_NORM);
            output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,0,t));
            output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,p/4096.0,t));
            output_.add_value(3,piw::makefloat_bounded_nb(1,-1,0,__clip(r/1024.0,keyboard_->roll_axis_window_),t));
            output_.add_value(4,piw::makefloat_bounded_nb(1,-1,0,__clip(y/1024.0,keyboard_->yaw_axis_window_),t));
            source_start(0,id_.restamp(t),output_);
            running_=true;

            maxpressure_=0;
            gated_=0;
            gated_count_=0;
            return;
        }

        if(skipping_)
        {
            return;
        }

        if(gated_>0 && gated_count_>0) { --gated_count_; output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,gated_,t)); }
        output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,p/4096.0,t));
        output_.add_value(3,piw::makefloat_bounded_nb(1,-1,0,__clip(r/1024.0,keyboard_->roll_axis_window_),t));
        output_.add_value(4,piw::makefloat_bounded_nb(1,-1,0,__clip(y/1024.0,keyboard_->yaw_axis_window_),t));

        maxpressure_=std::max(maxpressure_,(float)p);

        if(counter_<ESTIMATION_END)
        {
            counter_++;
            return;
        }

        if(counter_==ESTIMATION_END)
        {
            if(maxpressure_ > keyboard_->threshold2_)
            {
                output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,3,t));
                gated_=3;
                gated_count_=20;
            }
        }

        if(!gated_ && maxpressure_ > keyboard_->threshold1_)
        {
            output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,2,t));
            gated_=2;
            gated_count_=20;
        }

        counter_++;
    }
}


struct kbd::bundle_t::impl_t: piw::clockdomain_ctl_t, piw::clocksink_t, piw::thing_t, virtual pic::tracked_t, virtual public pic::lckobject_t
{
    impl_t(const char *n, const piw::cookie_t &c, const pic::notify_t &d): loop_(n,&keyboard_), keyboard_(c,d,loop_.get_name()), queue_(0,PIC_THREAD_PRIORITY_NORMAL), skipped_(false), leds_(KBD_KEYS)
    {
        piw::tsd_thing(this);
        set_source(piw::makestring("*",0));
        sink(this,"alphakbd");

        keyboard_.set_clock(this);

        loop_.start();

        queue_.run();
        tick_enable(false);
        timer_slow(TEMP_INTERVAL);
        timer_fast(POLL_INTERVAL);
    }

    void thing_timer_slow()
    {
        unsigned long long t = piw::tsd_time();
        pic::logmsg() << "instrument internal temperature: " << loop_.get_temperature() << " last tick: " << t-last_tick_time_ << " us" << " last poll: " << t-last_poll_time_ << " us";
    }

    void lightenq(const piw::data_nb_t &d)
    {
        queue_.add(__lightdeq,this,(void *)(d.give_copy()),0,0);
    }

    static void __set_led(void *self, unsigned key, unsigned color)
    {
        impl_t *i = (impl_t *)self;
        i->loop_.set_led(key,((color&1)<<2) + ((color&2)<<4));
    }

    static void __lightdeq(void *a, void *b, void *c, void *d)
    {
        impl_t *i = (impl_t *)a;
        piw::data_nb_t data = piw::data_nb_t::from_given((bct_data_t)b);
        i->leds_.update_leds(data, i, __set_led);
    }

    void thing_timer_fast()
    {
        unsigned long long t = piw::tsd_time();
        last_poll_time_=t;

        if(loop_.poll(t))
        {
            skipped_=true;
        }

        if(skipped_)
        {
            keyboard_.kbd_off(t);
            pic::logmsg() << "skipped data: keys off";
            skipped_=false;
        }
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        last_tick_time_=piw::tsd_time();

        if(loop_.poll(t))
        {
            skipped_=true;
        }

        if(skipped_)
        {
            keyboard_.kbd_off(t);
            pic::logmsg() << "skipped data: keys off";
            skipped_=false;
        }
    }

    ~impl_t()
    {
        tick_disable();
        cancel_timer_fast();
        cancel_timer_slow();
        queue_.quit();
        tracked_invalidate();
        //loop_.stop();
    }

    alpha1::active_t loop_;
    keyboard_t keyboard_;
    pic::safe_worker_t queue_;
    bool skipped_;
    unsigned long long last_tick_time_;
    unsigned long long last_poll_time_;
    piw::statusledconvertor_t leds_;
};

void ledsink_t::invoke(const piw::data_nb_t &d) const
{
    i_->lightenq(d);
}

kbd::bundle_t::bundle_t(const char *n, const piw::cookie_t &c, const pic::notify_t &d): _root(new impl_t(n,c,d))
{
}

kbd::bundle_t::~bundle_t()
{
    pic::logmsg() << "kbd bundle dtor";
    close();
}

void kbd::bundle_t::close()
{
    if(_root)
    {
        pic::logmsg() << "closing kbd " << _root->loop_.get_name();
        delete _root;
        _root=0;
    }
}

std::string kbd::bundle_t::name()
{
    PIC_ASSERT(_root);
    return _root->loop_.get_name();
}

void kbd::bundle_t::set_roll_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.roll_axis_window_=t;
}

float kbd::bundle_t::get_roll_axis_window()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.roll_axis_window_;
}

void kbd::bundle_t::set_yaw_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.yaw_axis_window_=t;
}

float kbd::bundle_t::get_yaw_axis_window()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.yaw_axis_window_;
}

void kbd::bundle_t::set_threshold1(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.threshold1_=t*4096.0;
}

float kbd::bundle_t::get_threshold1()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.threshold1_/4096.0;
}

void kbd::bundle_t::set_threshold2(float t)
{
    PIC_ASSERT(_root);
    _root->keyboard_.threshold2_=t*4096.0;
}

float kbd::bundle_t::get_threshold2()
{
    PIC_ASSERT(_root);
    return _root->keyboard_.threshold2_/4096.0;
}

piw::change_nb_t kbd::bundle_t::led_functor()
{
    PIC_ASSERT(_root);
    return piw::change_nb_t(pic::ref(new ledsink_t(_root))); 
}

int kbd::bundle_t::gc_traverse(void *v, void *a) const
{
    if(_root)
    {
        int r;
        if((r=_root->keyboard_.dead_.gc_traverse(v,a))!=0) return r;
        if((r=_root->keyboard_.gc_traverse(v,a))!=0) return r;
    }

    return 0;
}

int kbd::bundle_t::gc_clear()
{
    if(_root)
    {
        _root->keyboard_.dead_.gc_clear();
        _root->keyboard_.gc_clear();
    }

    return 0;
}
