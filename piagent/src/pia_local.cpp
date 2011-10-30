
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

#include "pia_local.h"
#include "pia_server.h"
#include "pia_glue.h"
#include "pia_buffer.h"
#include "pia_data.h"
#include "pia_error.h"

#include <pibelcanto/link.h>

#include <piembedded/pie_message.h>
#include <piembedded/pie_print.h>

#include <piagent/pia_network.h>

#include <picross/pic_log.h>
#include <picross/pic_error.h>
#include <picross/pic_ilist.h>
#include <picross/pic_strbase.h>

#include <stdlib.h>
#include <stddef.h>

#define PIA_TIMER_INTERVAL 1
#define PIA_SLOW_TIMER_SILENCE 30
#define PIA_SLOW_TIMER_START PIA_SLOW_TIMER_SILENCE
#define PIA_FAST_TIMER_SILENCE 30
#define PIA_FAST_TIMER_START PIA_FAST_TIMER_SILENCE
#define PIA_FAST_DECIMATOR 30

struct pia_lroot_t;
struct pia_lnode_t;

struct pia_locallist_t::impl_t
{
    pic::ilist_t<pia_lroot_t> servers_;
};

struct pia_linterlock_t: virtual pic::lckobject_t, virtual pic::atomic_counted_t
{
    unsigned fasttick_;
};

struct pia_ldelegate_t: pia_server_t::fast_receiver_t
{
    pia_ldelegate_t(const pia_cref_t &cpoint,pia_lnode_t *node, pia_eventq_t *mainq);

    bool evt_received(const pia_data_nb_t &,const pia_dataqueue_t &);
    bool data_received(const pia_data_nb_t &);

    pia_lnode_t *node_;
    pia_eventq_t *mainq_;
    pia_cref_t cpoint_;
    pic::ref_t<pia_linterlock_t> linterlock_;
    unsigned ddecimator_;
};

struct pia_lnode_t: pia_server_t
{
    pia_lnode_t(pia_lnode_t *p, bct_server_t *s, unsigned char l);
    pia_lnode_t(const pia_data_t &a, const pia_ctx_t &e, bct_server_t *s);
    ~pia_lnode_t();

    pia_lroot_t *root();

    void server_closed();
    void server_childgone();
    void server_changed(const pia_data_t &);
    void server_childadded();
    pic::ref_t<pia_server_t::fast_receiver_t> server_fast() { return pic::ref(new pia_ldelegate_t(cpoint_,this,glue()->mainq())); }
    unsigned server_fastflags() { return 0; }

    static void api_close(bct_server_host_ops_t **);
    static bct_data_t api_servername(bct_server_host_ops_t **);
    static bct_data_t api_path(bct_server_host_ops_t **);
    static int api_child_add(bct_server_host_ops_t **,unsigned char, bct_server_t *);
    static int api_shutdown(bct_server_host_ops_t **);
    static void api_changed(bct_server_host_ops_t **,bct_data_t);
    static int api_advertise(bct_server_host_ops_t **,const char *);
    static int api_unadvertise(bct_server_host_ops_t **,const char *);
    static void api_cancel(bct_server_host_ops_t **);
    static void api_setclock(bct_server_host_ops_t **,bct_clocksink_t *);
    static void api_set_source(bct_server_host_ops_t **,bct_fastdata_t *);
    static void api_setflags(bct_server_host_ops_t **,unsigned);

    static void job_transient_data(void *n_, const pia_data_t &d);
    static void job_transient_id(void *n_, const pia_data_t &d);
    static void job_close(void *n_, const pia_data_t &d);
    static void job_open(void *n_, const pia_data_t &d);
    static int format_visitor(void *f_, unsigned char n, pia_server_t *s);

    void transmit_tree();
    void transmit_fast_data();
    void transmit_fast_id();
    void transmit_data(const pia_data_t &d);
    void ongoing();

    bool iscomplete() { return ((flags()&(PLG_SERVER_FAST))==source_flags_);  }

    bct_server_host_ops_t *_client_ops;
    pia_job_t _job_open;
    pia_job_t _job_close;
    bct_server_t *_client;
    unsigned short source_flags_;
    pia_cref_t cpoint_;

    static bct_server_host_ops_t dispatch__;
};

struct pia_lroot_t: pia_buffer_t, pia_lnode_t, pic::element_t<>
{
    pia_lroot_t(pia_locallist_t::impl_t *l, const pia_data_t &a, const pia_ctx_t &e, bct_server_t *s);
    ~pia_lroot_t();

    void server_closed(void);

    void buffer_receive_fast(const unsigned char *msg, unsigned len);
    void buffer_fixup_fast(unsigned char *msg, unsigned len);
    void buffer_receive_slow(const unsigned char *msg, unsigned len);
    void buffer_fixup_slow(unsigned char *msg, unsigned len);

    static void job_sync(void *r_, const pia_data_t &d);
    static void job_timer(void *r_);

    unsigned long next_seq() { return ++sequence_; }

    pia_timerhnd_t _job_timer;
    pia_job_t _job_sync;
    pia_locallist_t::impl_t *_list;
    unsigned slowtick_;
    unsigned long sync_seq_;
    pic::ref_t<pia_linterlock_t> linterlock_;
    unsigned long sequence_;
    unsigned long old_tseq_;
    pia_buffer_t *buffer_;
};

pia_lroot_t *pia_lnode_t::root()
{
    return static_cast<pia_lroot_t *>(pia_server_t::root());
}

void pia_lnode_t::job_transient_id(void *n_, const pia_data_t & d)
{
    pia_lnode_t *n = (pia_lnode_t *)n_;
    unsigned char *b;
    unsigned l,o;

    if(n->root()->linterlock_->fasttick_ >= PIA_FAST_TIMER_SILENCE)
    {
        return;
    }

    pia_data_t path = n->path();
    l = pie_stanzalen_fevt(path.aspathlen(), d.wirelen(), 0);

    b=n->root()->buffer_begin_transmit_fast(l,false);
    o=pie_setstanza(b,l,BCTMTYPE_IDNT_EVT,(const unsigned char *)path.aspath(),path.aspathlen(), 0);
    pie_setdata(b+o,l-o,0,d.wirelen(),d.wiredata());
    n->root()->buffer_flush_fast();
    n->root()->buffer_end_transmit_fast();

    return;
}

void pia_lnode_t::job_transient_data(void *n_, const pia_data_t & d)
{
    pia_lnode_t *n = (pia_lnode_t *)n_;
    unsigned char *b;
    unsigned l,o;

    if(n->root()->linterlock_->fasttick_ >= PIA_FAST_TIMER_SILENCE)
    {
        return;
    }

    pia_data_t path = n->path();
    l = pie_stanzalen_fevt(path.aspathlen(), d.wirelen(), 0);

    b=n->root()->buffer_begin_transmit_fast(l,false);
    o=pie_setstanza(b,l,BCTMTYPE_FAST_EVT,(const unsigned char *)path.aspath(),path.aspathlen(), 0);
    pie_setdata(b+o,l-o,0,d.wirelen(),d.wiredata());
    n->root()->buffer_flush_fast();
    n->root()->buffer_end_transmit_fast();

    return;
}

pia_ldelegate_t::pia_ldelegate_t(const pia_cref_t &cpoint,pia_lnode_t *node, pia_eventq_t *mainq): node_(node), mainq_(mainq), cpoint_(cpoint), linterlock_(node->root()->linterlock_)
{
}

bool pia_ldelegate_t::evt_received(const pia_data_nb_t &d,const pia_dataqueue_t &q)
{
    ddecimator_=0;

    if(linterlock_->fasttick_ >= PIA_FAST_TIMER_SILENCE)
    {
        return true;
    }

    if(!d.is_null())
    {
        pia_data_nb_t d2;
        unsigned long long i=0;
        unsigned long long time = d.time();

        if(q.latest(d2,&i,time))
        {
            //fastdata_receive_data(d.restamp(time));
            mainq_->idle(cpoint_,pia_lnode_t::job_transient_data, node_, d2.make_normal(node_->entity()->glue()->allocator(), PIC_ALLOC_NB));
            i++;
        }
        else
        {
            if(!q.earliest(d2,&i,time))
            {
                mainq_->idle(cpoint_,pia_lnode_t::job_transient_id, node_, d.make_normal(node_->entity()->glue()->allocator(), PIC_ALLOC_NB));
                return true;
            }

            mainq_->idle(cpoint_,pia_lnode_t::job_transient_data, node_, d2.make_normal(node_->entity()->glue()->allocator(), PIC_ALLOC_NB));
            i++;
        }

        while(q.read(d2,&i,~0ULL))
        {
            mainq_->idle(cpoint_,pia_lnode_t::job_transient_data, node_, d2.make_normal(node_->entity()->glue()->allocator(), PIC_ALLOC_NB));
            i++;
        }
    }

    mainq_->idle(cpoint_,pia_lnode_t::job_transient_id, node_, d.make_normal(node_->entity()->glue()->allocator(), PIC_ALLOC_NB));
    return true;
}

bool pia_ldelegate_t::data_received(const pia_data_nb_t &d)
{
    if(linterlock_->fasttick_ < PIA_FAST_TIMER_SILENCE)
    {
        if((ddecimator_++)%PIA_FAST_DECIMATOR<2)
        {
            mainq_->idle(cpoint_,pia_lnode_t::job_transient_data, node_, d.make_normal(node_->entity()->glue()->allocator(), PIC_ALLOC_NB));
        }
    }

    return true;
}

struct _formatter
{
    unsigned char *msg;
    unsigned len;
    unsigned used;
    unsigned count;
};

int pia_lnode_t::format_visitor(void *f_, unsigned char n, pia_server_t *s)
{
    if(s->isvisible())
    {
        struct _formatter *f = (struct _formatter *)f_;
        int i=pie_settevtpath(f->msg+f->used,f->len-f->used, n, s->tseq());
        f->used+=i;
        f->count++;
    }

    return 1;
}

void pia_lnode_t::transmit_tree()
{
    unsigned char *b;
    unsigned l,o,v;
    struct _formatter f;

    if(root()->slowtick_ >= PIA_SLOW_TIMER_SILENCE)
    {
        return;
    }

    pia_data_t p = path();
    v = visible_children();
    l = pie_stanzalen_tevt(p.aspathlen(), v, 0);

    b=root()->buffer_begin_transmit_slow(l);
    o=pie_setstanza(b,l,BCTMTYPE_TREE_EVT,(const unsigned char *)p.aspath(),p.aspathlen(), 0);
    o+=pie_setevthdr(b+o,l-o,dseq(),nseq(),tseq());
    root()->buffer_end_transmit_slow();

    f.msg=b;
    f.used=o;
    f.len=l;
    f.count=0;

    visit(&f,format_visitor);
    f.used+=pie_setlastpath(f.msg+f.used,f.len-f.used);
}

void pia_lnode_t::transmit_fast_id()
{
    if(root()->linterlock_->fasttick_ >= PIA_FAST_TIMER_SILENCE)
        pic::logmsg() << addr() << " fast going active";

    root()->linterlock_->fasttick_=0;

    if(isfast())
    {
        glue()->mainq()->idle(cpoint_,job_transient_id, this, fastdata()->fastdata_current_normal(glue()->allocator(), true));
    }
}

void pia_lnode_t::transmit_fast_data()
{
    if(root()->linterlock_->fasttick_ >= PIA_FAST_TIMER_SILENCE)
        pic::logmsg() << addr() << " fast going active";

    root()->linterlock_->fasttick_=0;

    if(isfast())
    {
        glue()->mainq()->idle(cpoint_,job_transient_data, this, fastdata()->fastdata_current_normal(glue()->allocator(), false));
    }
}

void pia_lnode_t::transmit_data(const pia_data_t &d)
{
    unsigned char *b;
    unsigned l,o;

    if(root()->slowtick_ >= PIA_SLOW_TIMER_SILENCE)
    {
        return;
    }

    pia_data_t p = path();
    l = pie_stanzalen_devt(p.aspathlen(), d.wirelen(), 0);

    b=root()->buffer_begin_transmit_slow(l);
    o=pie_setstanza(b,l,BCTMTYPE_DATA_EVT,(const unsigned char *)p.aspath(),p.aspathlen(), 0);
    o+=pie_setevthdr(b+o,l-o,dseq(),nseq(),tseq());

    pie_setdata(b+o,l-o,flags(),d.wirelen(),d.wiredata());
    root()->buffer_end_transmit_slow();
}

void pia_lroot_t::job_sync(void *r_, const pia_data_t & d)
{
    pia_lroot_t *r = (pia_lroot_t *)r_;

    try
    {
        pia_mainguard_t guard(r->entity()->glue());
        r->sync_seq_ = r->tseq();
        r->transmit_tree();
        r->sync(1);
    }
    PIA_CATCHLOG_EREF(r->entity())
}

void pia_lnode_t::ongoing()
{
    pia_lroot_t *r = root();

    if(r->insync())
    {
        r->sync(0);
        r->_job_sync.idle(r->entity()->appq(),pia_lroot_t::job_sync,r,pia_data_t());
    }
}

void pia_lnode_t::server_childgone()
{
    set_nseq(root()->next_seq());
    transmit_tree();
}

void pia_lnode_t::server_childadded()
{
    set_nseq(root()->next_seq());
}

void pia_lnode_t::job_close(void *n_, const pia_data_t & d)
{
    pia_lnode_t *n = (pia_lnode_t *)n_;
    bct_server_plug_closed(n->_client,n->entity()->api());
}

void pia_lnode_t::job_open(void *n_, const pia_data_t & d)
{
    pia_lnode_t *n = (pia_lnode_t *)n_;
    n->_client->plg_state=PLG_STATE_OPENED;
    bct_server_plug_opened(n->_client,n->entity()->api());
}

bct_data_t pia_lnode_t::api_servername(bct_server_host_ops_t **s)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_logguard_t guard(n->entity()->glue());
        return n->addr().give_copy(n->entity()->glue()->allocator(), PIC_ALLOC_NORMAL);
    }
    PIA_CATCHLOG_EREF(n->entity())

    return 0;
}

bct_data_t pia_lnode_t::api_path(bct_server_host_ops_t **s)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_logguard_t guard(n->entity()->glue());
        return n->path().give();
    }
    PIA_CATCHLOG_EREF(n->entity())

    return 0;
}

void pia_lnode_t::server_changed(const pia_data_t &d)
{
    transmit_data(d);
}

void pia_lnode_t::api_changed(bct_server_host_ops_t **s, bct_data_t d)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            pia_data_t d2(pia_data_t::from_lent(d));

            if(n->current() != d2)
            {
                n->ongoing();
                n->change_slow(d2,++n->root()->sequence_);
            }
        }
    }
    PIA_CATCHLOG_EREF(n->entity())
}

void pia_lnode_t::api_close(bct_server_host_ops_t **s)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        n->close();
        delete n;
    }
    PIA_CATCHLOG_EREF(n->entity())
}

int pia_lnode_t::api_shutdown(bct_server_host_ops_t **s)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            n->close();
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(n->entity())

    return -1;
}

int pia_lnode_t::api_advertise(bct_server_host_ops_t **s, const char *i)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            pia_data_t id = n->entity()->glue()->allocate_address(i);
            if(!id) return PLG_STATUS_ADDR;
            n->glue()->advertise(id,n->root(),n->addr(), n->getcookie());
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(n->entity())

    return -1;
}

int pia_lnode_t::api_unadvertise(bct_server_host_ops_t **s, const char *i)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            pia_data_t id = n->entity()->glue()->allocate_address(i);
            if(!id) return PLG_STATUS_ADDR;
            n->glue()->unadvertise(id,n->root(),n->addr());
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(n->entity())

    return -1;
}

void pia_lnode_t::api_cancel(bct_server_host_ops_t **s)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            n->glue()->killadvertise(n->root());
        }
    }
    PIA_CATCHLOG_EREF(n->entity())
}

void pia_lnode_t::api_setflags(bct_server_host_ops_t **s, unsigned f)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(f != n->flags())
        {
            n->ongoing();
            n->set_flags(f);
            n->setvisible(n->iscomplete());
        }
    }
    PIA_CATCHLOG_EREF(n->entity())
}

void pia_lnode_t::api_set_source(bct_server_host_ops_t **s, bct_fastdata_t *f)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            n->ongoing();

            n->fastdata()->fastdata_set_upstream(f);

            if(f)
            {
                n->source_flags_|=PLG_SERVER_FAST;
                n->fastdata()->fastdata_enable(true,true);
            }
            else
            {
                n->source_flags_&=~PLG_SERVER_FAST;
            }

            n->setvisible(n->iscomplete());
        }
    }
    PIA_CATCHLOG_EREF(n->entity())
}

void pia_lnode_t::api_setclock(bct_server_host_ops_t **s, bct_clocksink_t *clock)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen())
        {
            n->ongoing();
            n->setclock(clock);
        }
    }
    PIA_CATCHLOG_EREF(n->entity())
}

void pia_lroot_t::job_timer(void *r_)
{
    pia_lroot_t *r = (pia_lroot_t *)r_;

    if(++r->slowtick_ == PIA_SLOW_TIMER_SILENCE)
    {
        pic::logmsg() << r->addr() << " slow going passive";
    }

    if(++r->linterlock_->fasttick_ == PIA_FAST_TIMER_SILENCE)
    {
        pic::logmsg() << r->addr() << " fast going passive";
    }
}

void pia_lroot_t::buffer_receive_slow(const unsigned char *buffer, unsigned len_)
{
    int x;
    uint32_t sseq0,tseq0,dseq0,nseq0,tseq2;
    unsigned short cookie;
    unsigned bt,pl;
    const unsigned char *p;
    int len = len_;
    int olen = len_;

    if(slowtick_ >= PIA_SLOW_TIMER_SILENCE)
    {
        old_tseq_ = tseq();
        pic::logmsg() << addr() << " slow going active " << old_tseq_;
    }

    slowtick_=0;

    if((x=pie_getheader(buffer,len,&cookie,&sseq0,&dseq0,&nseq0,&tseq0,&tseq2))<0)
    {
        return;
    }

    buffer+=x; len-=x;

    if(cookie)
    {
        printf("%s Help! Rape! %d\n",addr().hostdata(),olen);
        close();
        return;
    }

    while((x=pie_getstanza(buffer,len,&bt,&p,&pl))>0)
    {
        pia_server_t *good;
        pia_server_t *nb=find(p,pl,&good);
        PIC_ASSERT(len>=x);
        buffer+=x; len-=x;

        if(nb)
        {
            pia_lnode_t *n=(pia_lnode_t *)nb;

            switch(bt)
            {
                case BCTMTYPE_IDNT_REQ:
                    n->transmit_fast_id();
                    continue;

                case BCTMTYPE_FAST_REQ:
                    n->transmit_fast_data();
                    continue;

                case BCTMTYPE_DATA_REQ:
                    n->transmit_data(n->current());
                    continue;

                case BCTMTYPE_TREE_REQ:
                    //if(pl==0) pic::logmsg() << addr() << " received top level treq";
                    n->transmit_tree();
                    continue;
            }
        }
        else
        {
            ((pia_lnode_t *)good)->transmit_tree();
        }

        if((x=pie_skipstanza(buffer,len,bt))<0)
        {
            return;
        }

        PIC_ASSERT(len>=x);
        buffer+=x; len-=x;
    }
}

void pia_lroot_t::buffer_fixup_fast(unsigned char *buffer, unsigned len)
{
    pie_setheader(buffer,len,getcookie(),0,0,0,0,0);
}

void pia_lroot_t::buffer_receive_fast(const unsigned char *buffer, unsigned len)
{
}

void pia_lroot_t::buffer_fixup_slow(unsigned char *buffer, unsigned len)
{
    unsigned long new_tseq = tseq();
    pie_setheader(buffer,len,getcookie(),sync_seq_,dseq(),nseq(),new_tseq,old_tseq_);
    old_tseq_=new_tseq;
}

int pia_lnode_t::api_child_add(bct_server_host_ops_t **s, unsigned char name_, bct_server_t *child)
{
    pia_lnode_t *n = PIC_STRBASE(pia_lnode_t,s,_client_ops);
    int name = name_;

    try
    {
        pia_mainguard_t guard(n->entity()->glue());

        if(n->isopen() && name>=BCTLIMIT_PATHMIN && name<=BCTLIMIT_PATHMAX && child!=0)
        {
            n->ongoing();
            new pia_lnode_t(n,child,name_);
            n->set_nseq(++n->root()->sequence_);
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(n->entity())

    printf("child_add returning -1\n");
    return -1;
}

bct_server_host_ops_t pia_lnode_t::dispatch__ = 
{
    api_close,
    api_servername,
    api_path,
    api_child_add,
    api_shutdown,
    api_changed,
    api_advertise,
    api_unadvertise,
    api_cancel,
    api_setclock,
    api_set_source,
    api_setflags
};

pia_locallist_t::pia_locallist_t()
{
    impl_=new impl_t;
}

pia_locallist_t::~pia_locallist_t()
{
    try
    {
        kill(0);
    }
    PIA_CATCHLOG_PRINT()

    delete impl_;
}

void pia_locallist_t::server(const pia_data_t &n, const pia_ctx_t &e, bct_server_t *s)
{
    pia_lroot_t *r;

    for(r=impl_->servers_.head(); r!=0; r=impl_->servers_.next(r))
    {
        if(r->addr()==n)
        {
            PIC_THROW("server exists");
        }
    }

    new pia_lroot_t(impl_,n,e,s);
}

bool pia_locallist_t::client(const pia_data_t &n, const pia_ctx_t &e, bct_client_t *s)
{
    pia_lroot_t *r;

    for(r=impl_->servers_.head(); r!=0; r=impl_->servers_.next(r))
    {
        if(r->addr()==n)
        {
            r->add(e,s);
            return true;
        }
    }

    return false;
}

void pia_locallist_t::dump(const pia_ctx_t &e)
{
    pia_lroot_t *r;

    for(r=impl_->servers_.head(); r!=0; r=impl_->servers_.next(r))
    {
        if(r->entity().matches(e))
        {
            pic::logmsg() << "server: " << r->addr();
        }

        r->dump(e);
    }
}

void pia_locallist_t::kill(const pia_ctx_t &e)
{
    pia_lroot_t *r,*n;

    r=impl_->servers_.head();

    while(r)
    {
        n = impl_->servers_.next(r);

        if(r->entity().matches(e))
        {
            r->close();
        }
        else
        {
            r->kill(e);
        }

        r=n;
    }
}

pia_lnode_t::pia_lnode_t(pia_lnode_t *p, bct_server_t *s, unsigned char l): pia_server_t(p,l,pia_data_t(),0)
{
    unsigned f;
    pia_data_t d = pia_data_t::from_given(bct_server_plug_attached(s,&f));
    _client_ops = &dispatch__;
    _client=s;
    cpoint_ = pia_make_cpoint();
    source_flags_=0;

    set_flags(f);
    PIC_ASSERT(flags()==f);

    change_slow(d,root()->next_seq());

    setvisible(iscomplete());

    s->host_ops = &_client_ops;
    s->plg_state = PLG_STATE_ATTACHED;

    _job_open.idle(entity()->appq(),job_open,this,pia_data_t());
    ongoing();
}

pia_lnode_t::pia_lnode_t(const pia_data_t &a, const pia_ctx_t &e, bct_server_t *s): pia_server_t(a,e,pia_data_t(),0)
{

    setcookie((glue()->random()%0xffff)+1);
    cpoint_ = pia_make_cpoint();

    _client_ops = &dispatch__;
    _client=s;

    source_flags_=0;
    setvisible(true);

    s->host_ops = &_client_ops;
    s->plg_state = PLG_STATE_ATTACHED;
    _job_open.idle(entity()->appq(),job_open,this,pia_data_t());
}

pia_lnode_t::~pia_lnode_t()
{
    close();
}

pia_lroot_t::pia_lroot_t(pia_locallist_t::impl_t *l, const pia_data_t &a, const pia_ctx_t &e, bct_server_t *s): pia_buffer_t(e->glue(),a,pie_headerlen()), pia_lnode_t(a,e,s), _job_timer(e->glue())
{
    unsigned f;
   
    _list=l;

    slowtick_ = PIA_SLOW_TIMER_START;
    linterlock_ = pic::ref(new pia_linterlock_t);
    linterlock_->fasttick_ = PIA_FAST_TIMER_START;
    sync_seq_=0;
    sequence_=0;
    old_tseq_=0;

    ongoing();

    pia_data_t d = pia_data_t::from_given(bct_server_plug_attached(s,&f));
    PIC_ASSERT(!(f&PLG_SERVER_FAST));
    change_slow(d,root()->next_seq());
    set_flags(f);

    _list->servers_.append(this);
    _job_timer.start(job_timer,this,PIA_TIMER_INTERVAL);
    buffer_enable();
}

pia_lroot_t::~pia_lroot_t()
{
    buffer_disable();
}

void pia_lnode_t::server_closed()
{
    cpoint_->disable();
    source_flags_=0;

    _job_open.cancel();
    _job_close.idle(entity()->appq(),job_close,this,pia_data_t());
    _client->plg_state = PLG_STATE_DETACHED;

    ongoing();
}

void pia_lroot_t::server_closed()
{
    buffer_flush_slow(true);
    buffer_flush_slow(true);
    buffer_flush_slow(true);
    buffer_flush_slow(true);
    buffer_flush_slow(true);
    buffer_disable();
    _job_timer.cancel();
    pia_lnode_t::server_closed();
    glue()->killadvertise(this);
    _list->servers_.remove(this);
}
