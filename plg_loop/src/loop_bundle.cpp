
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

#include <picross/pic_log.h>
#include <picross/pic_ilist.h>
#include <picross/pic_weak.h>
#include <piw/piw_clock.h>
#include <piw/piw_clockclient.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_thing.h>

#include "loop_bundle.h"
#include "loop_slice.h"
#include "loop_mod.h"

#define MINSLICE 3000
#define MAXCHANNELS 8

#define STOP 0
#define PLAY 1
#define ONCE 2
#define TOGGLE 3

namespace
{
    struct channel_t : pic::element_t<>, virtual public pic::lckobject_t
    {
        channel_t(unsigned sig): signal_(sig),scalar_(0),out_(0)
        {
        }

        void begin(unsigned bs,unsigned long long time)
        {
            data_ = piw::makenorm_nb(time, bs, &out_,&scalar_);
            memset(out_, 0, bs*sizeof(float));
        }

        void send(unsigned bs,piw::xevent_data_buffer_t &eb, float v)
        {
            for(unsigned i=0;i<bs;i++)
            {
                out_[i]*=v;
            }

            *scalar_=out_[bs-1];
            eb.add_value(signal_,data_);
        }

        unsigned signal_;
        float *scalar_;
        float *out_;
        piw::data_nb_t data_;
    };
}

namespace loop
{
    struct player_t::impl_t : piw::root_t, piw::wire_t, piw::clocksink_t, piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, piw::event_data_sink_t, piw::thing_t, virtual pic::tracked_t, virtual public pic::lckobject_t
    {
        impl_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d,const pic::status_t &s): piw::root_t(0), piw::event_data_source_real_t(piw::pathnull(0)), up_(0), bmod_(0), interp_(1), transport_(false), playing_(STOP), awake_(false), volume_(1.0), clockdomain_(d), status_(s)
        {
            connect(c);
            loaded_.set(false);
            d->sink(this,"loop");
            set_clock(this);
            piw::tsd_thing(this);
            //tick_enable(false);
            tick_enable(true);
            fading_ = false;
            current_beat_ = -1;
            sr_ = 48000.f;
            bps_ = 2.f;
            fpb_ = sr_/bps_;
            current_ = 0;
            next_ = 0;
            beatoffset_ = 0.f;
            current_crossfade_ = 0.f;
            chop_ = 10.f;
            d->add_listener(pic::notify_t::method(this,&impl_t::clock_changed));
            clock_changed();
        }

        ~impl_t()
        {
            tracked_invalidate();
            close_thing();
            invalidate();
        }

        void root_closed() { invalidate(); }
        void wire_closed() { invalidate(); }
        void source_ended(unsigned seq) {}

        void set_volume(float v)
        {
            if(v<0.0) v=0.0;
            volume_=v;
        }

        float get_volume()
        {
            return volume_;
        }

        void invalidate()
        {
            source_shutdown();
            tick_disable();
            unsubscribe();
            channel_t *ch;
            while((ch=channels_.head()))
            {
                delete ch;
            }

            slice_t *s;
            while((s=slices_.head()))
            {
                delete s;
            }
        }

        void root_clock()
        {
            bct_clocksink_t *c(get_clock());

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

        void root_latency()
        { 
            set_sink_latency(get_latency());
            set_latency(get_latency());
        }

        void root_opened()
        {
            root_clock(); root_latency();
            connect_wire(this,source());
        }

        piw::wire_t *root_wire(const piw::event_data_source_t  &es)
        {
            subscribe_and_ping(es);
            return this;
        }

        static int __reset(void *i_, void *_)
        {
            impl_t *i = (impl_t *)i_;

            //i->interp_.recv_clock(0,piw::data_nb_t());

            i->current_ = 0;
            i->next_ = 0;
            i->current_beat_ = -1;
            i->current_crossfade_ = 0;

            if(i->awake_)
            {
                i->awake_=false;
                i->source_end(0);
            }

            return 0;
        }

        void unload()
        {
            loaded_.set(false);
            piw::tsd_fastcall(__reset,this,0);
        }

        void load(const char *name)
        {
            loaded_.set(false);
            piw::tsd_fastcall(__reset,this,0);

            try
            {
                loop_init(read_aiff(name,false));
                loaded_.set(true);
            }
            catch(const pic::error &e)
            {
                pic::logmsg() << "unable to load " << name << ": " << e.what();
            }
            catch(...)
            {
                pic::logmsg() << "unable to load " << name;
            }

        }

        void loop_init(const loop::loopref_t &ld)
        {
            channels_init(ld->num_channels());
            slices_init(ld);
            bmod_ = ld->beats();
        }

        void channels_init(unsigned n)
        {
            n = std::max(n,2U);
            channel_t *ch;
            
            while((ch=channels_.head()))
            {
                delete ch;
            }

            for(unsigned c=0; c<n; ++c)
            {
                channels_.append(new channel_t(c+1));
            }

            sigmask_ = (1ULL<<n)-1;
        }

        void slices_init(const loop::loopref_t &ld)
        {
            slice_t *s;

            while((s=slices_.head()))
            {
                delete s;
            }

            unsigned long pos = 0;
            for(unsigned t = 0; t < ld->num_transients(); ++t)
            {
                int len = (ld->transient(t)-pos);
                if(len>=MINSLICE)
                {
                    slices_.append(new slice_t(ld,pos,ld->transient(t)));
                    pos = ld->transient(t);
                }
            }

            if(pos==0)
            {
                int len = (ld->samples()-MINSLICE);
                if(len>=MINSLICE)
                {
                    slices_.append(new slice_t(ld,0,len));
                    pos = len;
                }
            }

            if(pos==0)
            {
                pic::msg() << "loop too short to be sliced" << pic::hurl;
            }

            int len = (ld->samples()-pos);
            if(len>=MINSLICE)
            {
                slices_.append(new slice_t(ld,pos,ld->samples()));
            }
        }

        void thing_dequeue_slow(const piw::data_t &status)
        {
            status_(status.as_bool());
        }

        void change_status(bool playing)
        {
            enqueue_slow_nb(piw::makebool_nb(playing));
        }

        void clocksink_ticked(unsigned long long from,unsigned long long to)
        {
            bool ot = transport_;
            piw::data_nb_t d;

            unsigned bs = get_buffer_size();

            if(clkiter_.isvalid())
            {
                while(clkiter_->nextsig(2,d,to))
                {
                    transport_ = d.as_norm()!=0.f;
                }
            }

            if(!transport_)
            {
                if(ot)
                {
                    __reset(this,0);
                }

                if(awake_)
                {
                    source_end(to);
                    awake_=false;
                    tick_suppress(true);
                }
                return;
            }

            if(clkiter_.isvalid())
            {
                while(clkiter_->nextsig(1,d,to))
                {
                    interp_.recv_clock(0,d);
                }
            }

            if(!interp_.clockvalid(0))
            {
                pic::logmsg() << "clock tick skipped " << from << "-" << to;
                return;
            }

            adjust_fpb();
            double interpolated_beat = interp_.interpolate_clock(0,to);
            float bto = interpolated_beat-beatoffset_;

            if(current_beat_<0)
            {
                if(bto==0.f)
                {
                    current_beat_ = bto;
                    return;
                }
                else
                {
                    current_beat_ = interp_.interpolate_clock(0,from)-beatoffset_;
                }
            }

            if(playing_)
            {
                pic::flipflop_t<bool>::guard_t g(loaded_);

                if(g.value())
                {
                    if(!awake_)
                    {
                        output_=piw::xevent_data_buffer_t(sigmask_,PIW_DATAQUEUE_SIZE_ISO);
                    }

                    chan_begin(bs,to);

                    {
                        pic::flipflop_t<bool>::guard_t g(loaded_);
                        if(g.value())
                        {
                            float bf = fmodf(current_beat_,bmod_);
                            float bt = fmodf(bto,bmod_);
                            render(bs,bf,bt);
                        }
                    }

                    chan_send(bs);

                    if(!awake_)
                    {
                        source_start(0,piw::pathnull_nb(to),output_);
                        awake_=true;
                    }
                }
            }
            else
            {
                if(awake_)
                {
                    source_end(to);
                    awake_=false;
                }
            }

            current_beat_ = bto;
        }

        bool resync(float bf, float bt)
        {
            slice_t *s = slices_.head();
            while(s)
            {
                if(brackets(s->beat_start(),s->beat_end(),bt))
                {
                    current_ = s;
                    float beatadj = FADETIME_SLICE*bps_;
                    current_crossfade_ = back(current_->beat_start(),beatadj,bmod_);
                    current_->reset(decay_);
                    fading_ = false;
                    return true;
                }

                s = slices_.next(s);
            }

            return false;
        }

        void clock_changed()
        {
            sr_ = (float)clockdomain_->get_sample_rate();
            setdecay();
        }

        void render(unsigned bs,float bfrom,float bto)
        {
            unsigned len = bs;
            unsigned offset = 0;

            if(!current_)
            {
                if(!resync(bfrom,bto))
                    return;

                unsigned n = frame_offset(bfrom,bto,current_->beat_start());
                n = std::min(n,len);
                len -= n;
                current_->render(outputs_,n,len,sr_,true);
                return;
            }

            while(true)
            {
                if(fading_)
                {
                    unsigned n = current_->fadeout(outputs_,offset,len,sr_);
                    unsigned nin = next_->fadein(outputs_,offset,len,sr_);
                    PIC_ASSERT(n==nin);

                    len -= n;
                    offset += n;

                    if(len==0)
                        return;
                    else
                        end_crossfade();
                }
                else
                {
                    float beatadj = FADETIME_SLICE*bps_;
                    float next_crossfade = back(current_->beat_end(),beatadj,bmod_);

                    if(brackets(current_crossfade_,next_crossfade,bto))
                    {
                        // not fading
                        current_->render(outputs_,offset,len,sr_,false);
                        return;
                    }

                    fading_ = true;
                    next_ = current_;

                    while(!brackets(current_crossfade_,next_crossfade,bto))
                    {
                        next_ = next(next_);
                        current_crossfade_ = next_crossfade;
                        next_crossfade = back(next_->beat_end(),beatadj,bmod_);
                    }

                    unsigned n = frame_offset(bfrom,bto,current_crossfade_);
                    n = std::min(n,len);
                    current_->render(outputs_,offset,n,sr_,false);
                    len -= n;
                    offset += n;
                    start_crossfade();
                }
            }
        }

        void adjust_fpb()
        {
            // limit and low pass
            float bps_in = std::min(16.0,interp_.get_speed(0)*1000000.0);
            bps_ = bps_in + 0.8*(bps_-bps_in);
            fpb_ = sr_/bps_;
        }

        unsigned frame_offset(float bf, float bt, float b)
        {
            if(brackets(bf,bt,b))
            {
                float beat_offset = distance(bf,b,bmod_);
                return (unsigned)(beat_offset*fpb_);
            }
            return 0;
        }

        void end_crossfade()
        {
            current_ = next_;
            fading_ = false;
        }

        void start_crossfade()
        {
            fading_ = true;
            next_->reset(decay_);
        }

        slice_t *next(slice_t *s)
        {
            s = slices_.next(s);
            if(!s)
            {
                if(playing_==ONCE)
                    playing_ = STOP;
                return slices_.head();
            }
            return s;
        }


        void chan_begin(unsigned bs,unsigned long long t)
        {
            unsigned i = 0;
            for(channel_t *c = channels_.head(); c; c = channels_.next(c), ++i)
            {
                c->begin(bs,t);
                outputs_[i] = c->out_;
            }
        }

        void chan_send(unsigned bs)
        {
            for(channel_t *c = channels_.head(); c; c = channels_.next(c))
            {
                c->send(bs,output_,volume_);
            }
        }

        void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &ei)
        {
            clkiter_=ei.iterator();
            clkiter_->reset(1,id.time());
            clkiter_->reset(2,id.time());
            tick_suppress(false);
        }

        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
        {
            clkiter_->reset(sig,t);
        }

        bool event_end(unsigned long long t)
        {
            __reset(this,0);
            transport_=false;
            clkiter_.clear();
            return true;
        }

        void setdecay()
        {
            piw::tsd_fastcall(__setdecay,this,0);
        }
        
        static int __setdecay(void *i_, void *_)
        {
            loop::player_t::impl_t *i = (loop::player_t::impl_t *)i_;
            i->decay_=1.0-i->chop_/i->sr_;
            //i->decay_=1.0;
            return 0;
        }

        bct_clocksink_t *up_;

        slicelist_t slices_;
        slice_t *current_, *next_;

        pic::ilist_t<channel_t> channels_;
        float *outputs_[MAXCHANNELS];

        pic::flipflop_t<bool> loaded_;
        float bmod_;
        piw::clockinterp_t interp_;
        bool transport_;
        unsigned playing_;
        bool awake_;

        float current_beat_;
        float fpb_;
        float sr_;
        float bps_;
        bool fading_;
        float volume_;
        float beatoffset_;
        float current_crossfade_;

        float chop_;
        float decay_;
        unsigned long long sigmask_;
        piw::xevent_data_buffer_t output_;
        piw::xevent_data_buffer_t::iter_t clkiter_;
        piw::clockdomain_ctl_t *clockdomain_;
        pic::status_t status_;
    };

    struct playsink_t: pic::sink_t<void(const piw::data_t &)>
    {
        playsink_t(player_t::impl_t *i,unsigned p) : impl_(i), play_(p)
        {
        }

        void invoke(const piw::data_t &d) const
        {
            if(d.as_norm()!=0)
            {
                if(play_==ONCE)
                {
                    unsigned long long t = d.time();
                    impl_->beatoffset_ = loop::mod(impl_->interp_.interpolate_clock(0,t),impl_->bmod_);
                    impl_->current_beat_ = -1;
                    impl_->playing_ = play_;
                    impl_->change_status(false);
                }
                else
                {
                    if(play_==TOGGLE)
                    {
                        if(impl_->playing_==PLAY)
                        {
                            impl_->beatoffset_ = 0.f;
                            impl_->playing_ = STOP;
                            impl_->change_status(false);
                        }
                        else
                        {
                            impl_->beatoffset_ = 0.f;
                            impl_->playing_ = PLAY;
                            impl_->change_status(true);
                        }    
                    }
                    else
                    {
                        if(impl_->playing_==play_)
                        {
                            return;
                        }

                        impl_->beatoffset_ = 0.f;
                        impl_->playing_ = play_;
                        impl_->change_status((play_==STOP)?false:true);
                    }
            
                }
                impl_->current_ = 0;
            }
        }

        bool iscallable() const
        {
            return impl_.isvalid();
        }

        bool compare(const pic::sink_t<void(const piw::data_t &)> *o) const
        {
            const playsink_t *c = dynamic_cast<const playsink_t *>(o);
            return c ? c->impl_==impl_ && c->play_==play_ : false;
        }

        pic::weak_t<player_t::impl_t> impl_;
        unsigned play_;
    };

    player_t::player_t(const piw::cookie_t &c,piw::clockdomain_ctl_t *d, const pic::status_t &s) : impl_(new impl_t(c,d,s)) {}
    player_t::~player_t() { delete impl_; }
    piw::cookie_t player_t::cookie() { return piw::cookie_t(impl_); }
    void player_t::load(const char *name) { impl_->load(name); }
    void player_t::unload() { impl_->unload(); }
    void player_t::set_volume(float v) { impl_->set_volume(v); }
    void player_t::set_chop(float c) { impl_->chop_=c; impl_->setdecay(); }
    float player_t::get_volume() { return impl_->get_volume(); }
    piw::change_t player_t::player(unsigned p) { return piw::change_t(pic::ref(new playsink_t(impl_,p))); }
}
