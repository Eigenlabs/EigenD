
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

#include <piw/piw_wavrecorder.h>
#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>
#include <piw/piw_clock.h>
#include <picross/pic_resources.h>

#define STOPPED  0
#define STARTUP  1
#define STARTED  2
#define SHUTDOWN 3

namespace
{
    static inline void swap32(unsigned char *p) { std::swap(p[0],p[3]); std::swap(p[1],p[2]); }
    static inline void swap16(unsigned char *p) { std::swap(p[0],p[1]); }

    struct audiobuf_t : pic::lckobject_t, pic::element_t<0>
    {
        audiobuf_t() { memset(this,0,sizeof(audiobuf_t)); }
        float left[PLG_CLOCK_BUFFER_SIZE];
        float right[PLG_CLOCK_BUFFER_SIZE];
        unsigned len;
    };

    struct wavfile_t: pic::lckobject_t, piw::thing_t
    {
        wavfile_t() : file_(0), frames_(0)
        {
            piw::tsd_thing(this);
        }

        ~wavfile_t()
        {
            close_thing();
            close_slow();
        }

        void open_fast(unsigned long sr, const char *filename)
        {
            enqueue_slow_nb(piw::makestring_nb(filename,sr));
        }

        void close_fast()
        {
            enqueue_slow_nb(piw::makenull_nb());
        }

        void thing_dequeue_slow(const piw::data_t &d)
        {
            if(d.is_string())
            {
                open_slow(d.time(),d.as_string());
                timer_slow(500);
                return;
            }

            if(d.is_null())
            {
                cancel_timer_slow();
                close_slow();
                return;
            }
        }

        static int __flip(void *w_, void *)
        {
            wavfile_t *w = (wavfile_t *)w_;
            w->slow_.takeover(w->fast_);
            return 0;
        }

        void thing_timer_slow()
        {
            write_slow();
        }

        void write_slow()
        {
            piw::tsd_fastcall(__flip,this,0);

            audiobuf_t *b;
            while((b=slow_.head()))
            {
                for(unsigned i=0; i<b->len; ++i)
                {
                    __writefloat(b->left[i]);
                    __writefloat(b->right[i]);
                }
                frames_ += b->len;
                delete b;
            }
        }

        void open_slow(unsigned long sr, const char *filename)
        {
            if(file_)
            {
                return;
            }

            pic::logmsg() << "opening " << filename;

            file_ = pic::fopen(filename,"wb");
            
            PIC_ASSERT(file_);
            __write("RIFF",4);
            __write32(0);  // total length of RIFF chunk, filled in on close
            __write("WAVE",4);
            __write("fmt ",4);
            __write32(16); // pcm
            __write16(3);  // data is float32
            __write16(2);  // stereo
            __write32(sr); // sample rate
            __write32(sr*(2*32/8)); // data rate in bytes/sec
            __write16(2*32/8);  // frame size in bytes
            __write16(32);  // sample size in bits
            __write("data",4);
            __write32(0); // data length in bytes, filled in on close
        }
        
        void close_slow()
        {
            if(!file_)
            {
                return;
            }

            write_slow();

            unsigned long len = frames_*2*4;
            pic::logmsg() << "closing, data length is " << len;
            fseek(file_, 40, SEEK_SET);
            __write32(len);
            fseek(file_, 4, SEEK_SET);
            __write32(len+44);
            fclose(file_);
            file_ = 0;
            frames_ = 0;
        }

        void record_silence(unsigned len)
        {
            audiobuf_t *b = new audiobuf_t;
            memset(b->left+b->len,0,len*sizeof(float));
            memset(b->right+b->len,0,len*sizeof(float));
            b->len = len;
            fast_.append(b);
        }

        void record(const float *left, const float *right, unsigned len)
        {
            audiobuf_t *b = new audiobuf_t;
            memcpy(b->left,left,len*sizeof(float));
            memcpy(b->right,right,len*sizeof(float));
            b->len = len;
            fast_.append(b);
        }

        void __write(const void *data, unsigned len)
        {
            if(fwrite(data,len,1,file_)!=1) PIC_THROW("error writing to wav file");
        }

        void __write32(int32_t x)
        {
#ifdef PI_BIGENDIAN
            swap32((unsigned char *)&x);
#endif
            __write(&x,4);
        }

        void __write16(int16_t x)
        {
#ifdef PI_BIGENDIAN
            swap16((unsigned char *)&x);
#endif
            __write(&x,2);
        }

        void __writefloat(float x)
        {
#ifdef PI_BIGENDIAN
            swap32((unsigned char *)&x);
#endif
            __write(&x,4);
        }

        FILE *file_;
        unsigned long frames_;
        pic::ilist_t<audiobuf_t,0> slow_;
        pic::ilist_t<audiobuf_t,0> fast_;
    };
}

struct piw::wavrecorder_t::impl_t: piw::wire_t, piw::clocksink_t, piw::decode_ctl_t, piw::decoder_t, piw::event_data_sink_t, virtual public pic::lckobject_t
{
    impl_t(piw::clockdomain_ctl_t *d) : piw::decoder_t(this), upstream_(0), filename_("audio"), clkdomain_(d), state_(STOPPED), time_(0), listener_(pic::notify_t::method(this,&impl_t::clock_change))
    {
        d->sink(this,"wavrecorder");
        tick_enable(true);
        clock_change();
    }

    ~impl_t()
    {
        clkdomain_->remove_listener(listener_);
        tracked_invalidate();
    }

    void clock_change()
    {
        samplerate_ = clkdomain_->get_sample_rate();
    }

    void event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &eb)
    {
        iterator_ = eb.iterator();
        piw::data_nb_t d;
        if(iterator_->latest(1,d,id.time())) left_=d;
        if(iterator_->latest(2,d,id.time())) right_=d;
    }

    bool event_end(unsigned long long t)
    {
        iterator_.clear();
        return true;
    }

    void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
    {
        iterator_->set_signal(sig,n);
        iterator_->reset(sig,t);
    }

    void set_clock(bct_clocksink_t *c)
    {
        if(upstream_) remove_upstream(upstream_);
        upstream_ = c;
        if(upstream_) add_upstream(upstream_);
    }

    void set_latency(unsigned l)
    {
    }

    void wire_closed()
    {
        unsubscribe();
    }

    piw::wire_t *wire_create(const piw::event_data_source_t &es)
    {
        subscribe(es);
        return this;
    }

    unsigned time2samples(unsigned bs,unsigned long long t)
    {
        int s = (t*samplerate_)/1000000;
        return std::min(bs,(unsigned)s);
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        if(state_==STOPPED)
        {
            return;
        }

        unsigned bs = get_buffer_size();

        if(state_==STARTUP)
        {
            if(time_<=t)
            {
                time_ = std::max(time_,f+1);
                unsigned samples = time2samples(bs,time_-f);
                file_.open_fast(clkdomain_->get_sample_rate(), filename_.c_str());
                record(samples, bs-samples);
                state_ = STARTED;
            }
            return;
        }

        if(state_==STARTED)
        {
            unsigned s;
            piw::data_nb_t d;
            if(iterator_.isvalid())
            {
                while(iterator_->next(3,s,d,t))
                {
                    switch(s)
                    {
                        case 1: left_ = d; break;
                        case 2: right_ = d; break;
                    }
                }
            }
            record(0,bs);
        }

        if(state_==SHUTDOWN)
        {
            if(time_<=t)
            {
                time_ = std::max(time_,f+1);
                unsigned samples = time2samples(bs,time_-f);
                record(0,samples);
                file_.close_fast();
                state_ = STOPPED;
                tick_suppress(true);
                return;
            }
        }
    }

    void record(unsigned start, unsigned len)
    {
        if(left_.is_array() && right_.is_array())
        {
            file_.record(left_.as_array()+start, right_.as_array()+start, len);
            left_ = piw::makenull_nb();
            right_ = piw::makenull_nb();
        }
        else
        {
            file_.record_silence(len);
        }
    }

    bct_clocksink_t *upstream_;
    std::string filename_;
    piw::clockdomain_ctl_t *clkdomain_;
    wavfile_t file_;
    piw::data_nb_t left_, right_;
    unsigned state_;
    unsigned long long time_;
    unsigned long samplerate_;
    pic::notify_t listener_;
    piw::xevent_data_buffer_t::iter_t iterator_;
};

static int __setfilename(void *i_, void *f_)
{
    piw::wavrecorder_t::impl_t *i = (piw::wavrecorder_t::impl_t *)i_;
    const char *f = (const char *)f_;
    i->filename_ = f;
    return 0;
}

namespace
{
    struct rsink_t: piw::change_nb_t::sinktype_t
    {
        rsink_t(piw::wavrecorder_t::impl_t *i) : impl_(i)
        {
        }

        void invoke(const piw::data_nb_t &d) const
        {
            if(d.as_norm()!=0.0)
            {
                if(impl_->state_==STOPPED)
                {
                    pic::logmsg() << "recording startup at " << d.time();
                    impl_->state_ = STARTUP;
                    impl_->time_ = d.time();
                    impl_->tick_suppress(false);
                }
            }
            else
            {
                if(impl_->state_==STARTED)
                {
                    pic::logmsg() << "recording shutdown at " << d.time();
                    impl_->state_ = SHUTDOWN;
                    impl_->time_ = d.time();
                }
            }
        }

        bool iscallable() const
        {
            return impl_.isvalid();
        }

        bool compare(const piw::change_nb_t::sinktype_t *o) const
        {
            const rsink_t *c = dynamic_cast<const rsink_t *>(o);
            return c && c->impl_==impl_;
        }

        pic::weak_t<piw::wavrecorder_t::impl_t> impl_;
    };
}

piw::wavrecorder_t::wavrecorder_t(piw::clockdomain_ctl_t *d): impl_(new impl_t(d)) {}
piw::wavrecorder_t::~wavrecorder_t() { delete impl_; }
void piw::wavrecorder_t::setfile(const char *filename) { piw::tsd_fastcall(__setfilename,impl_,(void *)filename); }
piw::cookie_t piw::wavrecorder_t::cookie() { return impl_->cookie(); }
piw::change_nb_t piw::wavrecorder_t::record() { return piw::change_nb_t(pic::ref(new rsink_t(impl_))); }

