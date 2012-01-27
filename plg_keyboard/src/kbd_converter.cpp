
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

#include "kbd_converter.h"
#include <lib_alpha2/alpha2_active.h>
#include <pibelcanto/plugin.h>
#include <lib_samplerate/lib_samplerate.h>
#include <picross/pic_log.h>
#include <piw/piw_resampler.h>

#define PERIOD_US_44 11609
#define PERIOD_US_48 10666
#define PERIOD_US_96 5333

#define BUFFER_SIZE_FRAMES 8192

struct kbd::converter_t::impl_t: virtual pic::lckobject_t
{
    impl_t(unsigned us): us_(us)
    {
    }

    virtual ~impl_t()
    {
    }

    virtual bool write(const float *buffer, unsigned bs) = 0;
    virtual void read(void (*consumer)(void *ctx,const float *b, unsigned dl, unsigned period), void *ctx) = 0;

    unsigned us_;
};

namespace
{
    struct generic_converter_t: kbd::converter_t::impl_t
    {
        generic_converter_t(unsigned q): kbd::converter_t::impl_t(PERIOD_US_48), resampler_("headphones",2,q)
        {
        }

        bool write(const float *buffer, unsigned bs)
        {
            resampler_.resampler_write_interleaved(buffer,bs);
            return false;
        }

        void read(void (*consumer)(void *ctx,const float *b, unsigned dl, unsigned period), void *ctx)
        {
            float buffer[512*2];

            unsigned obs = resampler_.resampler_read_interleaved(512,buffer);

            if(obs)
            {
                consumer(ctx,buffer,512,AUDIO_RATE_48);
            }
        }

        piw::resampler_t resampler_;
    };

    struct direct_converter_t: kbd::converter_t::impl_t
    {
        direct_converter_t(): kbd::converter_t::impl_t(0), buffer_len_(0)
        {
        }

        bool write(const float *buffer, unsigned bs)
        {
            if(buffer_len_+bs > BUFFER_SIZE_FRAMES)
            {
                pic::logmsg() << "conversion buffer overflow " << buffer_len_;
                buffer_len_ = 0;
            }

            memcpy(&buffer_[buffer_len_*2],buffer,bs*2*sizeof(float));
            buffer_len_ += bs;
            return true;
        }

        const float *check(unsigned bs)
        {
            return (bs<=buffer_len_)?buffer_:0;
        }

        void subtract(unsigned bs)
        {
            if(bs<=buffer_len_)
            {
                memcpy(&buffer_[0],&buffer_[bs*2],(buffer_len_-bs)*2*sizeof(float));
                buffer_len_ -= bs;
            }
        }

        float buffer_[PLG_CLOCK_BUFFER_SIZE*2];
        unsigned buffer_len_;
    };
    
    struct null_converter_t: direct_converter_t
    {
        void read(void (*consumer)(void *ctx,const float *b, unsigned dl, unsigned period), void *ctx)
        {
            const float *buffer;

            if((buffer=check(512))!=0)
            {
                consumer(ctx,buffer,512,AUDIO_RATE_48);
                subtract(512);
            }
        }

    };

    struct decimating_converter_t: direct_converter_t
    {
        decimating_converter_t():
            h0(8192/16384.0f),h1(5042/16384.0f),h3(-1277/16384.0f),
            h5(429/16384.0f),h7(-116/16384.0f),h9(18/16384.0f)
        {
            lR1=lR2=lR3=lR4=lR5=lR6=lR7=lR8=lR9=0.0f;
            rR1=rR2=rR3=rR4=rR5=rR6=rR7=rR8=rR9=0.0f;
        }

        void read(void (*consumer)(void *ctx,const float *b, unsigned dl, unsigned period), void *ctx)
        {
            const float *buffer;

            if(!(buffer=check(512)))
            {
                return;
            }

            for(unsigned i=0; i<512/2; ++i)
            {
                float lx0=buffer[4*i+0];
                float rx0=buffer[4*i+1];

                float lx1=buffer[4*i+2];
                float rx1=buffer[4*i+3];

                float lh9x0=h9*lx0;
                float rh9x0=h9*rx0;

                float lh7x0=h7*lx0;
                float rh7x0=h7*rx0;

                float lh5x0=h5*lx0;
                float rh5x0=h5*rx0;

                float lh3x0=h3*lx0;
                float rh3x0=h3*rx0;

                float lh1x0=h1*lx0;
                float rh1x0=h1*rx0;

                float lR10=lR9+lh9x0;
                float rR10=rR9+rh9x0;

                lR9=lR8+lh7x0;
                rR9=rR8+rh7x0;

                lR8=lR7+lh5x0;
                rR8=rR7+rh5x0;

                lR7=lR6+lh3x0;
                rR7=rR6+rh3x0;

                lR6=lR5+lh1x0;
                rR6=rR5+rh1x0;

                lR5=lR4+lh1x0+h0*lx1;
                rR5=rR4+rh1x0+h0*rx1;

                lR4=lR3+lh3x0;
                rR4=rR3+rh3x0;

                lR3=lR2+lh5x0;
                rR3=rR2+rh5x0;

                lR2=lR1+lh7x0;
                rR2=rR1+rh7x0;

                lR1=lh9x0;
                rR1=rh9x0;

                obuffer_[2*i+0]=lR10;
                obuffer_[2*i+1]=rR10;
            }

            consumer(ctx,obuffer_,512/2,AUDIO_RATE_96);
            subtract(512);
        }

        float lR1,lR2,lR3,lR4,lR5,lR6,lR7,lR8,lR9;
        float rR1,rR2,rR3,rR4,rR5,rR6,rR7,rR8,rR9;
        const float h0,h1,h3,h5,h7,h9;
        float obuffer_[PLG_CLOCK_BUFFER_SIZE*3];

    };

    struct resampling_converter_t: direct_converter_t
    {
        resampling_converter_t(unsigned q)
        {
            int e;
            src_state_ = src_new(((q<=4)?(4-q):4),2,&e);
            src_data_.end_of_input = 0;
            src_data_.src_ratio = 48000.0/44100.0;
        }

        ~resampling_converter_t()
        {
            src_delete(src_state_);
        }

        void read(void (*consumer)(void *ctx,const float *b, unsigned dl, unsigned period), void *ctx)
        {
            const float *buffer;

            if(!(buffer=check(512)))
            {
                return;
            }

            src_data_.input_frames = 512;
            src_data_.output_frames = 512*3/2;
            src_data_.data_in = (float *)buffer;
            src_data_.data_out = obuffer_;
            src_process(src_state_, &src_data_);

            consumer(ctx,obuffer_,src_data_.output_frames_gen,AUDIO_RATE_44);
            subtract(512);
            return;
        }

        SRC_STATE *src_state_;
        SRC_DATA src_data_;
        float obuffer_[512*3];
    };

    static bool is_dc(unsigned bs)
    {
        if(bs>512) return false;
        if(512%bs==0) return true;
        return false;
    }

    kbd::converter_t::impl_t *create_converter(unsigned long sr, unsigned bs, unsigned q)
    {
        if(is_dc(bs))
        {
            if(sr == 48000)
            {
                return new null_converter_t();
            }

            if(sr == 96000)
            {
                return new decimating_converter_t();
            }

            if(sr == 44100)
            {
                return new resampling_converter_t(q);
            }
        }

        return new generic_converter_t(q);
    }
};


kbd::converter_t::converter_t()
{
    impl_.set(0);
}

kbd::converter_t::~converter_t()
{
    impl_t *c = impl_.current();
    if(c)
    {
        impl_.set(0);
        delete c;
    }
}

unsigned long kbd::converter_t::reset(unsigned long sr, unsigned bs, unsigned q)
{
    impl_t *o = impl_.current();
    impl_t *n = create_converter(sr,bs,q);

    impl_.set(n);

    if(o) delete o;

    return n->us_;
}

bool kbd::converter_t::write(const float *buffer, unsigned bs)
{
    pic::flipflop_t<impl_t *>::guard_t g(impl_);
    if(!g.value()) return false;
    return g.value()->write(buffer,bs);
}


void kbd::converter_t::read(void (*consumer)(void *ctx,const float *interleaved, unsigned dl, unsigned period), void *ctx)
{
    pic::flipflop_t<impl_t *>::guard_t g(impl_);
    if(!g.value()) return;
    g.value()->read(consumer,ctx);
}

