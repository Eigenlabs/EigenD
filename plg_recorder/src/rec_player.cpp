
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

#include "rec_player.h"

#include <piw/piw_fastdata.h>
#include <piw/piw_aggregator.h>
#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>
#include <piw/piw_clockclient.h>
#include <piw/piw_address.h>
#include <picross/pic_ref.h>
#include <picross/pic_functor.h>
#include <picross/pic_time.h>

#include <list>
#include <map>
#include <vector>

#define PLAY_IDLE 0
#define PLAY_WAIT 1
#define PLAY_RUNNING 2

#define HEADROOM 2

namespace
{
    struct clockwire_t: piw::wire_t, piw::event_data_sink_t
    {
        clockwire_t(piw::clockinterp_t *i): interp_(i), transport_(false)
        {
        }

        ~clockwire_t()
        {
            invalidate();
        }

        void wire_closed()
        {
            invalidate();
        }

        void invalidate()
        {
            disconnect();
            unsubscribe();
        }

        void plumb(const piw::event_data_source_t &es)
        {
            subscribe(es);
        }

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t  &init)
        {
            iterator_=init.iterator();
            iterator_->reset(1,id.time());
            iterator_->reset(2,id.time());
            iterator_->reset(3,id.time());
        }

        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n) { iterator_->reset(sig,t); }

        void ticked(unsigned long long t)
        {
            if(iterator_.isvalid())
            {
                piw::data_nb_t d;
                while(iterator_->nextsig(1,d,t)) interp_->recv_clock(0,d);
                while(iterator_->nextsig(2,d,t)) interp_->recv_clock(1,d);
                while(iterator_->nextsig(3,d,t)) transport_ = d.as_norm()!=0.f;
            }
        }

        bool event_end(unsigned long long t)
        {
            ticked(t);
            transport_=false;
            iterator_.clear();
            return true;
        }

        piw::clockinterp_t *interp_;
        piw::xevent_data_buffer_t::iter_t iterator_;
        bool transport_;
    };

    struct datawire_t: public piw::wire_ctl_t, piw::event_data_source_real_t, virtual pic::counted_t
    {
        datawire_t(unsigned w,unsigned s,const piw::data_t &path): piw::event_data_source_real_t(path),  wire_(w), signals_(s), started_(false)
        {
            sigmask_ = (1ULL<<signals_)-1;
        }

        ~datawire_t()
        {
            source_shutdown();
        }

        void source_ended(unsigned seq)
        {
            started_=false;
        }

        unsigned long long buffer_chop(unsigned long long lt,unsigned long long t, float b, piw::clockinterp_t *clk)
        {
            if(!event_.isvalid())
            {
                return lt;
            }

            unsigned long long rt = t-evt_start_time_;
            unsigned long long et = event_.evt_time();

            float xx = evt_start_beat_+event_.evt_max_beat();

            if(b>=xx)
            {
                event_.clear();
                return 0ULL;
            }

            while(event_.isvalid() && event_.cur_time()-et<=rt)
            {
                unsigned long long t3 = event_.cur_time()-et+evt_start_time_;

                if(event_.cur_signal()==256)
                {
                    return t3;
                }

                output_.add_value(event_.cur_signal(),event_.cur_value().restamp(t3));
                event_.next();
            }

            if(!event_.isvalid())
            {
                return t;
            }

            return 0ULL;
        }
        
        unsigned long long buffer_unstretch(unsigned long long lt,unsigned long long t, float b, piw::clockinterp_t *clk)
        {
            if(!event_.isvalid())
            {
                return lt;
            }

            unsigned long long rt = t-evt_start_time_;
            unsigned long long et = event_.evt_time();

            while(event_.isvalid() && event_.cur_time()-et<=rt)
            {
                unsigned long long t3 = event_.cur_time()-et+evt_start_time_;

                if(event_.cur_signal()==256)
                {
                    return t3;
                }

                output_.add_value(event_.cur_signal(),event_.cur_value().restamp(t3));
                event_.next();
            }

            if(!event_.isvalid())
            {
                return t;
            }

            return 0ULL;
        }
        
        unsigned long long buffer_stretch(unsigned long long lt,unsigned long long t, float b, piw::clockinterp_t *clk)
        {
            float rb = b-rec_start_beat_;

            while(event_.isvalid() && event_.cur_beat()<=rb)
            {
                float b2 = rec_start_beat_+event_.cur_beat();
                unsigned long long t3 = clk->interpolate_time_backward(0,b2);
                //pic::logmsg() << "t3=" << t3 << " b2=" << b2 << " cb=" << event_.cur_beat() <<" sig=" << event_.cur_signal();

                if(t3<(lt+1))
                {
                    unsigned long long dt=(lt+1-t3);

                    if(dt>100)
                    {
                        pic::logmsg() << "**********************  adjusting time forward by " << dt << " us";
                    }

                    t3=lt+1;
                }

                //unsigned long long t3 = clk->interpolate_time(0,rec_start_beat_+event_.cur_beat(),lt);

                if(event_.cur_signal()==256)
                {
                    return t3;
                }

                output_.add_value(event_.cur_signal(),event_.cur_value().restamp(t3));
                event_.next();
            }

            if(!event_.isvalid())
            {
                return t;
            }

            return 0ULL;
        }
        
        void ticked(unsigned long long lt,unsigned long long t, float b, piw::clockinterp_t *clk, bool *running)
        {
            if(started_ && event_.isvalid())
            {
                unsigned long long end = 0ULL;

                switch(mode_)
                {
                    case PLAYER_MODE_STRETCH: end=buffer_stretch(lt,t,b,clk); break;
                    case PLAYER_MODE_UNSTRETCH: end=buffer_unstretch(lt,t,b,clk); break;
                    case PLAYER_MODE_CHOP: end=buffer_chop(lt,t,b,clk); break;
                    default: end=buffer_stretch(lt,t,b,clk); break;
                }
                
                if(end)
                {
                    if(source_end(end))
                    {
                        started_=false;
                    }
                }

                if(started_)
                {
                    *running=true;
                }
            }
        }

        void start_event(unsigned long long t, float b, piw::clockinterp_t *clk, const recorder::readevent_t &event, unsigned long long st, float sb, unsigned long long et, float eb, unsigned mode)
        {
            started_=true;
            rec_start_time_=st;
            rec_start_beat_=sb;
            evt_start_time_=et;
            evt_start_beat_=eb;
            mode_=mode;
            event_=event;

            output_ = piw::xevent_data_buffer_t(sigmask_,PIW_DATAQUEUE_SIZE_NORM);
            unsigned long long end = 0ULL;

            if(!event_.isvalid())
            {
                return;
            }

            switch(mode_)
            {
                case PLAYER_MODE_STRETCH: end=buffer_stretch(st,t,b,clk); break;
                case PLAYER_MODE_UNSTRETCH: end=buffer_unstretch(st,t,b,clk); break;
                case PLAYER_MODE_CHOP: end=buffer_chop(st,t,b,clk); break;
                default: end=buffer_stretch(st,t,b,clk); break;
            }

            piw::data_nb_t id = event_.evt_id();
            source_start(0,id.restamp(et),output_);

            if(end)
            {
                if(source_end(end))
                {
                    started_=false;
                }
            }
        }

        void shutdown(unsigned long long t)
        {
            if(started_)
            {
                started_=false;
                source_end(t);
            }
        }

        unsigned char wire_;
        unsigned signals_;
        bool started_;
        recorder::readevent_t event_;
        float rec_start_beat_;
        unsigned long long rec_start_time_;
        float evt_start_beat_;
        unsigned long long evt_start_time_;
        unsigned mode_;
        unsigned long long sigmask_;
        piw::xevent_data_buffer_t output_;
    };

    struct take_t : piw::root_ctl_t, pic::element_t<>
    {
        take_t(recorder::player_t::impl_t *i, unsigned n, unsigned s);
        ~take_t();

        void ticked(unsigned long long t);
        void start_playback(unsigned long long t, const std::string &n, const recorder::recording_t &, unsigned long, unsigned m);
        void stop_playback(unsigned long long t);

        int start_event(unsigned long long t, float b, piw::clockinterp_t *clk, const recorder::readevent_t &event, unsigned long long st, float sb, unsigned long long et, float eb, unsigned m)
        {
            for(unsigned i=0;i<wires_.size();i++)
            {
                if(!wires_[i]->started_)
                {
                    wires_[i]->start_event(t,b,clk,event,st,sb,et,eb,m);
                    return (int)i;
                }
            }

            return -1;
        }

        void abortname(const std::string &n)
        {
            if(state_ != PLAY_IDLE && rname_==n)
            {
                stop_playback(last_tick_+1);
            }
        }

        void abort()
        {
            if(state_ != PLAY_IDLE)
            {
                stop_playback(last_tick_+1);
            }
        }

        void abortcookie(unsigned long c)
        {
            if(state_ != PLAY_IDLE && cookie_==c)
            {
                stop_playback(last_tick_+1);
            }
        }

        void play(const recorder::recording_t &r, const std::string &n, unsigned long c, unsigned long long t, unsigned m);

        void reserve();

        void shutdown_wires(unsigned long long t)
        {
            for(unsigned i=0;i<wires_.size();i++)
            {
                wires_[i]->shutdown(t);
            }
        }

        recorder::player_t::impl_t *impl_;
        unsigned signals_;
        recorder::recording_t recording_;
        unsigned long cookie_;
        std::string rname_;
        unsigned state_;
        unsigned long long start_time_, last_tick_;
        float start_beat_;
        std::vector<pic::ref_t<datawire_t> > wires_;
        unsigned name_;
        bool free_;
        unsigned mode_;
    };

    struct cookiedata_t
    {
        cookiedata_t(const std::string &n, const recorder::recording_t &r, unsigned p) : name(n), recording(r), poly(p) {}
        std::string name;
        recorder::recording_t recording;
        unsigned poly;
    };

    struct interlock_t: virtual public pic::atomic_counted_t
    {
        interlock_t(recorder::player_t::impl_t *i): impl_(i) {}
        mutable pic::flipflop_t<recorder::player_t::impl_t *> impl_;
    };

    struct playsink_t: pic::sink_t<void(const piw::data_t &)>
    {
        playsink_t(const pic::ref_t<interlock_t> &i, const std::string &n, unsigned long c, const recorder::recording_t &r, unsigned m): impl_(i), cookie_(c), recording_(r), rname_(n), mode_(m)
        {
        }

        void invoke(const piw::data_t &p1) const;

        bool iscallable() const
        {
            return true;
        }

        bool compare(const pic::sink_t<void(const piw::data_t &)> *s) const
        {
            const playsink_t *c = dynamic_cast<const playsink_t *>(s);
            if(c && c->impl_==impl_ && c->recording_==recording_) return true;
            return false;
        }

        mutable pic::ref_t<interlock_t> impl_;
        unsigned long cookie_;
        recorder::recording_t recording_;
        std::string rname_;
        unsigned mode_;
    };
}

namespace recorder
{
    struct player_t::impl_t: piw::thing_t, piw::root_t, piw::clocksink_t
    {
        impl_t(piw::clockdomain_ctl_t *d, unsigned p, unsigned s, const piw::cookie_t &c): root_t(0), aggregator_(c,d), next_(0), freecount_(0), takecount_(0), signals_(s), poly_(p), interp_(2), clockwire_(&interp_), up_(0)
        {
            d->sink(this,"player");
            piw::tsd_thing(this);
            thing_trigger_slow();
            interlock_ = pic::ref(new interlock_t(this));
            tick_enable(false);
        }

        static int __pop(void *i_,void *t_)
        {
            impl_t *i = (impl_t *)i_;
            take_t **t = (take_t **)t_;
            *t = i->takes_.head();
            if(*t)
            {
                (*t)->remove();
                return 1;
            }
            return 0;
        }

        static int __next0(void *i_,void *t_,void *c_)
        {
            impl_t *i = (impl_t *)i_;
            take_t **t = (take_t **)t_;

            if(*t)
            {
                *t = i->takes_.next(*t);
            }
            else
            {
                *t = i->takes_.head();
            }

            if(*t)
            {
                if(!c_)
                {
                    return 1;
                }

                if((*t)->free_)
                {
                    (*t)->free_=false;
                    pic_atomicdec(&i->freecount_);
                    return 1;
                }
            }

            return 0;
        }

        static int __next(void *i_, void *t_)
        {
            return __next0(i_,t_,0);
        }

        static int __claimnext(void *i_, void *t_)
        {
            return __next0(i_,t_,i_);
        }

        static int __release(void *i_, void *t_)
        {
            impl_t *i = (impl_t *)i_;
            take_t *t = (take_t *)t_;
            t->free_ = true;
            pic_atomicinc(&i->freecount_);
            return 1;
        }

        ~impl_t()
        {
            tracked_invalidate();
            interlock_->impl_.set(0);

            take_t *t;
            while(piw::tsd_fastcall(__pop,this,&t))
            {
                delete t;
            }
        }

        piw::wire_t *root_wire(const piw::event_data_source_t &es)
        {
            piw::data_t path = es.path();

            if(path.as_pathlen()==0)
            {
                return 0;
            }

            if(path.as_path()[0] == 1)
            {
                clockwire_.plumb(es);
                return &clockwire_;
            }

            return 0;
        }

        void root_clock()
        {
            bct_clocksink_t *c = get_clock();

            if(up_)
            {
                remove_upstream(up_);
            }

            up_ = c;

            if(up_)
            {
                add_upstream(up_);
            }
        }

        void clocksink_ticked(unsigned long long f, unsigned long long t)
        {
            clockwire_.ticked(t);
            for(take_t *tk=takes_.head(); tk!=0; tk=takes_.next(tk))
            {
                tk->ticked(t);
            }
        }

        void root_latency()
        {
            set_sink_latency(get_latency());

            take_t *t=0;
            while(piw::tsd_fastcall(__next,this,&t))
            {
                t->set_latency(get_latency());
            }
        }

        void root_opened()
        {
            root_clock(); root_latency();
        }

        void root_closed()
        {
            root_clock(); root_latency();
        }

        piw::cookie_t cookie()
        {
            return piw::cookie_t(this);
        }

        unsigned long load(const std::string &name, const recording_t &r, unsigned p)
        {
            ++next_;

            cookies_.insert(std::make_pair(next_, cookiedata_t(name, r, p)));
            thing_trigger_slow();

            pic::msg() << "loaded cookie " << next_ << " for name " << name << pic::log;

            return next_;
        }

        std::string cookiename(unsigned long r)
        {
            std::map<unsigned long, cookiedata_t>::iterator ci = cookies_.find(r);

            if(ci == cookies_.end())
            {
                pic::msg() << "bad cookie " << r << pic::log;
                return "";
            }

            return ci->second.name;
        }

        unsigned long clonecookie(const std::string &name,unsigned poly)
        {
            std::map<unsigned long, cookiedata_t>::const_iterator i = cookies_.begin(), e = cookies_.end();

            while(i != e)
            {
                if(i->second.name==name)
                {
                    ++next_;
                    cookies_.insert(std::make_pair(next_, cookiedata_t(name, i->second.recording, poly)));
                    thing_trigger_slow();
                    pic::msg() << "cloned cookie " << next_ << " for name " << name << pic::log;
                    return next_;
                }

                ++i;
            }

            return 0;
        }

        unsigned long getcookie(const std::string &name)
        {
            std::map<unsigned long, cookiedata_t>::const_iterator i = cookies_.begin(), e = cookies_.end();

            while(i != e)
            {
                if(i->second.name==name)
                {
                    return i->first;
                }

                ++i;
            }

            return 0;
        }

        void abort(const std::string &name)
        {
            piw::tsd_fastcall(__abortname, this, (void *)&name);
        }

        void abortcookie(unsigned long cookie)
        {
            piw::tsd_fastcall(__abortcookie, this, (void *)&cookie);
        }

        void abortall()
        {
            piw::tsd_fastcall(__abortname, this, 0);
        }

        void unload(unsigned long r, bool c)
        {
            cookies_.erase(r);

            if(c)
            {
                piw::tsd_fastcall(__abortcookie, this, &r);
            }
        }

        piw::change_t player(unsigned long r, unsigned mode)
        {
            std::map<unsigned long, cookiedata_t>::iterator ci = cookies_.find(r);

            if(ci == cookies_.end())
            {
                pic::msg() << "bad cookie " << r << pic::log;
                return piw::change_t();
            }

            return piw::change_t(pic::ref(new playsink_t(interlock_,ci->second.name,r,ci->second.recording,mode))); 
        }

        void invalidate()
        {
        }

        void play(const recorder::recording_t &r, const std::string &n, unsigned long c, const piw::data_t &d, unsigned mode)
        {
            if(d.as_norm() > 0)
            {
                for(take_t *tk=takes_.head(); tk!=0; tk=takes_.next(tk))
                {
                    if(tk->free_)
                    {
                        tk->play(r,n,c,d.time(),mode);
                        trigger_slow();
                        return;
                    }
                }
                pic::logmsg() << "playback failed, no free slot";
            }
        }

        void thing_trigger_slow()
        {
            unsigned ot = takecount_;
            unsigned of = freecount_;

            std::map<unsigned long, cookiedata_t>::iterator ci;
            unsigned takes = 0;
            unsigned poly = 0;

            for(ci=cookies_.begin(); ci!=cookies_.end(); ci++)
            {
                takes+=ci->second.poly;

                if(ci->second.recording.wires() > poly)
                {
                    poly = ci->second.recording.wires();
                }
            }

            if(poly>poly_)
            {
                poly_=poly;

                take_t *tk=0;
                while(piw::tsd_fastcall(__claimnext,this,&tk))
                {
                    tk->reserve();
                    piw::tsd_fastcall(__release,this,tk);
                }
            }

            while(takecount_ < takes)
            {
                if(!new_take())
                {
                    break;
                }
            }

            while(freecount_ < HEADROOM)
            {
                if(!new_take())
                {
                    break;
                }
            }

            if(takecount_!=ot)
                pic::logmsg() << "count changed from " << ot << " to " << takecount_;
                
            if(freecount_!=of)
                pic::logmsg() << "freecount changed from " << of << " to " << freecount_;
        }

        static int __abortname(void *i_, void *n_)
        {
            impl_t *i = (impl_t *)i_;
            std::string *n = (std::string *)n_;

            for(take_t *tk=i->takes_.head(); tk!=0; tk=i->takes_.next(tk))
            {
                if(n)
                {
                    tk->abortname(*n);
                }
                else
                {
                    tk->abort();
                }
            }

            return 0;
        }

        static int __abortcookie(void *i_, void *c_)
        {
            impl_t *i = (impl_t *)i_;
            unsigned long c = *(unsigned long *)c_;

            for(take_t *tk=i->takes_.head(); tk!=0; tk=i->takes_.next(tk))
            {
                tk->abortcookie(c);
            }

            return 0;
        }

        static int __add(void *i_,void *t_)
        {
            impl_t *i=(impl_t *)i_;
            take_t *t=(take_t *)t_;
            i->takes_.append(t);
            t->free_ = true;
            pic_atomicinc(&i->freecount_);
            return 0;
        }

        void popfree(take_t *t)
        {
            t->free_=false;
            pic_atomicdec(&freecount_);
        }

        void pushfree(take_t *t)
        {
            t->free_=true;
            pic_atomicinc(&freecount_);
        }

        bool new_take()
        {
            unsigned name = aggregator_.find_unused();

            if(name==0)
            {
                pic::msg() << "no free outputs for take" << pic::log;
                return false;
            }

            take_t *t = new take_t(this,name,signals_);
            pic::logmsg() << "allocated new take " << (void *)t;
            piw::tsd_fastcall(__add,this,t);
            ++takecount_;
            return true;
        }

        piw::aggregator_t aggregator_;
        std::map<unsigned long, cookiedata_t> cookies_;
        unsigned next_;

        pic::ilist_t<take_t> takes_; // fast
        pic_atomic_t freecount_;
        unsigned takecount_;

        unsigned signals_;
        unsigned poly_;

        pic::ref_t<interlock_t> interlock_;

        piw::clockinterp_t interp_;
        clockwire_t clockwire_;
        bct_clocksink_t *up_;
    };
}

void playsink_t::invoke(const piw::data_t &d) const
{
    if(d.as_norm() != d.as_array_rest())
    {
        pic::flipflop_t<recorder::player_t::impl_t *>::guard_t g(impl_->impl_);

        if(g.value())
        {
            g.value()->play(recording_,rname_,cookie_,d,mode_);
        }
    }
}

take_t::take_t(recorder::player_t::impl_t *i, unsigned n, unsigned s): impl_(i), signals_(s), state_(PLAY_IDLE), start_time_(~0ULL), last_tick_(0), name_(n), free_(true)
{
    connect(impl_->aggregator_.get_output(name_));
    reserve();
    set_clock(impl_);
}

take_t::~take_t()
{
    impl_->aggregator_.clear_output(name_);
    wires_.clear();
    root_ctl_t::disconnect();
}

void take_t::start_playback(unsigned long long t, const std::string &n, const recorder::recording_t &r, unsigned long c, unsigned m)
{
    if(state_==PLAY_IDLE)
    {
        recording_ = r;
        cookie_ = c;
        rname_ = n;
        mode_ = m;

        if(t)
        {
            start_time_ = t;
        }
        else
        {
            start_time_ = last_tick_;
        }

        state_ = PLAY_WAIT;
        impl_->popfree(this);
        pic::logmsg() << "start playing on take " << (void *)this;
        //pic::msg() << "initiated playback at " << start_time_ << " current time " << last_tick_ << pic::log;
    }
}

void take_t::stop_playback(unsigned long long t)
{
    if(state_!=PLAY_IDLE)
    {
        shutdown_wires(t);
        start_time_ = ~0ULL;
        state_ = PLAY_IDLE;
        impl_->pushfree(this);
        //pic::msg() << "stopped playback at " << t << pic::log;
    }
}

void take_t::reserve()
{
    unsigned ws;
    pic::ref_t<datawire_t> w;

    while((ws=wires_.size())<impl_->poly_)
    {
        piw::data_t path = piw::pathprepend(piw::pathone(ws+1,0),1);
        w = pic::ref(new datawire_t(ws+1,signals_,path));
        connect_wire(w.ptr(),w->source());
        wires_.resize(ws+1);
        wires_[ws]=w;
    }
}

void take_t::play(const recorder::recording_t &r, const std::string &n, unsigned long c, unsigned long long t, unsigned m)
{
    if(!impl_->clockwire_.transport_)
    {
        pic::logmsg() << "transport stopped: not playing";
        return;
    }

    start_playback(t,n,r,c,m);
}

void take_t::ticked(unsigned long long t)
{
    if(!impl_->clockwire_.transport_ && state_!=PLAY_IDLE)
    {
        pic::logmsg() << "transport stopped at " << t;
        stop_playback(t);
    }

    if(!last_tick_)
    {
        last_tick_=t;
        return;
    }

    if(state_ == PLAY_WAIT)
    {
        if(t > start_time_)
        {
            if(start_time_ < last_tick_) start_time_=last_tick_;
            recording_.reset();
            state_ = PLAY_RUNNING;
            start_beat_ = impl_->interp_.interpolate_clock(0,start_time_);
            //pic::msg() << "running at " << start_time_ << " current time " << t << " current beat " <<start_beat_<<pic::log;
        }
    }

    if(state_ == PLAY_RUNNING)
    {
        float b = impl_->interp_.interpolate_clock(0,t);
        float rb = b-start_beat_;

        bool running = recording_.isvalid();

        for(unsigned i=0;i<wires_.size();i++)
        {
            wires_[i]->ticked(last_tick_,t,b,&impl_->interp_,&running);
        }

        while(recording_.isvalid())
        {
            recorder::readevent_t e = recording_.cur_event();

            if(e.evt_beat()>rb)
            {
                break;
            }

            float eb = start_beat_+e.evt_beat();
            unsigned long long et =
                impl_->interp_.interpolate_time_noncyclic(0,eb,last_tick_+1);
            int w = start_event(t,b,&impl_->interp_,e,start_time_,start_beat_,et,eb,mode_);

            if(w>=0)
            {
                wires_[w]->ticked(last_tick_,t,b,&impl_->interp_,&running);
            }

            recording_.next();
        }

        if(!running)
        {
            pic::msg() << "playback finished at " << t << pic::log;
            stop_playback(t);
        }
    }

    last_tick_ = t;
}

recorder::player_t::player_t(piw::clockdomain_ctl_t *d, unsigned p, unsigned s, const piw::cookie_t &c) : impl_(new impl_t(d,p,s,c)) {}
recorder::player_t::~player_t() { delete impl_; }
unsigned long recorder::player_t::load(const std::string &name, const recording_t &r, unsigned p) { return impl_->load(name,r,p); }
unsigned long recorder::player_t::getcookie(const std::string &name) { return impl_->getcookie(name); }
unsigned long recorder::player_t::clonecookie(const std::string &name, unsigned poly) { return impl_->clonecookie(name,poly); }
void recorder::player_t::unload(unsigned long r, bool c) { impl_->unload(r,c); }
void recorder::player_t::abort(const std::string &n) { impl_->abort(n); }
void recorder::player_t::abortcookie(unsigned long c) { impl_->abortcookie(c); }
void recorder::player_t::abortall() { impl_->abortall(); }
piw::cookie_t recorder::player_t::cookie() { return impl_->cookie(); }
piw::change_t recorder::player_t::player(unsigned long r, unsigned mode) { return impl_->player(r,mode); }
std::string recorder::player_t::cookiename(unsigned long r) { return impl_->cookiename(r); }

