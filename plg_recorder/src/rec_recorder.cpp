
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

#include <piw/piw_fastdata.h>
#include <piw/piw_thing.h>
#include <piw/piw_address.h>
#include <piw/piw_clockclient.h>
#include <picross/pic_ref.h>
#include <set>
#include <vector>
#include <iterator>
#include <math.h>

#include "rec_recorder.h"

#define RECORDER_IDLE 0
#define RECORDER_ARMED 1
#define RECORDER_RUNNING 2
#define RECORDER_SHUTDOWN 3
#define RECORDER_ARMING 4

#define START_START 0
#define START_DEFER 1
#define START_IGNORE 2

#define ID_NEW ~0U

#define SONGBEAT 0
#define BARBEAT 1

namespace 
{
    struct clockwire_t: piw::wire_t, piw::event_data_sink_t
    {
        clockwire_t(recorder::recorder_t::impl_t *impl): impl_(impl) {}
        ~clockwire_t() { invalidate(); }
        void wire_closed() { invalidate(); }
        void invalidate() { disconnect(); unsubscribe(); }
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei);
        bool event_end(unsigned long long t);
        void ticked(unsigned long long t);
        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n) { iterator_->reset(sig,t); }

        recorder::recorder_t::impl_t *impl_;
        piw::xevent_data_buffer_t::iter_t iterator_;
    };

    struct datawire_t: piw::wire_t, piw::event_data_sink_t
    {
        datawire_t(recorder::recorder_t::impl_t *i, const piw::event_data_source_t &es, unsigned channel);
        ~datawire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n);
        void delayed_start(unsigned long long t);
        void shutdown(unsigned long long t);
        void arm();
        void ticked(unsigned long long t);

        recorder::recorder_t::impl_t *impl_;
        piw::event_data_source_t source_;
        unsigned channel_;
        piw::data_t path_;
        unsigned id_;
        bool active_;
        recorder::writeevent_t event_;
        piw::dataholder_nb_t stored_event_;
        piw::xevent_data_buffer_t stored_buffer_;
        piw::xevent_data_buffer_t::iter_t iterator_;
    };

    typedef pic::lckmap_t<piw::data_t,datawire_t *>::lcktype datamap_t;
}

namespace recorder
{
    struct recorder_t::impl_t: piw::clocksink_t, piw::thing_t, piw::decode_ctl_t, virtual pic::lckobject_t
    {
        impl_t(recorder_t *r, piw::clockdomain_ctl_t *d, unsigned s);
        ~impl_t();

        piw::wire_t *wire_create(const piw::event_data_source_t &es);
        void set_clock(bct_clocksink_t *c);
        int record(unsigned duration);
        void abort();
        void clocksink_ticked(unsigned long long from,unsigned long long t);
        void thing_dequeue_slow(const piw::data_t &);
        void set_latency(unsigned l) { set_sink_latency(l); }

        int gate_activated(unsigned long long t, unsigned c);
        void beat_receive(unsigned clk, const piw::data_nb_t &);
        void id_receive(const piw::data_nb_t &);

        static int armer__(void *,void *);
        static int aborter__(void *,void *);

        void assign_ids();

        recorder_t *recorder_;
        bct_clocksink_t *up_;
        unsigned signals_;

        clockwire_t clockwire_;
        pic::flipflop_t<datamap_t> datawires_;

        unsigned beat_age_;

        unsigned state_;
        unsigned duration_;
        unsigned long long start_time_;
        float start_beat_;
        float end_beat_;
        unsigned long long activate_mask_;
        unsigned long long sigmask_;
        unsigned long long armed_time_;

        recordmaker_t maker_;

        piw::clockinterp_t clock_;
        piw::decoder_t decoder_;
    };
}

void recorder::recorder_t::impl_t::id_receive(const piw::data_nb_t &d)
{
    //clock_.recv_id(d);
}

void recorder::recorder_t::impl_t::beat_receive(unsigned clk, const piw::data_nb_t &d)
{
    if(!d.as_arraylen()) return;

    clock_.recv_clock(clk,d);

    beat_age_ = 2;

    if(state_==RECORDER_RUNNING)
    {
        float beat = clock_.interpolate_clock(SONGBEAT,d.time());

        if(beat >= end_beat_)
        {
            pic::msg() << "recording stopped: duration=" << (d.time()-start_time_)/1000ULL << "ms" << pic::log;
            state_=RECORDER_SHUTDOWN;
            enqueue_slow(piw::makefloat(0,1));
        }
    }
}

void recorder::recorder_t::impl_t::thing_dequeue_slow(const piw::data_t &d)
{
    if(!d.is_float()) return;

    switch(d.time())
    {
        case 1:
            recorder_->record_done(maker_.get_recording());
            break;
        case 2:
            recorder_->record_started(maker_.get_recording());
            break;
        case 3:
            recorder_->record_aborted();
            break;
    }
}

int recorder::recorder_t::impl_t::gate_activated(unsigned long long t, unsigned c)
{
    if(state_==RECORDER_RUNNING)
    {
        return START_START;
    }

    if(state_!=RECORDER_ARMED)
    {
        return START_IGNORE;
    }

    if(((1ULL<<c)&activate_mask_)==0)
    {
        return START_DEFER;
    }

    if(t<armed_time_)
    {
        return START_IGNORE;
    }

    float songbeat = clock_.interpolate_clock(SONGBEAT,t);
    float barbeat = clock_.interpolate_clock(BARBEAT,t);
    float barlen = clock_.get_mod(BARBEAT);

    float bbr = floor(barbeat+0.5f);
    float wholebeat = fmod(bbr, barlen);
    float fracbeat = barbeat-bbr;
    pic::msg() << "t=" << t << " barbeat=" << barbeat << " wholebeat=" << wholebeat << " fracbeat=" << fracbeat << pic::log;

    end_beat_ = songbeat + duration_*barlen;

    pic::msg() << "recording started at beat " << songbeat << " barlen " << barlen << " ending at beat " << end_beat_ << pic::log;

    start_time_=t;
    start_beat_= songbeat;

    maker_.set_tag(TAG_BAR_DURATION,piw::makefloat_nb(duration_,0));
    maker_.set_tag(TAG_START_BEAT,piw::makefloat_nb(barbeat,0));
    maker_.set_tag(TAG_START_SONG_BEAT,piw::makefloat_nb(songbeat,0));
    maker_.set_tag(TAG_BEAT_DURATION,piw::makefloat_nb(duration_*barlen,0));
    maker_.set_tag(TAG_BEAT_DELTA,piw::makefloat_nb(fracbeat,0));
    state_=RECORDER_RUNNING;
    enqueue_slow_nb(piw::makefloat_nb(songbeat,2));

    pic::flipflop_t<datamap_t>::guard_t g(datawires_);
    datamap_t::const_iterator i;

    for(i=g.value().begin(); i!=g.value().end(); i++)
    {
        if(((1ULL<<i->second->channel_)&activate_mask_)==0)
        {
            i->second->delayed_start(t);
        }
    }

    return START_START;
}

void datawire_t::delayed_start(unsigned long long t)
{
    if(id_==ID_NEW || active_ || impl_->state_!=RECORDER_RUNNING || !stored_event_.get().is_path())
    {
        return;
    }

    float b = impl_->clock_.interpolate_clock(SONGBEAT,t);
    unsigned long long st = impl_->start_time_;
    float sb = impl_->start_beat_;
    piw::data_nb_t d = stored_event_.get().restamp(t);

    event_ = impl_->maker_.new_event(t-st,b-sb,d);

    //piw::xevent_data_buffer_t ei = stored_init_.clone();
    //ei.update(stored_buffer_,t);

    iterator_ = stored_buffer_.iterator();
    for(unsigned i=0; i<impl_->signals_; ++i)
    {
        piw::data_nb_t d2;
        if(iterator_->latest(i+1,d2,t))
        {
            event_.add_value(t-st,b-sb,i+1,d2.restamp(t));
        }
    }

    active_=true;
}

void datawire_t::event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei)
{
    active_=false;

    if(id_==ID_NEW)
    {
        return;
    }

    int s = impl_->gate_activated(d.time(),channel_);

    if(s!=START_START)
    {
        if(s==START_DEFER)
        {
            stored_event_.set_nb(d);
            stored_buffer_=ei;
            return;
        }

        stored_event_.clear();
        return;
    }

    active_=true;

    unsigned long long st = impl_->start_time_;
    float sb = impl_->start_beat_;

    unsigned long long t = std::max(d.time(),st);
    float b = impl_->clock_.interpolate_clock(SONGBEAT,t);

    event_ = impl_->maker_.new_event(t-st,b-sb,d);
    iterator_ = ei.iterator();

    for(unsigned s=0; s<impl_->signals_; ++s)
    {
        piw::data_nb_t d2;
        if(iterator_->latest(s+1,d2,t))
        {
            unsigned long long t2 = std::max(d2.time(),st);
            b = impl_->clock_.interpolate_clock(SONGBEAT,t2);
            event_.add_value(t2-st,b-sb,s+1,d2);
        }
    }
}

void datawire_t::ticked(unsigned long long t)
{
    if(id_==ID_NEW)
    {
        return;
    }

    if(!active_)
    {
        return;
    }

    unsigned long long st = impl_->start_time_;
    float sb = impl_->start_beat_;

    piw::data_nb_t d;
    unsigned sig;

    while(iterator_->next(impl_->sigmask_,sig,d,t))
    {
        float b = impl_->clock_.interpolate_clock(SONGBEAT,d.time());
        event_.add_value(d.time()-st,b-sb,sig,d);
    }
}

bool datawire_t::event_end(unsigned long long t)
{
    if(id_==ID_NEW)
    {
        return true;
    }

    if(active_)
    {
        ticked(t);
        active_=false;
        unsigned long long st = impl_->start_time_;
        float sb = impl_->start_beat_;
        float b = impl_->clock_.interpolate_clock(SONGBEAT,t);
        event_.end_event(t-st,b-sb);
    }

    stored_event_.set_nb(piw::makenull_nb());
    iterator_.clear();
    return true;
}

void datawire_t::event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    iterator_->reset(sig,t);
}

void datawire_t::shutdown(unsigned long long t)
{
    unsubscribe();

    if(active_)
    {
        ticked(t);
        active_=false;
        unsigned long long st = impl_->start_time_;
        float sb = impl_->start_beat_;
        float b = impl_->clock_.interpolate_clock(SONGBEAT,t);
        event_.end_event(t-st,b-sb);
    }

}

void clockwire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei)
{
    //pic::logmsg() << "recorder clockwire event started";
    impl_->id_receive(id);
    iterator_ = ei.iterator();
    unsigned long long t =id.time();
    piw::data_nb_t d;
    if(iterator_->latest(1,d,t)) impl_->beat_receive(SONGBEAT,d);
    if(iterator_->latest(2,d,t)) impl_->beat_receive(BARBEAT,d);
    impl_->tick_suppress(false);
}

void clockwire_t::ticked(unsigned long long t)
{
    piw::data_nb_t d;
    while(iterator_->nextsig(1,d,t)) impl_->beat_receive(SONGBEAT,d);
    while(iterator_->nextsig(2,d,t)) impl_->beat_receive(BARBEAT,d);
}

bool clockwire_t::event_end(unsigned long long t)
{
    //pic::logmsg() << "recorder clockwire event ended";
    ticked(t);
    impl_->id_receive(piw::makenull_nb(t));
    impl_->tick_suppress(true);
    return true;
}

void recorder::recorder_t::impl_t::clocksink_ticked(unsigned long long from,unsigned long long t)
{
    clockwire_.ticked(t);
    if(state_==RECORDER_IDLE)
        return;

    if(beat_age_>0) beat_age_--;

    if(beat_age_==0)
    {
        state_ = RECORDER_SHUTDOWN;
        pic::msg() << "recording aborted: no clock" << pic::log;
        enqueue_slow_nb(piw::makefloat_nb(0,3));
    }

    if(state_==RECORDER_ARMING)
    {
        state_=RECORDER_ARMED;
        armed_time_ = t;
        pic::flipflop_t<datamap_t>::guard_t g(datawires_);
        datamap_t::const_iterator i;

        for(i=g.value().begin(); i!=g.value().end(); i++)
        {
            i->second->arm();
        }

        pic::logmsg() << "recorder is armed " << clock_.interpolate_clock(SONGBEAT,t) << " t=" << t;
    }

    if(state_==RECORDER_SHUTDOWN)
    {
        state_=RECORDER_IDLE;
        pic::flipflop_t<datamap_t>::guard_t g(datawires_);
        datamap_t::const_iterator i;

        pic::logmsg() << "recorder shutdown";

        for(i=g.value().begin(); i!=g.value().end(); i++)
        {
            i->second->shutdown(t);
        }

    }

    if(state_==RECORDER_RUNNING)
    {
        pic::flipflop_t<datamap_t>::guard_t g(datawires_);
        datamap_t::const_iterator i;

        for(i=g.value().begin(); i!=g.value().end(); i++)
        {
            i->second->ticked(t);
        }

        maker_.set_time(clock_.interpolate_clock(SONGBEAT,t)-start_beat_,t-start_time_);
    }
}

datawire_t::datawire_t(recorder::recorder_t::impl_t *impl, const piw::event_data_source_t &es, unsigned c): impl_(impl), source_(es), channel_(c), path_(es.path()), id_(ID_NEW), active_(false)
{
    impl_->datawires_.alternate().insert(std::make_pair(path_,this));
    impl_->datawires_.exchange();
}

void datawire_t::arm()
{
    subscribe_and_ping(source_);
}

void datawire_t::invalidate()
{
    if(impl_)
    {
        unsubscribe();
        impl_->datawires_.alternate().erase(path_);
        impl_->datawires_.exchange();
        impl_=0;
    }
}

recorder::recorder_t::impl_t::impl_t(recorder_t *r, piw::clockdomain_ctl_t *d, unsigned s): recorder_(r), up_(0), signals_(s), clockwire_(this), clock_(2), decoder_(this)
{
    d->sink(this,"recorder");

    state_=RECORDER_IDLE;
    beat_age_=0;
    activate_mask_ = (1<<2);
    sigmask_ = (1ULL<<signals_)-1;

    tick_enable(true);
    piw::tsd_thing(this);
}

recorder::recorder_t::impl_t::~impl_t()
{
    tracked_invalidate();
    decoder_.shutdown();
}

piw::wire_t *recorder::recorder_t::impl_t::wire_create(const piw::event_data_source_t &es)
{
    piw::data_t path = es.path();

    if(path.as_pathlen()==0)
    {
        return 0;
    }

    unsigned channel = path.as_path()[0];

    if(channel==1)
    {
        clockwire_.subscribe(es);
        return &clockwire_;
    }

    if(channel==100)
    {
        return 0;
    }

    datamap_t::iterator wi = datawires_.alternate().find(es.path());

    if(wi!=datawires_.alternate().end())
    {
        delete wi->second;
    }

    return new datawire_t(this,es,channel);
}

void recorder::recorder_t::impl_t::set_clock(bct_clocksink_t *c)
{
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

int recorder::recorder_t::impl_t::aborter__(void *impl_, void *)
{
    recorder::recorder_t::impl_t *impl = (recorder::recorder_t::impl_t *)impl_;

    if(impl->state_ != RECORDER_IDLE)
    {
        impl->state_ = RECORDER_SHUTDOWN;
        pic::msg() << "recorder is aborted" << pic::log;
    }

    return 0;
}

void recorder::recorder_t::impl_t::abort()
{
    piw::tsd_fastcall(aborter__,this,0);
}

int recorder::recorder_t::impl_t::armer__(void *impl_, void *dur_)
{
    unsigned duration = *(unsigned *)dur_;
    recorder::recorder_t::impl_t *impl = (recorder::recorder_t::impl_t *)impl_;

    if(impl->beat_age_==0)
    {
        pic::msg() << "can't record, no clock" << pic::log;
        return RECORD_ERR_NO_CLOCK;
    }

    if(impl->state_==RECORDER_RUNNING)
    {
        pic::msg() << "can't record, recording in progress" << pic::log;
        return RECORD_ERR_IN_PROG;
    }

    impl->duration_ = duration;
    impl->state_ = RECORDER_ARMING;
    pic::msg() << "recorder is arming" << pic::log;
    return RECORD_OK;
}

void recorder::recorder_t::impl_t::assign_ids()
{
    datamap_t::const_iterator i,e;

    i = datawires_.current().begin();
    e = datawires_.current().end();

    unsigned id=1;

    for(; i != e; ++i)
    {
        i->second->id_ = id++;
    }
}

int recorder::recorder_t::impl_t::record(unsigned duration)
{
    assign_ids();
    maker_.new_recording(signals_+1,datawires_.current().size());
    int r=piw::tsd_fastcall(armer__,this,&duration);
    return r;
}

recorder::recorder_t::recorder_t(piw::clockdomain_ctl_t *d, unsigned s) : impl_(new impl_t(this,d,s)) {}
recorder::recorder_t::~recorder_t() { delete impl_; }
void recorder::recorder_t::abort() { impl_->abort(); }
int recorder::recorder_t::record(unsigned duration) { return impl_->record(duration); }
piw::cookie_t recorder::recorder_t::cookie() { return impl_->decoder_.cookie(); }

