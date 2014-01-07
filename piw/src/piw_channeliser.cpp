

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

#include <piw/piw_channeliser.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_policy.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_thing.h>
#include <piw/piw_aggregator.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_ilist.h>
#include <map>

namespace {
    struct channeliser_input_t;
    struct channeliser_voice_t;

    struct channeliser_driver_t: piw::event_data_source_real_t, pic::element_t<>, piw::wire_ctl_t
    {
        channeliser_driver_t(channeliser_voice_t *voice, const piw::data_t &path);
        ~channeliser_driver_t();

        void claim(channeliser_input_t *owner, const piw::data_t &channel, const piw::data_nb_t &id);
        void event_start(unsigned seq, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void source_ended(unsigned seq);
        void release();

        channeliser_voice_t *voice_;
        channeliser_input_t *owner_;
        piw::dataholder_nb_t id_;
        piw::data_t channel_;
    };

    struct channeliser_relay_t: piw::wire_t, piw::event_data_sink_t, piw::event_data_source_real_t, piw::wire_ctl_t
    {
        channeliser_relay_t(channeliser_voice_t *voice, const piw::event_data_source_t &es);
        ~channeliser_relay_t() { invalidate(); }

        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void source_ended(unsigned seq);
        void invalidate();
        void wire_closed() { delete this; }

        channeliser_voice_t *voice_;
        piw::data_t path_;
        bool active_;
    };

    struct channeliser_voice_t: pic::element_t<>, piw::root_t
    {
        channeliser_voice_t(piw::channeliser_t::impl_t *root, unsigned index);
        ~channeliser_voice_t();

        void add_driver(const piw::data_t &path);
        void release(channeliser_driver_t *driver);

        channeliser_driver_t *claim(channeliser_input_t *owner, const piw::data_t &channel, const piw::data_nb_t &id);
        piw::wire_t *root_wire(const piw::event_data_source_t &es);

        void invalidate();
        void root_latency();
        void root_clock() { relay_root_.set_clock(get_clock()); }
        void root_closed() { invalidate(); }
        void root_opened() { root_clock(); root_latency(); }
        void output_incref();
        void output_decref();
        void input_incref();
        void input_decref();

        piw::channeliser_t::impl_t *root_;
        piw::data_t channel_;
        pic::flipflop_t<pic::lckmap_t<piw::data_t,channeliser_driver_t *>::lcktype> drivers_;
        unsigned refcount_input_,refcount_output_;
        piw::channeliser_t::channel_t *client_;
        pic::lckmap_t<piw::data_t,channeliser_relay_t *>::lcktype children_;
        piw::root_ctl_t driver_root_;
        piw::root_ctl_t relay_root_;
        unsigned index_;
    };

    struct channeliser_input_t: piw::wire_t, piw::event_data_sink_t
    {
        channeliser_input_t(piw::channeliser_t::impl_t *p, const piw::event_data_source_t &es);
        ~channeliser_input_t() { invalidate(); }

        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void source_ended(unsigned seq);

        void wire_closed() { delete this; }
        void invalidate();

        piw::channeliser_t::impl_t *parent_;
        channeliser_driver_t *driver_;
        piw::data_t path_;
    };

};

struct piw::channeliser_t::impl_t: root_t, virtual public pic::tracked_t, piw::thing_t
{
    impl_t(piw::clockdomain_ctl_t *cd, delegate_t *delegate, const cookie_t &output_cookie, unsigned initial_poly, unsigned poly_minfree);
    ~impl_t();

    void thing_trigger_slow();

    channeliser_driver_t *claim(channeliser_input_t *owner, const piw::data_nb_t &id);
    void release(channeliser_voice_t *voice, const piw::data_t &channel);

    piw::wire_t *root_wire(const event_data_source_t &es);
    void invalidate();
    void root_latency();
    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }
    void root_clock();

    delegate_t *delegate_;
    pic::lckmap_t<piw::data_t,channeliser_input_t *>::lcktype children_;
    pic::lckvector_t<channeliser_voice_t *>::lcktype voices_;
    unsigned poly_;
    pic::ilist_t<channeliser_voice_t> freelist_;
    pic::lckmap_t<piw::data_t,channeliser_voice_t *>::lcktype voicemap_;
    unsigned minfree_;
    piw::aggregator_t aggregator_;
    bct_clocksink_t *clock_;
};

piw::channeliser_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd, delegate_t *delegate, const cookie_t &output_cookie, unsigned initial_poly, unsigned poly_minfree): root_t(0), delegate_(delegate), aggregator_(output_cookie,cd), clock_(0)
{
    poly_=initial_poly;
    minfree_=poly_minfree;
    piw::tsd_thing(this);
    thing_trigger_slow();
}

static int add_to_free_voices__(void *impl_, void *voice_)
{
    ((piw::channeliser_t::impl_t *)impl_)->freelist_.append((channeliser_voice_t *)voice_);
    return 0;
}

void piw::channeliser_t::impl_t::thing_trigger_slow()
{
    pic::lckmap_t<piw::data_t,channeliser_input_t *>::lcktype::iterator ci;

    while(voices_.size() < poly_)
    {
        unsigned index = voices_.size();
        channeliser_voice_t *voice = new channeliser_voice_t(this,index);

        for(ci=children_.begin(); ci!=children_.end(); ci++)
        {
            voice->add_driver(ci->first);
        }

        voices_.push_back(voice);
        voice->driver_root_.set_clock(clock_);
        piw::tsd_fastcall(add_to_free_voices__,this,voice);
    }
}

void channeliser_voice_t::add_driver(const piw::data_t &path)
{
    channeliser_driver_t *driver = new channeliser_driver_t(this,path);
    drivers_.alternate().insert(std::make_pair(path,driver));
    drivers_.exchange();
    driver_root_.connect_wire(driver,driver->source());
}

channeliser_driver_t *piw::channeliser_t::impl_t::claim(channeliser_input_t *owner, const piw::data_nb_t &id)
{
    unsigned cl = id.as_pathchannellen();
    unsigned el = id.as_patheventlen();

    piw::data_t channel = piw::makepath(id.as_path(),cl,0);
    piw::data_nb_t event = piw::makepath_nb(id.as_pathevent(),el,id.time());

    pic::lckmap_t<piw::data_t,channeliser_voice_t *>::lcktype::iterator i;
    channeliser_voice_t *v;

    if((i=voicemap_.find(channel))!=voicemap_.end())
    {
        return i->second->claim(owner,channel,event);
    }

    unsigned c=0;
    v = freelist_.head(); while(v) { c++; v=freelist_.next(v); }

    if(c<=minfree_)
    {
        poly_ += 1+(minfree_-c);
        trigger_slow();
        pic::logmsg() << c << " free voices; triggering poly increase";
    }

    if((v=freelist_.head())!=0)
    {
        freelist_.remove(v);
        voicemap_.insert(std::make_pair(channel,v));
        return v->claim(owner,channel,id);
    }

    pic::logmsg() << "no free voices";
    return 0;
}

void piw::channeliser_t::impl_t::release(channeliser_voice_t *voice, const piw::data_t &channel)
{
    voicemap_.erase(channel);
    freelist_.append(voice);
}

piw::channeliser_t::impl_t::~impl_t()
{
    tracked_invalidate();
    invalidate();
}

channeliser_input_t::channeliser_input_t(piw::channeliser_t::impl_t *p, const piw::event_data_source_t &es): parent_(p), driver_(0), path_(es.path())
{
    parent_->children_.insert(std::make_pair(path_,this));
}

void channeliser_input_t::invalidate()
{
    if(parent_)
    {
        disconnect();
        parent_->children_.erase(path_);
        parent_=0;
    }
}

bool channeliser_input_t::event_end(unsigned long long t)
{
    if(driver_)
    {
        if(driver_->event_end(t))
        {
            driver_->release();
            driver_=0;
            return true;
        }

        return false;
    }

    return true;
}

void channeliser_input_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(driver_)
    {
        driver_->event_buffer_reset(sig,t,o,n);
    }
}

void channeliser_input_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    if(!driver_)
    {
        driver_ = parent_->claim(this,id);

        if(!driver_)
        {
            return;
        }

        //pic::logmsg() << "claimed " << driver_->voice_ << " for channel " << driver_->channel_ << " id " << driver_->id_.get();
    }

    driver_->event_start(seq,b);
}

void channeliser_input_t::source_ended(unsigned seq)
{
    if(driver_)
    {
        event_ended(seq);
        driver_->release();
        driver_=0;
    }
}

void piw::channeliser_t::impl_t::invalidate()
{
    tracked_invalidate();

    pic::lckmap_t<piw::data_t,channeliser_input_t *>::lcktype::iterator ci;

    while((ci=children_.begin())!=children_.end())
    {
        delete ci->second;
    }
}

piw::wire_t *piw::channeliser_t::impl_t::root_wire(const event_data_source_t &es)
{
    pic::lckmap_t<piw::data_t,channeliser_input_t *>::lcktype::iterator ci;

    if((ci=children_.find(es.path()))!=children_.end())
    {
        delete ci->second;
    }

    channeliser_input_t *w = new channeliser_input_t(this,es);
    w->subscribe_and_ping(es);

    for(unsigned i=0;i<voices_.size();i++)
    {
        voices_[i]->add_driver(es.path());
    }

    return w;
}

void piw::channeliser_t::impl_t::root_clock()
{
    for(unsigned i=0;i<voices_.size();i++)
    {
        voices_[i]->driver_root_.set_clock(get_clock());
    }
}

void piw::channeliser_t::impl_t::root_latency()
{
}

piw::cookie_t piw::channeliser_t::cookie()
{
    return piw::cookie_t(impl_);
}

piw::channeliser_t::channeliser_t(piw::clockdomain_ctl_t *cd, delegate_t *delegate, const cookie_t &output_cookie, unsigned initial_poly, unsigned poly_minfree): impl_(new impl_t(cd, delegate,output_cookie,initial_poly,poly_minfree))
{
}

piw::channeliser_t::~channeliser_t()
{
    delete impl_;
}

void channeliser_driver_t::release()
{
    voice_->release(this);
    owner_ = 0;
}

void channeliser_driver_t::source_ended(unsigned seq)
{
    if(owner_)
    {
        owner_->source_ended(seq);
    }
}

void channeliser_voice_t::release(channeliser_driver_t *driver)
{
    input_decref();
}

void channeliser_voice_t::output_incref()
{
    refcount_output_++;
    //pic::logmsg() << "voice " << this << " output_incref incount=" << refcount_input_ << " outcount=" << refcount_output_;
}

void channeliser_voice_t::output_decref()
{
    refcount_output_--;
    //pic::logmsg() << "voice " << this << " output_decref incount=" << refcount_input_ << " outcount=" << refcount_output_;

    if(refcount_input_==0 && refcount_output_==0)
    {
        root_->release(this,channel_);
    }
}

void channeliser_voice_t::input_incref()
{
    refcount_input_++;
    //pic::logmsg() << "voice " << this << " input_incref incount=" << refcount_input_ << " outcount=" << refcount_output_;
}

void channeliser_voice_t::input_decref()
{
    refcount_input_--;
    //pic::logmsg() << "voice " << this << " input_decref incount=" << refcount_input_ << " outcount=" << refcount_output_;

    if(refcount_input_==0 && refcount_output_==0)
    {
        root_->release(this,channel_);
    }
}

channeliser_voice_t::~channeliser_voice_t()
{
    tracked_invalidate();
    invalidate();
}

channeliser_voice_t::channeliser_voice_t(piw::channeliser_t::impl_t *root, unsigned index): piw::root_t(0), root_(root), refcount_input_(0), refcount_output_(0), index_(index)
{
    relay_root_.connect(root_->aggregator_.get_filtered_output(1+index_,piw::null_filter()));
    client_ = root_->delegate_->create_channel(piw::cookie_t(this));
    driver_root_.connect(client_->cookie());
}

void piw::channeliser_t::visit_channels(const void *arg)
{
    for(unsigned i=0;i<impl_->voices_.size();i++)
    {
        impl_->voices_[i]->client_->visited(arg);
    }
}

channeliser_relay_t::channeliser_relay_t ( channeliser_voice_t *voice, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), voice_(voice), path_(es.path()), active_(false)
{
    voice_->children_.insert(std::make_pair(path_,this));
}

bool channeliser_relay_t::event_end ( unsigned long long t )
{
    if(!active_)
    {
        return true;
    }

    if(source_end(t))
    {
        //pic::logmsg() << "voice " << voice_ << " relay event_end";
        active_ = false;
        voice_->output_decref();
        return true;
    }

    return false;
}

void channeliser_relay_t::source_ended(unsigned seq)
{
    if(active_)
    {
        event_ended(seq);
        //pic::logmsg() << "voice " << voice_ << " relay source_ended";
        active_ = false;
        voice_->output_decref();
    }
}

void channeliser_relay_t::event_buffer_reset ( unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n )
{
    source_buffer_reset(sig,t,o,n);
}

void channeliser_relay_t::event_start ( unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b )
{
    unsigned cl = voice_->channel_.as_pathlen();
    const unsigned char *cp = voice_->channel_.as_path();


    piw::data_nb_t newid(id);

    for(int i=cl-1; i>=0; i--)
    {
        newid = piw::pathprepend_nb(newid,cp[i]);
    }

    //pic::logmsg() << "voice " << voice_ << " relay event_start";

    if(!active_)
    {
        active_ = true;
        voice_->output_incref();
    }

    source_start(seq,newid,b);
}

void channeliser_relay_t::invalidate()
{
    if(voice_)
    {
        piw::wire_t::disconnect();
        piw::wire_ctl_t::disconnect();
        voice_->children_.erase(path_);
        voice_=0;
    }
}

piw::wire_t *channeliser_voice_t::root_wire(const piw::event_data_source_t &es)
{
    pic::lckmap_t<piw::data_t,channeliser_relay_t *>::lcktype::iterator ci;

    if((ci=children_.find(es.path()))!=children_.end())
    {
        delete ci->second;
    }

    channeliser_relay_t *w = new channeliser_relay_t(this,es);
    relay_root_.connect_wire(w,w->source());
    w->subscribe_and_ping(es);
    return w;
}

void channeliser_voice_t::invalidate()
{
    tracked_invalidate();

    root_->aggregator_.clear_output(1+index_);

    pic::lckmap_t<piw::data_t,channeliser_relay_t *>::lcktype::iterator ci;

    while((ci=children_.begin())!=children_.end())
    {
        delete ci->second;
    }

    index_ = 0;
}

void channeliser_voice_t::root_latency()
{
}

channeliser_driver_t *channeliser_voice_t::claim(channeliser_input_t *owner, const piw::data_t &channel, const piw::data_nb_t &id)
{
    pic::lckmap_t<piw::data_t,channeliser_driver_t *>::lcktype::const_iterator di;
    pic::flipflop_t<pic::lckmap_t<piw::data_t,channeliser_driver_t *>::lcktype>::guard_t g(drivers_);

    if((di=g.value().find(owner->path_)) == g.value().end())
    {
        return 0;
    }

    channeliser_driver_t *driver = di->second;
    channel_ = channel;
    driver->claim(owner,channel,id);
    input_incref();
    return driver;
}

channeliser_driver_t::channeliser_driver_t(channeliser_voice_t *voice, const piw::data_t &path): piw::event_data_source_real_t(path), voice_(voice), owner_(0)
{
}

void channeliser_driver_t::claim(channeliser_input_t *owner, const piw::data_t &channel, const piw::data_nb_t &id)
{
    owner_ = owner;
    id_.set_nb(id);
    channel_ = channel;
}

void channeliser_driver_t::event_start(unsigned seq, const piw::xevent_data_buffer_t &b)
{
    source_start(seq,id_.get(),b);
}

bool channeliser_driver_t::event_end(unsigned long long t)
{
    return source_end(t);
}

void channeliser_driver_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    source_buffer_reset(sig,t,o,n);
}

channeliser_driver_t::~channeliser_driver_t()
{
}
