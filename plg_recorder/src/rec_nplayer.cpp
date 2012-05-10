
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

#include "rec_nplayer.h"
#include <picross/pic_ilist.h>
#include <piw/piw_keys.h>

#define RAMP_SAMPLES 10
#define RAMP_INTERVAL 500
#define RAMP_INC 0.001f

#define ALL 0
#define INUSE 1

namespace
{
    struct kwire_t : piw::wire_ctl_t, piw::event_data_source_real_t, pic::element_t<ALL>, pic::element_t<INUSE>, virtual pic::lckobject_t
    {
        kwire_t(recorder::nplayer_t::impl_t *nplayer, unsigned n): piw::event_data_source_real_t(piw::pathone(n,0)), nplayer_(nplayer), name_(n), note_(0)
        {
        }

        ~kwire_t()
        {
            source_shutdown();
        }

        void play(unsigned n,unsigned v,unsigned long long l,unsigned long long t)
        {
            if(event_.get().is_path())
            {
                //pic::logmsg() << (void *)this << " nplayer ending early at " << t;
                source_end(t);
                t++;
            }

            time_ = t;
            inc_ = (float)v*RAMP_INC;
            current_ = 0;
            count_ = l/RAMP_INTERVAL;
            index_ = 0;
            note_ = n;
            event_.set_nb(piw::makenull_nb(0));
            buffer_ = piw::xevent_data_buffer_t(SIG2(1,2),PIW_DATAQUEUE_SIZE_NORM);
        }

        void ticked(unsigned long long f,unsigned long long t);

        void source_ended(unsigned seq)
        {
        }

        recorder::nplayer_t::impl_t *nplayer_;
        unsigned name_;
        piw::dataholder_nb_t event_;
        unsigned long long time_;
        unsigned index_;
        unsigned count_;
        unsigned note_;
        float inc_;
        float current_;
        piw::xevent_data_buffer_t buffer_;
    };

    struct controller_t: piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, virtual pic::lckobject_t
    {
        controller_t(unsigned size, const piw::cookie_t &c): piw::event_data_source_real_t(piw::pathnull(0)), size_(size)
        {
            connect(c);
            connect_wire(this,source());

            buffer_ = piw::xevent_data_buffer_t(SIG1(3),PIW_DATAQUEUE_SIZE_NORM);
            update_controller(size_);
            start_slow(piw::pathnull(piw::tsd_time()),buffer_);
        }

        ~controller_t()
        {
            end_slow(piw::tsd_time());
            source_shutdown();
            wire_ctl_t::disconnect();
        }

        void update_controller(unsigned size)
        {
            size_ = size;

            piw::data_t d = piw::dictnull(piw::tsd_time());
            piw::data_t t;

            t = piw::tupleadd(piw::tuplenull(0),piw::makelong(size_,0));
            d = piw::dictset(d,"columnlen",t);
            t = piw::tupleadd(piw::tuplenull(0),piw::makelong(0,0));
            d = piw::dictset(d,"columnoffset",t);
            t = piw::tupleadd(piw::tuplenull(0),piw::makefloat(0.0,0));
            d = piw::dictset(d,"courseoffset",t);
            t = piw::tupleadd(piw::tuplenull(0),piw::makelong(size_,0));
            d = piw::dictset(d,"courselen",t);

            buffer_.add_value(3,d);

        }

        unsigned size_;
        piw::xevent_data_buffer_t buffer_;
    };
};

struct recorder::nplayer_t::impl_t: piw::root_ctl_t, piw::clocksink_t, virtual pic::lckobject_t, virtual pic::tracked_t
{
    impl_t(const piw::cookie_t &dc,const piw::cookie_t &cc,unsigned poly,unsigned size,piw::clockdomain_ctl_t *d): controller_(size,cc)
    {
        connect(dc);

        for(unsigned i=0; i<poly; ++i)
        {
            kwire_t *w = new kwire_t(this,i+1);
            all_.append(w);
            connect_wire(w,w->source());
        }

        d->sink(this,"player");
        set_clock(this);
        valid_.set(true);
        tick_enable(true);
    }

    ~impl_t()
    {
        tick_disable();
        valid_.set(false);
        kwire_t *w;

        while((w=all_.head()))
        {
            delete w;
        }

    }

    void play(unsigned n,unsigned v,unsigned long long l,unsigned long long t)
    {
        pic::flipflop_t<bool>::guard_t g(valid_);
        if(g.value())
        {
            kwire_t *w = inuse_.head();

            while(w)
            {
                if(w->note_==n)
                {
                    break;
                }

                w = inuse_.next(w);
            }

            if(!w)
            {
                w = all_.head();
                inuse_.append(w);
            }

            w->play(n,v,l,t);
            all_.append(w);
            tick_suppress(false);
        }
    }

    void set_size(unsigned size)
    {
        controller_.update_controller(size);
    }

    void clocksink_ticked(unsigned long long f,unsigned long long t)
    {
        pic::flipflop_t<bool>::guard_t g(valid_);
        if(g.value())
        {
            kwire_t *w = inuse_.head();

            while(w)
            {
                w->ticked(f,t);
                w = inuse_.next(w);
            }

            if(!inuse_.head())
            {
                //pic::logmsg() << "suppressing tick";
                tick_suppress(true);
            }
        }
    }

    pic::flipflop_t<bool> valid_;
    pic::ilist_t<kwire_t,ALL> all_;
    pic::ilist_t<kwire_t,INUSE> inuse_;
    controller_t controller_;
};

void ::kwire_t::ticked(unsigned long long f,unsigned long long t)
{
    if(t<time_)
    {
        //pic::logmsg() << "before start t=" << t << " start=" << time_;
        return;
    }

    if(!(time_>f))
    {
        time_=f+1;
    }

    if(!event_.get().is_path())
    {
        // create a new event
        event_.set_nb(piw::pathappend_channel_nb(piw::pathone_nb(note_,time_),name_));
        source_start(0,event_,buffer_);
        buffer_.add_value(2,piw::makekey(note_,1,note_,note_,1,note_,piw::KEY_HARD,time_));
    }

    while(time_<=t)
    {
        // synthesize the values for the data part of the note signal
        buffer_.add_value(1,piw::makefloat_bounded_nb(1.f,0.f,0.f,current_,time_));

        time_ += RAMP_INTERVAL;
        ++index_;

        if(index_ < RAMP_SAMPLES)
        {
            current_ = std::min(1.f,current_+inc_);
        }

        if(index_ > count_)
        {
            //pic::logmsg() << (void *)this << " nplayer finishing at " << time_ << " in tick " << f << "-" << t;
            source_end(time_);
            pic::element_t<INUSE>::remove();
            return;
        }
    }
}

namespace
{
    struct playsink_t: pic::sink_t<void(const piw::data_t &)>
    {
        playsink_t(recorder::nplayer_t::impl_t *i,unsigned n,unsigned v,unsigned long long l) : impl_(i), note_(n), velocity_(v), length_(l)
        {
        }
        
        void invoke(const piw::data_t &d) const
        {
            if(d.as_norm()!=0)
            {
                impl_->play(note_,velocity_,length_,d.time());
            }
        }
            
        bool iscallable() const
        {       
            return impl_.isvalid();
        }           
                    
        bool compare(const pic::sink_t<void(const piw::data_t &)> *o) const
        {           
            const playsink_t *c = dynamic_cast<const playsink_t *>(o);
            return c ? c->impl_==impl_ && c->note_==note_ && c->velocity_==velocity_ && c->length_==length_ : false;
        }       
                    
        pic::weak_t<recorder::nplayer_t::impl_t> impl_;
        unsigned note_;
        unsigned velocity_;
        unsigned long long length_;
    };
}

recorder::nplayer_t::nplayer_t(const piw::cookie_t &dc,const piw::cookie_t &cc,unsigned poly,unsigned size,piw::clockdomain_ctl_t *d) : impl_(new impl_t(dc,cc,poly,size,d))
{
}

recorder::nplayer_t::~nplayer_t()
{
    delete impl_;
}

piw::change_t recorder::nplayer_t::play(unsigned note, unsigned velocity, unsigned long long length)
{
    return piw::change_t(pic::ref(new playsink_t(impl_,note,velocity,length)));
}

bct_clocksink_t *recorder::nplayer_t::get_clock()
{
    return impl_;
}

void recorder::nplayer_t::set_size(unsigned size)
{
    impl_->set_size(size);
}
