
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

#include <piw/piw_capture.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_config.h>
#include <picross/pic_resources.h>
#include <fstream>

#define SAMPLE_INCR 21

namespace
{
    struct event_record_t: public pic::atomic_counted_t, public piw::fastdata_t
    {
        event_record_t(const piw::data_nb_t &id, unsigned sig, unsigned ord): fastdata_t(PLG_FASTDATA_SENDER), id_(id), signal_(sig), ordinal_(ord)
        {
            piw::tsd_fastdata(this);
            enable(true,false,false);
        }

        void dump_wav(const char *prefix)
        {
            char buffer[128];
            sprintf(buffer,"%s.%u.wav",prefix,ordinal_);

            file_ = pic::fopen(buffer,"wb");
            __write("RIFF",4);
            __write32(0);  // total length of RIFF chunk, filled in on close
            __write("WAVE",4);
            __write("fmt ",4);
            __write32(16); // pcm
            __write16(3);  // data is float32
            __write16(1);  // stereo
            __write32(48000); // sample rate
            __write32(48000*(32/8)); // data rate in bytes/sec
            __write16(32/8);  // frame size in bytes
            __write16(32);  // sample size in bits
            __write("data",4);
            __write32(0); // data length in bytes, filled in on close

            pic::lcklist_t<piw::data_nb_t>::nbtype::iterator i;
            unsigned long len=0;
            for(i=buffer_.begin();i!=buffer_.end();i++)
            {
                piw::data_nb_t d(*i);
                if(d.is_array())
                {
                    unsigned al = d.as_arraylen();
                    const float *ad = d.as_array();

                    for(unsigned i=0;i<al;i++)
                    {
                        float f = ad[i];
                        __writefloat(f);
                        len += 4;
                    }
                }
            }

            pic::logmsg() << "closing, data length is " << len;
            fseek(file_, 40, SEEK_SET);
            __write32(len);
            fseek(file_, 4, SEEK_SET);
            __write32(len+44);
            fclose(file_);
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

        void dump(const char *prefix,bool abs)
        {
            char buffer[128];
            bool first = true;
            bool clipw = false;
            unsigned long long idt = id_.time();
            unsigned long index = 0;

            sprintf(buffer,"%s.%u",prefix,ordinal_);
            pic::logmsg() << buffer;

            std::fstream output(buffer,std::ios_base::out|std::ios_base::binary);

            output << "# " << id_ << "\n";

            pic::lcklist_t<piw::data_nb_t>::nbtype::iterator i;

            for(i=buffer_.begin();i!=buffer_.end();i++)
            {
                piw::data_nb_t d(*i);
                unsigned long long t = d.time();

                output << "# " << t << ' ' << t-idt << "\n";

                if(d.is_array())
                {
                    unsigned al = d.as_arraylen();
                    const float *ad = d.as_array();

                    for(unsigned i=0;i<al;i++)
                    {
                        if(abs)
                            output << t << ' ' << ad[i] << "\n";
                        else
                            output << index++ << ' ' << ad[i] << "\n";

                        if(first)
                        {
                            first=false;
                            if(ad[i]>0.999) pic::logmsg() << "event " << buffer << " starts with " << ad[i];
                        }

                        if(!clipw)
                        {
                            if(ad[i]>0.999) 
                            {
                                clipw=true;
                                pic::logmsg() << "event " << buffer << " clipped ";
                            }
                        }

                        t += SAMPLE_INCR;
                    }
                }
            }
        }

        bool fastdata_receive_event(const piw::data_nb_t &id, const piw::dataqueue_t &q)
        {
            ping(id.time(),q);
            return true;
        }

        bool fastdata_receive_data(const piw::data_nb_t &d)
        {
            buffer_.push_back(d);
            return true;
        }

        piw::data_nb_t id_;
        unsigned signal_;
        unsigned ordinal_;

        pic::lcklist_t<piw::data_nb_t>::nbtype buffer_;
        FILE *file_;
    };

    struct btof_wire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t
    {
        btof_wire_t(unsigned signal_,const piw::event_data_source_t &es, piw::capture_backend_t::impl_t *i);
        ~btof_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);

        piw::data_t path_;
        piw::capture_backend_t::impl_t *root_;
        unsigned signal_;
        pic::ref_t<event_record_t> record_;
    };

}

struct piw::capture_backend_t::impl_t: piw::decode_ctl_t
{
    impl_t(const std::string &name, unsigned signal, bool abs, bool wav): signal_(signal), decoder_(this), count_(1), name_(name), abs_(abs), wav_(wav)
    {
    }

    ~impl_t()
    {
    }

    void set_clock(bct_clocksink_t *c) { }
    void set_latency(unsigned l) {}

    wire_t *wire_create(const piw::event_data_source_t &es)
    {
        std::map<piw::data_t, btof_wire_t *>::iterator wi =  wires_.find(es.path());

        if(wi != wires_.end())
            delete wi->second;

        return new btof_wire_t(signal_,es,this);
    }

    unsigned signal_;
    std::map<piw::data_t, btof_wire_t *> wires_;
    piw::decoder_t decoder_;
    unsigned count_;
    std::string name_;
    bool abs_;
    bool wav_;
};

btof_wire_t::btof_wire_t(unsigned signal,const piw::event_data_source_t &es, piw::capture_backend_t::impl_t *i) : path_(es.path()), root_(i), signal_(signal)
{
    root_->wires_.insert(std::make_pair(path_,this));
    subscribe(es);
}

void btof_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    record_=pic::ref(new event_record_t(id,signal_,root_->count_++));
    record_->send_fast(id,b.signal(signal_));
}

void btof_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s==signal_)
        record_->send_fast(current_id(),n);
}

bool btof_wire_t::event_end(unsigned long long t)
{
    if(root_->wav_)
        record_->dump_wav(root_->name_.c_str());
    else
        record_->dump(root_->name_.c_str(),root_->abs_);
    record_.clear();
    return true;
}

void btof_wire_t::invalidate()
{
    unsubscribe();
    if(root_)
    {
        root_->wires_.erase(path_);
        root_ = 0;
    }
}

piw::capture_backend_t::capture_backend_t(const std::string &name, unsigned signal,bool abs,bool wav) : impl_(new impl_t(name,signal,abs,wav)) {}
piw::capture_backend_t::~capture_backend_t() { delete impl_; }
piw::cookie_t piw::capture_backend_t::cookie() { return impl_->decoder_.cookie(); }
