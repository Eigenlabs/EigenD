
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
#define strncasecmp _strnicmp
#endif

#include <pibelcanto/state.h>
#include <pibelcanto/link.h>
#include <piembedded/pie_message.h>
#include <piembedded/pie_string.h>
#include <piagent/pia_realnet.h>
#include <piagent/pia_fastalloc.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

int getpath(const char *s, unsigned char *b, unsigned l)
{
    pie_strreader_t c;
    pie_readstr_init(&c,s,strlen(s));
    return pie_parsepath(b,l,pie_readstr,&c);
}

static int str2type(const char *str)
{
    unsigned l = strlen(str);

    if(l==0)
    {
        return -1;
    }

    if(l<=4 && !strncasecmp(str,"null",l)) return BCTVTYPE_NULL;
    if(l<=4 && !strncasecmp(str,"path",l)) return BCTVTYPE_PATH;
    if(l<=6 && !strncasecmp(str,"string",l)) return BCTVTYPE_STRING;
    if(l<=5 && !strncasecmp(str,"double",l)) return BCTVTYPE_DOUBLE;
    if(l<=5 && !strncasecmp(str,"float",l)) return BCTVTYPE_FLOAT;
    if(l<=3 && !strncasecmp(str,"int",l)) return BCTVTYPE_INT;
    if(l<=4 && !strncasecmp(str,"bool",l)) return BCTVTYPE_BOOL;
    if(l<=4 && !strncasecmp(str,"blob",l)) return BCTVTYPE_BLOB;

    long v;
    char *p;

    v=strtol(str,&p,0);

    if(p!=str+l)
    {
        return -1;
    }

    return (int)v;
}

/*
static std::string escape(const char *s)
{
    std::string ret; ret.reserve(64);
    while(*s)
    {
        switch(*s)
        {
            case ' ': ret += "&20"; break;
            default:  ret += *s;
        }
        s++;
    }
    return ret;
}
*/

int main(int ac, char **av)
{
    if(ac!=5)
    {
        fprintf(stderr,"usage: dset server path encoding data\n");
        return -1;
    }

    unsigned char path[255];
    int pathlen=getpath(av[2],path,sizeof(path));

    if(pathlen<0)
    {
        fprintf(stderr,"invalid path\n");
        return -1;
    }

    int type;

    if((type=str2type(av[3]))<0)
    {
        fprintf(stderr,"data: can't parse type %s\n",av[3]);
        return -1;
    }

    /*
    unsigned char msg[1024],buf[256];
    int msglen,datlen;

    datlen=0;
    std::string s = escape(av[4]);

    if(type!=BCTVTYPE_NULL && (datlen=pie_str2data(type,buf,sizeof(buf),s.c_str(),s.size()))<0)
    {
        fprintf(stderr,"data: can't parse %s as type %d\n",av[4],type);
        return -1;
    }

    msglen=0;
    msglen+=pie_setheader(msg+msglen,sizeof(msg)-msglen,0,0,0,0,0);
    msglen+=pie_setstanza(msg+msglen,sizeof(msg)-msglen,BCTMTYPE_DATA_SET,path,pathlen,0);
    msglen+=pie_setdata(msg+msglen,sizeof(msg)-msglen,0,0,type,datlen,buf);

    pia::fastalloc_t alloc;
    pia::realnet_t net(&alloc,false);
    void *h;
    int err;

    if(!(h=net.network_open(BCTLINK_NAMESPACE_SLOW,av[1],true)))
    {
        fprintf(stderr,"net: can't open %s\n",av[1]);
        return -1;
    }

    if((err=net.network_write(h,msg,msglen)) < 0)
    {
        fprintf(stderr,"net: can't write to %s: %d\n",av[1],errno);
    }

    net.network_close(h);
    */

    return 0;
}
