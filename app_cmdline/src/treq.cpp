
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

#include <piagent/pia_realnet.h>
#include <piagent/pia_fastalloc.h>
#include <piembedded/pie_message.h>
#include <piembedded/pie_string.h>
#include <pibelcanto/state.h>
#include <pibelcanto/link.h>

#include <stdio.h>
#include <string.h>

#ifdef PI_WINDOWS
#include <conio.h>
#endif

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

    if(ac!=3 || (pathlen=getpath(av[2],path,sizeof(path)))<0 )
    {
        fprintf(stderr,"usage: treq server path\n");
        return -1;
    }

    if(!(h=net.network_open(BCTLINK_NAMESPACE_SLOW,av[1],true)))
    {
        fprintf(stderr,"net: can't open %s\n",av[1]);
        return -1;
    }

    msglen=0;
    msglen+=pie_setheader(msg+msglen,sizeof(msg)-msglen,0,0,0,0,0,0);
    msglen+=pie_setstanza(msg+msglen,sizeof(msg)-msglen,BCTMTYPE_TREE_REQ,path,pathlen,0);

    if(net.network_write(h,msg,msglen) < 0)
    {
        fprintf(stderr,"net: can't write to %s\n",av[1]);
    }

    net.network_close(h);
	
#ifdef PI_WINDOWS
	_getch();
#endif

    return 0;
}
