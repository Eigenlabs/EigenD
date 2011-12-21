
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

#include <piagent/pia_udpnet.h>
#include <piagent/pia_fastalloc.h>
#include <picross/pic_time.h>

namespace
{
    struct portecho_t
    {
        portecho_t(pia::udpnet_t *net, unsigned spc): net_(net), handle_(0), spc_(spc)
        {
            if(!(handle_=net_->network_open(spc_,"<nettest>",false)))
            {
                return;
            }

            if(net_->network_callback(handle_,handler,this)<0)
            {
                return;
            }
        }

        ~portecho_t()
        {
            if(handle_)
            {
                net_->network_close(handle_);
            }
        }

        static void handler(void *ctx, const unsigned char *msg, unsigned len)
        {
            portecho_t *self = (portecho_t *)ctx;
            //printf("received message spc=%u len=%u\n",self->spc_,len);
            self->net_->network_write(self->handle_,msg,len);
            //printf("send message spc=%u len=%u\n",self->spc_,len);
        }

        pia::udpnet_t *net_;
        void *handle_;
        unsigned spc_;
    };
}

int main(int ac, const char **av)
{
    pia::fastalloc_t alloc;
    pia::udpnet_t net(&alloc,false);

    portecho_t p0(&net,0);
    portecho_t p1(&net,1);
    portecho_t p2(&net,2);
    portecho_t p3(&net,3);
    portecho_t p4(&net,4);
    portecho_t p5(&net,5);

    while(true)
    {
        pic_microsleep(1000000);
    }
}
