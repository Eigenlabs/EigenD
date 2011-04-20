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

#include <piembedded/pie_wire.h>
#include "pia_bclock.h"

#include <picross/pic_config.h>

#include "pia_data.h"

#include <picross/pic_atomic.h>
#include <picross/pic_time.h>
#include <picross/pic_thread.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BK_LOCK(t) pic::rwmutex_t t
#define BK_LOCK_GUARD(g)
#define BK_LOCK_INIT(l)
#define BK_LOCK_TERM(l)
#define BK_LOCK_WRITE(l,f)  l.wlock()
#define BK_UNLOCK_WRITE(l,f)  l.wunlock()
#define BK_LOCK_READ(l,f)  l.rlock()
#define BK_UNLOCK_READ(l,f)  l.runlock()

typedef struct pie_pll_s
{
    int age;
    unsigned timeshift;

    int64_t gross_offset;
    long delay_reg;
    long delay;

    uint64_t last_clock;
    uint64_t last_time;
    int64_t offset;
    int64_t frequency;

    int no_delay_comp;
} pie_pll_t;

struct pie_clock_t::impl_t: pie_bkendpointops_t
{
    impl_t(pie_bkernel_t *kernel);
    ~impl_t();

    void close_callback(int hnd, void *ctx);
    void data_callback(int hnd, void *ctx, const unsigned char *header, const void *payload, unsigned paylen);

    void pie_clock_data(const void *payload, unsigned paylen);
    void pie_clock_tick(void);
    int pie_clock_term(void);
    uint64_t pie_clock_get(void);
    long pie_clock_get_delay(void);
    int64_t pie_clock_get_delta(void);
    uint64_t pie_clock_get_id(void);
    int pie_clock_get_mode(void);
    void pie_clock_set_loop(uint64_t id);

    pie_bkernel_t *kernel_;
    int clockep_;

    BK_LOCK(_lock);

    uint64_t pie_clk_slave;
    int pie_clk_isslave;
    uint64_t pie_clk_master;
    unsigned pie_clk_delay_ctr;
    unsigned pie_clk_election;
    uint64_t pie_clk_self;
    pie_pll_t pie_clk_mainpll;
    pie_pll_t pie_clk_testpll;

};

#define CLOCK_MSG_MASTER           1
#define CLOCK_MSG_REQUEST_RTT      2
#define CLOCK_MSG_RESPOND_RTT      3
#define CLOCK_MSG_LOOP             4

#define CLOCK_MSG_TYPE_SZ       1
#define CLOCK_MSG_ID_SZ         8
#define CLOCK_MSG_TIME_SZ       8

#define CLOCK_MSG_TYPE         (0)
#define CLOCK_MSG_MASTER_ID    (CLOCK_MSG_TYPE      + CLOCK_MSG_TYPE_SZ)
#define CLOCK_MSG_SLAVE_ID     (CLOCK_MSG_MASTER_ID + CLOCK_MSG_ID_SZ)
#define CLOCK_MSG_TX_TIME      (CLOCK_MSG_SLAVE_ID  + CLOCK_MSG_ID_SZ)
#define CLOCK_MSG_RX_TIME      (CLOCK_MSG_TX_TIME   + CLOCK_MSG_TIME_SZ)

#define CLOCK_MSG_LEN          (CLOCK_MSG_TYPE_SZ+2*CLOCK_MSG_ID_SZ+2*CLOCK_MSG_TIME_SZ)

#define ELECTION_TIMER         5 /* seconds */

static void __bcget_random_bytes(void *p_, unsigned sz)
{
    unsigned char *p = (unsigned char *)p_;

    while(sz>0)
    {
        *p++ = rand() & 0xff;
        sz--;
    }
}

static int _msg_extract(const unsigned char *payload, unsigned paylen, uint64_t *mid, uint64_t *sid, uint64_t *tx, uint64_t *rx)
{
    int type;

    if(paylen != CLOCK_MSG_LEN)
    {
        return -1;
    }

    type=payload[CLOCK_MSG_TYPE];

    pie_getu64(payload+CLOCK_MSG_MASTER_ID, 8, mid);
    pie_getu64(payload+CLOCK_MSG_SLAVE_ID, 8, sid);
    pie_getu64(payload+CLOCK_MSG_TX_TIME, 8, tx);
    pie_getu64(payload+CLOCK_MSG_RX_TIME, 8, rx);

    return type;
}

static void _msg_format(unsigned char *buffer, unsigned char type, uint64_t mid, uint64_t sid, uint64_t tx, uint64_t rx)
{
    memset(buffer,0,CLOCK_MSG_LEN);

    buffer[CLOCK_MSG_TYPE]=type;
    pie_setu64(buffer+CLOCK_MSG_MASTER_ID,8,mid);
    pie_setu64(buffer+CLOCK_MSG_SLAVE_ID,8,sid);
    pie_setu64(buffer+CLOCK_MSG_TX_TIME,8,tx);
    pie_setu64(buffer+CLOCK_MSG_RX_TIME,8,rx);
}

#define TRANSIT_TIME_FACTOR ((long)(1<<6))

static void pie_pll_update_delay(pie_pll_t *pll, uint64_t send_time,uint64_t recv_time)
{
    long this_delay=((recv_time-send_time) >> 1);
    long f=pll->delay_reg/TRANSIT_TIME_FACTOR;
    long delta=this_delay-f;

    if(this_delay>6000L) /* ignore delay extrema which may occur in dsp usb link */
    {
        return;
    }

    pll->delay_reg+=delta;
    pll->delay=pll->delay_reg/TRANSIT_TIME_FACTOR;
}

#define FREQUENCY_WEIGHT_SHIFT 16
#define PHASE_WEIGHT_SHIFT 26
#define PHASE_PRECISION_SHIFT 4
#define FREQUENCY_DIVISOR_SHIFT 40 /* 1000000000000, to within a percent */
#define SETTLED 300

static uint64_t pie_pll_interp(pie_pll_t *pll, uint64_t slave_clock)
{
    if(pll->last_time)
    {
        int64_t clock_change=slave_clock-pll->last_clock;
        return pll->last_time
            +clock_change
            +((clock_change*pll->frequency) >> (FREQUENCY_DIVISOR_SHIFT-PHASE_PRECISION_SHIFT))
            +(pll->offset >> PHASE_PRECISION_SHIFT);
    }

    return 0;
}

static uint64_t _get_pll(pie_pll_t *pll)
{
    return pie_pll_interp(pll,pic_microtime()-pll->gross_offset)+pll->delay;
}

static uint64_t _get_pll_ndc(pie_pll_t *pll)
{
    return pie_pll_interp(pll,pic_microtime()-pll->gross_offset);
}

static void pie_pll_update(pie_pll_t *pll, uint64_t master_time)
{
    if(pll->last_time)
    {
        uint64_t slave_clock = pic_microtime()-pll->gross_offset;
        uint64_t slave_time = pie_pll_interp(pll,slave_clock);
        int64_t offset=master_time-slave_time;
        int64_t interval=slave_time-pll->last_time;
        int64_t product=interval*offset;

        pll->offset = product >> (PHASE_WEIGHT_SHIFT-pll->timeshift-PHASE_PRECISION_SHIFT);
        pll->frequency = pll->frequency+(product >> (FREQUENCY_WEIGHT_SHIFT-2*pll->timeshift));

        //printf("upd pll: clk=%llu time=%llu master=%llu offset=%lld ivl=%lld prd=%lld shift=%d age=%d pllo=%lld pllf=%lld\n",slave_clock,slave_time,master_time,offset,interval,product,pll->timeshift,pll->age,pll->offset,pll->frequency);

        pll->last_clock=slave_clock;
        pll->last_time=slave_time;
    }
    else
    {
        pll->gross_offset=pic_microtime()-master_time;
        pll->last_clock=master_time;
        pll->last_time=master_time;
        pll->age=0;
        pll->delay_reg=0;
        pll->delay=0;
        pll->offset=0;
        pll->frequency=0;
        pll->timeshift=6;
    }
}

static void pie_pll_age(pie_pll_t *pll)
{
    if(!pll->last_time)
    {
        return;
    }

    if(pll->age>=SETTLED)
    {
        pll->timeshift=1;
        return;
    }

    pll->age++;

    if(pll->age>SETTLED-2*(64-(64>>1)))
    {
        pll->timeshift=2;
        return;
    }

    if(pll->age>SETTLED-2*(64-(64>>2)))
    {
        pll->timeshift=3;
        return;
    }

    if(pll->age>SETTLED-2*(64-(64>>3)))
    {
        pll->timeshift=4;
        return;
    }

    if(pll->age>SETTLED-2*(64-(64>>4)))
    {
        pll->timeshift=5;
        return;
    }
    
    pll->timeshift=6;
}

static void pie_pll_freewheel(pie_pll_t *pll)
{
    if(pll->last_time)
    {
        uint64_t slave_clock = pic_microtime()-pll->gross_offset;
        uint64_t slave_time = pie_pll_interp(pll,slave_clock);

        pll->offset = 0;
        pll->last_clock=slave_clock;
        pll->last_time=slave_time;
    }
    else
    {
        uint64_t t = pic_microtime();

        pll->last_clock=t;
        pll->last_time=t;
        pll->gross_offset=0;
        pll->age=SETTLED;
        pll->delay_reg=0;
        pll->delay=0;
        pll->offset=0;
        pll->frequency=0;
        pll->timeshift=1;
    }
}

static void pie_pll_reset(pie_pll_t *pll)
{
    pll->last_time=0;
}

void pie_clock_t::impl_t::pie_clock_tick(void)
{
    int hz = 0;
    BK_LOCK_GUARD(grd);
    unsigned char buffer[CLOCK_MSG_LEN];

    BK_LOCK_WRITE(_lock,grd);

    if(++pie_clk_delay_ctr >= PIE_CLOCK_TICK_FREQ)
    {
        pie_clk_delay_ctr=0;
        pie_pll_age(&pie_clk_mainpll);
        pie_pll_age(&pie_clk_testpll);
        hz = 1;
    }

    if(hz && pie_clk_master != pie_clk_self)
    {
        if(--pie_clk_election == 0)
        {
            printf("%llu becoming master\n",(unsigned long long)pie_clk_self);
            pie_clk_master=pie_clk_self;
            pie_clk_isslave=0;
        }
    }

    if(pie_clk_master==pie_clk_self)
    {
        pie_pll_freewheel(&pie_clk_mainpll);

        _msg_format(buffer,CLOCK_MSG_MASTER,pie_clk_self,pie_clk_slave,_get_pll(&pie_clk_mainpll),0);
        BK_UNLOCK_WRITE(_lock,grd);
        kernel_->pie_bkwrite(clockep_,buffer,CLOCK_MSG_LEN);
        BK_LOCK_WRITE(_lock,grd);

        if(hz && pie_clk_slave)
        {
            _msg_format(buffer,CLOCK_MSG_REQUEST_RTT,pie_clk_slave,pie_clk_self,_get_pll_ndc(&pie_clk_testpll),0);
            BK_UNLOCK_WRITE(_lock,grd);
            kernel_->pie_bkwrite(clockep_,buffer,CLOCK_MSG_LEN);
            BK_LOCK_WRITE(_lock,grd);
        }
    }
    else
    {
        if(hz && pie_clk_master)
        {
            _msg_format(buffer,CLOCK_MSG_REQUEST_RTT,pie_clk_master,pie_clk_self,_get_pll_ndc(&pie_clk_mainpll),0);
            BK_UNLOCK_WRITE(_lock,grd);
            kernel_->pie_bkwrite(clockep_,buffer,CLOCK_MSG_LEN);
            BK_LOCK_WRITE(_lock,grd);
        }

        if(pie_clk_isslave)
        {
            _msg_format(buffer,CLOCK_MSG_LOOP,pie_clk_self,0,_get_pll(&pie_clk_mainpll),0);
            BK_UNLOCK_WRITE(_lock,grd);
            kernel_->pie_bkwrite(clockep_,buffer,CLOCK_MSG_LEN);
            BK_LOCK_WRITE(_lock,grd);
        }
    }

    BK_UNLOCK_WRITE(_lock,grd);
}

void pie_clock_t::impl_t::pie_clock_data(const void *payload, unsigned paylen)
{
    int type;
    uint64_t mid = 0;
    uint64_t sid = 0;
    uint64_t rx = 0;
    uint64_t tx = 0;

    BK_LOCK_GUARD(grd);

    if((type=_msg_extract((const unsigned char *)payload,paylen,&mid,&sid,&tx,&rx)) < 0)
    {
        return;
    }

    BK_LOCK_WRITE(_lock,grd);

    switch(type)
    {
        case CLOCK_MSG_MASTER:
            if(mid != pie_clk_self)
            {
                pie_clk_election = ELECTION_TIMER;

                if(mid > pie_clk_master)
                {
                    printf("clock slaving to %llu\n",(unsigned long long)mid);
                    pie_clk_master=mid;
                    pie_pll_reset(&pie_clk_mainpll);
                    pie_pll_update(&pie_clk_mainpll,tx);
                    pie_clk_isslave=0;
                }

                if(pie_clk_master!=pie_clk_self)
                {
                    int isslave = (pie_clk_self==sid);

                    pie_pll_update(&pie_clk_mainpll,tx);

                    if(isslave != pie_clk_isslave)
                    {
                        pie_clk_isslave=isslave;

                        if(isslave)
                        {
                            printf("starting loopback\n");
                        }
                    }
                }
            }
            break;

        case CLOCK_MSG_REQUEST_RTT:
            if(mid == pie_clk_self)
            {
                unsigned char buffer[CLOCK_MSG_LEN];

                _msg_format(buffer,CLOCK_MSG_RESPOND_RTT,pie_clk_self,sid,tx,_get_pll(&pie_clk_mainpll));

                BK_UNLOCK_WRITE(_lock,grd);
                kernel_->pie_bkwrite(clockep_,buffer,CLOCK_MSG_LEN);
                BK_LOCK_WRITE(_lock,grd);
            }
            break;

        case CLOCK_MSG_RESPOND_RTT:
            if(pie_clk_master!=pie_clk_self && pie_clk_master==mid && sid == pie_clk_self)
            {
                pie_pll_update_delay(&pie_clk_mainpll,tx,_get_pll_ndc(&pie_clk_mainpll));
            }

            if(pie_clk_master==pie_clk_self && pie_clk_slave==mid && pie_clk_self==sid)
            {
                pie_pll_update_delay(&pie_clk_testpll,rx,_get_pll_ndc(&pie_clk_testpll));
            }

            break;

        case CLOCK_MSG_LOOP:
            if(pie_clk_master==pie_clk_self && mid==pie_clk_slave)
            {
                pie_pll_update(&pie_clk_testpll,tx);
            }
            break;
    }

    BK_UNLOCK_WRITE(_lock,grd);

}

uint64_t pie_clock_t::impl_t::pie_clock_get(void)
{
    uint64_t t;
    BK_LOCK_GUARD(grd);

    BK_LOCK_READ(_lock, grd);
    t=_get_pll(&pie_clk_mainpll);
    BK_UNLOCK_READ(_lock, grd);

    return t;
}

int64_t pie_clock_t::impl_t::pie_clock_get_delta(void)
{
    int64_t delta;
    BK_LOCK_GUARD(grd);

    BK_LOCK_READ(_lock, grd);
    delta = (pie_clk_master==pie_clk_self && pie_clk_slave != 0ULL) ? (_get_pll(&pie_clk_mainpll)-_get_pll(&pie_clk_testpll)) : 0LL;
    BK_UNLOCK_READ(_lock, grd);

    return delta;
}

int pie_clock_t::impl_t::pie_clock_get_mode(void)
{
    return pie_clk_self==pie_clk_master;
}

long pie_clock_t::impl_t::pie_clock_get_delay(void)
{
    return pie_clk_mainpll.delay;
}

uint64_t pie_clock_t::impl_t::pie_clock_get_id(void)
{
    return pie_clk_self;
}

void pie_clock_t::impl_t::pie_clock_set_loop(uint64_t id)
{
    BK_LOCK_GUARD(grd);

    BK_LOCK_WRITE(_lock, grd);

    if(pie_clk_master==pie_clk_self)
    {
        pie_clk_slave=id;
        pie_pll_reset(&pie_clk_testpll);
        printf("signalling loopback\n");
    }

    BK_UNLOCK_WRITE(_lock, grd);
}

pie_clock_t::impl_t::impl_t(pie_bkernel_t *kernel): kernel_(kernel)
{
    BK_LOCK_INIT(_lock); /* INITIALISED */

    __bcget_random_bytes(&pie_clk_self,sizeof(pie_clk_self));

    pie_clk_self |= 0x01;
    pie_clk_self &= 0x7fffffffffffffULL;
    pie_clk_master=0; /* INITIALISED, always starts as a slave */
    pie_clk_delay_ctr=0;
    pie_clk_slave=0;
    pie_clk_election=ELECTION_TIMER;
    pie_clk_isslave=0;
    pie_pll_reset(&pie_clk_mainpll);

    //printf("clock starting, my id is %llu\n",pie_clk_self);

    pie_bkaddr_t name;

    name.length=5;
    name.space=BCTLINK_NAMESPACE_CLOCK;
    memcpy(name.data,"clock",5);
    clockep_=kernel_->pie_bkopen(this,0,&name,0);
}

pie_clock_t::impl_t::~impl_t()
{
    kernel_->pie_bkclose(clockep_,0);
}

pie_clock_t::pie_clock_t(pie_bkernel_t *kernel): impl_(new impl_t(kernel))
{
}

pie_clock_t::~pie_clock_t()
{
    delete impl_;
}

void pie_clock_t::impl_t::close_callback(int hnd, void *ctx)
{
}

void pie_clock_t::impl_t::data_callback(int hnd, void *ctx, const unsigned char *header, const void *payload, unsigned paylen)
{
    pie_clock_data(payload,paylen);
}

void pie_clock_t::pie_clock_data(const void *payload, unsigned paylen)
{
    impl_->pie_clock_data(payload,paylen);
}

void pie_clock_t::pie_clock_tick(void)
{
    impl_->pie_clock_tick();
}

uint64_t pie_clock_t::pie_clock_get(void)
{
    return impl_->pie_clock_get();
}

long pie_clock_t::pie_clock_get_delay(void)
{
    return impl_->pie_clock_get_delay();
}

int64_t pie_clock_t::pie_clock_get_delta(void)
{
    return impl_->pie_clock_get_delta();
}

uint64_t pie_clock_t::pie_clock_get_id(void)
{
    return impl_->pie_clock_get_id();
}

int pie_clock_t::pie_clock_get_mode(void)
{
    return impl_->pie_clock_get_mode();
}

void pie_clock_t::pie_clock_set_loop(uint64_t id)
{
    return impl_->pie_clock_set_loop(id);
}
