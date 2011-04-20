
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

#include <piembedded/pie_message.h>
#include <piembedded/pie_string.h>
#include <piembedded/pie_wire.h>

#include <picross/pic_fastalloc.h>
#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_atomic.h>
#include <picross/pic_config.h>

#include <string.h>
#include <stdlib.h>
#include <picross/pic_stdint.h>

#include "pia_data.h"

#define FLOAT_PRECISION 4
#define DOUBLE_PRECISION 8

/*
 0:     type
 1-2:   scalar_len
 3-4:   vector_len
 5-12:  timestamp
 13-16: ubound
 17-20: lbound
 21-24: rest
 */

#define HOST_OK 1
#define WIRE_OK 2

struct rep_t: bct_data_s // rep_t         : 52
{                        // bct_data_s    : 48
    unsigned long flags; // unsigned long :  4

    // followed by
    // wire_hdr     25
    // wire_vector  vector_len*4
    // wire_scalar  scalar_len
    // wire_zero    1
    // (padding for alignment)
    // host_vector  vector_len*4
    // host_scalar  scalar_len
    // host_zero    1
};

#define PTRADD(p,o) (((unsigned char *)(p))+(o))
#define PTRDEL(p,o) (((unsigned char *)(p))-(o))

#define PAD16(p)           (((uintptr_t)(p)+0xf)&(~((uintptr_t)0xf)))

#ifdef ALIGN_16
#define PTR_WIRE_HDR(p)    PTRDEL(PAD16(PTRADD(p,sizeof(rep_t)+25)),25)
#else
#define PTR_WIRE_HDR(p)    PTRADD(p,sizeof(rep_t))
#endif

#define PTR_WIRE_VECTOR(p) ((float *)PTRADD(PTR_WIRE_HDR(p),25))
#define PTR_WIRE_SCALAR(p) PTRADD(PTR_WIRE_VECTOR(p),4*((p)->host_hdr.vector_len))
#define PTR_WIRE_ZERO(p)   PTRADD(PTR_WIRE_SCALAR(p),(p)->host_hdr.scalar_len)

#ifdef ALIGN_16
#define PTR_HOST_VECTOR(p) ((float *)(PAD16(PTRADD(PTR_WIRE_ZERO(p),1))))
#else
#define PTR_HOST_VECTOR(p) ((float *)(PTRADD(PTR_WIRE_ZERO(p),1)))
#endif

#define PTR_HOST_SCALAR(p) PTRADD(PTR_HOST_VECTOR(p),4*((p)->host_hdr.vector_len))
#define PTR_HOST_ZERO(p)   PTRADD(PTR_HOST_SCALAR(p),(p)->host_hdr.scalar_len)
#define WIRELEN(p)         (25+4*((p)->host_hdr.vector_len)+((p)->host_hdr.scalar_len))
#define ENDIAN(t)          (((t)==BCTVTYPE_FLOAT) || ((t)==BCTVTYPE_INT) || ((t)==BCTVTYPE_DOUBLE))

#ifdef PI_BIGENDIAN
#define SYS_RIGHTENDIAN 0
#else
#define SYS_RIGHTENDIAN 1
#endif

static void data_free(const bct_data_t o)
{
    rep_t *r = (rep_t *)o;
    pic::nb_free(r);
}

static const unsigned char *data_data(const bct_data_t o)
{
    rep_t *r = (rep_t *)o;

    if(r->flags&HOST_OK)
    {
        if(SYS_RIGHTENDIAN || !ENDIAN((r->host_hdr.type)&0x0f))
        {
            return PTR_WIRE_SCALAR(r);
        }
        else
        {
            return PTR_HOST_SCALAR(r);
        }
    }

    if(!SYS_RIGHTENDIAN)
    {
        switch((r->host_hdr.type)&0x0f)
        {
            case BCTVTYPE_DOUBLE:
            {
                pie_getf64(PTR_WIRE_SCALAR(r),8,(double *)PTR_HOST_SCALAR(r));
                break;
            }

            case BCTVTYPE_FLOAT:
            {
                pie_getf32(PTR_WIRE_SCALAR(r),4,(float *)PTR_HOST_SCALAR(r));
                break;
            }

            case BCTVTYPE_INT:
            {
                pie_get32(PTR_WIRE_SCALAR(r),4,(int32_t *)PTR_HOST_SCALAR(r));
                break;
            }
        }

        for(unsigned i=0;i<r->host_hdr.vector_len;i++)
        {
            pie_getf32((const unsigned char *)&PTR_WIRE_VECTOR(r)[i],4,&PTR_HOST_VECTOR(r)[i]);
        }
    }

    r->flags|=HOST_OK;

    if(SYS_RIGHTENDIAN || !ENDIAN((r->host_hdr.type)&0x0f))
    {
        return PTR_WIRE_SCALAR(r);
    }
    else
    {
        return PTR_HOST_SCALAR(r);
    }
}

static const float *data_vector(const bct_data_t o)
{
    rep_t *r = (rep_t *)o;
    data_data(o);

    if(SYS_RIGHTENDIAN)
    {
        return PTR_WIRE_VECTOR(r);
    }
    else
    {
        return PTR_HOST_VECTOR(r);
    }
}

static const unsigned char *wire_data(const bct_data_t o)
{
    rep_t *r = (rep_t *)o;

    if(r->flags&WIRE_OK)
    {
        return PTR_WIRE_HDR(r);
    }

    if(!SYS_RIGHTENDIAN)
    {
        switch((r->host_hdr.type)&0x0f)
        {
            case BCTVTYPE_DOUBLE:
            {
                pie_setf64(PTR_WIRE_SCALAR(r),8,*(double *)PTR_HOST_SCALAR(r));
                break;
            }

            case BCTVTYPE_FLOAT:
            {
                pie_setf32(PTR_WIRE_SCALAR(r),4,*(float *)PTR_HOST_SCALAR(r));
                break;
            }

            case BCTVTYPE_INT:
            {
                pie_set32(PTR_WIRE_SCALAR(r),4,*(int32_t *)PTR_HOST_SCALAR(r));
                break;
            }
        }

        for(unsigned i=0;i<r->host_hdr.vector_len;i++)
        {
            pie_setf32((unsigned char *)&PTR_WIRE_VECTOR(r)[i],4,PTR_HOST_VECTOR(r)[i]);
        }
    }

    r->flags|=WIRE_OK;
    return PTR_WIRE_HDR(r);
}

bct_data_ops_t dispatch__ =
{
    data_free,
    data_data,
    data_vector,
    wire_data,
};

bct_data_t allocate_host_raw(pic::nballocator_t *a, unsigned nb, unsigned long long ts, float u, float l, float rst,unsigned t, unsigned sl, unsigned char **hp, unsigned vl, float **vp)
{
    rep_t *r;

    unsigned rl;

    if(!SYS_RIGHTENDIAN && ENDIAN((t)&0x0f))
    {
        rl = sizeof(rep_t)+25+4*vl+sl+1;
        rl = rl+4*vl+sl;
    }
    else
    {
        rl = sizeof(rep_t)+25+4*vl+sl+1;
        rl = rl+4*vl;
    }
#ifdef ALIGN_16
    rl = rl+0xf+0xf;
#endif
    
    unsigned wl = 25+4*vl+sl;

    if(t!=BCTVTYPE_BLOB)
    {
        if(wl>BCTLIMIT_DATA)
            pic::msg() << "max wire length " << BCTLIMIT_DATA << ", requested " << wl << pic::hurl;
    }

    r = (rep_t *)pic::nb_malloc(nb,a,rl);

    r->host_ops=&dispatch__;
    r->count=1;
    r->flags=HOST_OK;
#ifdef DEBUG_DATA_ATOMICITY
    r->tid = pic_current_threadid();
    r->nb_usage = false;
#endif
    r->host_hdr.type = t;
    r->host_hdr.scalar_len = sl;
    r->host_hdr.vector_len = vl;
    r->host_hdr.wire_len = wl;
    r->host_hdr.time = ts;
    r->host_hdr.ubound = u;
    r->host_hdr.lbound = l;
    r->host_hdr.rest = rst;
    r->host_hdr.nb_mode = nb;

    unsigned char *wh = PTR_WIRE_HDR(r);

/*
    std::cout << ">>>>>> r = " << (uintptr_t)r << ", " << ((uintptr_t)r)%16 << std::endl;
    std::cout << ">>>>>> PTR_WIRE_HDR(r) = " << (uintptr_t)PTR_WIRE_HDR(r) << ", " << ((uintptr_t)PTR_WIRE_HDR(r))%16 << std::endl;
    std::cout << ">>>>>> PTR_WIRE_VECTOR(r) = " << (uintptr_t)PTR_WIRE_VECTOR(r) << ", " << ((uintptr_t)PTR_WIRE_VECTOR(r))%16 << std::endl;
    std::cout << ">>>>>> PTR_WIRE_SCALAR(r) = " << (uintptr_t)PTR_WIRE_SCALAR(r) << ", " << ((uintptr_t)PTR_WIRE_SCALAR(r))%16 << std::endl;
    std::cout << ">>>>>> PTR_WIRE_ZERO(r) = " << (uintptr_t)PTR_WIRE_ZERO(r) << ", " << ((uintptr_t)PTR_WIRE_ZERO(r))%16 << std::endl;
    std::cout << ">>>>>> PTR_HOST_VECTOR(r) = " << (uintptr_t)PTR_HOST_VECTOR(r) << ", " << ((uintptr_t)PTR_HOST_VECTOR(r))%16 << std::endl;
    std::cout << ">>>>>> PTR_HOST_SCALAR(r) = " << (uintptr_t)PTR_HOST_SCALAR(r) << ", " << ((uintptr_t)PTR_HOST_SCALAR(r))%16 << std::endl;
    std::cout << ">>>>>> PTR_HOST_ZERO(r) = " << (uintptr_t)PTR_HOST_ZERO(r) << ", " << ((uintptr_t)PTR_HOST_ZERO(r))%16 << std::endl;
  */  

    wh[0]=t;
    pie_setu16(&wh[1],2,sl);
    pie_setu16(&wh[3],2,vl);
    pie_setu64(&wh[5],8,ts);
    pie_setf32(&wh[13],4,r->host_hdr.ubound);
    pie_setf32(&wh[17],4,r->host_hdr.lbound);
    pie_setf32(&wh[21],4,r->host_hdr.rest);

    if(!SYS_RIGHTENDIAN && ENDIAN((t)&0x0f))
    {
        *(unsigned char *)PTR_WIRE_ZERO(r)=0;
        *(unsigned char *)PTR_HOST_ZERO(r)=0;
        if(hp) *hp = PTR_HOST_SCALAR(r);
    }
    else
    {
        *(unsigned char *)PTR_WIRE_ZERO(r)=0;
        if(hp) *hp = PTR_WIRE_SCALAR(r);
    }

    if(!SYS_RIGHTENDIAN)
    {
        if(vp) *vp = PTR_HOST_VECTOR(r);
    }
    else
    {
        if(vp) *vp = PTR_WIRE_VECTOR(r);
        r->flags|=WIRE_OK;
    }

    return r;
}

bct_data_t allocate_wire_raw(pic::nballocator_t *a, unsigned nb, unsigned wl, const unsigned char *wp)
{
    if(wl==0)
    {
        return allocate_host_raw(a,nb,0,0,0,0,BCTVTYPE_NULL,0,0,0,0);
    }

    PIC_ASSERT(wl>=25);

    rep_t *r;
    unsigned short sl,vl,rl;
    unsigned t;

    t=wp[0];
    pie_getu16(&wp[1],2,&sl);
    pie_getu16(&wp[3],2,&vl);

    unsigned ewl = 25U+4U*vl+sl;
    if(wl!=ewl)
    {
        pic::msg() << "wirelen expected " << ewl << " got " << wl << pic::hurl;
    }

    if(!SYS_RIGHTENDIAN && ENDIAN((t)&0x0f))
    {
        rl = sizeof(rep_t)+25+4*vl+sl+1;
        rl = rl+4*vl+sl;
    }
    else
    {
        rl = sizeof(rep_t)+25+4*vl+sl+1;
        rl = rl+4*vl;
    }
#ifdef ALIGN_16
    rl = rl+0xf+0xf;
#endif

    r = (rep_t *)nb_malloc(nb,a,rl);

    r->host_ops=&dispatch__;
    r->count=1;
    r->flags=WIRE_OK;
#ifdef DEBUG_DATA_ATOMICITY
    r->tid = pic_current_threadid();
    r->nb_usage = false;
#endif
    r->host_hdr.type = t;
    r->host_hdr.scalar_len = sl;
    r->host_hdr.vector_len = vl;
    r->host_hdr.wire_len = wl;
    r->host_hdr.nb_mode = nb;

    pie_getu64(&wp[5],8,(uint64_t *)&r->host_hdr.time);
    pie_getf32(&wp[13],4,&r->host_hdr.ubound);
    pie_getf32(&wp[17],4,&r->host_hdr.lbound);
    pie_getf32(&wp[21],4,&r->host_hdr.rest);

    if(!SYS_RIGHTENDIAN && ENDIAN((t)&0x0f))
    {
        *(unsigned char *)PTR_WIRE_ZERO(r)=0;
        *(unsigned char *)PTR_HOST_ZERO(r)=0;
    }
    else
    {
        *(unsigned char *)PTR_WIRE_ZERO(r)=0;
    }

    if(SYS_RIGHTENDIAN)
    {
        r->flags|=HOST_OK;
    }

    memcpy(PTR_WIRE_HDR(r),wp,wl);

    return r;
}


/*
 * pia_data_base_t
 */

int pia_data_base_t::compare_path(const pia_data_base_t &other) const
{
    if(type()!=BCTVTYPE_PATH || other.type()!=BCTVTYPE_PATH) return -1;

    unsigned al=aspathlen();
    unsigned bl=other.aspathlen();

    if(al<bl) return -1;
    if(bl<al) return 1;

    return memcmp(aspath(),other.aspath(),al);
}

int pia_data_base_t::compare(const pia_data_base_t &b_,bool ts) const
{
    unsigned at,bt,al,bl;
    bct_data_t a = data_;
    bct_data_t b = b_.data_;

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

long pia_data_base_t::hash() const
{
    unsigned l = hostlen();
    const unsigned char *d = (const unsigned char *)hostdata();
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

bct_data_t pia_data_base_t::give_copy(pic::nballocator_t *a, unsigned nb) const
{
    if (!data_) return 0;

    unsigned hl=hostlen();
    unsigned vl=veclen();

    unsigned char *p; float *p2;
    bct_data_t d = allocate_host_raw(a,nb,time(),ubound(),lbound(),rest(),type()|units(),hl,&p,vl,&p2);
#ifdef DEBUG_DATA_ATOMICITY
    d->tid = 0;
    d->nb_usage = false;
#endif
    if(hl) memcpy(p,hostdata(),hl);
    if(vl) memcpy(p2,vec(),sizeof(float)*vl);

    return d;
}

static bct_data_t make_copy(pic::nballocator_t *a, unsigned nb, bct_data_t o)
{
    if (!o) return 0;

    unsigned hl=bct_data_len(o);
    unsigned vl=bct_data_veclen(o);

    unsigned char *p; float *p2;
    bct_data_t d = allocate_host_raw(a,nb,bct_data_time(o),bct_data_ubound(o),bct_data_lbound(o),bct_data_rest(o),bct_data_type(o)|bct_data_units(o),hl,&p,vl,&p2);
#ifdef DEBUG_DATA_ATOMICITY
    d->tid = 0;
    d->nb_usage = false;
#endif
    if(hl) memcpy(p,bct_data_data(o),hl);
    const float *v = bct_data_vector(o);
    if(vl) memcpy(p2,v,sizeof(float)*vl);

    return d;
}


/*
 * pia_data_t
 */

pia_data_t pia_data_t::allocate_host(pic::nballocator_t *a, unsigned nb, unsigned long long ts,float u,float l,float r,unsigned t, unsigned hl, unsigned char **hp, unsigned vl, float **vp)
{
    return pia_data_t::from_given(allocate_host_raw(a,nb,ts,u,l,r,t,hl,hp,vl,vp));
}

pia_data_t pia_data_t::allocate_wire(pic::nballocator_t *a, unsigned nb, unsigned wl, const unsigned char *wp)
{
    return pia_data_t::from_given(allocate_wire_raw(a,nb,wl,wp));
}

pia_data_t pia_data_t::allocate_bool(pic::nballocator_t *a, bool v, unsigned long long ts)
{
    float *fv;
    unsigned char *dp;

    pia_data_t d = allocate_host(a,PIC_ALLOC_NORMAL,ts,1,0,0,BCTVTYPE_BOOL,sizeof(char),&dp,1,&fv);
    *fv=v?1.0:0.0;
    *dp=v?1:0;
    return d;
}

pia_data_t pia_data_t::allocate_string(pic::nballocator_t *a, const char *s, unsigned sl,unsigned long long ts)
{
    float *v;
    unsigned char *dp;

    pia_data_t d = allocate_host(a,PIC_ALLOC_NORMAL,ts,1,-1,0,BCTVTYPE_STRING,sl,&dp,1,&v);
    *v=0;
    memcpy(dp,s,sl);
    return d;
}

pia_data_t pia_data_t::allocate_cstring(pic::nballocator_t *a, const char *s, unsigned sl,unsigned long long ts)
{
    float *v;
    unsigned char *dp;

    pia_data_t d = allocate_host(a,PIC_ALLOC_NORMAL,ts,1,-1,0,BCTVTYPE_STRING,sl,&dp,1,&v);
    *v=0;
    memcpy(dp,s,sl);
    return d;
}

pia_data_t pia_data_t::allocate_cstring_nb(pic::nballocator_t *a, const char *s, unsigned sl,unsigned long long ts)
{
    float *v;
    unsigned char *dp;

    pia_data_t d = allocate_host(a,PIC_ALLOC_NB,ts,1,-1,0,BCTVTYPE_STRING,sl,&dp,1,&v);
    *v=0;
    memcpy(dp,s,sl);
    return d;
}

pia_data_t pia_data_t::path_append(pic::nballocator_t *a, const pia_data_t &d, unsigned e)
{
    unsigned char *dp;
    float *v;
    unsigned pl = d.aspathlen();

    pia_data_t nd = allocate_host(a,PIC_ALLOC_NORMAL,0,1,-1,0,BCTVTYPE_PATH,pl+2,&dp,1,&v);
    *v=0;
    dp[0]=0;
    memcpy(&dp[1],d.aspath(),pl);
    dp[1+pl]=e;

    return nd;
}

pia_data_t pia_data_t::allocate_path(pic::nballocator_t *a, unsigned pl, const unsigned char *p, unsigned long long ts)
{
    unsigned char *dp;
    float *v;

    pia_data_t nd = allocate_host(a,PIC_ALLOC_NORMAL,ts,1,-1,0,BCTVTYPE_PATH,pl+1,&dp,1,&v);
    *v=0;
    dp[0]=0;
    memcpy(&dp[1],p,pl);

    return nd;
}

pia_data_t pia_data_t::allocate_buffer(pic::nballocator_t *a, unsigned hl, unsigned char **hp)
{
    return pia_data_t::allocate_host(a,PIC_ALLOC_NB,0,1,-1,0,BCTVTYPE_BLOB,hl,hp,0,0);
}

/*
 * pia_data_nb_t
 */

pia_data_nb_t pia_data_nb_t::allocate_host(pic::nballocator_t *a, unsigned long long ts,float u,float l,float r,unsigned t, unsigned hl, unsigned char **hp, unsigned vl, float **vp)
{
    return pia_data_nb_t::from_given(allocate_host_raw(a,PIC_ALLOC_NB,ts,u,l,r,t,hl,hp,vl,vp));
}

pia_data_nb_t pia_data_nb_t::allocate_wire(pic::nballocator_t *a, unsigned wl, const unsigned char *wp)
{
    return pia_data_nb_t::from_given(allocate_wire_raw(a,PIC_ALLOC_NB,wl,wp));
}

/*
 * pia_dataholder_nb_t
 */

pia_dataholder_nb_t::pia_dataholder_nb_t() : data_(0) { }

pia_dataholder_nb_t::~pia_dataholder_nb_t()
{
    clear();
}

pia_dataholder_nb_t::pia_dataholder_nb_t(const pia_dataholder_nb_t &h): data_(h.data_)
{
    pia_data_incref_fast(data_);
}

pia_dataholder_nb_t &pia_dataholder_nb_t::operator=(const pia_dataholder_nb_t &h)
{
    if(data_!=h.data_)
    {
        clear();
        data_=h.data_;
        pia_data_incref_fast(data_);
    }
    return *this;
}

void pia_dataholder_nb_t::set(const pia_data_nb_t &d)
{
    clear();

    data_ = d.give();
}

void pia_dataholder_nb_t::clear()
{
    if(data_)
    {
        pia_data_decref_fast(data_);
        data_ = 0;
    }
}

const pia_data_nb_t pia_dataholder_nb_t::get_nb() const
{
    if(data_)
    {
        return pia_data_nb_t::from_lent(data_);
    }
    else
    {
        return pia_data_nb_t();
    }
}

const pia_data_t pia_dataholder_nb_t::get_normal(pic::nballocator_t *a) const
{
    if(data_)
    {
        return pia_data_t::from_given(make_copy(a, PIC_ALLOC_NORMAL, data_));
    }
    else
    {
        return pia_data_t();
    }
}

pia_dataholder_nb_t::operator pia_data_nb_t() const
{
    return get_nb();
}
