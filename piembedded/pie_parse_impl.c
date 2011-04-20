
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


#include <pibelcanto/state.h>
#include "pie_parse.h"
#include "pie_message.h"
#include "pie_wire.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

void pie_skipspace(piu_input_t r, void *ra)
{
    int ch;

    while((ch=r(ra,1)) >= 0 && (ch==' ' || ch=='\t'))
    {
        r(ra,0);
    }
}

static int __ReadHexChar(piu_input_t r, void *ra)
{
    int ch;

    ch=r(ra,1);

    if(ch>='0' && ch<='9') 
    {
        r(ra,0);
        return ch-'0';
    }

    if(ch>='a' && ch<='f') 
    {
        r(ra,0);
        return ch-'a'+10;
    }

    if(ch>='A' && ch<='F') 
    {
        r(ra,0);
        return ch-'A'+10;
    }

    return -1;
}

static int __ReadHexByte(piu_input_t r, void *ra)
{
    int d1,d2;

    if((d1=__ReadHexChar(r,ra))>=0)
    {
        if((d2=__ReadHexChar(r,ra))>=0)
        {
            return d1*16+d2;
        }
    }

    return -1;
}

int pie_parsestring2(unsigned char *buf,unsigned size,piu_input_t r, void *ra,const char *term, const char *incl)
{
    int ch;
    unsigned l=0;

    while((ch=r(ra,1))>=0)
    {
        if(term)
        {
            if(strchr(term,ch))
            {
                break;
            }
        }

        if(incl)
        {
            if(!strchr(incl,ch))
            {
                break;
            }
        }

        r(ra,0);

        if(ch=='&')
        {
            if((ch=__ReadHexByte(r,ra)) < 0)
            {
                return -1;
            }
        }

        if(l>=size)
        {
            return -1;
        }

        buf[l++]=ch;
    }

    return l;
}

int pie_parsebuffer(unsigned char *buf,unsigned size,piu_input_t r, void *ra)
{
    int d1,d2;
    unsigned l=0;

    while((d1=__ReadHexChar(r,ra))>=0)
    {
        if((d2=__ReadHexChar(r,ra))<0)
        {
            return -1;
        }

        if(l>=size)
        {
            return -1;
        }

        buf[l++]=d1*16+d2;
    }

    return l;
}

int pie_parsepath(unsigned char *buf,unsigned size,piu_input_t r, void *ra)
{
    int ch,d;
    unsigned l=-1;

    pie_skipspace(r,ra);

    if(((ch=r(ra,1))>='0' && ch<='9') || ch=='.' || ch==':')
    {
        l=0;

        while(((ch=r(ra,1))>='0' && ch<='9') || ch=='.' || ch==':')
        {
            if(r(ra,1)=='.' || r(ra,1)==':')
            {
                r(ra,0);
                continue;
            }

            d=0;

            while((ch=r(ra,1)) >='0' && ch<='9') 
            {
                d=d*10+ch-'0';
                r(ra,0);
            }

            if(l>=size)
            {
                return -1;
            }

            buf[l++]=d;
        }
    }

    return l;
}

int pie_parseaddress(char *server, unsigned slen, unsigned char *path, unsigned plen, piu_input_t r, void *ra)
{
    int x;

    pie_skipspace(r,ra);

    if((x=pie_parsestring2((unsigned char *)server,slen-1,r,ra,"#",0))<0)
    {
        return -1;
    }

    server[x]=0;

    pie_skipspace(r,ra);

    if(r(ra,1)!='#')
    {
        path[0]=0;
        return 0;
    }

    r(ra,0);

    if((x=pie_parsepath(path,plen-1,r,ra))<0)
    {
        return -1;
    }

    path[x]=0;
    return x;
}

int pie_parsestring(unsigned char *buf, unsigned size, piu_input_t r, void *ra)
{
    int x,q=0;

    pie_skipspace(r,ra);

    if(r(ra,1)=='"')
    {
        r(ra,0);
        q=1;
    }

    if((x=pie_parsestring2(buf,size,r,ra,q?"\"":" \t\r\n\f",0))<0)
    {
        return -1;
    }

    if(q)
    {
        if(r(ra,1)!='"')
        {
            return -1;
        }

        r(ra,0);
    }

    return x;
}

#ifdef __cplusplus
}
#endif

