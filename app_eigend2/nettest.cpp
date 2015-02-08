
#include "nettest.h"

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
#include <picross/pic_tool.h>
#include <picross/pic_resources.h>

#include <string.h>

namespace
{
    struct porttester_t
    {
        porttester_t(pia::udpnet_t *net, unsigned spc): net_(net), handle_(0), received_(false), spc_(spc)
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

        ~porttester_t()
        {
            if(handle_)
            {
                net_->network_close(handle_);
            }
        }

        void send()
        {
            unsigned char msg[32];
            unsigned len = sizeof(msg);
            memset(msg,0,len);
            net_->network_write(handle_,msg,len);
            //printf("send message spc=%u len=%u\n",spc_,len);
        }

        static void handler(void *ctx, const unsigned char *msg, unsigned len)
        {
            porttester_t *self = (porttester_t *)ctx;
            //printf("received message spc=%u len=%u\n",self->spc_,len);
            self->received_ = true;
        }

        bool ok() 
        {
            return received_;
        }

        pia::udpnet_t *net_;
        void *handle_;
        bool received_;
        unsigned spc_;
    };
}

bool eigend::test_network()
{
    pic::bgprocess_t echo(pic::private_exe_dir(),"echod");

    echo.start();

    pia::fastalloc_t alloc;
    pia::udpnet_t net(&alloc,false);

    porttester_t p0(&net,0);
    porttester_t p1(&net,1);
    porttester_t p2(&net,2);
    porttester_t p3(&net,3);
    porttester_t p4(&net,4);
    porttester_t p5(&net,5);

    int tries = 30;

    while(tries>0)
    {
        bool sent = false;

        if(!p0.ok()) { p0.send(); sent = true; }
        if(!p1.ok()) { p1.send(); sent = true; }
        if(!p2.ok()) { p2.send(); sent = true; }
        if(!p3.ok()) { p3.send(); sent = true; }
        if(!p4.ok()) { p4.send(); sent = true; }
        if(!p5.ok()) { p5.send(); sent = true; }

        if(!sent)
        {
            return true;
        }

        tries--;
        pic_microsleep(50000);
    }

    return false;
}
