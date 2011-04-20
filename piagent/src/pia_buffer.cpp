
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

pia_buffer_t::pia_buffer_t(pia::manager_t::impl_t *glue, const pia_data_t &addr, unsigned hlen): header_(hlen), slowloop_(glue->mainq()), slowsocket_(glue->allocator()), fastloop_(glue->mainq()), fastsocket_(glue->allocator()), glue_(glue)
{
    PIC_ASSERT(header_<BCTLINK_MAXPAYLOAD);

    pia_data_t fqa = glue->expand_address(addr);

    slowsocket_.open(glue->network(),BCTLINK_NAMESPACE_SLOW,fqa);
    fastsocket_.open(glue->network(),BCTLINK_NAMESPACE_FAST,fqa);

    slowbuffer_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_NORMAL,glue_->allocator(),BCTLINK_MAXPAYLOAD);
    fastbuffer_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_NORMAL,glue_->allocator(),BCTLINK_MAXPAYLOAD);
    fastused_=header_;
    slowused_=header_;
}

pia_buffer_t::~pia_buffer_t()
{
    slowsocket_.close();
    fastsocket_.close();
    slowjob_transmit_.cancel();
    fastjob_transmit_.cancel();
    if(slowbuffer_) pic::nb_free(slowbuffer_);
    if(fastbuffer_) pic::nb_free(fastbuffer_);
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
    if(slowused_>header_)
    {
        buffer_fixup_slow(slowbuffer_,slowused_);
        slowsocket_.write(slowbuffer_,slowused_);
    }
    else
    {
        if(force)
        {
            buffer_fixup_slow(slowbuffer_,header_);
            slowsocket_.write(slowbuffer_,header_);
        }
    }

    slowused_=header_;
}

unsigned char *pia_buffer_t::buffer_transmit_slow(unsigned len)
{
    PIC_ASSERT(len<=BCTLINK_MAXPAYLOAD-header_);

    if(slowused_+len > BCTLINK_SMALLPAYLOAD)
    {
        buffer_flush_slow();
    }

    unsigned char *m = &slowbuffer_[slowused_];
    slowused_+=len;
    slowjob_transmit_.idle(slowloop_,buffer_transmit_slow_thunk,this,pia_data_t());

    return m;
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
    if(fastused_>header_)
    {
        buffer_fixup_fast(fastbuffer_,fastused_);
        fastsocket_.write(fastbuffer_,fastused_);
    }

    fastused_=header_;
}

unsigned char *pia_buffer_t::buffer_transmit_fast(unsigned len,bool autoflush)
{
    PIC_ASSERT(len<=BCTLINK_MAXPAYLOAD-header_);

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
