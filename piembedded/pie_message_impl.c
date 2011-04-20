
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
#include "pie_message.h"
#include "pie_wire.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <math.h>

unsigned pie_headerlen()
{
  return 22;
}

int pie_setheader(unsigned char *b, unsigned l, uint16_t cookie, uint32_t sseq,uint32_t dseq, uint32_t nseq, uint32_t tseq, uint32_t tseq2)
{
    if(l<22)
    {
        return -1;
    }

    pie_setu32(&b[0],4,nseq);
    pie_setu32(&b[4],4,tseq);
    pie_setu32(&b[8],4,dseq);
    pie_setu32(&b[12],4,sseq);
    pie_setu32(&b[16],4,tseq2);
    pie_setu16(&b[20],2,cookie);

    return 22;
}

int pie_getheader(const unsigned char *b, unsigned l, uint16_t *cookie, uint32_t *sseq,uint32_t *dseq, uint32_t *nseq, uint32_t *tseq, uint32_t *tseq2)
{
    if(l<22)
    {
        return -1;
    }

    pie_getu32(&b[0],4,nseq);
    pie_getu32(&b[4],4,tseq);
    pie_getu32(&b[8],4,dseq);
    pie_getu32(&b[12],4,sseq);
    pie_getu32(&b[16],4,tseq2);
    pie_getu16(&b[20],2,cookie);

    return 22;
}

int pie_setevthdr(unsigned char *b, unsigned l, uint32_t dseq, uint32_t nseq, uint32_t tseq)
{
    if(l<12)
    {
        return -1;
    }

    pie_setu32(&b[0],4,nseq);
    pie_setu32(&b[4],4,tseq);
    pie_setu32(&b[8],4,dseq);

    return 12;
}

int pie_getevthdr(const unsigned char *b, unsigned l, uint32_t *dseq, uint32_t *nseq, uint32_t *tseq)
{
    if(l<12)
    {
        return -1;
    }

    pie_getu32(&b[0],4,nseq);
    pie_getu32(&b[4],4,tseq);
    pie_getu32(&b[8],4,dseq);

    return 12;
}

int pie_setstanza(unsigned char *b, unsigned l, unsigned bt, const unsigned char *pref, unsigned pl, unsigned char suffix)
{
    int sl;

    sl=suffix?1:0;

    if(l<pl+sl+2)
    {
        return -1;
    }

    b[0]=bt;
    b[1]=pl+sl;

    memcpy(&b[2],pref,pl);

    if(suffix)
    {
        b[2+pl]=suffix;
    }

    return pl+sl+2;
}

unsigned pie_stanzalen_req(unsigned pl, unsigned char suffix)
{
    return (pl+2)+(suffix?1:0);
}

int pie_getstanza(const unsigned char *b, unsigned l, unsigned *bt, const unsigned char **pref, unsigned *pl)
{
    if(l<2)
    {
        return -1;
    }

    *bt=b[0];
    *pl=b[1];

    if(l<2+*pl)
    {
        return -1;
    }

    *pref=&b[2];

    return 2+*pl;
}

unsigned pie_stanzalen_tevt(unsigned pl, unsigned paths, unsigned char suffix)
{
    return 12+(2+pl+5*paths+1)+(suffix?1:0);
}

unsigned pie_stanzalen_tset(unsigned pl, unsigned paths, unsigned char suffix)
{
    return (2+pl+1*paths+1)+(suffix?1:0);
}

int pie_settsetlist(unsigned char *b, unsigned l, unsigned dl, const void *dp)
{
    if(l<dl+1)
    {
        return -1;
    }

    memcpy(b,dp,dl);
    b[dl]=0;

    return dl+1;
}

int pie_settsetpath(unsigned char *b, unsigned l, unsigned char path)
{
    if(l<1)
    {
        return -1;
    }

    b[0]=path;

    return 1;
}

int pie_settevtpath(unsigned char *b, unsigned l, unsigned char path, uint32_t nseq)
{
    if(l<5)
    {
        return -1;
    }

    b[0]=path;
    pie_setu32(&b[1],4,nseq);

    return 5;
}

int pie_gettsetpath(const unsigned char *b, unsigned l, unsigned char *path)
{
    if(l<1)
    {
        return -1;
    }

    *path=b[0];
    return 1;
}

int pie_gettsetlist(const unsigned char *b, unsigned l, const unsigned char **dp, unsigned *lp)
{
    const unsigned char *p = b;

    if(l==0)
    {
        return -1;
    }

    while(*p)
    {
        p++;
    }

    *dp=b;
    *lp=p-b;

    return (p-b)+1;
}

int pie_gettevtpath(const unsigned char *b, unsigned l, unsigned char *path, uint32_t *nseq)
{
    if(l<1)
    {
        return -1;
    }

    *path=b[0];

    if(*path==0)
    {
        return 1;
    }

    if(l<5)
    {
        return -1;
    }

    pie_getu32(&b[1],4,nseq);

    return 5;
}

int pie_setlastpath(unsigned char *b, unsigned l)
{
    if(l<1)
    {
        return -1;
    }

    b[0]=0;
    return 1;
}

int pie_getdata(const unsigned char *b, unsigned l, unsigned *df, uint16_t *dl, const unsigned char **dp)
{
    if(l<3)
    {
        return -1;
    }

    *df=b[0];
    pie_getu16(&b[1],2,dl);

    if(l<3+*dl)
    {
        return -1;
    }

    *dp = &b[3];

    return 3+*dl;
}

unsigned pie_stanzalen_dset(unsigned pl, unsigned dl, unsigned char suffix)
{
    return (2+pl+dl+3)+(suffix?1:0);
}

unsigned pie_stanzalen_devt(unsigned pl, unsigned dl, unsigned char suffix)
{
    return 12+(2+pl+dl+3)+(suffix?1:0);
}

unsigned pie_stanzalen_fevt(unsigned pl, unsigned dl, unsigned char suffix)
{
    return (2+pl+dl+3)+(suffix?1:0);
}

unsigned pie_datalen(unsigned dl)
{
    return dl+3;
}

int pie_setdata(unsigned char *b, unsigned l, unsigned df, uint16_t dl, const void *dp)
{
    if(l<dl+3)
    {
        return -1;
    }

    b[0]=df;
    pie_setu16(&b[1],2,dl);

    memcpy(&b[3],dp,dl);

    return 3+dl;
}

int pie_skiptset(const unsigned char *msg, unsigned len)
{
    unsigned char path;
    int used,x;

    used=0;

    for(;;)
    {
        if((x=pie_gettsetpath(msg,len,&path))<0)
        {
            return -1;
        }
    
        msg+=x; len-=x; used+=x;
        if(!path) break;
    }

    return used;
}

int pie_skiptevt(const unsigned char *msg, unsigned len)
{
    unsigned char path;
    uint32_t seq;
    int used,x;

    used=0;

    if((x=pie_skipevthdr(msg,len))<0)
    {
        return -1;
    }

    msg+=x; len-=x; used+=x;

    for(;;)
    {
        if((x=pie_gettevtpath(msg,len,&path,&seq))<0)
        {
            return -1;
        }
    
        msg+=x; len-=x; used+=x;
        if(!path) break;
    }

    return used;
}

int pie_skipdevt(const unsigned char *msg, unsigned len)
{
    int used,x;

    used=0;

    if((x=pie_skipevthdr(msg,len))<0)
    {
        return -1;
    }

    msg+=x; len-=x; used+=x;

    if((x=pie_skipdata(msg,len))<0)
    {
        return -1;
    }

    msg+=x; len-=x; used+=x;
    return used;
}

int pie_skipevthdr(const unsigned char *msg, unsigned len)
{
    if(len<12)
    {
        return -1;
    }

    return 12;
}

int pie_skipdset(const unsigned char *msg, unsigned len)
{
    unsigned df;
    uint16_t dl;
    const unsigned char *dp;
    return pie_getdata(msg,len,&df,&dl,&dp);
}

int pie_skipdata(const unsigned char *msg, unsigned len)
{
    unsigned df;
    uint16_t dl;
    const unsigned char *dp;
    return pie_getdata(msg,len,&df,&dl,&dp);
}

int pie_skipstanza(const unsigned char *b, unsigned l, unsigned char bt)
{
    switch(bt)
    {
        case BCTMTYPE_DATA_REQ: return 0;
        case BCTMTYPE_TREE_REQ: return 0;
        case BCTMTYPE_DATA_EVT: return pie_skipdevt(b,l);
        case BCTMTYPE_TREE_EVT: return pie_skiptevt(b,l);
    }

    return -1;
}

int pie_setindex(unsigned char *b, unsigned l, uint16_t cookie, uint16_t dl, const unsigned char *dp)
{
    if(l<4+dl)
    {
        return -1;
    }

    pie_setu16(&b[0],2,cookie);
    pie_setu16(&b[2],2,dl);

    memcpy(&b[4],dp,dl);

    return 4+dl;
}

int pie_getindex(const unsigned char *b, unsigned l, uint16_t *cookie, uint16_t *dl, const unsigned char **dp)
{
    if(l<4)
    {
        return -1;
    }

    pie_getu16(&b[0],2,cookie);
    pie_getu16(&b[2],2,dl);

    if(l<4+*dl)
    {
        return -1;
    }

    *dp = &b[4];
    return 4+*dl;
}

int pie_getrpc(const unsigned char *b, unsigned l, const unsigned char **p, unsigned *pl, unsigned *bt, uint64_t *cookie, unsigned *nl, const unsigned char **np, int *st, uint16_t *dl, const unsigned char **dp)
{
    if(l<14)
    {
        return -1;
    }

    *bt=b[0];
    *pl=b[1];
    *st=((int)(b[2]))-1;
    pie_getu16(&b[3],2,dl);
    pie_getu64(&b[5],8,cookie);

    *nl = b[13];
    *np = &b[14]+*pl+*dl;

    if(l<14+*pl+*dl+*nl)
    {
        return -1;
    }

    *p = &b[14];
    *dp = &b[14+*pl];

    return 14+*dl+*pl+*nl;
}

int pie_setrpc(unsigned char *b, unsigned l, const unsigned char *p, unsigned pl, unsigned bt, const uint64_t *cookie, unsigned nl, const unsigned char *np, int st, uint16_t dl, const void *dp)
{
    if(l<dl+pl+nl+14)
    {
        return -1;
    }

    b[0]=bt;
    b[1]=pl;
    b[2]=1+st;
    pie_setu16(&b[3],2,dl);
    pie_setu64(&b[5],8,*cookie);

    b[13]=nl;
    memcpy(&b[14+pl+dl],np,nl);

    memcpy(&b[14],p,pl);
    memcpy(&b[14+pl],dp,dl);

    return 14+dl+pl+nl;
}

#ifdef __cplusplus
}
#endif

