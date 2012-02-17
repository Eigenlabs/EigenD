
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

#include <pibelcanto/plugin.h>
#include <piw/piw_data.h>
#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>
#include <piembedded/pie_wire.h>
#include <piembedded/pie_string.h>
#include <piembedded/pie_iostream.h>
#include <piembedded/pie_print.h>
#include <picross/pic_log.h>
#include <sstream>
#include <limits>

#ifdef DEBUG_DATA_ATOMICITY
void piw::data_atomicity_assertion(bct_data_t d)
{
    if(d && d->tid && !pic_threadid_equal(d->tid, pic_current_threadid()))
    {
        std::stringstream oss;
        oss << "piw_data thread mismatch ";
        oss << d->tid;
        oss << "(data thread) != ";
        oss << pic_current_threadid();
        oss << "(current thread) [";
        pie_print(bct_data_wirelen(d), bct_data_wiredata(d), pie::ostreamwriter, &oss);
        oss << "]";
        PIC_THROW(oss.str().c_str());
    }
}
#endif

static bct_data_t __allocate_host(unsigned nb,unsigned long long ts,float u, float l, float r,unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp)
{
    bct_entity_t e = piw::tsd_getcontext();
    PIC_ASSERT(e);
    bct_data_t d = bct_entity_allocate_host(e,nb,ts,u,l,r,t,dl,dp,vl,vp);
    PIC_ASSERT(d);
    return d;
}

static bct_data_t __allocate_wire(unsigned nb,unsigned dl, const unsigned char *dp)
{
    bct_entity_t e = piw::tsd_getcontext();
    PIC_ASSERT(e);
    bct_data_t d = bct_entity_allocate_wire(e,nb,dl,dp);
    PIC_ASSERT(d);
    return d;
}

template <class T>
static T __makepath(unsigned nb,unsigned long long ts,const unsigned char *v, unsigned l,unsigned c)
{
    unsigned char *dp; T d; float *vv;
    if(c>l) c=l;
    d=T::from_given(__allocate_host(nb,ts,1,0,0,BCTVTYPE_PATH,l+1,&dp,1,&vv));
    *dp=c;
    memcpy(dp+1,v,l); *vv=0;
    return d;
}

template <class T>
static T __parsepath(unsigned nb,const char *path, unsigned long long t)
{
    pie_strreader_t c;
    pie_readstr_init(&c,path,strlen(path));
    unsigned char buf[2048];
    int l;

    if((l=pie_parsepath(buf,sizeof(buf),&pie_readstr,&c))<0)
    {
        pic::msg() << "Can't parse " << path << pic::hurl;
    }

    return __makepath<T>(nb,t,buf,l,0);
}

template <class T>
static T __makearray_ex(unsigned nb,unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs)
{
    T d; unsigned char *dp;
    PIC_ASSERT(lbound<=rest && rest<=ubound);
    d=T::from_given(__allocate_host(nb,ts, ubound, lbound, rest, BCTVTYPE_FLOAT, sizeof(float), &dp, nfloats, pfloat));
    *fs = (float *)dp;
    return d;
}

template <class T>
static T __makelong_bounded_ex(unsigned nb,float ubound, float lbound, float rest, long f, unsigned long long t)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,t,ubound,lbound,rest,BCTVTYPE_INT,sizeof(long),&dp,1,&vv));
    *(long *)dp=f;
    *vv = piw::normalise(ubound,lbound,rest,f);
    return d;
}

template <class T>
static T __makefloat_bounded_ex(unsigned nb,float ubound, float lbound, float rest, float f, unsigned long long t)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,t,ubound,lbound,rest,BCTVTYPE_FLOAT,sizeof(float),&dp,1,&vv));
    *(float *)dp=f;
    *vv = piw::normalise(ubound,lbound,rest,f);
    return d;
}

template <class T>
static T __makedouble_bounded_ex(unsigned nb,float ubound, float lbound, float rest, double f, unsigned long long t)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,t,ubound,lbound,rest,BCTVTYPE_DOUBLE,sizeof(double),&dp,1,&vv));
    *(double *)dp=f;
    *vv = piw::normalise(ubound,lbound,rest,f);
    return d;
}

template <class T>
static T __makefloat_bounded_units_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, float f, unsigned long long t)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,t,ubound,lbound,rest,u|BCTVTYPE_FLOAT,sizeof(float),&dp,1,&vv));
    *(float *)dp=f;
    *vv = piw::normalise(ubound,lbound,rest,f);
    return d;
}

template <class T>
static T __makedouble_bounded_units_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, double f, unsigned long long t)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,t,ubound,lbound,rest,u|BCTVTYPE_DOUBLE,sizeof(double),&dp,1,&vv));
    *(double *)dp=f;
    *vv = piw::normalise(ubound,lbound,rest,f);
    return d;
}

template <class T>
static T __makelong_bounded_units_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, long f, unsigned long long t)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,t,ubound,lbound,rest,u|BCTVTYPE_INT,sizeof(long),&dp,1,&vv));
    *(long *)dp=f;
    *vv = piw::normalise(ubound,lbound,rest,f);
    return d;
}

template <class T>
static T __makewire(unsigned nb,unsigned dl, const unsigned char *dp)
{
    return T::from_given(__allocate_wire(nb,dl,dp));
}

template <class T>
static T __pathprepend_chaff(unsigned nb,const T &o, unsigned p)
{
    unsigned char *dp;
    const unsigned char *op;
    float *vv;
    unsigned ol,oc;
    T d;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);

    ol=o.host_length();
    op=(const unsigned char *)o.host_data();
    oc=*op;

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,1+ol,&dp,1,&vv));

    dp[0]=oc+1;
    dp[1]=p;
    memcpy(dp+2,op+1,ol-1);

    *vv=0;

    return d;
}

template <class T>
static T __pathprepend_grist(unsigned nb,const T &o, unsigned p)
{
    unsigned char *dp;
    const unsigned char *op;
    float *vv;
    unsigned ol,oc;
    T d;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);

    ol=o.host_length();
    op=(const unsigned char *)o.host_data();
    oc=*op;

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,1+ol,&dp,1,&vv));

    dp[0]=oc;
    memcpy(dp+1,op+1,oc);
    dp[1+oc]=p;
    memcpy(dp+1+oc+1,op+1+oc,ol-1-oc);

    *vv=0;

    return d;
}

template <class T>
static T __pathappend_chaff(unsigned nb,const T &o, unsigned p)
{
    unsigned char *dp;
    const unsigned char *op;
    float *vv;
    unsigned ol,oc;
    T d;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);

    ol=o.host_length();
    op=(const unsigned char *)o.host_data();
    oc=*op;

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,1+ol,&dp,1,&vv));

    dp[0]=oc+1;
    memcpy(dp+1,op+1,oc);
    dp[1+oc]=p;
    memcpy(dp+1+oc+1,op+1+oc,ol-1-oc);

    *vv=0;

    return d;
}

template <class T>
static T __pathtruncate(unsigned nb,const T &o)
{
    unsigned char *dp; float *vv;
    T d;
    unsigned ol;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);
    ol=o.host_length();

    if(ol==0)
    {
        return o;
    }

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,ol-1,&dp,1,&vv));

    memcpy(dp,o.host_data(),ol-1);
    *vv=0;

    return d;
}

template <class T>
static T __pathpretruncate(unsigned nb,const T &o)
{
    unsigned char *dp; float *vv;
    T d;
    unsigned ol,oc;
    const unsigned char *op;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);

    ol=o.host_length();
    op=(const unsigned char *)o.host_data();
    oc=*op;

	if(ol<=1 || oc==0)
    {
        return o;
    }

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,ol-1,&dp,1,&vv));

    dp[0]=oc-1;
    memcpy(&dp[1],op+2,ol-1);
    *vv=0;

    return d;
}

template <class T>
static T __pathpretruncate(unsigned nb,const T &o, unsigned l)
{
    if (0==l) return o;

    unsigned char *dp; float *vv;
    T d;
    unsigned ol,oc;
    const unsigned char *op;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);

    ol=o.host_length();
    op=(const unsigned char *)o.host_data();
    oc=*op;

	if(ol<=l)
    {
        return o;
    }

    oc = oc-std::min(oc,l);

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,ol-l,&dp,1,&vv));

    dp[0]=oc;
    memcpy(&dp[1],op+l+1,ol-l);
    *vv=0;

    return d;
}

template <class T>
static T __pathreplacegrist(unsigned nb,const T &o,unsigned g)
{
    unsigned char *dp; float *vv;
    T d;
    unsigned ol;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);
    ol=o.host_length();

    if(ol==0)
    {
        return o;
    }

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,ol,&dp,1,&vv));

    memcpy(dp,o.host_data(),ol-1);
    dp[ol-1]=g;
    *vv=0;

    return d;
}

template <class T>
static T __pathgristpretruncate(unsigned nb,const T &o)
{
    unsigned char *dp; float *vv;
    T d;
    unsigned ol,oc;
    const unsigned char *op;

    PIC_ASSERT(o.type()==BCTVTYPE_PATH);

    ol=o.host_length();
    op=(const unsigned char *)o.host_data();
    oc=*op;

    if(ol<=1 || oc==0 || ol-1-oc==0)
    {
        return o;
    }

    d=T::from_given(__allocate_host(nb,o.time(),1,0,0,BCTVTYPE_PATH,ol-1,&dp,1,&vv));

    dp[0]=oc;
    memcpy(&dp[1],op+1,oc);
    memcpy(&dp[1+oc],&op[2+oc],ol-1-oc-1);
    *vv=0;

    return d;
}

template <class T>
static T __makeblob_ex(unsigned nb,unsigned long long ts, unsigned size, unsigned char **pdata)
{
    float *vp;
    T d = T::from_given(__allocate_host(nb,ts, 1,-1,0, BCTVTYPE_BLOB, size, pdata, 1, &vp));
    *vp=0;
    return d;
}

template <class T>
static T __makedouble(unsigned nb,unsigned long long ts,double v)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,ts,1000000,-1000000,0,BCTVTYPE_DOUBLE,sizeof(double),&dp,1,&vv));

    *(double *)dp=v;
    
    if(v>0)
    {
        *vv = v/1000000.0;
    }
    else
    {
        *vv = -v/1000000.0;
    }
    
    return d;
}

template <class T>
static T __makefloat(unsigned nb,unsigned long long ts,float v)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,ts,1000000,-1000000,0,BCTVTYPE_FLOAT,sizeof(float),&dp,1,&vv));

    *(float *)dp=v;
    
    if(v>0)
    {
        *vv = v/1000000.0;
    }
    else
    {
        *vv = -v/1000000.0;
    }
    
    return d;
}

template <class T>
static T __makelong(unsigned nb,unsigned long long ts,long v)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,ts,1000000,-1000000,0,BCTVTYPE_INT,sizeof(long),&dp,1,&vv));

    *(long *)dp=v;

    if(v>0)
    {
        *vv = ((float)v)/(1000000.0);
    }
    else
    {
        *vv = ((float)v)/(1000000.0);
    }

    return d;
}

template <class T>
static T __makestring(unsigned nb,unsigned long long ts,const char *v, unsigned l)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,ts,1,0,0,BCTVTYPE_STRING,l,&dp,1,&vv));
    memcpy(dp,v,l); *vv=0;
    return d;
}

template <class T>
static T __makebool(unsigned nb,unsigned long long ts,bool v)
{
    unsigned char *dp; T d; float *vv;
    d=T::from_given(__allocate_host(nb,ts,1,0,0,BCTVTYPE_BOOL,sizeof(char),&dp,1,&vv));

    if(v)
    {
        *dp=1;
        *vv=1.0;
    }
    else
    {
        *dp=0;
        *vv=0.0;
    }

    return d;
}

template <class T>
static T __makenull(unsigned nb,unsigned long long ts)
{
    unsigned char *dp; T d;
    d=T::from_given(__allocate_host(nb,ts,0,0,0,BCTVTYPE_NULL,0,&dp,0,0));
    return d;
}

template <class T>
static T __dictnull_ex(unsigned nb,unsigned long long ts)
{
    float *vp;
    unsigned char *dp;
    T d = T::from_given(__allocate_host(nb,ts,1,-1,0,BCTVTYPE_DICT,0,&dp,1,&vp));
    *vp = 0;
    return d;
}

template <class T>
static T __dictdel_ex(unsigned nb, const T &old, const std::string &key)
{
    const unsigned char *b;
    unsigned r,r2;
    unsigned l,l2;
    const char *keyp = key.c_str();
    unsigned keyl = key.size();

    b = (const unsigned char *)old.host_data();
    r = old.host_length();
    l = 0;

    r2 = r;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(kl==keyl && !strncmp(keyp,(const char *)(b+l+4),kl))
        {
            r2 -= (kl+vl+4);
            break;
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    float *vp;
    unsigned char *dp;
    T d2 = T::from_given(__allocate_host(nb,old.time(),1,-1,0,BCTVTYPE_DICT,r2,&dp,1,&vp));

    r = old.host_length();
    l = 0;
    l2 = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;

        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(kl!=keyl || strncmp(keyp,(const char *)(b+l+4),kl))
        {
            memcpy(dp+l2,b+l,4+kl+vl);
            l2+=(4+kl+vl);
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    *vp=0;
    return d2;
}

template <class T>
static T __dictset_ex(unsigned nb,const T &old, const std::string &key, const T &v)
{
    const unsigned char *b;
    unsigned r,r2;
    unsigned l,l2;
    const char *keyp = key.c_str();
    unsigned keyl = key.size();

    b = (const unsigned char *)old.host_data();
    r = old.host_length();
    l = 0;

    r2 = r;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(kl==keyl && !strncmp(keyp,(const char *)(b+l+4),kl))
        {
            r2 -= (kl+vl+4);
            break;
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    r2 += (4+keyl+v.wire_length());

    float *vp;
    unsigned char *dp;
    T d2 = T::from_given(__allocate_host(nb,old.time(),1,-1,0,BCTVTYPE_DICT,r2,&dp,1,&vp));

    r = old.host_length();
    l = 0;
    l2 = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;

        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(kl!=keyl || strncmp(keyp,(const char *)(b+l+4),kl))
        {
            memcpy(dp+l2,b+l,4+kl+vl);
            l2+=(4+kl+vl);
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    pie_setu16(dp+l2,2,keyl);
    pie_setu16(dp+l2+2,2,v.wire_length());
    memcpy(dp+l2+4,keyp,key.size());
    memcpy(dp+l2+4+keyl,v.wire_data(),v.wire_length());

    *vp=0;
    return d2;
}

template <class T>
static int __dict_findkey(const T &d, const char *keyp, unsigned keyl)
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    unsigned c;

    b = (const unsigned char *)d.host_data();
    r = d.host_length();
    l = 0;
    c = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(kl==keyl && !strncmp(keyp,(const char *)(b+l+4),kl))
        {
            return c;
        }

        l+=(4+kl+vl); r-=(4+kl+vl); c++;
    }

    return -1;
}


template <class T>
static T __dictupdate_ex(unsigned nb,const T &old, const T &upd)
{
    const unsigned char *b;
    unsigned r,r2;
    unsigned l,l2;

    b = (const unsigned char *)old.host_data();
    r = old.host_length();
    l = 0;

    r2 = r;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(__dict_findkey<T>(upd,(const char *)(b+l+4),kl)>=0)
        {
            r2 -= (kl+vl+4);
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    r2 += upd.host_length();

    float *vp;
    unsigned char *dp;
    T d2 = T::from_given(__allocate_host(nb,old.time(),1,-1,0,BCTVTYPE_DICT,r2,&dp,1,&vp));

    r = old.host_length();
    l = 0;
    l2 = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;

        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(__dict_findkey<T>(upd,(const char *)(b+l+4),kl)<0)
        {
            memcpy(dp+l2,b+l,4+kl+vl);
            l2+=(4+kl+vl);
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    memcpy(dp+l2,upd.host_data(),upd.host_length());
    *vp=0;
    return d2;
}

template <class T>
static T __tuplenull_ex(unsigned nb,unsigned long long ts)
{
    float *vp;
    unsigned char *dp;
    T d = T::from_given(__allocate_host(nb,ts,1,-1,0,BCTVTYPE_TUPLE,0,&dp,1,&vp));
    *vp = 0;
    return d;
}

template <class T>
static T __tupledel_ex(unsigned nb, const T &old, unsigned i)
{
    const unsigned char *b;
    unsigned r,r2;
    unsigned l,l2;
    unsigned c;

    b = (const unsigned char *)old.host_data();
    r = old.host_length();
    l = 0;
    c = 0;

    r2 = r;

    while(r>=2)
    {
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&vl)>0);

        if(r-2<vl)
            break;

        if(c==i)
        {
            r2 -= (vl+2);
            break;
        }

        c++;
        l+=(2+vl);
        r-=(2+vl);
    }

    float *vp;
    unsigned char *dp;
    T d2 = T::from_given(__allocate_host(nb,old.time(),1,-1,0,BCTVTYPE_DICT,r2,&dp,1,&vp));

    r = old.host_length();
    l = 0;
    l2 = 0;

    c = 0;

    while(r>=4)
    {
        uint16_t vl;

        PIC_ASSERT(pie_getu16(b+l,r,&vl)>0);

        if(r-2<vl)
            break;

        if(c!=i)
        {
            memcpy(dp+l2,b+l,2+vl);
            l2+=(2+vl);
        }

        c++;
        l+=(2+vl);
        r-=(2+vl);
    }

    *vp=old.as_norm();
    return d2;
}

template <class T>
static T __tupleadd_ex(unsigned nb,const T &old, const T &v)
{
    const unsigned char *b;
    unsigned r,r2;
    unsigned l;

    b = (const unsigned char *)old.host_data();
    r = old.host_length();
    l = 0;

    r2 = r;

    while(r>=2)
    {
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&vl)>0);

        if(r-2<vl)
            break;

        l+=(2+vl);
        r-=(2+vl);
    }

    r2 += (2+v.wire_length());

    float *vp;
    unsigned char *dp;
    T d2 = T::from_given(__allocate_host(nb,old.time(),1,-1,0,BCTVTYPE_TUPLE,r2,&dp,1,&vp));

    r = old.host_length();
    l = 0;

    while(r>=2)
    {
        uint16_t vl;

        PIC_ASSERT(pie_getu16(b+l,r,&vl)>0);

        if(r-2<vl)
            break;

        memcpy(dp+l,b+l,2+vl);

        l+=(2+vl);
        r-=(2+vl);
    }

    pie_setu16(dp+l,2,v.wire_length());
    memcpy(dp+l+2,v.wire_data(),v.wire_length());

    *vp=old.as_norm();
    return d2;
}

template <class T>
static T __tuplenorm_ex(unsigned nb,const T &old, float norm)
{
    const unsigned char *b;
    unsigned r;

    b = (const unsigned char *)old.host_data();
    r = old.host_length();

    float *vp;
    unsigned char *dp;
    T d2 = T::from_given(__allocate_host(nb,old.time(),1,-1,0,BCTVTYPE_TUPLE,r,&dp,1,&vp));
    memcpy(dp,b,r);

    *vp = norm;

    return d2;
}

bct_data_t piw::makecopy(unsigned nb, unsigned long long ts, bct_data_t &d)
{
    float *vp;
    unsigned char *dp;

    bct_data_t r = __allocate_host(nb,ts,bct_data_ubound(d),bct_data_lbound(d),bct_data_rest(d),bct_data_type(d)|bct_data_units(d),bct_data_len(d),&dp,bct_data_veclen(d),&vp);
    memcpy(dp,bct_data_data(d),bct_data_len(d));
    const float *v = bct_data_vector(d);
    memcpy(vp,v,bct_data_veclen(d)*4);

    return r;
}

piw::data_t piw::makearray_ex(unsigned nb,unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs) { return __makearray_ex<data_t>(nb,ts,ubound,lbound,rest,nfloats,pfloat,fs); }
piw::data_t piw::makenorm_ex(unsigned nb,unsigned long long ts, unsigned nfloats, float **pfloat,float **fs) { return makearray_ex(nb,ts,1,-1,0,nfloats,pfloat,fs); }
piw::data_t piw::makefloat_bounded_ex(unsigned nb,float ubound, float lbound, float rest, float f, unsigned long long t) { return __makefloat_bounded_ex<data_t>(nb,ubound,lbound,rest,f,t); }
piw::data_t piw::makedouble_bounded_ex(unsigned nb,float ubound, float lbound, float rest, double f, unsigned long long t) { return __makedouble_bounded_ex<data_t>(nb,ubound,lbound,rest,f,t); }
piw::data_t piw::makelong_bounded_ex(unsigned nb,float ubound, float lbound, float rest, long f, unsigned long long t) { return __makelong_bounded_ex<data_t>(nb,ubound,lbound,rest,f,t); }
piw::data_t piw::makefloat_bounded_units_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, float f, unsigned long long t) { return __makefloat_bounded_units_ex<data_t>(nb,u,ubound,lbound,rest,f,t); }
piw::data_t piw::makedouble_bounded_units_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, double f, unsigned long long t) { return __makedouble_bounded_units_ex<data_t>(nb,u,ubound,lbound,rest,f,t); }
piw::data_t piw::makelong_bounded_units_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, long f, unsigned long long t) { return __makelong_bounded_units_ex<data_t>(nb,u,ubound,lbound,rest,f,t); }
piw::data_t piw::makewire_ex(unsigned nb,unsigned dl, const unsigned char *dp) { return (__makewire<data_t>(nb,dl,dp)); }
piw::data_t piw::pathnull_ex(unsigned nb,unsigned long long t) { return (__makepath<data_t>(nb,t,0,0,0)); }
piw::data_t piw::pathone_ex(unsigned nb,unsigned v,unsigned long long t) { unsigned char vv=v; return (__makepath<data_t>(nb,t,&vv,1,0)); }
piw::data_t piw::pathprepend_ex(unsigned nb,const data_t &d, unsigned p) { return (__pathprepend_chaff<data_t>(nb,d,p)); }
piw::data_t piw::pathtwo_ex(unsigned nb,unsigned v1,unsigned v2,unsigned long long t) { return pathprepend_ex(nb,pathone_ex(nb,v2,t),v1); }
piw::data_t piw::pathprepend_grist_ex(unsigned nb,const data_t &d, unsigned p) { return (__pathprepend_grist<data_t>(nb,d,p)); }
piw::data_t piw::pathappend_chaff_ex(unsigned nb,const data_t &d, unsigned p) { return (__pathappend_chaff<data_t>(nb,d,p)); }
piw::data_t piw::pathtruncate_ex(unsigned nb,const data_t &d) { return (__pathtruncate<data_t>(nb,d)); }
piw::data_t piw::pathpretruncate_ex(unsigned nb,const data_t &d) { return (__pathpretruncate<data_t>(nb,d)); }
piw::data_t piw::pathpretruncate_ex(unsigned nb,const data_t &d,unsigned l) { return (__pathpretruncate<data_t>(nb,d,l)); }
piw::data_t piw::pathreplacegrist_ex(unsigned nb,const data_t &d,unsigned g) { return (__pathreplacegrist<data_t>(nb,d,g)); }
piw::data_t piw::pathgristpretruncate_ex(unsigned nb,const data_t &d) { return (__pathgristpretruncate<data_t>(nb,d)); }
piw::data_t piw::makeblob_ex(unsigned nb,unsigned long long ts, unsigned size, unsigned char **pdata) { return __makeblob_ex<data_t>(nb,ts,size,pdata); }
piw::data_t piw::makedouble_ex(unsigned nb,double v, unsigned long long t) { return (__makedouble<data_t>(nb,t,v)); }
piw::data_t piw::makefloat_ex(unsigned nb,float v, unsigned long long t) { return (__makefloat<data_t>(nb,t,v)); }
piw::data_t piw::makelong_ex(unsigned nb,long v, unsigned long long t) { return (__makelong<data_t>(nb,t,v)); }
piw::data_t piw::makepath_ex(unsigned nb,const unsigned char *p, unsigned pl, unsigned long long t) { return __makepath<data_t>(nb,t,p,pl,0); }
piw::data_t piw::parsepath_ex(unsigned nb,const char *path, unsigned long long t) { return __parsepath<data_t>(nb,path,t); }
piw::data_t piw::makestring_len_ex(unsigned nb,const char *p, unsigned l, unsigned long long t) { return (__makestring<data_t>(nb,t,p,l)); }
piw::data_t piw::makestring_ex(unsigned nb,const char *p, unsigned long long t) { return makestring_len_ex(nb,p,strlen(p),t); }
piw::data_t piw::makestring_ex(unsigned nb,const std::string &p, unsigned long long t) { return makestring_len_ex(nb,p.c_str(),p.length(),t); }
piw::data_t piw::makebool_ex(unsigned nb,bool v, unsigned long long t) { return (__makebool<data_t>(nb,t,v)); }
piw::data_t piw::makenull_ex(unsigned nb,unsigned long long t) { if(!t) return data_t(); return (__makenull<data_t>(nb,t)); }
piw::data_t piw::dictnull_ex(unsigned nb,unsigned long long ts) { return __dictnull_ex<data_t>(nb,ts); }
piw::data_t piw::dictdel_ex(unsigned nb,const data_t &old, const std::string &key) { return __dictdel_ex<data_t>(nb, old, key); }
piw::data_t piw::dictset_ex(unsigned nb,const data_t &old, const std::string &key, const data_t &v) { return __dictset_ex<data_t>(nb, old, key, v); }
piw::data_t piw::dictupdate_ex(unsigned nb,const data_t &old, const data_t &upd) { return __dictupdate_ex<data_t>(nb, old, upd); }
piw::data_t piw::tuplenull_ex(unsigned nb,unsigned long long ts) { return __tuplenull_ex<data_t>(nb,ts); }
piw::data_t piw::tupledel_ex(unsigned nb,const data_t &old, unsigned i) { return __tupledel_ex<data_t>(nb, old, i); }
piw::data_t piw::tupleadd_ex(unsigned nb,const data_t &old, const data_t &v) { return __tupleadd_ex<data_t>(nb, old, v); }
piw::data_t piw::tuplenorm_ex(unsigned nb,const data_t &old, float norm) { return __tuplenorm_ex<data_t>(nb, old, norm); }

piw::data_nb_t piw::makearray_nb_ex(unsigned nb,unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs) { return __makearray_ex<data_nb_t>(nb,ts,ubound,lbound,rest,nfloats,pfloat,fs); }
piw::data_nb_t piw::makenorm_nb_ex(unsigned nb,unsigned long long ts, unsigned nfloats, float **pfloat,float **fs) { return makearray_nb_ex(nb,ts,1,-1,0,nfloats,pfloat,fs); }
piw::data_nb_t piw::makefloat_bounded_nb_ex(unsigned nb,float ubound, float lbound, float rest, float f, unsigned long long t) { return __makefloat_bounded_ex<data_nb_t>(nb,ubound,lbound,rest,f,t); }
piw::data_nb_t piw::makedouble_bounded_nb_ex(unsigned nb,float ubound, float lbound, float rest, double f, unsigned long long t) { return __makedouble_bounded_ex<data_nb_t>(nb,ubound,lbound,rest,f,t); }
piw::data_nb_t piw::makelong_bounded_nb_ex(unsigned nb,float ubound, float lbound, float rest, long f, unsigned long long t) { return __makelong_bounded_ex<data_nb_t>(nb,ubound,lbound,rest,f,t); }
piw::data_nb_t piw::makefloat_bounded_units_nb_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, float f, unsigned long long t) { return __makefloat_bounded_units_ex<data_nb_t>(nb,u,ubound,lbound,rest,f,t); }
piw::data_nb_t piw::makedouble_bounded_units_nb_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, double f, unsigned long long t) { return __makedouble_bounded_units_ex<data_nb_t>(nb,u,ubound,lbound,rest,f,t); }
piw::data_nb_t piw::makelong_bounded_units_nb_ex(unsigned nb,unsigned u,float ubound, float lbound, float rest, long f, unsigned long long t) { return __makelong_bounded_units_ex<data_nb_t>(nb,u,ubound,lbound,rest,f,t); }
piw::data_nb_t piw::makewire_nb_ex(unsigned nb,unsigned dl, const unsigned char *dp) { return (__makewire<data_nb_t>(nb,dl,dp)); }
piw::data_nb_t piw::pathnull_nb_ex(unsigned nb,unsigned long long t) { return (__makepath<data_nb_t>(nb,t,0,0,0)); }
piw::data_nb_t piw::pathone_nb_ex(unsigned nb,unsigned v,unsigned long long t) { unsigned char vv=v; return (__makepath<data_nb_t>(nb,t,&vv,1,0)); }
piw::data_nb_t piw::pathprepend_nb_ex(unsigned nb,const data_nb_t &d, unsigned p) { return (__pathprepend_chaff<data_nb_t>(nb,d,p)); }
piw::data_nb_t piw::pathtwo_nb_ex(unsigned nb,unsigned v1,unsigned v2,unsigned long long t) { return pathprepend_nb_ex(nb,pathone_nb_ex(nb,v2,t),v1); }
piw::data_nb_t piw::pathprepend_grist_nb_ex(unsigned nb,const data_nb_t &d, unsigned p) { return (__pathprepend_grist<data_nb_t>(nb,d,p)); }
piw::data_nb_t piw::pathappend_chaff_nb_ex(unsigned nb,const data_nb_t &d, unsigned p) { return (__pathappend_chaff<data_nb_t>(nb,d,p)); }
piw::data_nb_t piw::pathtruncate_nb_ex(unsigned nb,const data_nb_t &d) { return (__pathtruncate<data_nb_t>(nb,d)); }
piw::data_nb_t piw::pathpretruncate_nb_ex(unsigned nb,const data_nb_t &d) { return (__pathpretruncate<data_nb_t>(nb,d)); }
piw::data_nb_t piw::pathpretruncate_nb_ex(unsigned nb,const data_nb_t &d,unsigned l) { return (__pathpretruncate<data_nb_t>(nb,d,l)); }
piw::data_nb_t piw::pathreplacegrist_nb_ex(unsigned nb,const data_nb_t &d,unsigned g) { return (__pathreplacegrist<data_nb_t>(nb,d,g)); }
piw::data_nb_t piw::pathgristpretruncate_nb_ex(unsigned nb,const data_nb_t &d) { return (__pathgristpretruncate<data_nb_t>(nb,d)); }
piw::data_nb_t piw::makeblob_nb_ex(unsigned nb,unsigned long long ts, unsigned size, unsigned char **pdata) { return __makeblob_ex<data_nb_t>(nb,ts,size,pdata); }
piw::data_nb_t piw::makedouble_nb_ex(unsigned nb,double v, unsigned long long t) { return (__makedouble<data_nb_t>(nb,t,v)); }
piw::data_nb_t piw::makefloat_nb_ex(unsigned nb,float v, unsigned long long t) { return (__makefloat<data_nb_t>(nb,t,v)); }
piw::data_nb_t piw::makelong_nb_ex(unsigned nb,long v, unsigned long long t) { return (__makelong<data_nb_t>(nb,t,v)); }
piw::data_nb_t piw::makepath_nb_ex(unsigned nb,const unsigned char *p, unsigned pl, unsigned long long t) { return __makepath<data_nb_t>(nb,t,p,pl,0); }
piw::data_nb_t piw::parsepath_nb_ex(unsigned nb,const char *path, unsigned long long t) { return __parsepath<data_nb_t>(nb,path,t); }
piw::data_nb_t piw::makestring_len_nb_ex(unsigned nb,const char *p, unsigned l, unsigned long long t) { return (__makestring<data_nb_t>(nb,t,p,l)); }
piw::data_nb_t piw::makestring_nb_ex(unsigned nb,const char *p, unsigned long long t) { return makestring_len_nb_ex(nb,p,strlen(p),t); }
piw::data_nb_t piw::makestring_nb_ex(unsigned nb,const std::string &p, unsigned long long t) { return makestring_len_nb_ex(nb,p.c_str(),p.length(),t); }
piw::data_nb_t piw::makebool_nb_ex(unsigned nb,bool v, unsigned long long t) { return (__makebool<data_nb_t>(nb,t,v)); }
piw::data_nb_t piw::makenull_nb_ex(unsigned nb,unsigned long long t) { if(!t) return data_nb_t(); return (__makenull<data_nb_t>(nb,t)); }
piw::data_nb_t piw::dictnull_nb_ex(unsigned nb,unsigned long long ts) { return __dictnull_ex<data_nb_t>(nb,ts); }
piw::data_nb_t piw::dictdel_nb_ex(unsigned nb,const data_nb_t &old, const std::string &key) { return __dictdel_ex<data_nb_t>(nb, old, key); }
piw::data_nb_t piw::dictset_nb_ex(unsigned nb,const data_nb_t &old, const std::string &key, const data_nb_t &v) { return __dictset_ex<data_nb_t>(nb, old, key, v); }
piw::data_nb_t piw::dictupdate_nb_ex(unsigned nb,const data_nb_t &old, const data_nb_t &upd) { return __dictupdate_ex<data_nb_t>(nb, old, upd); }
piw::data_nb_t piw::tuplenull_nb_ex(unsigned nb,unsigned long long ts) { return __tuplenull_ex<data_nb_t>(nb,ts); }
piw::data_nb_t piw::tupledel_nb_ex(unsigned nb,const data_nb_t &old, unsigned i) { return __tupledel_ex<data_nb_t>(nb, old, i); }
piw::data_nb_t piw::tupleadd_nb_ex(unsigned nb,const data_nb_t &old, const data_nb_t &v) { return __tupleadd_ex<data_nb_t>(nb, old, v); }
piw::data_nb_t piw::tuplenorm_nb_ex(unsigned nb,const data_nb_t &old, float norm) { return __tuplenorm_ex<data_nb_t>(nb, old, norm); }

/*
 * piw::data_base_t
 */

bct_data_t piw::data_base_t::give_copy(unsigned nb) const
{
    if (!_rep) return 0;

    float *vp;
    unsigned char *dp;

    bct_data_t d = __allocate_host(nb,time(),as_array_ubound(),as_array_lbound(),as_array_rest(),type()|units(),host_length(),&dp,as_arraylen(),&vp);
#ifdef DEBUG_DATA_ATOMICITY
    d->tid = 0;
    d->nb_usage = false;
#endif
    memcpy(dp,host_data(),host_length());
    memcpy(vp,as_array(),as_arraylen()*4);

    return d;
}

int piw::data_base_t::compare_grist(const data_base_t &other) const
{
    if(!is_path() || !other.is_path()) return -1;

    unsigned al=as_pathlen();
    unsigned bl=other.as_pathlen();
    unsigned ac=as_pathchafflen();
    unsigned bc=other.as_pathchafflen();
    unsigned ag=al-ac;
    unsigned bg=bl-bc;

    if(ag<bg) return -1;
    if(bg<ag) return 1;

    return memcmp(&as_path()[ac],&other.as_path()[bc],ag);
}

int piw::data_base_t::compare(const data_base_t &other, bool ts) const
{
    unsigned at,bt,al,bl;
    bct_data_t a = _rep;
    bct_data_t b = other._rep;

    if(a && bct_data_type(a)==BCTVTYPE_NULL) a=0;
    if(b && bct_data_type(b)==BCTVTYPE_NULL) b=0;

    if(!a)
    {
        if(!b) return 0;
        return -1;
    }

    if(!b)
    {
        return 1;
    }

    if(ts)
    {
        unsigned long long ats, bts;

        ats=bct_data_time(a);
        bts=bct_data_time(b);

        if(ats<bts) return -1;
        if(ats>bts) return 1;
    }

    at=bct_data_type(a);
    bt=bct_data_type(b);

    if(at<bt) return -1;
    if(at>bt) return 1;

    al=bct_data_len(a);
    bl=bct_data_len(b);

    if(al<bl) return -1;
    if(al>bl) return 1;

    return memcmp(bct_data_data(a), bct_data_data(b), al);
}

int piw::data_base_t::compare_path(const data_base_t &other) const
{
    if(!is_path() || !other.is_path()) return -1;

    unsigned al=as_pathlen();
    unsigned bl=other.as_pathlen();

    if(al<bl) return -1;
    if(bl<al) return 1;

    return memcmp(as_path(),other.as_path(),al);
}

int piw::data_base_t::compare_path_beginning(const piw::data_base_t &other) const
{
    if(!is_path() || !other.is_path()) return -1;

    unsigned al=as_pathlen();
    unsigned bl=other.as_pathlen();

    return memcmp(as_path(),other.as_path(),std::min(al,bl));
}

int piw::data_base_t::compare_path_lexicographic(const data_base_t &other) const
{
    if(!is_path() || !other.is_path()) return -1;

    unsigned al=as_pathlen();
    unsigned bl=other.as_pathlen();
    unsigned ml=std::min(al,bl);

    const unsigned char *ap = as_path();
    const unsigned char *bp = other.as_path();

    for(unsigned i=0;i<ml;i++)
    {
        if(*ap!=*bp) 
        {
            return ((int)(*ap))-((int)(*bp));
        }

        ap++; bp++;
    }

    if(al<bl) return -1;
    if(bl<al) return 1;
    return 0;
}

unsigned piw::data_base_t::as_dict_nkeys() const
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    unsigned c;

    b = (const unsigned char *)host_data();
    r = host_length();
    l = 0;
    c = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        c++; l+=(4+kl+vl); r-=(4+kl+vl);
    }

    return c;
}

std::string piw::data_base_t::as_dict_key(unsigned n) const
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    unsigned c;

    b = (const unsigned char *)host_data();
    r = host_length();
    l = 0;
    c = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(c==n)
            return std::string((const char *)(b+l+4),kl);

        c++; l+=(kl+vl+4); r-=(kl+vl+4);
    }

    return "";
}

std::string piw::data_base_t::repr() const
{
    if(is_string())
    {
        return as_stdstr();
    }
    else
    {
        std::ostringstream os;
        os << *this;
        return os.str();
    }
}

long piw::data_base_t::hash() const
{
    unsigned l = host_length();
    const unsigned char *d = (const unsigned char *)host_data();
    long hash = 0;
    unsigned char seed = 0;

    for(unsigned i=0;i<l;i++)
    {
        seed += (13+d[i]);
        seed &= 0xff;

        ((unsigned char *)&hash)[i%sizeof(hash)] ^= seed;
        ((unsigned char *)&hash)[i%sizeof(hash)] &= 0xff;
    }

    return hash;
}

unsigned piw::data_base_t::as_tuplelen() const
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    unsigned c;

    b = (const unsigned char *)host_data();
    r = host_length();
    l = 0;
    c = 0;

    while(r>=2)
    {
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&vl)>0);

        if(r-2<vl)
            break;

        c++;
        l+=(2+vl);
        r-=(2+vl);
    }

    return c;
}

std::ostream &operator<<(std::ostream &o, const piw::data_base_t &d)
{
    pie_print(d.wire_length(),d.wire_data(), pie::ostreamwriter, &o);
    return o;
}

std::ostream &operator<<(std::ostream &o, const piw::fullprinter_t<piw::data_t> &d)
{
    pie_printfull(d.data_.wire_length(),d.data_.wire_data(), pie::ostreamwriter, &o);
    return o;
}

std::ostream &operator<<(std::ostream &o, const piw::fullprinter_t<piw::data_nb_t> &d)
{
    pie_printfull(d.data_.wire_length(),d.data_.wire_data(), pie::ostreamwriter, &o);
    return o;
}


/*
 * static template functions for child classes of piw::data_base_t
 */

template <class T>
static T __as_dict_value(unsigned nb, const T *o, unsigned n)
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    unsigned c;

    b = (const unsigned char *)o->host_data();
    r = o->host_length();
    l = 0;
    c = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(c==n)
        {
            return __makewire<T>(nb,vl,b+l+kl+4);
        }

        c++; l+=(kl+vl+4); r-=(kl+vl+4);
    }

    return __makenull<T>(nb,0);
}

template <class T>
static T __as_dict_lookup(unsigned nb, const T *o, const std::string &key)
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    const char *keyp = key.c_str();
    unsigned keyl = key.size();

    b = (const unsigned char *)o->host_data();
    r = o->host_length();
    l = 0;

    while(r>=4)
    {
        uint16_t kl;
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&kl)>0);
        PIC_ASSERT(pie_getu16(b+l+2,r-2,&vl)>0);

        if(r-4<(kl+vl))
            break;

        if(kl==keyl && !strncmp(keyp,(const char *)(b+l+4),kl))
        {
            return __makewire<T>(nb,vl,b+l+kl+4);
        }

        l+=(4+kl+vl); r-=(4+kl+vl);
    }

    return __makenull<T>(nb,0);
}

template <class T>
static T __as_tuple_value(unsigned nb, const T *o, unsigned i)
{
    const unsigned char *b;
    unsigned r;
    unsigned l;
    unsigned c;

    b = (const unsigned char *)o->host_data();
    r = o->host_length();
    l = 0;
    c = 0;

    while(r>=2)
    {
        uint16_t vl;
        PIC_ASSERT(pie_getu16(b+l,r,&vl)>0);

        if(r-2<vl)
            break;

        if(c==i)
        {
            return __makewire<T>(nb,vl,b+l+2);
        }

        c++;
        l+=(vl+2);
        r-=(vl+2);
    }

    return __makenull<T>(nb,0);
}

template <class T>
static T __restamp(const T *o, unsigned long long t)
{
    float *vp;
    unsigned char *dp;

    bct_data_t r = __allocate_host(PIC_ALLOC_NB,t,o->as_array_ubound(),o->as_array_lbound(),o->as_array_rest(),o->type()|o->units(),o->host_length(),&dp,o->as_arraylen(),&vp);
    T d = T::from_given(r);
    memcpy(dp,o->host_data(),o->host_length());
    memcpy(vp,o->as_array(),o->as_arraylen()*4);

    return d;
}


/*
 * piw::data_t
 */

piw::data_t piw::data_t::as_dict_value(unsigned n) const
{
    return __as_dict_value<data_t>(PIC_ALLOC_NORMAL, this, n);
}

piw::data_t piw::data_t::as_dict_lookup(const std::string &key) const
{
    return __as_dict_lookup<data_t>(PIC_ALLOC_NORMAL, this, key);
}

piw::data_t piw::data_t::as_tuple_value(unsigned i) const
{
    return __as_tuple_value<data_t>(PIC_ALLOC_NORMAL, this, i);
}

piw::data_t piw::data_t::realloc_real(unsigned nb) const
{
    float *vp;
    unsigned char *dp;

    bct_data_t r = __allocate_host(nb,time(),as_array_ubound(),as_array_lbound(),as_array_rest(),type()|units(),host_length(),&dp,as_arraylen(),&vp);
    data_t d = piw::data_t::from_given(r);
    memcpy(dp,host_data(),host_length());
    memcpy(vp,as_array(),as_arraylen()*4);

    return d;
}

piw::data_t piw::data_t::restamp(unsigned long long t) const
{
    return __restamp<data_t>(this, t);
}

piw::data_nb_t piw::data_t::make_nb() const
{
    return piw::data_nb_t::from_given(give_copy(PIC_ALLOC_NB));
}

/*
 * piw::data_nb_t
 */

piw::data_nb_t piw::data_nb_t::as_dict_value(unsigned n) const
{
    return __as_dict_value<data_nb_t>(PIC_ALLOC_NB, this, n);
}

piw::data_nb_t piw::data_nb_t::as_dict_lookup(const std::string &key) const
{
    return __as_dict_lookup<data_nb_t>(PIC_ALLOC_NB, this, key);
}

piw::data_nb_t piw::data_nb_t::as_tuple_value(unsigned i) const
{
    return __as_tuple_value<data_nb_t>(PIC_ALLOC_NB, this, i);
}

piw::data_nb_t piw::data_nb_t::realloc_real(unsigned nb) const
{
    float *vp;
    unsigned char *dp;

    data_nb_t d = piw::data_nb_t::from_given(__allocate_host(nb,time(),as_array_ubound(),as_array_lbound(),as_array_rest(),type()|units(),host_length(),&dp,as_arraylen(),&vp));
    memcpy(dp,host_data(),host_length());
    memcpy(vp,as_array(),as_arraylen()*4);

    return d;
}

piw::data_nb_t piw::data_nb_t::restamp(unsigned long long t) const
{
    return __restamp<data_nb_t>(this, t);
}

piw::data_t piw::data_nb_t::make_normal() const
{
    return piw::data_t::from_given(give_copy(PIC_ALLOC_NORMAL));
}

/*
 * piw::change_t
 */

piw::change_t piw::changelist()
{
    return piw::change_t::list();
}

void piw::changelist_connect(piw::change_t &cl, const piw::change_t &f)
{
    pic::functorlist_connect(cl,f);
}

void piw::changelist_disconnect(piw::change_t &cl, const piw::change_t &f)
{
    pic::functorlist_disconnect(cl,f);
}

piw::change_t piw::indirectchange()
{
    return piw::change_t::indirect(piw::change_t());
}

void piw::indirectchange_set(piw::change_t &cl, const piw::change_t &f)
{
    pic::indirect_settarget(cl,f);
}

void piw::indirectchange_clear(piw::change_t &cl)
{
    pic::indirect_clear(cl);
}

/*
 * piw::change_nb_t
 */

piw::change_nb_t piw::changelist_nb()
{
    return piw::change_nb_t::list();
}

void piw::changelist_connect_nb(piw::change_nb_t &cl, const piw::change_nb_t &f)
{
    pic::functorlist_connect(cl,f);
}

void piw::changelist_disconnect_nb(piw::change_nb_t &cl, const piw::change_nb_t &f)
{
    pic::functorlist_disconnect(cl,f);
}

piw::change_nb_t piw::indirectchange_nb()
{
    return piw::change_nb_t::indirect(piw::change_nb_t());
}

void piw::indirectchange_set_nb(piw::change_nb_t &cl, const piw::change_nb_t &f)
{
    pic::indirect_settarget(cl,f);
}

void piw::indirectchange_clear_nb(piw::change_nb_t &cl)
{
    pic::indirect_clear(cl);
}


namespace
{
    struct make_change_normal_t: piw::change_t::sinktype_t
    {
        make_change_normal_t(const piw::change_nb_t &c) : change_(c) {}

        bool iscallable() const { return true; }

        void invoke(const piw::data_t &d) const
        {
            change_.invoke(d.make_nb());
        }

        int gc_visit(void *v, void *a) const
        {
            return change_.gc_traverse(v,a);
        }

        piw::change_nb_t change_;
    };

    struct make_change_nb_t: piw::change_nb_t::sinktype_t
    {
        make_change_nb_t(const piw::change_t &c) : change_(c) {}

        bool iscallable() const { return true; }

        void invoke(const piw::data_nb_t &d) const
        {
            change_.invoke(d.make_normal());
        }

        int gc_visit(void *v, void *a) const
        {
            return change_.gc_traverse(v,a);
        }

        piw::change_t change_;
    };

    struct trig_sink_t : piw::change_t::sinktype_t
    {
        trig_sink_t(const piw::change_nb_t &target, const piw::data_nb_t &value) : target_(target), value_(value)
        {
        }

        void invoke(const piw::data_t &d) const
        {
            if(d.as_norm()!=0.f)
            {
                target_(value_.restamp(d.time()));
            }
        }

        bool iscallable() const
        {
            return true;
        }

        int gc_visit(void *v, void *a) const
        {
            return target_.gc_traverse(v,a);
        }

        pic::flipflop_functor_t<piw::change_nb_t> target_;
        piw::data_nb_t value_;
    };

    struct change2_sink_t: piw::change_t::sinktype_t
    {
        change2_sink_t(const piw::change_t &c1, const piw::change_t &c2): c1_(c1), c2_(c2)
        {
        }

        void invoke(const piw::data_t &d) const
        {
            c1_(d);
            c2_(d);
        }

        bool iscallable() const
        {
            return true;
        }

        int gc_visit(void *v, void *a) const
        {
            int r;
            if((r=c1_.gc_traverse(v,a))!=0) return r;
            if((r=c2_.gc_traverse(v,a))!=0) return r;
            return 0;
        }

        pic::flipflop_functor_t<piw::change_t> c1_;
        pic::flipflop_functor_t<piw::change_t> c2_;
    };

    struct change2_nb_sink_t: piw::change_nb_t::sinktype_t, public piw::thing_t
    {
        change2_nb_sink_t(const piw::change_t &c1, const piw::change_t &c2): c1_(c1), c2_(c2)
        {
            piw::tsd_thing(this);
        }

        void invoke(const piw::data_nb_t &d) const
        {
            const_cast<change2_nb_sink_t*>(this)->enqueue_slow_nb(d);
        }

        void thing_dequeue_slow(const piw::data_t &d)
        {
            c1_(d);
            c2_(d);
        }

        bool iscallable() const
        {
            return true;
        }

        int gc_visit(void *v, void *a) const
        {
            int r;
            if((r=c1_.gc_traverse(v,a))!=0) return r;
            if((r=c2_.gc_traverse(v,a))!=0) return r;
            return 0;
        }

        pic::flipflop_functor_t<piw::change_t> c1_;
        pic::flipflop_functor_t<piw::change_t> c2_;
    };

    struct change_nb2_sink_t: piw::change_nb_t::sinktype_t, public piw::thing_t
    {
        change_nb2_sink_t(const piw::change_nb_t &c1, const piw::change_nb_t &c2): c1_(c1), c2_(c2)
        {
            piw::tsd_thing(this);
        }

        void invoke(const piw::data_nb_t &d) const
        {
            c1_(d);
            c2_(d);
        }

        bool iscallable() const
        {
            return true;
        }

        int gc_visit(void *v, void *a) const
        {
            int r;
            if((r=c1_.gc_traverse(v,a))!=0) return r;
            if((r=c2_.gc_traverse(v,a))!=0) return r;
            return 0;
        }

        pic::flipflop_functor_t<piw::change_nb_t> c1_;
        pic::flipflop_functor_t<piw::change_nb_t> c2_;
    };
}

piw::change_t piw::make_change_normal(const piw::change_nb_t &c)
{
    return piw::change_t(pic::ref(new make_change_normal_t(c)));
}

piw::change_nb_t piw::make_change_nb(const piw::change_t &c)
{
    return piw::change_nb_t(pic::ref(new make_change_nb_t(c)));
}

piw::change_t piw::trigger(const piw::change_nb_t &c, const piw::data_nb_t &v)
{
    return piw::change_t(pic::ref(new trig_sink_t(c,v)));
}

piw::change_t piw::change2(const piw::change_t &c1, const piw::change_t &c2)
{
    return piw::change_t(pic::ref(new change2_sink_t(c1,c2)));
}

piw::change_nb_t piw::change2_nb(const piw::change_t &c1, const piw::change_t &c2)
{
    return piw::change_nb_t(pic::ref(new change2_nb_sink_t(c1,c2)));
}

piw::change_nb_t piw::change_nb2(const piw::change_nb_t &c1, const piw::change_nb_t &c2)
{
    return piw::change_nb_t(pic::ref(new change_nb2_sink_t(c1,c2)));
}

/*
 * piw::dataholder_nb_t
 */

piw::dataholder_nb_t::dataholder_nb_t() : data_(0) { }

piw::dataholder_nb_t::dataholder_nb_t(const piw::data_nb_t &d) : data_(0)
{
    set_nb(d);
}

piw::dataholder_nb_t::dataholder_nb_t(const piw::data_t &d) : data_(0)
{
    set_normal(d);
}

piw::dataholder_nb_t::~dataholder_nb_t()
{
    piw::tsd_fastcall(clear__,this,0);
}

piw::dataholder_nb_t::dataholder_nb_t(const piw::dataholder_nb_t &h): data_(0)
{
    piw::tsd_fastcall(copy_constructor__,this,h.data_);
}

piw::dataholder_nb_t &piw::dataholder_nb_t::operator=(const piw::dataholder_nb_t &h)
{
    piw::tsd_fastcall(assignment__,this,h.data_);
    return *this;
}

int piw::dataholder_nb_t::copy_constructor__(void *self_, void *arg_)
{
    piw::dataholder_nb_t *self = (piw::dataholder_nb_t *)self_;
    bct_data_t d = (bct_data_t)arg_;

    self->data_ = d;
    piw_data_incref_fast(self->data_);

    return 1;
}

int piw::dataholder_nb_t::assignment__(void *self_, void *arg_)
{
    piw::dataholder_nb_t *self = (piw::dataholder_nb_t *)self_;
    bct_data_t d = (bct_data_t)arg_;

    if(self->data_!=d)
    {
        self->clear();
        self->data_=d;
        piw_data_incref_fast(self->data_);
    }

    return 1;
}

int piw::dataholder_nb_t::clear__(void *self_, void *arg_)
{
    piw::dataholder_nb_t *self = (piw::dataholder_nb_t *)self_;

    self->clear();

    return 1;
}

int piw::dataholder_nb_t::set_normal__(void *self_, void *arg_)
{
    piw::dataholder_nb_t *self = (piw::dataholder_nb_t *)self_;

    piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)arg_);
    self->set_nb(d);

    return 1;
}

void piw::dataholder_nb_t::set_normal(const piw::data_t &d)
{
    piw::tsd_fastcall(set_normal__,this,(void *)d.give_copy(PIC_ALLOC_NB));
}

void piw::dataholder_nb_t::set_nb(const piw::data_nb_t &d)
{
    clear();

    data_ = d.give();
}

void piw::dataholder_nb_t::clear()
{
    if(data_)
    {
        piw_data_decref_fast(data_);
        data_ = 0;
    }
}

bool piw::dataholder_nb_t::is_empty()
{
    return 0 == data_;
}

const piw::data_nb_t piw::dataholder_nb_t::get() const
{
    if(data_)
    {
        return piw::data_nb_t::from_lent(data_);
    }
    else
    {
        return piw::data_nb_t();
    }
}

piw::dataholder_nb_t::operator piw::data_nb_t() const
{
    return get();
}

std::ostream &operator<<(std::ostream &o, const piw::dataholder_nb_t &d)
{
    pie_print(d.get().wire_length(),d.get().wire_data(), pie::ostreamwriter, &o);
    return o;
}

namespace
{
    struct merger_t: virtual pic::lckobject_t
    {
        merger_t(const piw::data_t &m)
        {
            override_.set_normal(m);
        }

        piw::data_nb_t operator()(const piw::data_nb_t &input) const
        {
            return piw::dictupdate_nb(input,override_);
        }

        bool operator==(const merger_t &m) const { return override_==m.override_; }

        piw::dataholder_nb_t override_;
    };
}

piw::d2d_nb_t piw::dict_merger(const piw::data_t &o)
{
    return piw::d2d_nb_t::callable(merger_t(o));
}
