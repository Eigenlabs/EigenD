
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

#include <pibelcanto/state.h>

#include "pie_print.h"
#include "pie_message.h"
#include "pie_wire.h"

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

static const char *__PiHexDigits = "0123456789abcdef";

static void __WriteHex(piu_output_t w, void *wa, int h)
{
    w(wa,__PiHexDigits[(h)/16]);
    w(wa,__PiHexDigits[(h)%16]);
}

static void __WriteHex16(piu_output_t w, void *wa, unsigned short v)
{
    w(wa,__PiHexDigits[(v>>12)&0x0f]);
    w(wa,__PiHexDigits[(v>>8)&0x0f]);
    w(wa,__PiHexDigits[(v>>4)&0x0f]);
    w(wa,__PiHexDigits[(v)&0x0f]);
}

static void __WriteHex32(piu_output_t w, void *wa, unsigned long v)
{
    w(wa,__PiHexDigits[(v>>28)&0x0f]);
    w(wa,__PiHexDigits[(v>>24)&0x0f]);
    w(wa,__PiHexDigits[(v>>20)&0x0f]);
    w(wa,__PiHexDigits[(v>>16)&0x0f]);
    w(wa,__PiHexDigits[(v>>12)&0x0f]);
    w(wa,__PiHexDigits[(v>>8)&0x0f]);
    w(wa,__PiHexDigits[(v>>4)&0x0f]);
    w(wa,__PiHexDigits[(v)&0x0f]);
}

void pie_printbuffer(const unsigned char *buf,unsigned size,piu_output_t w, void *wa)
{
    while(size>0)
    {
        __WriteHex(w,wa,*buf);
        buf++; size--;
    }
}

void pie_printstring(const char *buf,unsigned size,piu_output_t w, void *wa)
{
    while(size>0)
    {
        if(*buf<32 || *buf>=127)
        {
            w(wa,'&');
            __WriteHex(w,wa,*buf);
        }
        else
        {
            w(wa,*buf);
        }

        buf++; size--;
    }
}

void pie_printdict(const unsigned char *buf,unsigned size,piu_output_t w, void *wa)
{
    unsigned l=0;

    w(wa,'{');

    while(size>0)
    {
        uint16_t kl;
		uint16_t vl;
        
		if(pie_getu16(buf+l,size,&kl)<0)
            break;

        l+=2; size-=2;


        if(pie_getu16(buf+l,size,&vl)<0)
            break;

        l+=2; size-=2;

        if(size<(kl+vl))
            break;

        pie_printstring((const char *)buf+l,kl,w,wa);
        l+=kl; size-=kl;

        w(wa,':');

        pie_print(vl,buf+l,w,wa);
        l+=vl; size-=vl;

        if(size>0)
            w(wa,',');
    }

    w(wa,'}');
}

static void __WriteU64(uint64_t v,piu_output_t w, void *wa)
{
    uint64_t d1 = v/10;

    if(d1!=0)
    {
        __WriteU64(d1,w,wa);
    }

    w(wa,__PiHexDigits[v%10]);
}

static void __WriteInt0(long v,piu_output_t w, void *wa)
{
    long d1 = v/10;

    if(d1!=0)
    {
        __WriteInt0(d1,w,wa);
    }

    w(wa,__PiHexDigits[v%10]);
}

static void __WriteInt(long v,piu_output_t w, void *wa)
{
    if(v<0)
    {
        v=-v;
        w(wa,'-');
    }
    __WriteInt0(v,w,wa);
}

static void __WriteFloat(double v,piu_output_t w, void *wa)
{
    if(v<0)
    {
        v=-v;
        w(wa,'-');
    }

    {
		double i = floor(v);
		double f = v-i;

		__WriteInt0((long)i,w,wa);

		w(wa,'.');

		do
		{
			f=f*10.0;
			{
				int q = (int)f;
				f=f-q;
				w(wa,__PiHexDigits[q]);
			}
		} while(f>0);
	}
}

static void __WritePath(const unsigned char *buf, unsigned chaff, unsigned size, piu_output_t w, void *wa)
{
    unsigned i;

    if(size==0)
    {
        w(wa,'.');
        return;
    }

    if(chaff>size)
    {
        chaff=size;
    }

    if(chaff>0)
    {
        for(i=0;i<chaff;i++)
        {
            if(i>0) w(wa,'.');
            __WriteInt(buf[i],w,wa);
        }

        w(wa,':');
    }

    if(chaff==size)
    {
        w(wa,'.');
        return;
    }

    for(i=chaff;i<size;i++)
    {
        if(i>chaff) w(wa,'.');
        __WriteInt(buf[i],w,wa);
    }
}

static void __WriteWire(unsigned len, const unsigned char *buf, piu_output_t w, void *wa, int full)
{
    int32_t vl;
    float vf;
    double vd;

    unsigned short type,sl,vecl;
    float ub,lb,rb;

    if(len<25)
    {
        return;
    }

    type=buf[0];
    pie_getu16(&buf[1],2,&sl);
    pie_getu16(&buf[3],2,&vecl);
    pie_getf32(&buf[13],4,&ub);
    pie_getf32(&buf[17],4,&lb);
    pie_getf32(&buf[21],4,&rb);

    if(len<25+4*vecl+sl)
    {
        return;
    }

    {
		//const unsigned char *vp = &buf[25];
		const unsigned char *sp = &buf[25+4*vecl];
		
		switch(type&0x0f)
		{
			case BCTVTYPE_INT:
				if(pie_get32(sp,sl,&vl)>=0)
				{
					__WriteInt(vl,w,wa);
				}
				break;
			case BCTVTYPE_STRING:
				pie_printstring((const char *)sp,sl,w,wa);
				break;
			case BCTVTYPE_DOUBLE:
				if(pie_getf64(sp,sl,&vd)>=0)
				{
					__WriteFloat(vd,w,wa);
				}
				break;
			case BCTVTYPE_FLOAT:
				if(pie_getf32(sp,sl,&vf)>=0)
				{
					__WriteFloat(vf,w,wa);
				}
				break;
			case BCTVTYPE_PATH:
				__WritePath(sp+1,*sp,sl-1,w,wa);
				break;
			case BCTVTYPE_BOOL:
				w(wa,(*sp)?'y':'n');
				break;
			case BCTVTYPE_BLOB:
				pie_printbuffer(sp,sl,w,wa);
				break;
			case BCTVTYPE_DICT:
				pie_printdict(sp,sl,w,wa);
				break;
			case BCTVTYPE_NULL:
				break;
		}
	}

    if(full)
    {
        unsigned i;
        float f;
        uint64_t t;

        pie_getu64(&buf[5],8,&t);

        w(wa,'<');
        __WriteFloat(lb,w,wa);
        w(wa,',');
        __WriteFloat(ub,w,wa);
        w(wa,',');
        __WriteFloat(rb,w,wa);
        w(wa,'>');
        w(wa,' ');
        w(wa,'@');
        __WriteU64(t,w,wa);
        w(wa,' ');

        for(i=0;i<vecl;i++)
        {
            pie_getf32(&buf[25+4*i],4,&f);
            if(i>0) w(wa,',');
            __WriteFloat(f,w,wa);
        }
    }
}

void pie_print(unsigned len, const void *buf, piu_output_t w, void *wa)
{
    __WriteWire(len,buf,w,wa,0);
}

void pie_printfull(unsigned len, const void *buf, piu_output_t w, void *wa)
{
    __WriteWire(len,buf,w,wa,1);
}

void pie_printmsg(const void *m_, unsigned l, int nl, piu_output_t w, void *wa)
{
    uint16_t cookie;
    uint32_t sseq,nseq,dseq,tseq,tseq2;
    int o;
    unsigned bt,pl;
    const unsigned char *p;
    const unsigned char *m = (const unsigned char *)m_;
    unsigned ml=l;

    if((o=pie_getheader(m,l,&cookie,&sseq,&dseq,&nseq,&tseq,&tseq2))<0)
    {
        return;
    }

    m+=o; l-=o;

    if(cookie!=0)
    {
        pie_printstring("server ",7,w,wa);
        __WriteHex16(w,wa,cookie); w(wa,' ');
        __WriteHex32(w,wa,sseq); w(wa,' ');
        __WriteHex32(w,wa,dseq); w(wa,' ');
        __WriteHex32(w,wa,nseq); w(wa,' ');
        __WriteHex32(w,wa,tseq); w(wa,' ');
        __WriteHex32(w,wa,tseq2); w(wa,' ');
    }
    else
    {
        pie_printstring("client ",7,w,wa);
    }

    __WriteInt(ml,w,wa);

    w(wa,nl?'\n':' ');

    while((o=pie_getstanza(m,l,&bt,&p,&pl))>0)
    {   
        m+=o; l-=o;

        switch(bt)
        {
            case BCTMTYPE_DATA_EVT:
                pie_printstring(" devt ",6,w,wa);
                break;
            case BCTMTYPE_TREE_EVT:    
                pie_printstring(" tevt ",6,w,wa);
                break;
            case BCTMTYPE_TREE_REQ:
                pie_printstring(" treq ",6,w,wa);
                break;
            case BCTMTYPE_DATA_REQ:
                pie_printstring(" dreq ",6,w,wa);
                break;
            case BCTMTYPE_IDNT_EVT:    
                pie_printstring(" ievt ",6,w,wa);
                break;
            case BCTMTYPE_IDNT_REQ:
                pie_printstring(" ireq ",6,w,wa);
                break;
            case BCTMTYPE_FAST_EVT:    
                pie_printstring(" fevt ",6,w,wa);
                break;
            case BCTMTYPE_FAST_REQ:
                pie_printstring(" freq ",6,w,wa);
                break;
            default:
                return;
        }

        __WritePath(p,0,pl,w,wa);

        if(bt==BCTMTYPE_DATA_EVT || bt==BCTMTYPE_TREE_EVT)
        {
            if((o=pie_getevthdr(m,l,&dseq,&nseq,&tseq))<0)
            {
                return;
            }

            m+=o; l-=o;

            w(wa,' '); __WriteHex32(w,wa,dseq);
            w(wa,' '); __WriteHex32(w,wa,nseq);
            w(wa,' '); __WriteHex32(w,wa,tseq);
        }

        if(bt==BCTMTYPE_TREE_EVT)
        {
            unsigned char n;

            while((o=pie_gettevtpath(m,l,&n,&tseq))>0)
            {
                m+=o; l-=o;

                if(!n)
                {
                    break;
                }

                w(wa,' ');
                __WriteInt(n,w,wa);
                w(wa,'=');
                __WriteHex32(w,wa,tseq);
            }
        }

        if(bt==BCTMTYPE_DATA_EVT || bt==BCTMTYPE_FAST_EVT || bt==BCTMTYPE_IDNT_EVT)
        {
            unsigned df;
            unsigned short dl;
            const unsigned char *dp;

            if((o=pie_getdata(m,l,&df,&dl,&dp))<0)
            {
                return;
            }

            m+=o; l-=o;

            w(wa,'=');
            pie_printfull(dl,dp,w,wa);
            w(wa,':');
            __WriteHex(w,wa,df);
        }

        if(nl) w(wa,'\n');
    }
}

#ifdef __cplusplus
}
#endif

