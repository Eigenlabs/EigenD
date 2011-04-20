
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

#include <picross/pic_config.h>
#ifdef PI_WINDOWS
#include <picross/pic_windows.h>
//#define NOMINMAX
//#include <windows.h>
#define sleep Sleep
#endif

#include <iostream>

#include <piembedded/pie_print.h>
#include <piembedded/pie_string.h>
#include <piembedded/pie_message.h>

#include <pibelcanto/link.h>
#include <piagent/pia_realnet.h>
#include <piagent/pia_fastalloc.h>

static void stdio_writer(void *a, char ch)
{
    fputc(ch,(FILE *)a);
}

static void printer(void *ctx, const unsigned char *msg, unsigned len)
{
    int x;
    unsigned short c;
    unsigned short dl;
    const unsigned char *dp;

    printf("msg:");

    while(len>0)
    {
        if((x=pie_getindex(msg,len,&c,&dl,&dp))<0)
        {
            break;
        }

        len-=x; msg+=x;
        pie_print(dl,dp,stdio_writer,(void *)stdout);
    }

    printf("\n");
}

int main(int ac, char **av)
{
    void *h;
    pia::fastalloc_t alloc;
    pia::realnet_t net(&alloc,false);

    if(ac!=2)
    {
        std::cerr << "usage: %s group" << av[0] << "\n";
        return -1;
    }

    if(!(h=net.network_open(BCTLINK_NAMESPACE_INDEX,av[1],false)))
    {
        fprintf(stderr,"net: can't open %s\n",av[1]);
        return -1;
    }

    if(net.network_callback(h,printer,0)<0)
    {
        fprintf(stderr,"net: can't establish callback\n");
        net.network_close(h);
        return -1;
    }

    while(1)
    {
        sleep(10);
    }
}
