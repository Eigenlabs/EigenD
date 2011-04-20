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

/*
 * This file is designed to be included into the target application.
 * The target application should implement the required support
 * functions.
 */

#include <pibelcanto/link.h>

#include "pia_bkernel.h"
#include "pia_data.h"

#include <picross/pic_atomic.h>
#include <picross/pic_time.h>
#include <picross/pic_thread.h>
#include <picross/pic_config.h>
#include <picross/pic_log.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BK_MAX_ENDPOINTS (5*1024)
#define BK_MAX_DEVICES 8
#define BK_MAX_GROUPS (5*1024)

#define BK_LOCK(t) pic::rwmutex_t t
#define BK_LOCK_GUARD(g)
#define BK_LOCK_INIT(l)
#define BK_LOCK_TERM(l)
#define BK_LOCK_WRITE(l,f)  l.wlock()
#define BK_UNLOCK_WRITE(l,f)  l.wunlock()
#define BK_LOCK_READ(l,f)  l.rlock()
#define BK_UNLOCK_READ(l,f)  l.runlock()

#define BK_ENDPOINT_MASK ((BK_MAX_ENDPOINTS+7)/8)
#define BK_DEVICE_MASK ((BK_MAX_DEVICES+7)/8)
#define BK_GROUP_MASK ((BK_MAX_GROUPS+7)/8)

#define MASK_INDEX(h) ((h)/8)
#define MASK_BIT(h) (1<<((h)%8))

#define BK_TICKS_GROUP_DEATH (5)
#define BK_TICKS_ROUTE_LIFE (5)

typedef struct _BkGroup
{
    unsigned char endpoints[BK_ENDPOINT_MASK];
    unsigned char devices[BK_DEVICE_MASK];
    unsigned char devices_acc[BK_DEVICE_MASK];
    unsigned char attachs[BK_DEVICE_MASK];
    unsigned endpoint_count;
    pic_atomic_t in_use;
    unsigned death_tick;
    pie_bkaddr_t name;
    unsigned short hash;
    unsigned dead;
} BkGroup;

typedef struct _BkEndpoint
{
    pie_bkendpointops_t *ops;
    void *ctx;
    int group;
    pic_atomic_t in_use;
    int server;
} BkEndpoint;

typedef struct _BkDevice
{
    pie_bkdeviceops_t *ops;
    void *ctx;
    unsigned headroom;
    pie_bkaddr_t addr;
    unsigned dead;
    pic_atomic_t in_use;
} BkDevice;

struct pie_bkernel_t::impl_t
{
    impl_t(const pie_bkaddr_t *id);
    ~impl_t();

    int pie_bkopen(pie_bkendpointops_t *ops, void *ctx, const pie_bkaddr_t *group, int promisc);
    void *pie_bkcontext(int ep);
    int pie_bkclose(int ep, void **arg);
    int pie_bkgroupname(int ep, pie_bkaddr_t *);
    int pie_bkwrite(int ep, const void *payload, unsigned paylen);
    unsigned pie_bkheadroom(void);
    void pie_bkgetaddr(pie_bkaddr_t *);
    void pie_bktick(void);
    int pie_bkadddevice(void *ctx, const pie_bkaddr_t *id, unsigned headroom, pie_bkdeviceops_t *ops);
    void pie_bkremovedevice(int dev, int unhook);
    int pie_bkfinddevice(void *ctx);
    int pie_bkfinddevice2(pie_bkdeviceops_t *, int (*)(void *ctx, void *arg), void *arg);
    void *pie_bkfinddevice3(pie_bkdeviceops_t *, int (*)(void *ctx, void *arg), void *arg);
    void pie_bkdata(int dev, const unsigned char *header, const void *payload, unsigned paylen);
    int pie_bkernel_creategroup(const pie_bkaddr_t *name, int dev, int promisc, int create);
    void bk_route_in_group(int dev, const pie_bkaddr_t *n);
    void bk_route_in(int dev, const unsigned char *payload, unsigned paylen);

    void __BkRecalcHeadroom(void);
    void bk_route_accumulate(void);
    void bk_expire_group(void);
    void bk_route_send(void);

    BK_LOCK(__BkLock);
    BkEndpoint *__BkEndpoints[BK_MAX_ENDPOINTS];
    BkDevice *__BkDevices[BK_MAX_ENDPOINTS];
    BkGroup *__BkGroups[BK_MAX_GROUPS];
    unsigned long __BkCounter;
    pie_bkaddr_t __BkRootAddr;
    unsigned __pie_bkheadroom;
    int __BkRootValid;
    pie_bkaddr_t __BkRouteName;
    unsigned short __BkRouteHash;
    unsigned __BkDivider;
    unsigned __BkDeviceCount;
};

static void __bkget_random_bytes(void *p_, unsigned sz)
{
    unsigned char *p = (unsigned char *)p_;

    while(sz>0)
    {
        *p++ = rand() & 0xff;
        sz--;
    }
}

/*
static int __BkMaskNotEmpty(const unsigned char *m, unsigned l)
{
    unsigned i;

    for(i=0;i<l;i++)
    {
        if(m[i])
        {
            return 1;
        }
    }

    return 0;
}
*/

static int __BkCmpName(const pie_bkaddr_t *a, const pie_bkaddr_t *b)
{
    if(a->space==b->space && a->length==b->length)
    {
        return !memcmp(a->data,b->data,a->length);
    }

    return 0;
}

static unsigned char __BkFirstBitTable[256] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

static unsigned char __BkFirstBitMasks[8] = {
    0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80
};

/*
static unsigned char __BkBitCountTable[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

static unsigned __BkBitCount(const unsigned char *table, unsigned size)
{
    unsigned i,c=0;

    for(i=0;i<size;i++)
    {
        c+=__BkBitCountTable[table[i]];
    }

    return c;
}
*/

static int __BkNextBit(const unsigned char *table, unsigned size, int start)
{
    unsigned i,o;
    unsigned char m;

    o=start%8;
    i=start/8;

    if(i<size)
    {
        m=table[i]&__BkFirstBitMasks[o];

        if(m)
        {
            return __BkFirstBitTable[m]+8*i;
        }

        while(++i<size)
        {
            m=table[i];
            if(m)
            {
                return __BkFirstBitTable[m]+8*i;
            }
        }
    }

    return -1;
}

void pie_bkernel_t::impl_t::__BkRecalcHeadroom(void)
{
    int i;
    unsigned h = 0;

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        if(__BkDevices[i] && !__BkDevices[i]->dead)
        {
            if(__BkDevices[i]->headroom > h)
            {
                h = __BkDevices[i]->headroom;
            }
        }
    }

    __pie_bkheadroom = h+BCTLINK_HEADER;
}

void pie_bkernel_t::pie_bkaddrhash(const pie_bkaddr_t *addr, void *hash, unsigned hlen)
{
    unsigned char *dp = (unsigned char *)hash;
    unsigned char i,seed;

    memset(dp, 0, hlen);
    seed = addr->length+addr->space;
    seed&=0xff;

    for(i=0; i<addr->length && i<sizeof(addr->data); i++)
    {
        seed += (13+addr->data[i]);
        seed&=0xff;

        dp[i%hlen] ^= seed;
        dp[i%hlen] &= 0xff;
    }
}

pie_bkernel_t::impl_t::impl_t(const pie_bkaddr_t *id)
{
    int i;

    BK_LOCK_INIT(__BkLock);

    for(i=0;i<BK_MAX_ENDPOINTS;i++)
    {
        __BkEndpoints[i] = 0;
    }

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        __BkDevices[i] = 0;
    }

    for(i=0;i<BK_MAX_GROUPS;i++)
    {
        __BkGroups[i] = 0;
    }

    __BkRecalcHeadroom();
    __BkCounter=0;


    if(id)
    {
        memcpy(&__BkRootAddr,id,sizeof(pie_bkaddr_t));
        __BkRootValid = 1;
    }
    else
    {
        unsigned char buffer[6];
        __bkget_random_bytes(buffer,6);
        __BkRootAddr.length = 5+12;
        memcpy(__BkRootAddr.data,(const void *)"/rnd/XXXXXXXXXXXX",5+12);
        pie_bin2hex(buffer,6,(char *)__BkRootAddr.data+5);
        __BkRootValid = 0;
    }

    __BkRouteName.space=BCTLINK_NAMESPACE_ROUTE;
    __BkRouteName.length=5;
    memcpy(&__BkRouteName.data,"route",5);
    pie_bkaddrhash(&__BkRouteName,&__BkRouteHash,sizeof(__BkRouteHash));

    __BkDivider=0;
    __BkDeviceCount=0;
}

pie_bkernel_t::impl_t::~impl_t()
{
}

void pie_bkernel_t::impl_t::pie_bkgetaddr(pie_bkaddr_t *addr)
{
    BK_LOCK_GUARD(grd);
    int i;
    pie_bkaddr_t *p = &__BkRootAddr;

    BK_LOCK_WRITE(__BkLock, grd);

    if(!__BkRootValid)
    {
        for(i=0;i<BK_MAX_DEVICES;i++)
        {
            if(__BkDevices[i])
            {
                p = &__BkDevices[i]->addr;
                break;
            }
        }
    }

    memcpy(addr,p,sizeof(pie_bkaddr_t));
    addr->data[addr->length++]='/';
    pie_bin2hex(&__BkCounter,sizeof(__BkCounter),(char *)addr->data+addr->length);
    __BkCounter++;

    BK_UNLOCK_WRITE(__BkLock, grd);
}

void *pie_bkernel_t::impl_t::pie_bkcontext(int ep)
{
    BkEndpoint *e;
    void *c = 0;
    BK_LOCK_GUARD(grd);

    BK_LOCK_READ(__BkLock,grd);

    if((e=__BkEndpoints[ep]) != 0)
    {
        c=e->ctx;
    }

    BK_UNLOCK_READ(__BkLock,grd);
    return c;
}

int pie_bkernel_t::impl_t::pie_bkclose(int h, void **arg)
{
    BkEndpoint *e;
    BkGroup *g;
    BK_LOCK_GUARD(grd);

    BK_LOCK_WRITE(__BkLock,grd);

    if(!(e=__BkEndpoints[h]))
    {
        BK_UNLOCK_WRITE(__BkLock,grd);
        return -1;
    }

    //e->ops=0;

    if(e->group >= 0)
    {
        g=__BkGroups[e->group];
        e->group=-1;
        g->endpoints[MASK_INDEX(h)] &= ~MASK_BIT(h);
        g->endpoint_count--;
    }

    BK_UNLOCK_WRITE(__BkLock,grd);

    while(e->in_use != 0)
    {
        pic_thread_yield();
    }

    BK_LOCK_WRITE(__BkLock,grd);
    __BkEndpoints[h]=0;
    BK_UNLOCK_WRITE(__BkLock,grd);

    if(arg)
    {
        *arg = e->ctx;
    }

    free(e);
    return 0;
}

int pie_bkernel_t::impl_t::pie_bkernel_creategroup(const pie_bkaddr_t *name, int dev, int promisc, int create)
{
    BkGroup *g;
    unsigned short hash;
    BK_LOCK_GUARD(grd);
    unsigned char devices[BK_DEVICE_MASK];
    int i,f;

    memset(devices,0,sizeof(devices));
    pie_bkernel_t::pie_bkaddrhash(name,&hash,sizeof(hash));

    BK_LOCK_WRITE(__BkLock,grd);

    for(i=0,f=-1;i<BK_MAX_GROUPS;i++)
    {
        if((g=__BkGroups[i]))
        {
            if(!g->dead && hash==g->hash && __BkCmpName(name,&g->name))
            {
                pic_atomicinc(&g->in_use);
                BK_UNLOCK_WRITE(__BkLock,grd);
                return i;
            }
        }
        else
        {
            if(f<0) f=i;
        }
    }

    if(!create)
    {
        BK_UNLOCK_WRITE(__BkLock,grd);
        return -1;
    }

    if(dev>=0 && __BkDeviceCount<=1)
    {
        BK_UNLOCK_WRITE(__BkLock,grd);
        return -1;
    }

    if(f<0 || !(g=(BkGroup *)malloc(sizeof(BkGroup))))
    {
        BK_UNLOCK_WRITE(__BkLock,grd);
        return -1;
    }

    g->endpoint_count = 0;
    g->death_tick = 0;
    g->hash = hash;
    g->in_use=1;
    g->dead = 0;

    memset(g->endpoints,0,sizeof(g->endpoints));
    memset(g->devices,0,sizeof(g->devices));
    memset(g->devices_acc,0,sizeof(g->devices));
    memset(g->attachs,0,sizeof(g->attachs));

    memcpy(&g->name,name,sizeof(g->name));

    __BkGroups[f] = g;

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        if(__BkDevices[i] && !__BkDevices[i]->dead)
        {
            g->attachs[MASK_INDEX(i)] |= MASK_BIT(i);
            devices[MASK_INDEX(i)] |= MASK_BIT(i);
            if(promisc)
            {
                g->devices[MASK_INDEX(i)] |= MASK_BIT(i);
                g->devices_acc[MASK_INDEX(i)] |= MASK_BIT(i);
            }
            pic_atomicinc(&__BkDevices[i]->in_use);
        }
    }

    BK_UNLOCK_WRITE(__BkLock,grd);

    for(i=__BkNextBit(devices,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(devices,BK_DEVICE_MASK,i+1))
    {
        (__BkDevices[i]->ops->addgroup_callback)(__BkDevices[i]->ctx,name);
        pic_atomicdec(&__BkDevices[i]->in_use);
    }

    return f;
}

void pie_bkernel_t::impl_t::bk_route_accumulate(void)
{
    BK_LOCK_GUARD(grd);
    int g;

    BK_LOCK_WRITE(__BkLock,grd);

    for(g=0;g<BK_MAX_GROUPS;g++)
    {
        if(__BkGroups[g])
        {
            memcpy(__BkGroups[g]->devices, __BkGroups[g]->devices_acc, BK_DEVICE_MASK);
            memset(__BkGroups[g]->devices_acc,0,BK_DEVICE_MASK);
        }
    }

    BK_UNLOCK_WRITE(__BkLock,grd);
}

void pie_bkernel_t::impl_t::bk_expire_group(void)
{
    BkGroup *g;
    BK_LOCK_GUARD(grd);
    unsigned char endpoints[BK_ENDPOINT_MASK];
    int i,j;

    memset(endpoints,0,sizeof(endpoints));

    BK_LOCK_WRITE(__BkLock,grd);

    for(j=0;j<BK_MAX_GROUPS;j++)
    {
        g=__BkGroups[j];

        if(!g)
        {
            continue;
        }

        if(g->endpoint_count>0)
        {
            g->death_tick=0;
            continue;
        }

        g->death_tick++;

        if(g->death_tick < BK_TICKS_GROUP_DEATH || g->in_use>0 )
        {
            continue;
        }

        g->dead=1;

    printf("expire  group %d %c%c%c%c%c%c%c%c\n",j,
                g->name.data[0], g->name.data[1], g->name.data[2], g->name.data[3], g->name.data[4], g->name.data[5], g->name.data[6], g->name.data[7]);

        for(i=__BkNextBit(g->endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(g->endpoints,BK_ENDPOINT_MASK,i+1))
        {
            __BkEndpoints[i]->group=-1;
            pic_atomicinc(&__BkEndpoints[i]->in_use);
        }

        for(i=__BkNextBit(g->attachs,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(g->attachs,BK_DEVICE_MASK,i+1))
        {
            pic_atomicinc(&__BkDevices[i]->in_use);
        }
    }

    BK_UNLOCK_WRITE(__BkLock,grd);

    for(j=0;j<BK_MAX_GROUPS;j++)
    {
        g=__BkGroups[j];

        if(!g || !g->dead)
        {
            continue;
        }

        for(i=__BkNextBit(g->endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(g->endpoints,BK_ENDPOINT_MASK,i+1))
        {
            (__BkEndpoints[i]->ops->close_callback)(i, __BkEndpoints[i]->ctx);
        }

        for(i=__BkNextBit(g->attachs,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(g->attachs,BK_DEVICE_MASK,i+1))
        {
            (__BkDevices[i]->ops->delgroup_callback)(__BkDevices[i]->ctx, &g->name);
        }
    }

    BK_LOCK_WRITE(__BkLock,grd);

    for(j=0;j<BK_MAX_GROUPS;j++)
    {
        g=__BkGroups[j];

        if(!g || !g->dead)
        {
            continue;
        }

        for(i=__BkNextBit(g->endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(g->endpoints,BK_ENDPOINT_MASK,i+1))
        {
            pic_atomicdec(&__BkEndpoints[i]->in_use);
        }

        for(i=__BkNextBit(g->attachs,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(g->attachs,BK_DEVICE_MASK,i+1))
        {
            pic_atomicdec(&__BkDevices[i]->in_use);
        }

        __BkGroups[j] = 0;
        free(g);
    }

    BK_UNLOCK_WRITE(__BkLock,grd);
}

int pie_bkernel_t::impl_t::pie_bkgroupname(int h, pie_bkaddr_t *name)
{
    BkEndpoint *e;
    BK_LOCK_GUARD(grd);

    BK_LOCK_READ(__BkLock, grd);

    if(!(e=__BkEndpoints[h]))
    {
        BK_UNLOCK_READ(__BkLock, grd);
        return -1;
    }

    if(e->group < 0)
    {
        BK_UNLOCK_READ(__BkLock, grd);
        return -1;
    }

    memcpy(name,&__BkGroups[e->group]->name, sizeof(__BkGroups[e->group]->name));
    BK_UNLOCK_READ(__BkLock, grd);
    return 0;
}

unsigned pie_bkernel_t::impl_t::pie_bkheadroom(void)
{
    return __pie_bkheadroom;
}

int pie_bkernel_t::impl_t::pie_bkadddevice(void *ctx, const pie_bkaddr_t *id, unsigned headroom, pie_bkdeviceops_t *ops)
{
    BK_LOCK_GUARD(grd);
    int i,j;
    BkDevice *d;
    unsigned char groups[BK_GROUP_MASK];

    if(!(d=(BkDevice *)malloc(sizeof(BkDevice))))
    {
        return -1;
    }

    d->ctx = ctx;
    d->ops = ops;
    d->headroom = headroom;
    d->dead=0;
    d->in_use=1;

    if(id)
    {
        memcpy(&d->addr,id,sizeof(pie_bkaddr_t));
    }

    BK_LOCK_WRITE(__BkLock,grd);

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        if(!__BkDevices[i])
        {
            goto found_slot;
        }
    }

    BK_UNLOCK_WRITE(__BkLock,grd);
    free(d);
    return -1;

found_slot:

    __BkDevices[i] = d;
    memset(groups,0,sizeof(groups));

    for(j=0;j<BK_MAX_GROUPS;j++)
    {
        if(!__BkGroups[j] || __BkGroups[j]->dead)
        {
            continue;
        }

        groups[MASK_INDEX(j)] |= MASK_BIT(j);
        pic_atomicinc(&__BkGroups[j]->in_use);
    }

    __BkRecalcHeadroom();

    BK_UNLOCK_WRITE(__BkLock,grd);

    (ops->addgroup_callback)(ctx,&__BkRouteName);

    for(j=__BkNextBit(groups,BK_GROUP_MASK,0); j>=0; j=__BkNextBit(groups,BK_GROUP_MASK,j+1))
    {
        (ops->addgroup_callback)(ctx,&__BkGroups[j]->name);
    }

    BK_LOCK_WRITE(__BkLock,grd);

    for(j=__BkNextBit(groups,BK_GROUP_MASK,0); j>=0; j=__BkNextBit(groups,BK_GROUP_MASK,j+1))
    {
        __BkGroups[j]->attachs[MASK_INDEX(i)] |= MASK_BIT(i);
        pic_atomicdec(&__BkGroups[j]->in_use);
    }

    pic_atomicdec(&d->in_use);
    __BkDeviceCount++;

    BK_UNLOCK_WRITE(__BkLock,grd);

    return i;
}

void pie_bkernel_t::impl_t::pie_bkremovedevice(int h, int uh)
{
    BK_LOCK_GUARD(grd);
    BkDevice *d;
    int i;
    unsigned char groups[BK_GROUP_MASK];

    BK_LOCK_WRITE(__BkLock,grd);

    if(!(d=__BkDevices[h]))
    {
        BK_UNLOCK_WRITE(__BkLock,grd);
        return;
    }

    memset(groups,0,sizeof(groups));

    for(i=0;i<BK_MAX_GROUPS;i++)
    {
        if(!__BkGroups[i] || __BkGroups[i]->dead)
        {
            continue;
        }

        __BkGroups[i]->devices_acc[MASK_INDEX(h)] &= ~MASK_BIT(h);
        __BkGroups[i]->devices[MASK_INDEX(h)] &= ~MASK_BIT(h);
        __BkGroups[i]->attachs[MASK_INDEX(h)] &= ~MASK_BIT(h);

        if(uh)
        {
            groups[MASK_INDEX(i)] |= MASK_BIT(i);
            pic_atomicinc(&__BkGroups[i]->in_use);
        }
    }

    d->dead=1;

    if(uh)
    {
        BK_UNLOCK_WRITE(__BkLock,grd);

        (d->ops->delgroup_callback)(d->ctx,&__BkRouteName);

        for(i=__BkNextBit(groups,BK_GROUP_MASK,0); i>=0; i=__BkNextBit(groups,BK_GROUP_MASK,i+1))
        {
            (d->ops->delgroup_callback)(d->ctx,&__BkGroups[i]->name);
        }

        BK_LOCK_WRITE(__BkLock,grd);
    }

    for(i=__BkNextBit(groups,BK_GROUP_MASK,0); i>=0; i=__BkNextBit(groups,BK_GROUP_MASK,i+1))
    {
        pic_atomicdec(&__BkGroups[i]->in_use);
    }

    __BkDevices[h] = 0;
    BK_UNLOCK_WRITE(__BkLock,grd);

    while(d->in_use != 0)
    {
        pic_thread_yield();
    }

    BK_LOCK_WRITE(__BkLock,grd);
    __BkDevices[h] = 0;
    __BkDeviceCount--;
    BK_UNLOCK_WRITE(__BkLock,grd);

    free(d);
}

int pie_bkernel_t::impl_t::pie_bkfinddevice(void *ctx)
{
    BK_LOCK_GUARD(grd);
    int i;

    BK_LOCK_READ(__BkLock,grd);

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        if(__BkDevices[i] && __BkDevices[i]->ctx == ctx)
        {
            BK_UNLOCK_READ(__BkLock,grd);
            return i;
        }
    }

    BK_UNLOCK_READ(__BkLock,grd);
    return -1;
}

int pie_bkernel_t::impl_t::pie_bkfinddevice2(pie_bkdeviceops_t *ops, int (*finder)(void *ctx, void *arg), void *arg)
{
    BK_LOCK_GUARD(grd);
    int i;

    BK_LOCK_READ(__BkLock,grd);

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        if(__BkDevices[i] && __BkDevices[i]->ops==ops && (!finder || (*finder)(__BkDevices[i]->ctx,arg)))
        {
            BK_UNLOCK_READ(__BkLock,grd);
            return i;
        }
    }

    BK_UNLOCK_READ(__BkLock,grd);
    return -1;
}

void *pie_bkernel_t::impl_t::pie_bkfinddevice3(pie_bkdeviceops_t *ops, int (*finder)(void *ctx, void *arg), void *arg)
{
    BK_LOCK_GUARD(grd);
    int i;

    BK_LOCK_READ(__BkLock,grd);

    for(i=0;i<BK_MAX_DEVICES;i++)
    {
        if(__BkDevices[i] && __BkDevices[i]->ops==ops && (!finder || (*finder)(__BkDevices[i]->ctx,arg)))
        {
            arg = __BkDevices[i]->ctx;
            BK_UNLOCK_READ(__BkLock,grd);
            return arg;
        }
    }

    BK_UNLOCK_READ(__BkLock,grd);
    return 0;
}

int pie_bkernel_t::impl_t::pie_bkwrite(int h, const void *payload, unsigned paylen)
{
    BkEndpoint *e;
    BK_LOCK_GUARD(grd);
    BkGroup *g;
    int i;
    unsigned char endpoints[BK_ENDPOINT_MASK];
    unsigned char devices[BK_DEVICE_MASK];
    unsigned char header[BCTLINK_HEADER];
    unsigned len;

    BK_LOCK_READ(__BkLock,grd);

    if(!(e=__BkEndpoints[h]))
    {
        BK_UNLOCK_READ(__BkLock,grd);
        return -1;
    }

    if(e->group < 0)
    {
        BK_UNLOCK_READ(__BkLock,grd);
        return -1;
    }

    g = __BkGroups[e->group];

    memcpy(endpoints,g->endpoints,sizeof(endpoints));
    memcpy(devices,g->devices,sizeof(devices));

    for(i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,i+1))
    {
        pic_atomicinc(&__BkEndpoints[i]->in_use);
    }

    for(i=__BkNextBit(devices,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(devices,BK_DEVICE_MASK,i+1))
    {
        pic_atomicinc(&__BkDevices[i]->in_use);
    }

    BK_UNLOCK_READ(__BkLock,grd);

    len=paylen+BCTLINK_HEADER;

    header[BCTLINK_MAGIC1]=BCTLINK_MAGIC1VAL;
    header[BCTLINK_MAGIC2]=BCTLINK_MAGIC2VAL;
    header[BCTLINK_LEN_HI]=(len>>8)&0xff;
    header[BCTLINK_LEN_LO]=len&0xff;
    header[BCTLINK_GROUP_LEN]=g->name.length;
    header[BCTLINK_NAMESPACE]=g->name.space;

    memcpy(&header[BCTLINK_GROUP],g->name.data,BCTLINK_GROUP_SIZE);

    for(i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,i+1))
    {
        if(i!=h) (__BkEndpoints[i]->ops->data_callback)(i, __BkEndpoints[i]->ctx,header,payload,paylen);
        pic_atomicdec(&__BkEndpoints[i]->in_use);
    }

    for(i=__BkNextBit(devices,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(devices,BK_DEVICE_MASK,i+1))
    {
        __BkDevices[i]->ops->write_callback(__BkDevices[i]->ctx, &g->name, header, payload, paylen);
        pic_atomicdec(&__BkDevices[i]->in_use);
    }

    return 0;
}

void pie_bkernel_t::impl_t::bk_route_send(void)
{
    BK_LOCK_GUARD(grd);
    unsigned char devices[BK_DEVICE_MASK];
    unsigned char groups[BK_GROUP_MASK];
    int d,g;
    unsigned paylen, namlen, namspc;
    unsigned char *paydata;
    unsigned char header[BCTLINK_HEADER];
    unsigned char payload[BCTLINK_SMALLPAYLOAD];

    memset(devices,0,sizeof(devices));
    memset(groups,0,sizeof(groups));

    header[BCTLINK_GROUP_LEN]=__BkRouteName.length;
    header[BCTLINK_NAMESPACE]=__BkRouteName.space;
    memcpy(&header[BCTLINK_GROUP],__BkRouteName.data,__BkRouteName.length);

    BK_LOCK_READ(__BkLock,grd);

    for(g=0;g<BK_MAX_GROUPS;g++)
    {
        if(__BkGroups[g])
        {
            groups[MASK_INDEX(g)] |= MASK_BIT(g);
            pic_atomicinc(&__BkGroups[g]->in_use);
        }
    }

    for(d=0;d<BK_MAX_DEVICES;d++)
    {
        if(__BkDevices[d])
        {
            devices[MASK_INDEX(d)] |= MASK_BIT(d);
            pic_atomicinc(&__BkDevices[d]->in_use);
        }
    }

    BK_UNLOCK_READ(__BkLock,grd);

    for(d=__BkNextBit(devices,BK_DEVICE_MASK,0); d>=0; d=__BkNextBit(devices,BK_DEVICE_MASK,d+1))
    {
        paylen=0;

        for(g=__BkNextBit(groups,BK_GROUP_MASK,0); g>=0; g=__BkNextBit(groups,BK_GROUP_MASK,g+1))
        {
            int f=0;

            BK_LOCK_READ(__BkLock,grd);

            if(__BkGroups[g]->endpoint_count>0)
            {
                f=1;
            }

            BK_UNLOCK_READ(__BkLock,grd);

            if(f)
            {
                namlen=__BkGroups[g]->name.length;
                namspc=__BkGroups[g]->name.space;

                if(paylen+namlen+2 > sizeof(payload))
                {
                    header[BCTLINK_MAGIC1]=BCTLINK_MAGIC1VAL;
                    header[BCTLINK_MAGIC2]=BCTLINK_MAGIC2VAL;
                    header[BCTLINK_LEN_HI]=((paylen+BCTLINK_HEADER)>>8)&0xff;
                    header[BCTLINK_LEN_LO]=(paylen+BCTLINK_HEADER)&0xff;

                    __BkDevices[d]->ops->write_callback(__BkDevices[d]->ctx, &__BkRouteName, header, payload, paylen);

                    paylen=0;
                }

                paydata = &payload[paylen];

                paydata[0]=namspc;
                paydata[1]=namlen;

                memcpy(&paydata[2],__BkGroups[g]->name.data,namlen);

                paylen+=2+namlen;
            }
        }

        if(paylen>0)
        {
            header[BCTLINK_MAGIC1]=BCTLINK_MAGIC1VAL;
            header[BCTLINK_MAGIC2]=BCTLINK_MAGIC2VAL;
            header[BCTLINK_LEN_HI]=((paylen+BCTLINK_HEADER)>>8)&0xff;
            header[BCTLINK_LEN_LO]=(paylen+BCTLINK_HEADER)&0xff;
            __BkDevices[d]->ops->write_callback(__BkDevices[d]->ctx, &__BkRouteName, header, payload, paylen);
        }
    }

    for(d=__BkNextBit(devices,BK_DEVICE_MASK,0); d>=0; d=__BkNextBit(devices,BK_DEVICE_MASK,d+1))
    {
        pic_atomicdec(&__BkDevices[d]->in_use);
    }

    for(g=__BkNextBit(groups,BK_GROUP_MASK,0); g>=0; g=__BkNextBit(groups,BK_GROUP_MASK,g+1))
    {
        pic_atomicdec(&__BkGroups[g]->in_use);
    }
}

void pie_bkernel_t::impl_t::bk_route_in_group(int dev, const pie_bkaddr_t *n)
{
    int g,f;
    BK_LOCK_GUARD(grd);
    unsigned char old[BK_DEVICE_MASK];

    if((g=pie_bkernel_creategroup(n,dev,0,0))<0)
    {
        return;
    }

    BK_LOCK_WRITE(__BkLock,grd);

    memcpy(old,__BkGroups[g]->devices,BK_DEVICE_MASK);
    __BkGroups[g]->devices_acc[MASK_INDEX(dev)] |= MASK_BIT(dev);
    __BkGroups[g]->devices[MASK_INDEX(dev)] |= MASK_BIT(dev);

    f=0; if(memcmp(old,__BkGroups[g]->devices,BK_DEVICE_MASK)) f=1;

    BK_UNLOCK_WRITE(__BkLock,grd);

    if(f)
    {
        bk_route_send();
    }

    pic_atomicdec(&__BkGroups[g]->in_use);
}

void pie_bkernel_t::impl_t::bk_route_in(int dev, const unsigned char *payload, unsigned paylen)
{
    unsigned char hdr[3];
    pie_bkaddr_t name;
    unsigned i=0;

    while(paylen>=2)
    {
        memcpy(hdr,&payload[i],3);

        if(2U+hdr[1] <= paylen)
        {
            name.space=hdr[0];
            name.length=hdr[1];
            memcpy(name.data,&payload[i+2],name.length);
            i+=(2+name.length);
            paylen-=(2+name.length);
            bk_route_in_group(dev,&name);
        }
        else
        {
            printf("malformed routing packet %d vs %d\n",2+hdr[1],paylen);
            return;
        }
    }
}

static void __debug_addr(const unsigned char *header)
{
    unsigned i;
    printf("addr.len=%d addr.space=%d addr.name=\"",header[BCTLINK_GROUP_LEN],header[BCTLINK_NAMESPACE]);
    for(i=0; i<BCTLINK_GROUP_SIZE; ++i)
    {
        unsigned char c = header[BCTLINK_GROUP+i];
        printf("%c",isprint(c) ? c : '.');
    }
    printf("\"\n");
}

void pie_bkernel_t::impl_t::pie_bkdata(int dev, const unsigned char *header, const void *payload, unsigned paylen)
{
    pie_bkaddr_t gn;
    unsigned short hash;
    BK_LOCK_GUARD(grd);
    BkGroup *g;
    int i;
    unsigned plen;
    unsigned char endpoints[BK_ENDPOINT_MASK];
    //unsigned char devices[BK_DEVICE_MASK];

    if(header[BCTLINK_MAGIC1] != BCTLINK_MAGIC1VAL || header[BCTLINK_MAGIC2] != BCTLINK_MAGIC2VAL)
    {
        printf("malformed packet: bad magic\n");
        return;
    }

    plen=(header[BCTLINK_LEN_HI] << 8 | header[BCTLINK_LEN_LO] << 0 );

    if(plen!=paylen+BCTLINK_HEADER)
    {
        printf("malformed packet: too short %d for payload %d\n",paylen+BCTLINK_HEADER,plen);
        __debug_addr(header);
        return;
    }

    paylen=plen-BCTLINK_HEADER;

    gn.length = header[BCTLINK_GROUP_LEN];
    gn.space = header[BCTLINK_NAMESPACE];

    if(gn.length>BCTLINK_GROUP_SIZE)
    {
        printf("malformed packet: address too big (%d)\n",gn.length);
        __debug_addr(header);
        return;
    }

    memcpy(gn.data,&header[BCTLINK_GROUP],gn.length);
    pie_bkaddrhash(&gn,&hash,sizeof(hash));

    if(hash==__BkRouteHash && __BkCmpName(&gn,&__BkRouteName))
    {
        bk_route_in(dev,(const unsigned char *)payload,paylen);
        return;
    }

    BK_LOCK_READ(__BkLock,grd);

    for(i=0;i<BK_MAX_GROUPS;i++)
    {
        g = __BkGroups[i];

        if(g && !g->dead && g->hash == hash && __BkCmpName(&g->name,&gn))
        {
            goto found_group;
        }
    }

    BK_UNLOCK_READ(__BkLock,grd);
    return;

found_group:

    if(paylen==0)
    {
        BK_UNLOCK_READ(__BkLock,grd);
        return;
    }

    memcpy(endpoints,g->endpoints,sizeof(endpoints));

    //memcpy(devices,g->devices,sizeof(devices));
    //devices[MASK_INDEX(dev)] &= ~MASK_BIT(dev);

    for(i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,i+1))
    {
        pic_atomicinc(&__BkEndpoints[i]->in_use);
    }

    /*
    for(i=__BkNextBit(devices,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(devices,BK_DEVICE_MASK,i+1))
    {
        pic_atomicinc(&__BkDevices[i]->in_use);
    }
    */

    BK_UNLOCK_READ(__BkLock,grd);

    for(i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,0); i>=0; i=__BkNextBit(endpoints,BK_ENDPOINT_MASK,i+1))
    {
        (__BkEndpoints[i]->ops->data_callback)(i, __BkEndpoints[i]->ctx,header,payload,paylen);
        pic_atomicdec(&__BkEndpoints[i]->in_use);
    }

    /*
    for(i=__BkNextBit(devices,BK_DEVICE_MASK,0); i>=0; i=__BkNextBit(devices,BK_DEVICE_MASK,i+1))
    {
        __BkDevices[i]->ops->write_callback(__BkDevices[i]->ctx, &g->name, header, payload, paylen);
        pic_atomicdec(&__BkDevices[i]->in_use);
    }
    */

}

int pie_bkernel_t::impl_t::pie_bkopen(pie_bkendpointops_t *ops, void *ctx, const pie_bkaddr_t *name, int promisc)
{
    BkEndpoint *e;
    BK_LOCK_GUARD(grd);
    int i,f,g;

    if((g=pie_bkernel_creategroup(name,-1,promisc,1))<0)
    {
        return -1;
    }

    if(!(e=(BkEndpoint *)malloc(sizeof(BkEndpoint))))
    {
        pic_atomicdec(&__BkGroups[g]->in_use);
        return -1;
    }

    BK_LOCK_WRITE(__BkLock,grd);

    for(i=0;i<BK_MAX_ENDPOINTS;i++)
    {
        if(!__BkEndpoints[i])
        {
            goto found_slot;
        }
    }

    pic_atomicdec(&__BkGroups[g]->in_use);
    BK_UNLOCK_WRITE(__BkLock,grd);
    free(e);
    return -1;

found_slot:

    e->ops = ops;
    e->ctx = ctx;
    e->group = g;
    e->in_use=0;
    __BkEndpoints[i]=e;

    f=__BkGroups[g]->endpoint_count;
    __BkGroups[g]->endpoint_count++;
    __BkGroups[g]->endpoints[MASK_INDEX(i)] |= MASK_BIT(i);

    BK_UNLOCK_WRITE(__BkLock,grd);

    if(f==0)
    {
        bk_route_send();
    }

    return i;
}

void pie_bkernel_t::impl_t::pie_bktick(void)
{
    __BkDivider++;

    bk_expire_group();

    if(__BkDivider%(BK_TICKS_ROUTE_LIFE*2)==0)
    {
        bk_route_accumulate();
    }

    if(__BkDivider%BK_TICKS_ROUTE_LIFE==0)
    {
        bk_route_send();
    }
}

void pie_bkernel_t::pie_bin2hex(const void *bin, unsigned binlen, char *hex)
{
    const char *digits = "0123456789abcdef";
    const char *in = (const char *)bin;

    while(binlen>0)
    {
        *hex++ = digits[((*in)>>4)&0x0f];
        *hex++ = digits[(*in)&0x0f];
        in++; binlen--;
    }
}

int pie_bkernel_t::pie_bkopen(pie_bkendpointops_t *ops, void *ctx, const pie_bkaddr_t *group, int promisc)
{
    return impl_->pie_bkopen(ops,ctx,group,promisc);
}

void *pie_bkernel_t::pie_bkcontext(int ep)
{
    return impl_->pie_bkcontext(ep);
}

int pie_bkernel_t::pie_bkclose(int ep, void **arg)
{
    return impl_->pie_bkclose(ep,arg);
}

int pie_bkernel_t::pie_bkgroupname(int ep, pie_bkaddr_t *addr)
{
    return impl_->pie_bkgroupname(ep,addr);
}

int pie_bkernel_t::pie_bkwrite(int ep, const void *payload, unsigned paylen)
{
    return impl_->pie_bkwrite(ep,payload,paylen);
}

unsigned pie_bkernel_t::pie_bkheadroom(void)
{
    return impl_->pie_bkheadroom();
}

void pie_bkernel_t::pie_bkgetaddr(pie_bkaddr_t *addr)
{
    impl_->pie_bkgetaddr(addr);
}

void pie_bkernel_t::pie_bktick(void)
{
    impl_->pie_bktick();
}

int pie_bkernel_t::pie_bkadddevice(void *ctx, const pie_bkaddr_t *id, unsigned headroom, pie_bkdeviceops_t *ops)
{
    return impl_->pie_bkadddevice(ctx,id,headroom,ops);
}

void pie_bkernel_t::pie_bkremovedevice(int dev, int unhook)
{
    return impl_->pie_bkremovedevice(dev,unhook);
}

int pie_bkernel_t::pie_bkfinddevice(void *ctx)
{
    return impl_->pie_bkfinddevice(ctx);
}

int pie_bkernel_t::pie_bkfinddevice2(pie_bkdeviceops_t *ops, int (*filter)(void *ctx, void *arg), void *arg)
{
    return impl_->pie_bkfinddevice2(ops,filter,arg);
}

void *pie_bkernel_t::pie_bkfinddevice3(pie_bkdeviceops_t *ops, int (*filter)(void *ctx, void *arg), void *arg)
{
    return impl_->pie_bkfinddevice3(ops,filter,arg);
}

void pie_bkernel_t::pie_bkdata(int dev, const unsigned char *header, const void *payload, unsigned paylen)
{
    impl_->pie_bkdata(dev,header,payload,paylen);
}

pie_bkernel_t::pie_bkernel_t(const pie_bkaddr_t *id): impl_(new impl_t(id))
{
}

pie_bkernel_t::~pie_bkernel_t()
{
    delete impl_;
}
