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

#ifndef __PIA_SRC_NETWORK__
#define __PIA_SRC_NETWORK__

#include <piagent/pia_network.h>
#include <pibelcanto/link.h>

#include "pia_eventq.h"
#include "pia_data.h"

class pia_sockref_t: pic::nocopy_t
{
    public:
        pia_sockref_t(pic::nballocator_t *a): allocator_(a), socket_(0), queue_(0) {}
        ~pia_sockref_t() { close(); }

        void open(pia::network_t *n, unsigned space, const pia_data_t &name)
        {
            PIC_ASSERT(name.asstringlen() <= BCTLINK_GROUP_SIZE);
            close();
            network_=n;
            socket_=network_->network_open(space,name.asstring(),true);
        }

        void close() { if(socket_) { cancel(); network_->network_close(socket_); socket_=0; } }
        void callback(pia_eventq_t *q, void (*cb)(void *, const pia_data_t &),void *a) { cancel(); cpoint_=pia_make_cpoint(); queue_=q, callback_=cb, context_=a; network_->network_callback(socket_,socket_callback,this); }
        void cancel() { if(!queue_) return; network_->network_callback(socket_,0,0); cpoint_->disable(); queue_=0; }
        void write(const void *msg, unsigned len) { network_->network_write(socket_,msg,len); }

    private:

        static void socket_callback(void *s_, const unsigned char *msg, unsigned len)
        {
            pia_sockref_t *s = (pia_sockref_t *)s_;
            unsigned char *dp;
            pia_data_t d = pia_data_t::allocate_buffer(s->allocator_,len,&dp);
            memcpy(dp,msg,len);
            s->queue_->idle(s->cpoint_,s->callback_,s->context_,d);
        }
        
    private:
        pic::nballocator_t *allocator_;
        void *socket_;
        pia::network_t *network_;
        pia_eventq_t *queue_;
        void (*callback_)(void *, const pia_data_t &);
        void *context_;
        pia_cref_t cpoint_;
};

#endif
