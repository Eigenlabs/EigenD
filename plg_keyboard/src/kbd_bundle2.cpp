
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

#include "kbd_bundle2.h"
#include "kbd_converter.h"

#include <picross/pic_config.h>

#include <lib_alpha2/alpha2_active.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_clock.h>
#include <piw/piw_status.h>
#include <piw/piw_resampler.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_safeq.h>

#include <math.h>
#include <memory>
#include <string.h>
#include <cmath>
#include <stdio.h>

#ifndef PI_WINDOWS
#include <unistd.h>
#endif 

#include <fcntl.h>

#define ESTIMATION_END 10
#define KBD_LATENCY 5000
#define KEY_SCALE 100.0

#define THRESHOLD_GATE 100
#define THRESHOLD_HARD 800
#define AXIS_WINDOW 1.0f
#define PEDAL_HYSTERESIS 60

#define TEMP_INTERVAL 30000

#define DEBOUNCE_US 20000

#define TEST_STOPPED 0
#define TEST_ARMED 1
#define TEST_STARTING 2
#define TEST_STARTED 3
#define TEST_STOPPING 4

#define MIC_OFF 0
#define MIC_ON 1
#define MIC_FADEUP 2
#define MIC_FADEDOWN 3

#define AUDIO_BLOCKSIZE 512

#define PI 3.14159f

#define BWMSG_KLASS "Insufficient USB Bandwidth"
#define BWMSG_TITLE "Insufficient USB Bandwidth"
#define BWMSG_MSG   "Your USB bus doesn't have enough USB bandwidth available " \
                    "for your Eigenharp to be able to function correctly.\n\n" \
                    "Your instrument or pedals might not be working until you " \
                    "plug your Eigenharp into another USB bus or remove other devices."

#define DRVMSG_KLASS "Out of Date USB Driver"
#define DRVMSG_TITLE "Out of Date USB Driver"
#define DRVMSG_MSG   "Your USB Driver is out of date.\n\n" \
                     "Please download and install the latest USB driver.\n\n" \
                     "Drivers can be downloaded from\n\n" \
                     "http://www.eigenlabs.com"
namespace
{
    struct ledsink_t: pic::sink_t<void(const piw::data_nb_t &)>
    {
        ledsink_t(kbd::kbd_impl_t *i): i_(i) {}
        void invoke(const piw::data_nb_t &p1) const;
        bool iscallable() const { return i_.isvalid(); }
        bool compare(const pic::sink_t<void(const piw::data_nb_t &)> *s) const
        {
            const ledsink_t *c = dynamic_cast<const ledsink_t *>(s);
            if(c && c->i_==i_) return true;
            return false;
        }

        pic::weak_t<kbd::kbd_impl_t> i_;
    };

    struct kwire_t;
    struct pedal_t;
    struct test_wire_t;

    struct keyboard_t: piw::root_ctl_t, piw::thing_t, alpha2::active_t::delegate_t, virtual pic::lckobject_t
    {
        
        keyboard_t( const piw::cookie_t &c,const pic::notify_t &d,const unsigned num_keys);
        virtual ~keyboard_t();   
        void kbd_dead(unsigned reason);
        void shutdown();
        void thing_trigger_slow();
        void kbd_key( unsigned long long t, unsigned key, unsigned p, int r, int y );
        void kbd_keydown(unsigned long long t, const unsigned short *newmap);
        void kbd_off( unsigned long long t ); 
        void kbd_mic(unsigned char s,unsigned long long t, const float *d);
        void pedal_down( unsigned long long t,unsigned pedal, unsigned p );
        unsigned kbd_num_keys() const;
        void midi_data( unsigned long long t,const unsigned char *data, unsigned len);
        void set_midi_sink(void (*cb)(void *,const unsigned char *,unsigned),void *ctx);
        piw::data_nb_t mic_data(unsigned long long t, unsigned bs);
        void set_mic_quality(unsigned q);
        void mic_reset();

        virtual const unsigned int get_coursecount() = 0;
        virtual const unsigned int* get_courses_array() = 0;
        const piw::data_nb_t get_courses_tuple();

        void create_kwires();

        virtual void kbd_enqueue(unsigned long long t, unsigned key, unsigned a, unsigned p, int r, int y){};
        virtual void dump_keydown(const unsigned short *map){};

        void learn_pedal_min(unsigned pedal);
        void learn_pedal_max(unsigned pedal);
        unsigned get_pedal_min(unsigned pedal);
        unsigned get_pedal_max(unsigned pedal);
        void set_pedal_min(unsigned pedal, unsigned value);
        void set_pedal_max(unsigned pedal, unsigned value);
        
        std::auto_ptr<kwire_t>* kwires_;
        std::auto_ptr<pedal_t> pedal1_;
        std::auto_ptr<pedal_t> pedal2_;
        std::auto_ptr<pedal_t> pedal3_;
        std::auto_ptr<pedal_t> pedal4_;
        std::auto_ptr<test_wire_t> test_wire_;
        unsigned short curmap_[9],skpmap_[9];
        pic::notify_t dead_;
        float threshold1_,threshold2_,yaw_axis_window_,roll_axis_window_;
        bool active_;
        const unsigned num_keys_;
        void (*midi_sink_)(void *,const unsigned char *,unsigned);
        void *midi_sink_ctx_;
        piw::resampler_t resampler_;
        piw::dataholder_nb_t courses_;
    };

    struct blob_t
    {
        blob_t(unsigned k_, unsigned long long t_, unsigned a_, unsigned p_,int r_, int y_): k(k_),a(a_),p(p_),t(t_),r(r_),y(y_) {}
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
        breath_t(const piw::data_t &d,keyboard_t *k): cwire_t(d,k), active_(false),ramp_(0.f) {}
        void update(const blob_t *b);

        bool active_;
        float ramp_;
    };

    struct pedal_t : cwire_t
    {
        pedal_t( const piw::data_t &d,keyboard_t *k ) : cwire_t(d,k), active_(false),last_(0), min_(0), max_(4095), init_(false), first_(-1) {}
        void update( unsigned p, unsigned long long t);

        void learn_min() { min_=last_; pic::logmsg() << "pedal min set to " << min_; }
        void learn_max() { max_=last_; pic::logmsg() << "pedal max set to " << max_; }
        void set_min(unsigned v) { min_=v; pic::logmsg() << "pedal min set to " << min_; }
        void set_max(unsigned v) { max_=v; pic::logmsg() << "pedal max set to " << max_; }
        unsigned get_min() { return min_; }
        unsigned get_max() { return max_; }

        bool active_;            
        unsigned last_,min_,max_;
        bool init_;
        long first_;
    };

    struct strip_t: cwire_t
    {
        strip_t(const piw::data_t &d,keyboard_t *k): cwire_t(d,k),active_(false),count_(20) {}
        void update(const blob_t *b);

        bool active_;
        unsigned count_;
        unsigned short origin_;
    };

    struct test_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        test_wire_t(const piw::data_t &path, piw::root_ctl_t *r) : piw::event_data_source_real_t(path), duration_(0), start_(0), end_(0), state_(TEST_STOPPED)
        {
            r->connect_wire(this,source());
        }

        ~test_wire_t() { source_shutdown(); }

        void ticked(unsigned long long f, unsigned long long t)
        {
            if(state_==TEST_STARTING && start_<=t)
            {
                if(start_<(f+1))
                {
                    pic::logmsg() << "missed start " << start_ << ", advancing to " << (f+1);
                    start_=f+1;
                }
                output_ = piw::xevent_data_buffer_t(8,PIW_DATAQUEUE_SIZE_TINY);
                output_.add_value(4,piw::makefloat_bounded_nb(1,0,0,1,start_));
                source_start(0,piw::pathnull_nb(start_),output_);
                state_=TEST_STARTED;
                pic::logmsg() << "test started in tick " << f << "-" << t;
                return;
            }

            if(state_==TEST_STARTED && end_<=t)
            {
                if(end_<(f+1))
                {
                    pic::logmsg() << "missed end " << end_ << ", advancing to " << (f+1);
                    end_=f+1;
                }
                output_.add_value(4,piw::makefloat_bounded_nb(1,0,0,0,end_));
                state_=TEST_STOPPING;
                pic::logmsg() << "test stopping in tick " << f << "-" << t;
                return;
            }

            if(state_==TEST_STOPPING)
            {
                source_end(t);
                state_=TEST_STOPPED;
                pic::logmsg() << "test stopped at " << t;
                return;
            }
        }

        void source_ended(unsigned seq) {}

        void arm(unsigned ms)
        {
            if(state_==TEST_STOPPED)
            {
                duration_ = 1000*ms;
                state_ = TEST_ARMED;
                pic::logmsg() << "test armed, duration " << ms;
            }
        }

        void ping(unsigned long long t)
        {
            if(state_==TEST_ARMED)
            {
                start_ = t;
                end_ = t+duration_;
                state_ = TEST_STARTING;
                pic::logmsg() << "test starting at " << t;
            }
        }

        unsigned long long duration_;
        unsigned long long start_;
        unsigned long long end_;
        unsigned state_;
        piw::xevent_data_buffer_t output_;
    };

    struct kwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        kwire_t(unsigned,unsigned,unsigned,const piw::data_t &,keyboard_t *);
        ~kwire_t() { source_shutdown(); keyboard_ = 0; }

        void source_ended(unsigned seq) {}

        void update(const blob_t *b);

        unsigned index_,row_,column_;
        piw::data_nb_t id_;
        keyboard_t *keyboard_;
        float maxpressure_;
        unsigned counter_;
        unsigned gated_;
        unsigned gated_count_;
        bool running_;
        bool skipping_;
        unsigned long long ts_;
        piw::xevent_data_buffer_t output_;
        float cur_pressure_, cur_roll_, cur_yaw_;
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

        if(!active_)
            return;

        if(--count_ != 0)
        {
            return;
        }

        count_ = 20;

        int dx = origin_-b->p;
        float f = (float)dx/4096.0;
        output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,f,b->t));
        output_.add_value(2,piw::makefloat_bounded_nb(1,-1,0,((float)b->p/2048.0)-1.0,b->t));
    }

    void breath_t::update(const blob_t *b)
    {
        if(b->a != active_)
        {
            active_ = b->a;
            if(active_)
            {
                output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_NORM);
                ramp_ = 0.f;
                source_start(0,id_.restamp(b->t),output_);
            }
            else
            {
                output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,b->t));
                source_end(b->t);
                return;
            }
        }

        if(!active_)
            return;

        if(ramp_<1.f)
        {
            ramp_ += 0.05f;
        }
        else
        {
            ramp_ = 1.f;
        }

        int b0 = b->p-2048;
        float bf = ramp_*float(b0)/2048.0;
        output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,bf,b->t));
    }

    void pedal_t::update( unsigned p, unsigned long long t)
    {
        if(!init_)
        {
            if(first_<0)
            {
                pic::logmsg() << "first pedal " << p;
                first_=p;
                return;
            }

            if(abs(((long)p)-first_)>10)
            {
                pic::logmsg() << "pedal diff " << p << " from " << first_;
                init_=true;
            }
        }

        if(abs((long)p-(long)last_)<2)
        {
            return;
        }

        bool active = false;
        unsigned min = min_;
        if(min_>max_)
        {
            min = (4*min)/5;
        }

        if(min<max_)
        {
            if(p>min+PEDAL_HYSTERESIS) active = true;
        }
        else if(min>max_)
        {
            if(p<min-PEDAL_HYSTERESIS) active = true;
        }

        last_ = p;

        if( active != active_)
        {
            active_ = active;

            if(active_)
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

        if(!active_)
        {
            return;
        }

        float bf;

        if(min < max_)
        {
            bf = p-min;
            bf = bf/float(max_-min);
        }
        else
        {
            bf = min-p;
            bf = bf/float(min-max_);
        }

        output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,bf,t));
    }

    keyboard_t::keyboard_t( const piw::cookie_t &c,const pic::notify_t &d,const unsigned num_keys): dead_(d), active_(true), num_keys_(num_keys), resampler_("mic",1,0)
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
       
        kwires_ = new std::auto_ptr<kwire_t>[num_keys];
        pedal1_ = std::auto_ptr<pedal_t>( new pedal_t(piw::pathone(6,0),this));    
        pedal2_ = std::auto_ptr<pedal_t>( new pedal_t(piw::pathone(7,0),this));    
        pedal3_ = std::auto_ptr<pedal_t>( new pedal_t(piw::pathone(8,0),this));    
        pedal4_ = std::auto_ptr<pedal_t>( new pedal_t(piw::pathone(9,0),this));    
        test_wire_ = std::auto_ptr<test_wire_t>(new test_wire_t(piw::pathone(3,0),this));
    }

    keyboard_t::~keyboard_t()
    {
        delete [] kwires_;
        tracked_invalidate();
    }

    void keyboard_t::kbd_dead(unsigned reason)
    {
        active_ = false;

        if(reason==PIPE_BAD_DRIVER)
        {
            pic::logmsg() << "bad usb driver";
            piw::tsd_alert(DRVMSG_KLASS,DRVMSG_TITLE,DRVMSG_MSG);
        }

        if(reason==PIPE_NO_BANDWIDTH)
        {
            pic::logmsg() << "insufficient USB bandwidth";
            piw::tsd_alert(BWMSG_KLASS,BWMSG_TITLE,BWMSG_MSG);
        }

        trigger_slow();
    }

    void keyboard_t::set_midi_sink(void (*cb)(void *,const unsigned char *,unsigned),void *ctx)
    {
        midi_sink_ = cb;
        midi_sink_ctx_ = ctx;
    }

    void keyboard_t::shutdown()
    {
        pic::logmsg() << "shutting down";
        root_ctl_t::disconnect();
        dead_();
        pic::logmsg() << "finished shutting down";
    }

    void keyboard_t::thing_trigger_slow()
    {
        pic::logmsg() << "kbd bundle shutdown";
        shutdown();
    }

     void keyboard_t::kbd_key(unsigned long long t, unsigned key, unsigned p, int r, int y)
     {
         unsigned w = alpha2::active_t::key2word(key);
         unsigned short m = alpha2::active_t::key2mask(key);

         if(!(skpmap_[w]&m))
         {
             curmap_[w]|=m;
             kbd_enqueue(t,key,1,p,r,y);
             test_wire_->ping(t);
         }
      }
    
     void keyboard_t::kbd_keydown(unsigned long long t, const unsigned short *newmap)
     {
        //dump_keydown(newmap);

        for(unsigned w=0; w<9; w++)
        {
            skpmap_[w] &= newmap[w];

            if(curmap_[w] == newmap[w]) continue;

            unsigned keybase = alpha2::active_t::word2key(w);

            for(unsigned k=0; k<16; k++)
            {
                unsigned short mask = alpha2::active_t::key2mask(k);

                if( (curmap_[w]&mask) && !(newmap[w]&mask) )
                {
                    kbd_enqueue(t,keybase+k,0,0,0,0);
                }
            }

            curmap_[w] = newmap[w];
          }    
      }

    void keyboard_t::set_mic_quality(unsigned q)
    {
        resampler_.set_quality(q);
    }

    void keyboard_t::mic_reset()
    {
        resampler_.reset();
    }

    const piw::data_nb_t keyboard_t::get_courses_tuple()
    {
        if(!courses_.is_empty())
        {
            return courses_.get();
        }

        piw::data_nb_t courses = piw::tuplenull_nb(0);
        for(unsigned i = 0; i < get_coursecount(); ++i)
        {
            courses = piw::tupleadd_nb(courses, piw::makelong_nb(get_courses_array()[i],0));
        }

        courses_.set_nb(courses);

        return courses_.get();
    }

    piw::data_nb_t keyboard_t::mic_data(unsigned long long t, unsigned bs)
    {
        float *outbuffer, *fs;
        piw::data_nb_t outdata = piw::makenorm_nb(t,bs,&outbuffer,&fs);
        memset(outbuffer,0,bs*sizeof(float)); *fs=0;
        resampler_.resampler_read_interleaved(bs,outbuffer);
        return outdata;
    }

    void keyboard_t::kbd_mic(unsigned char s,unsigned long long t, const float *d)
    {
        resampler_.resampler_write_interleaved(d,16);
    }

    void keyboard_t::kbd_off(unsigned long long t)
    {
        for(unsigned w=0; w<9; w++)
        {
            unsigned keybase = alpha2::active_t::word2key(w);

            for(unsigned k=0; k<16; k++)
            {
                unsigned short mask = alpha2::active_t::key2mask(k);

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

    void keyboard_t::midi_data(unsigned long long t, const unsigned char *data, unsigned len)
    {
#if 0
        pic::msg_t msg;
        msg << "MIDI in: len=" << len;

        for(unsigned i=0;i<len;i++)
        {
            msg << ' ' << std::hex << (unsigned)(data[i]);
        }

        pic::log(msg);
#endif

        if(midi_sink_)
        {
            midi_sink_(midi_sink_ctx_,data,len);
        }
    }

    void keyboard_t::pedal_down(unsigned long long t, unsigned pedal, unsigned p)
    {
        switch(pedal)
        {
            case 1: pedal1_->update(p,t); break;
            case 2: pedal2_->update(p,t); break;
            case 3: pedal3_->update(p,t); break;
            case 4: pedal4_->update(p,t); break;
        }
    }

    unsigned keyboard_t::kbd_num_keys() const
    {
        return num_keys_;
    }

    void keyboard_t::learn_pedal_min(unsigned pedal)
    {
        switch(pedal)
        {
            case 1: pedal1_->learn_min(); break;
            case 2: pedal2_->learn_min(); break;
            case 3: pedal3_->learn_min(); break;
            case 4: pedal4_->learn_min(); break;
        }
    }

    void keyboard_t::learn_pedal_max(unsigned pedal)
    {
        switch(pedal)
        {
            case 1: pedal1_->learn_max(); break;
            case 2: pedal2_->learn_max(); break;
            case 3: pedal3_->learn_max(); break;
            case 4: pedal4_->learn_max(); break;
        }
    }

    unsigned keyboard_t::get_pedal_min(unsigned pedal)
    {
        switch(pedal)
        {
            case 1: return pedal1_->get_min();
            case 2: return pedal2_->get_min();
            case 3: return pedal3_->get_min();
            case 4: return pedal4_->get_min();
        }

        return 0;
    }

    unsigned keyboard_t::get_pedal_max(unsigned pedal)
    {
        switch(pedal)
        {
            case 1: return pedal1_->get_max();
            case 2: return pedal2_->get_max();
            case 3: return pedal3_->get_max();
            case 4: return pedal4_->get_max();
        }

        return 4095;
    }

    void keyboard_t::set_pedal_min(unsigned pedal, unsigned value)
    {
        switch(pedal)
        {
            case 1: pedal1_->set_min(value); break;
            case 2: pedal2_->set_min(value); break;
            case 3: pedal3_->set_min(value); break;
            case 4: pedal4_->set_min(value); break;
        }
    }

    void keyboard_t::set_pedal_max(unsigned pedal, unsigned value)
    {
        switch(pedal)
        {
            case 1: pedal1_->set_max(value); break;
            case 2: pedal2_->set_max(value); break;
            case 3: pedal3_->set_max(value); break;
            case 4: pedal4_->set_max(value); break;
        }
    }

    void keyboard_t::create_kwires()
    {
        unsigned previous_coursekeys = 0;
        unsigned row = 1;
        unsigned col = 1;
        
        unsigned coursecount = get_coursecount();
        for(unsigned k=1; k <= kbd_num_keys() && row <= coursecount; k++)
        {
            unsigned coursekeys = get_courses_array()[row-1];
            if((k-previous_coursekeys) <= coursekeys)
            {
                col = k-previous_coursekeys; 
            }
            else
            {
                row++;
                previous_coursekeys += coursekeys;
                col = k-previous_coursekeys; 
            }
            kwires_[k-1] = std::auto_ptr<kwire_t>(new kwire_t(k,row,col,piw::pathtwo(1,k,0),this));
        }
    }

    static const unsigned int ALPHA_COURSECOUNT = 6;
    static const unsigned int ALPHA_COURSES[ALPHA_COURSECOUNT] = {24,24,24,24,24,12};

    struct alpha_kbd: keyboard_t
    {
        alpha_kbd( const piw::cookie_t &c,const pic::notify_t &d): keyboard_t( c, d, KBD_KEYS )
        {
            create_kwires();

            strip1_ = std::auto_ptr<strip_t>(new strip_t(piw::pathone(2,0),this));
            strip2_ = std::auto_ptr<strip_t>(new strip_t(piw::pathone(4,0),this));
            breath_ = std::auto_ptr<breath_t>(new breath_t(piw::pathone(5,0),this));
        }   

        ~alpha_kbd()
        {
        
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
                    if(key < kbd_num_keys())
                    {
                        kwires_[key]->update(&blob);
                    }
                    break;
            }
        }

        void dump_keydown(const unsigned short *map)
        {
            if(memcmp(map,curmap_,18)==0)
                return;

            pic::msg_t m = pic::logmsg();
            for(unsigned k=0; k<kbd_num_keys(); ++k)
            {
                m << (alpha2::active_t::keydown(k,map)?"X":"-");
            }
        }

        virtual const unsigned int get_coursecount()
        {
            return ALPHA_COURSECOUNT;
        }

        virtual const unsigned int* get_courses_array()
        {
            return ALPHA_COURSES;
        }

        std::auto_ptr<strip_t> strip1_;
        std::auto_ptr<strip_t> strip2_;
        std::auto_ptr<breath_t> breath_;
    };
    
    static const unsigned int TAU_COURSECOUNT = 7;
    static const unsigned int TAU_COURSES[TAU_COURSECOUNT] = {16,16,20,20,12,4,4};
        
    struct tau_kbd: keyboard_t
    {
        tau_kbd(const piw::cookie_t &c,const pic::notify_t &d): keyboard_t( c, d, TAU_KBD_KEYS + TAU_MODE_KEYS )
        {
            create_kwires();

            strip1_ = std::auto_ptr<strip_t>(new strip_t(piw::pathone(2,0),this));
            breath1_ = std::auto_ptr<breath_t>(new breath_t(piw::pathone(5,0),this));
        }   

        ~tau_kbd()
        {
        
        }

        void kbd_enqueue(unsigned long long t, unsigned key, unsigned a, unsigned p, int r, int y)
        {
            blob_t blob(key,t,a,p,r,y);

            switch(key)
            {
                case TAU_KBD_STRIP1:  strip1_->update(&blob); break;
                case TAU_KBD_BREATH1: breath1_->update(&blob); break;
    
                default:
                    if(key<TAU_KBD_KEYS || key>=(TAU_KBD_KEYS+5))
                    {
                        if(key >= (TAU_KBD_KEYS+5))
                        {
                            blob.p *= 4095;
                            key -= 5;
                        }
                        kwires_[key]->update(&blob);
                    }
                    break;
            }
        }

        void dump_keydown(const unsigned short *map)
        {
            if(memcmp(map,curmap_,18)==0)
                return;

            pic::msg_t msg = pic::logmsg();
            for(unsigned k=0; k<TAU_KBD_KEYS; ++k)
                msg << (alpha2::active_t::keydown(k,map)?"X":"-");
            for(unsigned m=0; m<TAU_MODE_KEYS; ++m)
                msg << (alpha2::active_t::keydown(TAU_KBD_KEYS+5+m,map)?"X":"-");
        }

        virtual const unsigned int get_coursecount()
        {
            return TAU_COURSECOUNT;
        }

        virtual const unsigned int* get_courses_array()
        {
            return TAU_COURSES;
        }

        std::auto_ptr<strip_t> strip1_;
        std::auto_ptr<breath_t> breath1_;
    };

    kwire_t::kwire_t(unsigned i, unsigned r, unsigned c, const piw::data_t &path, keyboard_t *k): piw::event_data_source_real_t(path), index_(i), row_(r), column_(c), id_(piw::pathone_nb(i,0)), keyboard_(k), maxpressure_(0), counter_(0), gated_(0),gated_count_(0), running_(false), ts_(0), output_(63,PIW_DATAQUEUE_SIZE_NORM),cur_pressure_(0), cur_roll_(0), cur_yaw_(0)
    {
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
                output_ = piw::xevent_data_buffer_t(31,PIW_DATAQUEUE_SIZE_NORM);
                output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,0,t));
                output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,0,t));
                output_.add_value(3,piw::makefloat_bounded_nb(1,-1,0,0,t));
                output_.add_value(4,piw::makefloat_bounded_nb(1,-1,0,0,t));
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

            source_start(0,id_.restamp(t),output_);
            piw::data_nb_t position = piw::tuplenull_nb(t);
            position = piw::tupleadd_nb(position, piw::makefloat_nb(row_,t));
            position = piw::tupleadd_nb(position, piw::makefloat_nb(column_,t));
            piw::data_nb_t key = piw::tuplenull_nb(t);
            key = piw::tupleadd_nb(key, piw::makelong_nb(index_,t));
            key = piw::tupleadd_nb(key, position);
            key = piw::tupleadd_nb(key, piw::makelong_nb(index_,t));
            key = piw::tupleadd_nb(key, position);
            output_.add_value(5, key);
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
        if(p!=cur_pressure_) output_.add_value(2,piw::makefloat_bounded_nb(1,0,0,p/4096.0,t));
        if(r!=cur_roll_) output_.add_value(3,piw::makefloat_bounded_nb(1,-1,0,__clip(r/1024.0,keyboard_->roll_axis_window_),t));
        if(y!=cur_yaw_) output_.add_value(4,piw::makefloat_bounded_nb(1,-1,0,__clip(y/1024.0,keyboard_->yaw_axis_window_),t));
        cur_pressure_=p; cur_roll_=r; cur_yaw_=y;

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
                gated_=3;
                gated_count_=20;
                output_.add_value(1,piw::makefloat_bounded_nb(3,0,0,3,t));
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

    struct mic_output_t: piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, piw::clockdomain_ctl_t, piw::clocksink_t, piw::thing_t
    {
        mic_output_t(const piw::cookie_t &c, keyboard_t *kbd, alpha2::active_t *l): event_data_source_real_t(piw::pathnull(0)), kbd_(kbd), gain_(0.f), state_(MIC_OFF), changed_(0), mic_enabled_(false), mic_pad_(false), mic_type_(0), restart_(false), loop_(l)
        {
            sink(this,"mic output");
            set_clock(this);
            connect(c);
            connect_wire(this,source());
            buffer_.set_signal(1,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_ISO));
            start_slow(piw::pathnull(piw::tsd_time()),buffer_);
            tick_enable(false);
            piw::tsd_thing(this);
            enable(false,true);
            pad(false);
            type(1);
            gain(21);
        }

        void invalidate()
        {
            tick_disable();
            end_slow(piw::tsd_time());
        }

        void clocksink_ticked(unsigned long long f, unsigned long long t)
        {
            if(state_==MIC_OFF)
            {
                return;
            }

            unsigned bs = piw::clocksink_t::get_buffer_size();
            unsigned long sr = piw::clocksink_t::get_sample_rate();

            if(ibs_!=bs || isr_!=sr)
            {
                ibs_ = bs;
                isr_ = sr;
                kbd_->mic_reset();
            }

            piw::data_nb_t d = kbd_->mic_data(t,bs);

            if(state_==MIC_ON)
            {
                buffer_.add_value(1,d);
            }
            else
            {
                float *outbuffer, *fs;
                piw::data_nb_t outdata = piw::makenorm_nb(t,bs,&outbuffer,&fs); *fs=0;
                memcpy(outbuffer,d.as_array(),bs*sizeof(float));
                for(unsigned i=0; i<bs; ++i)
                {
                    outbuffer[i] *= gain_;
                    if(state_==MIC_FADEUP)
                    {
                        gain_ += 0.0001f;
                        if(gain_>=1.f)
                        {
                            gain_ = 1.f;
                            pic::logmsg() << "mic is on";
                            state_ = MIC_ON;
                        }
                    }

                    if(state_==MIC_FADEDOWN)
                    {
                        gain_ -= 0.0001f;
                        if(gain_<=0.f)
                        {
                            gain_ = 0.f;
                            pic::logmsg() << "mic is off";
                            state_ = MIC_OFF;
                            loop_->mic_suppress(true);
                            trigger_slow();
                        }
                    }
                }
                buffer_.add_value(1,outdata);
            }
        }

        ~mic_output_t()
        {
            invalidate();
        }

        void enable(bool e,bool f)
        {
            if(mic_enabled_==e && !f)
                return;

            if(e)
            {
                // turn on and fade up
                pic::logmsg() << "fading up";
                mic_enabled_ = true;
                ibs_ = piw::clocksink_t::get_buffer_size();
                isr_ = piw::clocksink_t::get_sample_rate();
                kbd_->mic_reset();
                loop_->mic_enable(true);
                loop_->mic_suppress(false);
                state_ = MIC_FADEUP;
            }
            else
            {
                // fade down and turn off
                pic::logmsg() << "fading down";
                restart_ = false;
                state_ = MIC_FADEDOWN;
            }
        }

        void pad(bool p)
        {
            mic_pad_ = p;
            if(mic_enabled_)
            {
                changed_ |= 1;
                restart_ = true;
                pic::logmsg() << "pad set, fading down";
                state_ = MIC_FADEDOWN;
            }
            else
            {
                loop_->mic_pad(mic_pad_);
            }
        }

        void type(unsigned t)
        {
            mic_type_ = t;
            if(mic_enabled_)
            {
                changed_ |= 2;
                restart_ = false;
                state_ = MIC_FADEDOWN;
                disabled_();
            }
            else
            {
                loop_->mic_type(mic_type_);
            }
        }

        void gain(unsigned g)
        {
            loop_->mic_gain(g);
        }

        void thing_trigger_slow()
        {
            pic::logmsg() << " ----------- trigger changed= " << changed_ << " pad=" << mic_pad_ << " type=" << mic_type_ << " restart=" << restart_;

            if(changed_&1)
                loop_->mic_pad(mic_pad_);

            if(changed_&2)
                loop_->mic_type(mic_type_);

            changed_ = 0;

            if(restart_)
            {
                loop_->mic_suppress(false);
                state_ = MIC_FADEUP;
            }
            else
            {
                loop_->mic_enable(false);
                mic_enabled_ = false;
            }
        }

        piw::xevent_data_buffer_t buffer_;
        keyboard_t *kbd_;
        float gain_;
        unsigned state_;
        unsigned changed_;
        bool mic_enabled_;
        bool mic_pad_;
        unsigned mic_type_;
        bool restart_;
        alpha2::active_t *loop_;
        pic::notify_t disabled_;
        unsigned ibs_;
        unsigned long isr_;
    };


    struct headphone_input_t: piw::root_t, piw::wire_t, piw::event_data_sink_t, piw::clocksink_t, piw::thing_t, piw::clockdomain_t
    {
        headphone_input_t(alpha2::active_t *l): root_t(0), loop_(l), upstream_(0), test_tone_warmup_(0), converter_(), enabled_(false), quality_(0)
        {
            piw::tsd_thing(this);
            piw::tsd_clockdomain(this);

            sink(this,"kbd audio input");

            const char *tt = getenv("PI_KBD_TEST_FREQ");
            test_tone_ = (tt!=0);

            if(test_tone_)
            {
                test_tone_freq_ = atof(tt);
                const char *tg = getenv("PI_KBD_TEST_GAIN");
                test_tone_gain_ = (tg!=0)?atof(tg):1.f;
                pic::logmsg() << "test tone is enabled, freq=" << test_tone_freq_ << " gain=" << test_tone_gain_;
            }

            test_tone_phase_ = 0.f;
            test_tone_warmup_ = 0;
            tick_enable(true);

            try
            {
                loop_->headphone_enable(false);
                loop_->headphone_gain(127-70);
            }
            catch(pic::error &e)
            {
                pic::logmsg() << "could not initialise audio output: " << e.what();
            }
        }

        void headphone_enable(bool e)
        {
            if(e!=enabled_)
            {
                enabled_ = e;
                loop_->headphone_enable(e);

                if(e)
                {
                    ibs_ = get_buffer_size();
                    isr_ = get_sample_rate();
                    unsigned long period;

                    if((period=converter_.reset(isr_,ibs_,quality_))!=0)
                    {
                        timer_fast_us(period/1000,period%1000);
                        pic::logmsg() << "timer period now " << period;
                    }
                    else
                    {
                        cancel_timer_fast();
                    }

                    tick_suppress(false);
                }
                else
                {
                    cancel_timer_fast();
                    tick_suppress(true);
                }
            }
        }

        void headphone_gain(unsigned g)
        {
            loop_->headphone_gain(g);
        }

        void headphone_limit(bool l)
        {
            loop_->headphone_limit(l);
        }

        ~headphone_input_t()
        {
            invalidate();
            close_thing();
            close_domain();
        }

        void invalidate()
        {
            unsubscribe();
            tick_disable();
            cancel_timer_fast();
        }

        void clockdomain_source_changed()
        {
        }

        static void __writer(void *ctx, const float *interleaved, unsigned bs, unsigned period)
        {
            ((headphone_input_t *)ctx)->loop_->audio_write(interleaved,bs,period);
        }

        void thing_timer_fast()
        {
            if(enabled_)
            {
                converter_.read(__writer,this);
            }
        }

        void set_quality(unsigned q)
        {
            quality_ = q;

            if(enabled_)
            {
                unsigned long period;

                if((period=converter_.reset(isr_,ibs_,quality_))!=0)
                {
                    timer_fast_us(period/1000,period%1000);
                    pic::logmsg() << "timer period now " << period;
                }
                else
                {
                    cancel_timer_fast();
                }
            }
        }

        void event_start(unsigned seq, const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b)
        {
            iterator_ = b.iterator();
        }

        bool event_end(unsigned long long)
        {
            iterator_.clear();
            return true;
        }

        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
        {
            iterator_->set_signal(sig,n);
            iterator_->reset(sig,t);
        }

        void wire_closed()
        {
            unsubscribe();
        }

        piw::wire_t *root_wire(const piw::event_data_source_t &es)
        {
            subscribe_and_ping(es);
            return this;
        }

        void root_closed()
        {
        }

        void root_clock()
        {
            if(upstream_)
            {
                remove_upstream(upstream_);
            }

            upstream_ = get_clock();

            if(upstream_)
            {
                add_upstream(upstream_);
            }
        }

        void root_opened()
        {
            root_clock();
            root_latency();
        }

        void root_latency()
        {
        }

        void clocksink_ticked(unsigned long long f, unsigned long long t)
        {
            float out[PLG_CLOCK_BUFFER_SIZE*2];
            unsigned bs = get_buffer_size();
            unsigned long sr = get_sample_rate();

            if(ibs_!=bs || isr_!=sr)
            {
                ibs_ = bs;
                isr_ = sr;
                unsigned long period;

                if((period=converter_.reset(isr_,ibs_,quality_))!=0)
                {
                    timer_fast_us(period/1000,period%1000);
                    pic::logmsg() << "timer period now " << period;
                }
                else
                {
                    cancel_timer_fast();
                }
            }

            memset(out,0,2*PLG_CLOCK_BUFFER_SIZE*sizeof(float));

            if(test_tone_)
            {
                if(test_tone_warmup_<100)
                {
                    ++test_tone_warmup_;
                    return;
                }

                for(unsigned i=0; i<bs; i++)
                {
                    //out[2*i] = test_tone_gain_*std::sin(test_tone_phase_*2.f*PI);
                    //out[2*i+1] = -out[2*i];
                    out[i] = test_tone_gain_*std::sin(test_tone_phase_*2.f*PI);

                    test_tone_phase_ += (test_tone_freq_/sr);

                    if(test_tone_phase_>1.f)
                    {
                        test_tone_phase_ -= 1.f;
                    }
                }
            }

            if(iterator_.isvalid())
            {
                piw::data_nb_t left_out,right_out;

                while(iterator_->nextsig(1,left_out,t)) { }
                while(iterator_->nextsig(2,right_out,t)) { }

                if(left_out.is_array() && left_out.as_arraylen()==bs)
                {
                    const float *l = left_out.as_array();

                    for(unsigned i=0; i<bs; i++)
                    {
                        out[2*i+0] += l[i];
                    }
                }

                if(right_out.is_array() && right_out.as_arraylen()==bs)
                {
                    const float *r = right_out.as_array();

                    for(unsigned i=0; i<bs; i++)
                    {
                        out[2*i+1] += r[i];
                    }
                }
            }

            if(converter_.write(out,bs))
            {
                converter_.read(__writer,this);
            }
        }

        alpha2::active_t *loop_;
        bct_clocksink_t *upstream_;
        piw::xevent_data_buffer_t::iter_t iterator_;
        bool test_tone_;
        float test_tone_phase_;
        float test_tone_freq_;
        float test_tone_gain_;
        unsigned test_tone_warmup_;
        kbd::converter_t converter_;
        bool enabled_;
        unsigned ibs_;
        unsigned long isr_;
        unsigned quality_;
    };
}

struct kbd::kbd_impl_t: piw::clockdomain_ctl_t, piw::clocksink_t, piw::thing_t, pic::safe_worker_t, virtual pic::tracked_t, virtual public pic::lckobject_t
{
    kbd_impl_t(pic::usbdevice_t *device, const piw::cookie_t &ac, keyboard_t *pkeyboard): pic::safe_worker_t(10,PIC_THREAD_PRIORITY_NORMAL), device_(device), loop_(device,pkeyboard,false), pkeyboard_(pkeyboard), skipped_(false), mic_output_(new mic_output_t(ac, pkeyboard_,&loop_)), headphone_input_(new headphone_input_t(&loop_)), leds_(pkeyboard->get_coursecount(),pkeyboard->get_courses_array())
    {
        init();
    }

    kbd_impl_t(pic::usbdevice_t *device, keyboard_t *pkeyboard): pic::safe_worker_t(10,PIC_THREAD_PRIORITY_NORMAL), device_(device), loop_(device,pkeyboard,false), pkeyboard_(pkeyboard), skipped_(false), headphone_input_(new headphone_input_t(&loop_)), leds_(pkeyboard->get_coursecount(),pkeyboard->get_courses_array())
    {
        init();
    }

    kbd_impl_t(const char *name, keyboard_t *pkeyboard): pic::safe_worker_t(10,PIC_THREAD_PRIORITY_NORMAL), device_(new pic::usbdevice_t(name,0)), loop_(device_,pkeyboard,true), pkeyboard_(pkeyboard), skipped_(false), leds_(pkeyboard->get_coursecount(),pkeyboard->get_courses_array())
    {
        init();
    }

    void init()
    {
        piw::tsd_thing(this);
        set_source(piw::makestring("*",0));
        sink(this,"alphakbd");

        pkeyboard_->set_clock(this);

        loop_.start();

        run();
        tick_enable(false);
        timer_slow(TEMP_INTERVAL);
    }

    void set_mic_quality(unsigned q)
    {
        if(mic_output_.get())
        {
            pkeyboard_->set_mic_quality(q);
        }
    }

    void set_hp_quality(unsigned q)
    {
        if(headphone_input_.get())
        {
            headphone_input_->set_quality(q);
        }
    }

    piw::data_t get_courses()
    {
        return pkeyboard_->get_courses_tuple().make_normal();
    }

    piw::cookie_t audio_cookie()
    {
        return piw::cookie_t(headphone_input_.get());
    }

    void thing_timer_slow()
    {
        pic::logmsg() << "instrument internal temperature: " << loop_.get_temperature();
    }

    void lightenq(const piw::data_nb_t &d)
    {
        if(getenv("PI_NOLEDS")!=0)
            return;

        add(__lightdeq,this,(void *)(d.give_copy()),0,0);
    }

    static void __set_led(void *self, unsigned key, unsigned color)
    {
        kbd_impl_t *i = (kbd_impl_t *)self;
        i->loop_.msg_set_led(key,color);
    }

    static void __lightdeq(void *a, void *b, void *c, void *d)
    {
        kbd_impl_t *i = (kbd_impl_t *)a;
        piw::data_nb_t data = piw::data_nb_t::from_given((bct_data_t)b);
        i->leds_.update_leds(data, i, __set_led);
    }

    bool ping()
    {
        if(pkeyboard_->active_)
        {
            loop_.msg_flush();
        }

        return false;
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        if(loop_.poll(t))
        {
            skipped_=true;
        }

        if(skipped_)
        {
            pkeyboard_->kbd_off(t);
            pic::logmsg() << "skipped data: keys off";
            skipped_=false;
        }

        pkeyboard_->test_wire_->ticked(f,t);
    }

    ~kbd_impl_t()
    {
        mic_output_.reset();
        headphone_input_.reset();
        tick_disable();
        cancel_timer_slow();
        quit();
        tracked_invalidate();
        delete pkeyboard_;
        loop_.invalidate();
    }

    pic::usbdevice_t *device_;
    alpha2::active_t loop_;
    keyboard_t *pkeyboard_;
    bool skipped_;
    std::auto_ptr<mic_output_t> mic_output_;
    std::auto_ptr<headphone_input_t> headphone_input_;
    piw::statusledconvertor_t leds_;
};

void ledsink_t::invoke(const piw::data_nb_t &d) const
{
    i_->lightenq(d);
}

static int __armtest(void *i_, void *d_)
{
    kbd::kbd_impl_t *i = (kbd::kbd_impl_t *)i_;
    unsigned d = *(unsigned *)d_;
    i->pkeyboard_->test_wire_->arm(d);
    return 0;
}


// ---------------- alpha2 (legacy)

kbd::alpha2_bundle_legacy_t::alpha2_bundle_legacy_t(const char *n,const piw::cookie_t &kc, const pic::notify_t &d): _root(new kbd::kbd_impl_t(n,new alpha_kbd(kc,d)))
{
}

kbd::alpha2_bundle_legacy_t::~alpha2_bundle_legacy_t()
{
    close();
}

void kbd::alpha2_bundle_legacy_t::close()
{
    if(_root)
    {
        pic::logmsg() << "closing kbd " << _root->loop_.get_name();
        delete _root;
        delete _root->device_;
        _root=0;
    }
}

std::string kbd::alpha2_bundle_legacy_t::name()
{
    PIC_ASSERT(_root);
    return _root->loop_.get_name();
}

piw::data_t kbd::alpha2_bundle_legacy_t::get_courses()
{
    PIC_ASSERT(_root);
    return _root->get_courses();
}

void kbd::alpha2_bundle_legacy_t::set_roll_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->roll_axis_window_=t;
}

float kbd::alpha2_bundle_legacy_t::get_roll_axis_window()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->roll_axis_window_;
}

void kbd::alpha2_bundle_legacy_t::set_yaw_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->yaw_axis_window_=t;
}

float kbd::alpha2_bundle_legacy_t::get_yaw_axis_window()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->yaw_axis_window_;
}

void kbd::alpha2_bundle_legacy_t::set_threshold1(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->threshold1_=t*4096.0;
}

float kbd::alpha2_bundle_legacy_t::get_threshold1()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->threshold1_/4096.0;
}

void kbd::alpha2_bundle_legacy_t::set_threshold2(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->threshold2_=t*4096.0;
}

float kbd::alpha2_bundle_legacy_t::get_threshold2()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->threshold2_/4096.0;
}

piw::change_nb_t kbd::alpha2_bundle_legacy_t::led_functor()
{
    PIC_ASSERT(_root);

    return piw::change_nb_t(pic::ref(new ledsink_t(_root))); 
}

int kbd::alpha2_bundle_legacy_t::gc_traverse(void *v, void *a) const
{
    if(_root)
    {
        int r;
        if((r=_root->pkeyboard_->dead_.gc_traverse(v,a))!=0) return r;
        if((r=_root->pkeyboard_->gc_traverse(v,a))!=0) return r;
    }

    return 0;
}

int kbd::alpha2_bundle_legacy_t::gc_clear()
{
    if(_root)
    {
        _root->pkeyboard_->dead_.gc_clear();
        _root->pkeyboard_->gc_clear();
    }

    return 0;
}

void kbd::alpha2_bundle_legacy_t::testmsg_write_lib(unsigned l,unsigned p,unsigned r,unsigned y)
{
    _root->loop_.msg_test_write_lib(l,p,r,y);
}

void kbd::alpha2_bundle_legacy_t::testmsg_finish_lib()
{
    _root->loop_.msg_test_finish_lib();
    _root->loop_.msg_flush();
}

void kbd::alpha2_bundle_legacy_t::testmsg_write_seq(unsigned l,unsigned k,unsigned t)
{
    _root->loop_.msg_test_write_seq(l,k,t);
}

void kbd::alpha2_bundle_legacy_t::testmsg_finish_seq()
{
    _root->loop_.msg_test_finish_seq();
    _root->loop_.msg_flush();
}
void kbd::alpha2_bundle_legacy_t::start_test(unsigned duration)
{
    _root->loop_.msg_test_start();
    _root->loop_.msg_flush();
}

void kbd::alpha2_bundle_legacy_t::arm_recording(unsigned ms)
{
    piw::tsd_fastcall(__armtest,_root,&ms);
}

void kbd::alpha2_bundle_legacy_t::learn_pedal_min(unsigned pedal)
{
    _root->pkeyboard_->learn_pedal_min(pedal);
}

void kbd::alpha2_bundle_legacy_t::learn_pedal_max(unsigned pedal)
{
    _root->pkeyboard_->learn_pedal_max(pedal);
}

unsigned kbd::alpha2_bundle_legacy_t::get_pedal_min(unsigned pedal)
{
    return _root->pkeyboard_->get_pedal_min(pedal);
}

unsigned kbd::alpha2_bundle_legacy_t::get_pedal_max(unsigned pedal)
{
    return _root->pkeyboard_->get_pedal_max(pedal);
}

void kbd::alpha2_bundle_legacy_t::set_pedal_min(unsigned pedal, unsigned value)
{
    _root->pkeyboard_->set_pedal_min(pedal,value);
}

void kbd::alpha2_bundle_legacy_t::set_pedal_max(unsigned pedal, unsigned value)
{
    _root->pkeyboard_->set_pedal_max(pedal,value);
}

void kbd::alpha2_bundle_legacy_t::restart()
{
    _root->loop_.restart();
}


// ---------------- alpha2 (new)

kbd::alpha2_bundle_t::alpha2_bundle_t(pic::usbdevice_t *device,const piw::cookie_t &kc, const piw::cookie_t &ac, const pic::notify_t &d): _root(new kbd::kbd_impl_t(device,ac,new alpha_kbd(kc,d)))
{
}

kbd::alpha2_bundle_t::~alpha2_bundle_t()
{
    close();
}

void kbd::alpha2_bundle_t::close()
{
    if(_root)
    {
        pic::logmsg() << "closing kbd " << _root->loop_.get_name();
        delete _root;
        _root=0;
    }
}

std::string kbd::alpha2_bundle_t::name()
{
    PIC_ASSERT(_root);
    return _root->loop_.get_name();
}

piw::data_t kbd::alpha2_bundle_t::get_courses()
{
    PIC_ASSERT(_root);
    return _root->get_courses();
}

void kbd::alpha2_bundle_t::set_threshold1(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->threshold1_=t*4096.0;
}

float kbd::alpha2_bundle_t::get_threshold1()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->threshold1_/4096.0;
}

void kbd::alpha2_bundle_t::set_threshold2(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->threshold2_=t*4096.0;
}

float kbd::alpha2_bundle_t::get_threshold2()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->threshold2_/4096.0;
}

void kbd::alpha2_bundle_t::set_roll_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->roll_axis_window_=t;
}

float kbd::alpha2_bundle_t::get_roll_axis_window()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->roll_axis_window_;
}

void kbd::alpha2_bundle_t::set_yaw_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->yaw_axis_window_=t;
}

float kbd::alpha2_bundle_t::get_yaw_axis_window()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->yaw_axis_window_;
}

piw::change_nb_t kbd::alpha2_bundle_t::led_functor()
{
    PIC_ASSERT(_root);

    return piw::change_nb_t(pic::ref(new ledsink_t(_root))); 
}

int kbd::alpha2_bundle_t::gc_traverse(void *v, void *a) const
{
    if(_root)
    {
        int r;
        if((r=_root->pkeyboard_->dead_.gc_traverse(v,a))!=0) return r;
        if((r=_root->pkeyboard_->gc_traverse(v,a))!=0) return r;
    }

    return 0;
}

int kbd::alpha2_bundle_t::gc_clear()
{
    if(_root)
    {
        _root->pkeyboard_->dead_.gc_clear();
        _root->pkeyboard_->gc_clear();
    }

    return 0;
}

void kbd::alpha2_bundle_t::testmsg_write_lib(unsigned l,unsigned p,unsigned r,unsigned y)
{
    _root->loop_.msg_test_write_lib(l,p,r,y);
}

void kbd::alpha2_bundle_t::testmsg_finish_lib()
{
    _root->loop_.msg_test_finish_lib();
    _root->loop_.msg_flush();
}

void kbd::alpha2_bundle_t::testmsg_write_seq(unsigned l,unsigned k,unsigned t)
{
    _root->loop_.msg_test_write_seq(l,k,t);
}

void kbd::alpha2_bundle_t::testmsg_finish_seq()
{
    _root->loop_.msg_test_finish_seq();
    _root->loop_.msg_flush();
}
void kbd::alpha2_bundle_t::start_test(unsigned duration)
{
    _root->loop_.msg_test_start();
    _root->loop_.msg_flush();
}

void kbd::alpha2_bundle_t::arm_recording(unsigned ms)
{
    piw::tsd_fastcall(__armtest,_root,&ms);
}

void kbd::alpha2_bundle_t::learn_pedal_min(unsigned pedal)
{
    _root->pkeyboard_->learn_pedal_min(pedal);
}

void kbd::alpha2_bundle_t::learn_pedal_max(unsigned pedal)
{
    _root->pkeyboard_->learn_pedal_max(pedal);
}

unsigned kbd::alpha2_bundle_t::get_pedal_min(unsigned pedal)
{
    return _root->pkeyboard_->get_pedal_min(pedal);
}

unsigned kbd::alpha2_bundle_t::get_pedal_max(unsigned pedal)
{
    return _root->pkeyboard_->get_pedal_max(pedal);
}

void kbd::alpha2_bundle_t::set_pedal_min(unsigned pedal, unsigned value)
{
    _root->pkeyboard_->set_pedal_min(pedal,value);
}

void kbd::alpha2_bundle_t::set_pedal_max(unsigned pedal, unsigned value)
{
    _root->pkeyboard_->set_pedal_max(pedal,value);
}

piw::cookie_t kbd::alpha2_bundle_t::audio_cookie()
{
    return _root->audio_cookie();
}

void kbd::alpha2_bundle_t::set_hp_quality(unsigned q)
{
    _root->set_hp_quality(q);
}

void kbd::alpha2_bundle_t::headphone_enable(bool e)
{
    _root->headphone_input_->headphone_enable(e);
}

void kbd::alpha2_bundle_t::headphone_limit(bool l)
{
    _root->headphone_input_->headphone_limit(l);
}

void kbd::alpha2_bundle_t::headphone_gain(unsigned g)
{
    _root->headphone_input_->headphone_gain(g);
}

void kbd::alpha2_bundle_t::set_mic_quality(unsigned q)
{
    _root->set_mic_quality(q);
}

void kbd::alpha2_bundle_t::mic_enable(bool e)
{
    _root->mic_output_->enable(e,false);
}

void kbd::alpha2_bundle_t::mic_type(unsigned t)
{
    _root->mic_output_->type(t);
}

void kbd::alpha2_bundle_t::mic_gain(unsigned g)
{
    _root->mic_output_->gain(g);
}

void kbd::alpha2_bundle_t::mic_pad(bool p)
{
    _root->mic_output_->pad(p);
}

void kbd::alpha2_bundle_t::mic_disabled(const pic::notify_t &n)
{
    _root->mic_output_->disabled_ = n;
}

void kbd::alpha2_bundle_t::mic_automute(bool e)
{
    _root->loop_.mic_automute(e);
}

void kbd::alpha2_bundle_t::loopback_gain(float f)
{
    _root->loop_.loopback_gain(f);
}

void kbd::alpha2_bundle_t::loopback_enable(bool e)
{
    _root->loop_.loopback_enable(e);
}

void kbd::alpha2_bundle_t::restart()
{
    _root->loop_.restart();
}



// ---------------- tau

kbd::tau_bundle_t::tau_bundle_t(pic::usbdevice_t *device,const piw::cookie_t &kc,const pic::notify_t &d): _root(new kbd::kbd_impl_t(device,new tau_kbd(kc,d)))
{
    _root->loop_.set_tau_mode(true);
}

kbd::tau_bundle_t::~tau_bundle_t()
{
    close();
}

void kbd::tau_bundle_t::close()
{
    if(_root)
    {
        pic::logmsg() << "closing kbd " << _root->loop_.get_name();
        delete _root;
        _root=0;
    }
}

std::string kbd::tau_bundle_t::name()
{
    PIC_ASSERT(_root);
    return _root->loop_.get_name();
}

piw::data_t kbd::tau_bundle_t::get_courses()
{
    PIC_ASSERT(_root);
    return _root->get_courses();
}

void kbd::tau_bundle_t::set_threshold1(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->threshold1_=t*4096.0;
}

float kbd::tau_bundle_t::get_threshold1()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->threshold1_/4096.0;
}

void kbd::tau_bundle_t::set_threshold2(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->threshold2_=t*4096.0;
}

float kbd::tau_bundle_t::get_threshold2()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->threshold2_/4096.0;
}

void kbd::tau_bundle_t::set_roll_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->roll_axis_window_=t;
}

float kbd::tau_bundle_t::get_roll_axis_window()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->roll_axis_window_;
}

void kbd::tau_bundle_t::set_yaw_axis_window(float t)
{
    PIC_ASSERT(_root);
    _root->pkeyboard_->yaw_axis_window_=t;
}

float kbd::tau_bundle_t::get_yaw_axis_window()
{
    PIC_ASSERT(_root);
    return _root->pkeyboard_->yaw_axis_window_;
}

piw::change_nb_t kbd::tau_bundle_t::led_functor()
{
    PIC_ASSERT(_root);
    return piw::change_nb_t(pic::ref(new ledsink_t(_root))); 
}

int kbd::tau_bundle_t::gc_traverse(void *v, void *a) const
{
    if(_root)
    {
        int r;
        if((r=_root->pkeyboard_->dead_.gc_traverse(v,a))!=0) return r;
        if((r=_root->pkeyboard_->gc_traverse(v,a))!=0) return r;
    }

    return 0;
}

int kbd::tau_bundle_t::gc_clear()
{
    if(_root)
    {
        _root->pkeyboard_->dead_.gc_clear();
        _root->pkeyboard_->gc_clear();
    }

    return 0;
}

void kbd::tau_bundle_t::testmsg_write_lib(unsigned l,unsigned p,unsigned r,unsigned y)
{
    _root->loop_.msg_test_write_lib(l,p,r,y);
}

void kbd::tau_bundle_t::testmsg_finish_lib()
{
    _root->loop_.msg_test_finish_lib();
    _root->loop_.msg_flush();
}

void kbd::tau_bundle_t::testmsg_write_seq(unsigned l,unsigned k,unsigned t)
{
    _root->loop_.msg_test_write_seq(l,k,t);
}

void kbd::tau_bundle_t::testmsg_finish_seq()
{
    _root->loop_.msg_test_finish_seq();
    _root->loop_.msg_flush();
}
void kbd::tau_bundle_t::start_test(unsigned duration)
{
    _root->loop_.msg_test_start();
    _root->loop_.msg_flush();
}

void kbd::tau_bundle_t::arm_recording(unsigned ms)
{
    piw::tsd_fastcall(__armtest,_root,&ms);
}

void kbd::tau_bundle_t::learn_pedal_min(unsigned pedal)
{
    _root->pkeyboard_->learn_pedal_min(pedal);
}

void kbd::tau_bundle_t::learn_pedal_max(unsigned pedal)
{
    _root->pkeyboard_->learn_pedal_max(pedal);
}

unsigned kbd::tau_bundle_t::get_pedal_min(unsigned pedal)
{
    return _root->pkeyboard_->get_pedal_min(pedal);
}

unsigned kbd::tau_bundle_t::get_pedal_max(unsigned pedal)
{
    return _root->pkeyboard_->get_pedal_max(pedal);
}

void kbd::tau_bundle_t::set_pedal_min(unsigned pedal, unsigned value)
{
    _root->pkeyboard_->set_pedal_min(pedal,value);
}

void kbd::tau_bundle_t::set_pedal_max(unsigned pedal, unsigned value)
{
    _root->pkeyboard_->set_pedal_max(pedal,value);
}

piw::cookie_t kbd::tau_bundle_t::audio_cookie()
{
    return _root->audio_cookie();
}

void kbd::tau_bundle_t::set_hp_quality(unsigned q)
{
    _root->set_hp_quality(q);
}

void kbd::tau_bundle_t::headphone_enable(bool e)
{
    _root->headphone_input_->headphone_enable(e);
}

void kbd::tau_bundle_t::headphone_limit(bool l)
{
    _root->headphone_input_->headphone_limit(l);
}

void kbd::tau_bundle_t::headphone_gain(unsigned g)
{
    _root->headphone_input_->headphone_gain(g);
}

void kbd::tau_bundle_t::restart()
{
    _root->loop_.restart();
}
