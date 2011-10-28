
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

#include "pia_buffer.h"
#include "pia_glue.h"
#include "pia_data.h"
#include <picross/pic_fastalloc.h>

#include <piagent/pia_network.h>
#include <pibelcanto/link.h>

#include <stdlib.h>
#include <string.h>

#define PTRADD(p,o) (((unsigned char *)(p))+(o))

pia_buffer_t::pia_buffer_t(pia::manager_t::impl_t *glue, const pia_data_t &addr, unsigned hlen): header_(hlen), slowloop_(glue->mainq()), slowbuffer_(0), slowused_(hlen), slowactive_(0), slowsocket_(glue->allocator()), fastloop_(glue->mainq()), fastbuffer_(0), fastused_(hlen), fastactive_(0), fastsocket_(glue->allocator()), glue_(glue)
{
    PIC_ASSERT(header_<BCTLINK_MAXPAYLOAD);

    pia_data_t fqa = glue->expand_address(addr);

    slowsocket_.open(glue->network(),BCTLINK_NAMESPACE_SLOW,fqa);
    fastsocket_.open(glue->network(),BCTLINK_NAMESPACE_FAST,fqa);
}

pia_buffer_t::~pia_buffer_t()
{
    slowsocket_.close();
    fastsocket_.close();
    slowjob_transmit_.cancel();
    fastjob_transmit_.cancel();

    slow_buffer_release();
    fast_buffer_release();
}

void pia_buffer_t::buffer_enable()
{
    slowsocket_.callback(slowloop_,buffer_receive_slow_thunk,this);
    fastsocket_.callback(fastloop_,buffer_receive_fast_thunk,this);
}

void pia_buffer_t::buffer_disable()
{
    slowsocket_.cancel();
    fastsocket_.cancel();
    slowjob_transmit_.cancel();
    fastjob_transmit_.cancel();
}

inline void pia_buffer_t::slow_buffer_prepare()
{
    if(!slowbuffer_)
    {
        slowbuffer_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_NORMAL,glue_->allocator(),BCTLINK_MAXPAYLOAD);
        slowused_ = header_;
    }
}

inline void pia_buffer_t::slow_buffer_release()
{
    if(slowbuffer_ && !slowactive_)
    {
        pic::nb_free(slowbuffer_);
        slowbuffer_ = 0;
    }
}

inline void pia_buffer_t::fast_buffer_prepare()
{
    if(!fastbuffer_)
    {
        fastbuffer_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_NORMAL,glue_->allocator(),BCTLINK_MAXPAYLOAD);
        fastused_ = header_;
    }
}

inline void pia_buffer_t::fast_buffer_release()
{
    if(fastbuffer_ && !fastactive_)
    {
        pic::nb_free(fastbuffer_);
        fastbuffer_ = 0;
    }
}

/*
static void stdio_writer(void *a, char ch)
{
    fputc(ch,(FILE *)a);
}
*/

void pia_buffer_t::buffer_receive_slow_thunk(void *b_, const pia_data_t &d)
{
    pia_buffer_t *b = (pia_buffer_t *)b_;
    b->buffer_receive_slow(d.hostdata(),d.hostlen());
}

void pia_buffer_t::buffer_transmit_slow_thunk(void *b_, const pia_data_t &d)
{
    ((pia_buffer_t *)b_)->buffer_flush_slow();
}

void pia_buffer_t::buffer_flush_slow(bool force)
{
    if(slowbuffer_)
    {
        if(slowused_>header_)
        {
            buffer_fixup_slow(slowbuffer_,slowused_);
            slowsocket_.write(slowbuffer_,slowused_);
            slow_buffer_release();
        }
        else
        {
            if(force)
            {
                buffer_fixup_slow(slowbuffer_,header_);
                slowsocket_.write(slowbuffer_,header_);
                slow_buffer_release();
            }
        }
    }
    slowused_ = header_;
}

unsigned char *pia_buffer_t::buffer_begin_transmit_slow(unsigned len)
{
    PIC_ASSERT(len<=BCTLINK_MAXPAYLOAD-header_);

    slowactive_++;
    slow_buffer_prepare();

    if(slowused_+len > BCTLINK_SMALLPAYLOAD)
    {
        buffer_flush_slow();
    }

    unsigned char *m = &slowbuffer_[slowused_];
    slowused_+=len;
    slowjob_transmit_.idle(slowloop_,buffer_transmit_slow_thunk,this,pia_data_t());

    return m;
}

void pia_buffer_t::buffer_end_transmit_slow()
{
    slowactive_--;
}

void pia_buffer_t::buffer_receive_fast_thunk(void *b_, const pia_data_t &d)
{
    pia_buffer_t *b = (pia_buffer_t *)b_;
    b->buffer_receive_fast(d.hostdata(),d.hostlen());
}

void pia_buffer_t::buffer_transmit_fast_thunk(void *b_, const pia_data_t &)
{
    ((pia_buffer_t *)b_)->buffer_flush_fast();
}

void pia_buffer_t::buffer_flush_fast()
{
    if(fastbuffer_)
    {
        if(fastused_>header_)
        {
            buffer_fixup_fast(fastbuffer_,fastused_);
            fastsocket_.write(fastbuffer_,fastused_);
            fast_buffer_release();
        }
    }
    fastused_ = header_;
}

unsigned char *pia_buffer_t::buffer_begin_transmit_fast(unsigned len,bool autoflush)
{
    PIC_ASSERT(len<=BCTLINK_MAXPAYLOAD-header_);

    fastactive_++;
    fast_buffer_prepare();

    if(fastused_+len > BCTLINK_SMALLPAYLOAD)
    {
        buffer_flush_fast();
    }

    unsigned char *m = &fastbuffer_[fastused_];
    fastused_+=len;

    if(autoflush)
        fastjob_transmit_.idle(fastloop_,buffer_transmit_fast_thunk,this,pia_data_t());

    return m;
}

void pia_buffer_t::buffer_end_transmit_fast()
{
    fastactive_--;
}
