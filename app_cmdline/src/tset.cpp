
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

#include <piagent/pia_fastalloc.h>
#include <piagent/pia_realnet.h>
#include <piembedded/pie_message.h>
#include <piembedded/pie_string.h>
#include <pibelcanto/state.h>
#include <pibelcanto/link.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int getpath(const char *s, unsigned char *b, unsigned l)
{
    pie_strreader_t c;
    pie_readstr_init(&c,s,strlen(s));
    return pie_parsepath(b,l,pie_readstr,&c);
}

int main(int ac, char **av)
{
    unsigned char path[255],msg[1024];
    int msglen,pathlen=0;
    pia::fastalloc_t alloc;
    pia::realnet_t net(&alloc,false);
    void *h;
    unsigned n;

    if(ac<3 || (pathlen=getpath(av[2],path,sizeof(path)))<0)
    {
        fprintf(stderr,"usage: tset server path [...]\n");
        return -1;
    }

    const char *agent = av[1];
    av+=3; ac-=3;

    if(!(h=net.network_open(BCTLINK_NAMESPACE_SLOW,agent,true)))
    {
        fprintf(stderr,"net: can't open %s\n",av[1]);
        return -1;
    }

    msglen=0;
    msglen+=pie_setheader(msg+msglen,sizeof(msg)-msglen,0,0,0,0,0,0);
    msglen+=pie_setstanza(msg+msglen,sizeof(msg)-msglen,BCTMTYPE_TREE_SET,path,pathlen,0);

    while(ac>0)
    {
        n=atoi(av[0]);

        if(n<1 || n>255)
        {
            fprintf(stderr,"path components must be 1-255\n");
            return -1;
        }

        msglen+=pie_settsetpath(msg+msglen,sizeof(msg)-msglen,n);
        av++; ac--;
    }

    msglen+=pie_setlastpath(msg+msglen,sizeof(msg)-msglen);

    if(net.network_write(h,msg,msglen) < 0)
    {
        fprintf(stderr,"net: can't write to %s\n",agent);
    }

    net.network_close(h);
    return 0;
}
