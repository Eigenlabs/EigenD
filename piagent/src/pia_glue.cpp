
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

#include <picross/pic_error.h>
#include <picross/pic_strbase.h>
#include <picross/pic_time.h>

#include "pia_data.h"
#include "pia_glue.h"
#include "pia_error.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct pia_timer_t: pic::element_t<>, virtual pic::lckobject_t
{
    void (*cb_)(void *);
    void *ctx_;
    unsigned period_;
    unsigned bias_;
};

struct synccaller_t
{
    int (*cb)(void *, void *,void *, void *);
    void *a1;
    void *a2;
    void *a3;
    void *a4;
    int r;
    pic::semaphore_t g;
};

static void synccaller(void *ctx)
{
    synccaller_t *s = (synccaller_t *)ctx;

    try
    {
        s->r = (s->cb)(s->a1,s->a2,s->a3,s->a4);
    }
    CATCHLOG()

    s->g.up();
}

int pia::manager_t::impl_t::fastcall(int (*cb)(void *, void *, void *, void *), void *a1, void *a2, void *a3, void *a4)
{
    if(handle_->service_isfast())
    {
        return (cb)(a1,a2,a3,a4);
    }
    else
    {
        if(!fastactive_)
        {
            pic::mutex_t::guard_t g(fast_lock_);
            return (cb)(a1,a2,a3,a4);
        }
    }

    synccaller_t s;

    s.cb=cb;
    s.a1=a1;
    s.a2=a2;
    s.a3=a3;
    s.a4=a4;

    fastq_.idlecall(pia_make_cpoint(),synccaller,&s);

    s.g.untimeddown();

    return s.r;
}

void pia::manager_t::impl_t::context_add(pia::context_t::impl_t *ctx)
{
    contexts_.append(ctx);
}

void pia::manager_t::impl_t::idle_callback(void *g_, const pia_data_t & d)
{
    pia::manager_t::impl_t *g = (pia::manager_t::impl_t *)g_;
    g->handle_->service_gone();
}


void pia::manager_t::impl_t::context_del(pia::context_t::impl_t *ctx)
{
    contexts_.remove(ctx);

    if(!contexts_.head())
    {
        auxq_.idle(cpoint_,idle_callback, this, pia_data_t());
    }
}

void pia::context_t::impl_t::kill()
{
    if(!killed_)
    {
        killed_=true;
        glue_->kill(this);
    }
}

void pia::context_t::impl_t::exit()
{
    if(!killed_)
    {
        exited_=true;
        killed_=true;
        glue_->kill(this);
    }
}

void pia::manager_t::impl_t::kill(const pia_ctx_t &e)
{
    proxy_.kill(e);
    local_.kill(e);
    index_.kill(e);
    window_.kill(e);
    //thing_.kill(e);
    clock_.kill(e);
    //fast_.kill(e);
    //rpc_.kill(e);
}

pia::context_t::impl_t *pia::context_t::impl_t::from_entity(bct_entity_t e)
{
    return PIC_STRBASE(pia::context_t::impl_t,e,ops_);
}

static bct_data_t api_allocate_wire(bct_entity_t e_, unsigned nb, unsigned l, const unsigned char *dp)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->allocate_wire_raw(nb,l,dp);
    }
    PIA_CATCHLOG_EREF(e)
    return 0;
}

static bct_data_t api_allocate_host(bct_entity_t e_, unsigned nb, unsigned long long ts, float u, float l, float r, unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->allocate_host_raw(nb,ts,u,l,r,t,dl,dp,vl,vp);
    }
    PIA_CATCHLOG_EREF(e)
    return 0;
}

static int api_window(bct_entity_t e_, bct_window_t *s)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->create_window(e,s);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static int api_thing(bct_entity_t e_, bct_thing_t *s)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->create_thing(e,s);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static unsigned long long api_time(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->time();
    }
    PIA_CATCHLOG_EREF(e)

    return 0;
}

static unsigned long long api_time_btoh(bct_entity_t e_, unsigned long long t)
{
    return t;
}

static unsigned long long api_time_htob(bct_entity_t e_, unsigned long long t)
{
    return t;
}

static int api_server(bct_entity_t e_, const char *n, bct_server_t *s)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        pia_data_t nd = e->expand_address(n);
        if(!nd) return PLG_STATUS_ADDR;
        e->glue()->create_server(e,nd,s);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static int api_client(bct_entity_t e_, const char *n, bct_client_t *s, int fast)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        pia_data_t nd = e->expand_address(n);
        if(!nd) return PLG_STATUS_ADDR;
        e->glue()->create_client(e,nd,s,fast);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)

    return -1;
}

static int api_index(bct_entity_t e_, const char *n, bct_index_t *s)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        pia_data_t nd = e->expand_address(n);
        if(!nd) return PLG_STATUS_ADDR;
        e->glue()->create_index(e,nd,s);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static int api_killed(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->killed()?1:0;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static int api_fastdata(bct_entity_t e_, bct_fastdata_t *s)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->create_fastdata(e,s);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static void api_lock(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_logguard_t guard(e->glue());
        e->lock();
    }
    PIA_CATCHLOG_EREF(e)
}

static void api_unlock(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        e->unlock();
    }
    PIA_CATCHLOG_EREF(e)
}

static void api_kill(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        e->kill();
    }
    PIA_CATCHLOG_EREF(e)
}

static void *api_alloc(bct_entity_t e_, unsigned nb, unsigned size, void (**dealloc)(void *, void *), void **dealloc_arg)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->allocator()->allocator_xmalloc(nb,size,dealloc,dealloc_arg);
    }
    PIA_CATCHLOG_EREF(e)
    return 0;
}

static int fastcaller(void *cb_, void *e_, void *a1, void *a2)
{
    int (*cb)(bct_entity_t, void *, void *) = (int(*)(bct_entity_t,void *,void *))cb_;
    bct_entity_t e = (bct_entity_t)e_;

    return (cb)(e,a1,a2);
}

static int api_fastcall(bct_entity_t e_, int (*cb)(bct_entity_t, void *, void *), void *a1, void *a2)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->fastcall(fastcaller,(void *)cb,(void *)e_,a1,a2);
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

void pia::manager_t::impl_t::winch(const char *msg)
{
    auxq()->idle(cpoint_,winch_callback, (void *)this, allocate_cstring(msg));
}

void pia::manager_t::impl_t::log(const pia_data_t &msg)
{
    auxq()->idle(cpoint_,log_callback, (void *)this, msg);
}

void pia::manager_t::impl_t::log(const char *msg)
{
    log(allocate_cstring_nb(msg));
}

void pia::context_t::impl_t::log(const pia_data_t &msg)
{
    glue_->auxq()->idle(cpoint_,log_callback, (void *)this, msg);
}

void pia::manager_t::impl_t::log_callback(void *e_, const pia_data_t & d)
{
    if(d.type() == BCTVTYPE_STRING)
    {
        pia::manager_t::impl_t *e = (pia::manager_t::impl_t *)e_;

        if(e->log_.iscallable())
        {
            e->log_(d.asstring());
        }
        else
        {
            fprintf(stderr,"(on stdio) pia: %s\n", d.asstring());
        }
    }
}

void pia::manager_t::impl_t::winch_callback(void *e_, const pia_data_t & d)
{
    if(d.type() == BCTVTYPE_STRING)
    {
        pia::manager_t::impl_t *e = (pia::manager_t::impl_t *)e_;

        if(e->winch_.iscallable())
        {
            e->winch_(d.asstring());
        }
    }
}

void pia::context_t::impl_t::log_callback(void *e_, const pia_data_t & d)
{
    if(d.type() == BCTVTYPE_STRING)
    {
        pia::context_t::impl_t *e = (pia::context_t::impl_t *)e_;

        if(e->log_.iscallable())
        {
            e->log_(d.asstring());
        }
        else
        {
            fprintf(stderr,"(on stdio) %s: %s\n", (const char *)(e->tag_.hostdata()), d.asstring());
        }
    }
}

static bct_data_t api_user(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->user().give();
    }
    PIA_CATCHLOG_EREF(e)
    return 0;
}

static bct_data_t api_unique(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->unique().give();
    }
    PIA_CATCHLOG_EREF(e)
    return 0;
}

static void api_log(bct_entity_t e_, bct_data_t msg)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_logguard_t guard(e->glue());
        e->log(pia_data_t::from_lent(msg));
    }
    catch(...)
    {
    }
}

static int api_clocksource(bct_entity_t e_, bct_data_t n, unsigned bs, unsigned long sr, bct_clocksource_t *s)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);
    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->create_clocksource(e, pia_data_t::from_lent(n), bs, sr, s);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)
    return -1;
}

static void api_winch(bct_entity_t e_, const char *w)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->winch(w);
    }
    PIA_CATCHLOG_EREF(e)
}

static bool api_is_fast(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        return e->glue()->handle()->service_isfast();
    }
    PIA_CATCHLOG_EREF(e)
    return false;
}

static void api_incref(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        e->weak_inc();
    }
    PIA_CATCHLOG_EREF(e)
}

static void api_decref(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        e->weak_dec();
    }
    PIA_CATCHLOG_EREF(e)
}

static bct_entity_t api_new(bct_entity_t e_, bool isgui, const char *user, const char *tag)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        int grp;
        pic::f_string_t logger = e->glue()->handle()->service_context(isgui,tag,&grp);
        pia::context_t::impl_t *i = new pia::context_t::impl_t(e->glue(),user,grp,pic::status_t(),logger,tag,false);
        return i->api();
    }
    PIA_CATCHLOG_EREF(e)
    return 0;
}

static void api_dump(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->dump(e);
    }
    PIA_CATCHLOG_EREF(e)
}

static void *api_winctx(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        return e->glue()->winctx();
    }
    PIA_CATCHLOG_EREF(e)

    return 0;
}

static void api_exit(bct_entity_t e_)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        e->exit();
    }
    PIA_CATCHLOG_EREF(e)
}

static int api_clockdomain(bct_entity_t e_, bct_clockdomain_t *d)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        e->glue()->create_clockdomain( e, d);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)

    return -1;
}

static int api_rpcserver(bct_entity_t e_, bct_rpcserver_t *rs, const char *id)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        pia_data_t nd = e->expand_address(id);
        if(!nd) return PLG_STATUS_ADDR;
        e->glue()->create_rpcserver( e, rs, nd);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)

    return -1;
}

static int api_rpcclient(bct_entity_t e_, bct_rpcclient_t *rc, const char *id, bct_data_t p, bct_data_t n, bct_data_t v, unsigned long t)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_mainguard_t guard(e->glue());
        pia_data_t nd = e->expand_address(id);
        if(!nd) return PLG_STATUS_ADDR;
        e->glue()->create_rpcclient( e, rc,nd,pia_data_t::from_lent(p),pia_data_t::from_lent(n),pia_data_t::from_lent(v),t);
        return 1;
    }
    PIA_CATCHLOG_EREF(e)

    return -1;
}

static bct_dataqueue_t api_dataqueue(bct_entity_t e_,unsigned len)
{
    pia::context_t::impl_t *e = pia::context_t::impl_t::from_entity(e_);

    try
    {
        pia_logguard_t guard(e->glue());
        return e->glue()->allocate_dataqueue(len).give();
    }
    PIA_CATCHLOG_EREF(e)

    return 0;
}

bct_entity_ops_t pia::context_t::impl_t::dispatch__ =
{
    1,
    api_server,
    api_client,
    api_index,
    api_fastdata,
    api_thing,
    api_window,
    api_allocate_wire,
    api_allocate_host,
    api_time,
    api_time_btoh,
    api_time_htob,
    api_lock,
    api_unlock,
    api_kill,
    api_fastcall,
    api_alloc,
    api_log,
    api_clocksource,
    api_clockdomain,
    api_killed,
    api_user,
    api_unique,
    api_rpcserver,
    api_rpcclient,
    api_dataqueue,
    api_exit,
    api_dump,
	api_winctx,
    api_winch,
    api_is_fast,
    api_new,
    api_incref,
    api_decref
};

void pia::context_t::impl_t::idle_callback(void *e_, const pia_data_t & d)
{
    pia::context_t::impl_t *e = (pia::context_t::impl_t *)e_;
	e->gone_.invoke(e->exited_);
    pia_mainguard_t guard(e->glue());
    e->weak_dec();
}


pia::manager_t::manager_t(pia::controller_t *c, pic::nballocator_t *a, network_t *n, const pic::f_string_t &log, const pic::f_string_t &winch,void *winctx)
{
    impl_=new pia::manager_t::impl_t(c,a,n,log,winch,winctx);
}

pia::manager_t::~manager_t()
{
    if(impl_)
    {
        delete impl_;
    }
}

void pia::manager_t::impl_t::process_main(unsigned long long now, unsigned long long *timer, bool *activity)
{
    pia_mainguard_t guard(this);

    if(mainq()->run(now))
    {
        *activity = true;
    }

    if(mainq()->next() < *timer)
    {
        *timer=mainq()->next();
    }
}

void pia::manager_t::impl_t::process_fast(unsigned long long now, unsigned long long *timer, bool *activity)
{
    pia_logguard_t guard(this);

    if(fastq()->run(now))
    {
        *activity = true;
    }

    if(fastq()->next() < *timer)
    {
        *timer=fastq()->next();
    }
}

void pia::manager_t::impl_t::service_aux()
{
    if(pic_atomiccas(&auxflag_,0,1))
    {
        handle()->service_ctx(-1);
        return;
    }
}

void pia::manager_t::impl_t::service_main()
{
    handle()->service_main();
}

void pia::manager_t::impl_t::service_fast()
{
    handle()->service_fast();
}

void pia::context_t::impl_t::service_ctx()
{
    if(pic_atomiccas(&queued_,0,1))
    {
        glue_->handle()->service_ctx(group_);
    }
}

void pia::manager_t::impl_t::create_rpcserver(const pia_ctx_t &e, bct_rpcserver_t *i, const pia_data_t &id)
{
    PIC_ASSERT(!e->killed());
    rpc_.server(e,i,id);
}

void pia::manager_t::impl_t::create_rpcclient(const pia_ctx_t &e, bct_rpcclient_t *i, const pia_data_t &id, const pia_data_t &p, const pia_data_t &n, const pia_data_t &v, unsigned long t)
{
    PIC_ASSERT(!e->killed());
    rpc_.client(e,i,id,p,n,v,t);
}

void pia::manager_t::impl_t::create_clocksource(const pia_ctx_t &e, const pia_data_t &n, unsigned bs, unsigned sr, bct_clocksource_t *s)
{
    PIC_ASSERT(!e->killed());
    clock_.add_source(n,bs,sr,e,s); 
}

void pia::manager_t::impl_t::create_clockdomain(const pia_ctx_t &e, bct_clockdomain_t *d)
{
    PIC_ASSERT(!e->killed());
    clock_.add_domain( e, d); 
}

void pia::manager_t::impl_t::create_client(const pia_ctx_t &e, const pia_data_t &addr, bct_client_t *c, bool fast) 
{
    PIC_ASSERT(!e->killed());

    if(!local_.client(addr,e,c))
    {
        proxy_.client(addr,e,c,fast); 
    }
}

void pia::manager_t::impl_t::dump_killed()
{
    pia_mainguard_t guard(this);

    pia::context_t::impl_t *i;
    for(i=contexts_.head(); i!=0; i=contexts_.next(i))
    {
        if(i->killed())
        {
            dump(i);
        }
    }
}

void pia::manager_t::impl_t::dump(const pia_ctx_t &e)
{
    pic::logmsg() << "context dump ";
    proxy_.dump(e);
    local_.dump(e);
    index_.dump(e);
    thing_.dump(e);
    fast_.dump(e);
    clock_.dump(e);
    rpc_.dump(e);
    pic::logmsg() << "context dump end";
}

void pia::manager_t::impl_t::create_server(const pia_ctx_t &e, const pia_data_t &addr, bct_server_t *s) 
{
    PIC_ASSERT(!e->killed());
    proxy_.killserver(addr);
    local_.server(addr,e,s); 
}

void pia::manager_t::impl_t::create_window(const pia_ctx_t &e, bct_window_t *i) 
{
    PIC_ASSERT(!e->killed());
    window_.window(e,i); 
}

void pia::manager_t::impl_t::create_thing(const pia_ctx_t &e, bct_thing_t *i) 
{
    PIC_ASSERT(!e->killed());
    thing_.thing(e,i); 
}

void pia::manager_t::impl_t::create_fastdata(const pia_ctx_t &e, bct_fastdata_t *i) 
{
    PIC_ASSERT(!e->killed());
    fast_.fastdata(e,i); 
}

void pia::manager_t::impl_t::create_index(const pia_ctx_t &e, const pia_data_t &addr, bct_index_t *i) 
{
    PIC_ASSERT(!e->killed());
    index_.index(addr,e,i); 
}

pia_data_t pia::manager_t::impl_t::unique() const
{
    unsigned long long u = chuff_;
    std::ostringstream s;
    u ^= pic_microtime();
    u &= 0xffffffffffffULL;
    s << std::hex << u;
    return allocate_cstring_nb(s.str().c_str());
}

pia_data_t pia::context_t::impl_t::collapse_address_relative(const pia_data_t &addr, const pia_data_t &user)
{
    PIC_ASSERT(addr.type()==BCTVTYPE_STRING);

    const char *a = addr.asstring();
    unsigned al = addr.asstringlen();
    const char *acp;

    if(al<3 || !(acp=strchr(a,':')))
    {
        return addr;
    }

    unsigned aul = (acp-a)-1;
    unsigned aal = al-aul-3;

    const char *u = user.asstring();
    unsigned ul = user.asstringlen();
    const char *ucp;

    if(ul<3 || !(ucp=strchr(u,':')))
    {
        return addr;
    }

    unsigned uul = (ucp-u)-1;

    if(aul!=uul || strncmp(u+1,a+1,uul))
    {
        return addr;
    }

    float *v;
    unsigned char *dp;

    pia_data_t d = glue()->allocate_host(PIC_ALLOC_NB,0,1,-1,0,BCTVTYPE_STRING,aal+2,&dp,1,&v);
    *v=0;

    dp[0]='<';
    memcpy(dp+1,acp+1,aal);
    dp[aal+1]='>';

    return d;

}
pia_data_t pia::context_t::impl_t::collapse_address(const pia_data_t &addr)
{
    PIC_ASSERT(addr.type()==BCTVTYPE_STRING);

    const char *a = addr.asstring();
    unsigned al = addr.asstringlen();
    const char *cp;

    if(al<3 || !(cp=strchr(a,':')))
    {
        return addr;
    }

    unsigned aul = (cp-a)-1;
    unsigned aal = al-aul-3;

    const char *u = user().asstring();
    unsigned ul = std::min(BCTLINK_GROUP_SIZE-(al+1),user().asstringlen());

    if(aul!=ul || strncmp(u,a+1,ul))
    {
        return addr;
    }

    float *v;
    unsigned char *dp;

    pia_data_t d = glue()->allocate_host(PIC_ALLOC_NB,0,1,-1,0,BCTVTYPE_STRING,aal+2,&dp,1,&v);
    *v=0;

    dp[0]='<';
    memcpy(dp+1,cp+1,aal);
    dp[aal+1]='>';

    return d;

}

pia_data_t pia::context_t::impl_t::expand_address_relative(const char *a,const pia_data_t &user)
{
    unsigned al = strlen(a);

    if(al<3 || strchr(a,':'))
    {
        return glue()->allocate_cstring_nb(a);
    }

    const char *u = user.asstring();
    unsigned ul = user.asstringlen();
    const char *ucp;

    if(ul<3 || !(ucp=strchr(u,':')))
    {
        return glue()->allocate_cstring_nb(a);
    }

    unsigned uul = (ucp-u)-1;

    float *v;
    unsigned char *dp;

    pia_data_t d = glue()->allocate_host(PIC_ALLOC_NB,0,1,-1,0,BCTVTYPE_STRING,al-2+uul+1+2,&dp,1,&v);
    *v=0;

    dp[0]='<';
    memcpy(dp+1,u+1,uul);
    dp[ul+1]=':';
    memcpy(dp+uul+1+1,a+1,al-2);
    dp[1+ul+1+al-2]='>';

    return d;

}

pia_data_t pia::context_t::impl_t::expand_address(const char *a)
{
    unsigned al = strlen(a);

    if(al<3 || strchr(a,':'))
    {
        return glue()->allocate_cstring_nb(a);
    }

    const char *u = user().asstring();
    unsigned ul = std::min(BCTLINK_GROUP_SIZE-(al+1),user().asstringlen());

    float *v;
    unsigned char *dp;

    pia_data_t d = glue()->allocate_host(PIC_ALLOC_NB,0,1,-1,0,BCTVTYPE_STRING,al-2+ul+1+2,&dp,1,&v);
    *v=0;

    dp[0]='<';
    memcpy(dp+1,u,ul);
    dp[ul+1]=':';
    memcpy(dp+ul+1+1,a+1,al-2);
    dp[1+ul+1+al-2]='>';

    return d;

}

bool pia::manager_t::impl_t::global_lock()
{
    return global_lock_.trywlock();
}

void pia::manager_t::impl_t::global_unlock()
{
    global_lock_.wunlock();
    //dump_killed();
}

void pia::manager_t::impl_t::fast_pause()
{
    fastactive_ = 0;
}

void pia::manager_t::impl_t::fast_resume()
{
    fastactive_ = 1;
}

void pia::manager_t::impl_t::process_ctx(int grp, unsigned long long now, bool *activity)
{
    pia_logguard_t guard(this);
    pia::context_t::impl_t *h;
    bool aux;

    for(;;)
    {
        {   pia_mainguard_t guard(this);

            aux=false;
            if(!auxbusy_)
            {
                auxbusy_=true;
                aux=true;
            }
        }

        if(aux)
        {
            if(pic_atomiccas(&auxflag_,1,0))
            {
                auxq_.run(now);
            }
        }

        {   pia_mainguard_t guard(this);

            if(aux)
            {
                auxbusy_ = false;
            }

            for(h=contexts_.head(); h!=0; h=contexts_.next(h))
            {
                if(h->group_==grp && pic_atomiccas(&(h->queued_),1,0))
                {
                    h->weak_inc();
                    contexts_.append(h);
                    goto found;
                }
            }

            return;
        }

    found:

        {
            pic::rwmutex_t::rguard_t rwguard(global_lock_);

            if(h->enter())
            {
                if(h->appq()->run(now))
                {
                    *activity = true;
                }

                h->leave();
            }
        }

        {   pia_mainguard_t guard(this);
            h->weak_dec();
        }
    }
}

void pia::manager_t::process_fast(unsigned long long now, unsigned long long *timer, bool *activity)
{
    impl_->process_fast(now,timer,activity);
}

void pia::manager_t::process_main(unsigned long long now, unsigned long long *timer, bool *activity)
{
    impl_->process_main(now,timer,activity);
}

void pia::manager_t::process_ctx(int grp,unsigned long long now, bool *activity)
{
    impl_->process_ctx(grp,now,activity);
}

bool pia::manager_t::window_state(unsigned w)
{
    return impl_->window_state(w);
}

bool  pia::manager_t::global_lock()
{
    return impl_->global_lock();
}

void pia::manager_t::global_unlock()
{
    impl_->global_unlock();
}

void pia::manager_t::fast_pause()
{
    return impl_->fast_pause();
}

void pia::manager_t::fast_resume()
{
    impl_->fast_resume();
}

unsigned pia::manager_t::window_count()
{
    return impl_->window_count();
}

std::string pia::manager_t::window_title(unsigned w)
{
    return impl_->window_title(w);
}

void pia::manager_t::set_window_state(unsigned w, bool o)
{
    impl_->set_window_state(w,o);
}

pia::context_t pia::manager_t::context(int grp, const char *user, const pic::status_t &gone, const pic::f_string_t &log, const char *tag)
{
    pia_mainguard_t guard(impl_);
    pia::context_t::impl_t *i = new pia::context_t::impl_t(impl_,user,grp,gone,log,tag,false);
    return pia::context_t(i);
}

static void main_pinger(void *g_) { ((pia::manager_t::impl_t *)g_)->service_main(); }
static void fast_pinger(void *g_) { ((pia::manager_t::impl_t *)g_)->service_fast(); }
static void aux_pinger(void *g_) { ((pia::manager_t::impl_t *)g_)->service_aux(); }
static void ctx_pinger(void *c_) { ((pia::context_t::impl_t *)c_)->service_ctx(); }

pia::context_t::impl_t::impl_t(pia::manager_t::impl_t *g, const char *u, int grp, const pic::status_t &gone, const pic::f_string_t &log, const char *t, bool strong): gone_(gone), log_(log), glue_(g), appq_(g->allocator(),ctx_pinger,this), queued_(0), killed_(false), exited_(false), group_(grp)
{
    ops_=&dispatch__;
    strong_=strong?1:0;
    weak_=1;
    cpoint_=pia_make_cpoint();
    tag_ = glue_->allocate_cstring(t?t:"");
    user_ = glue_->allocate_cstring(u?u:"");
    g->context_add(this);
}

pia::context_t::impl_t::~impl_t()
{
    cpoint_->disable();
}

pia::manager_t::impl_t::impl_t(pia::controller_t *h, pic::nballocator_t *a, network_t *n, const pic::f_string_t &log, const pic::f_string_t &winch,void *winctx): handle_(h), seed_(pic_microtime()), network_(n), allocator_(a), auxq_(a,aux_pinger,this), fastq_(a,fast_pinger,this), mainq_(a,main_pinger,this),
    index_(this), clock_(this), rpc_(this), log_(log), auxflag_(0), auxbusy_(false), winctx_(winctx), winch_(winch), fast_lock_(true), fastactive_(1)
{
    unsigned char *p = (unsigned char *)&chuff_;
    cpoint_=pia_make_cpoint();
    timer_cpoint_=pia_make_cpoint();

    srand(pic_microtime());

    for(unsigned i=0;i < sizeof(chuff_);i++)
    {
        p[i]=(unsigned char)rand();
    }

    chuff_ |= 0x800000000000ULL;
    timer_count_=0;

    mainq_.timer(timer_cpoint_,timer_callback,this,1000);
}

pia::manager_t::impl_t::~impl_t()
{
    cpoint_->disable();
    timer_cpoint_->disable();
}

void pia::manager_t::impl_t::timer_callback(void *impl_)
{
    impl_t *impl = (impl_t *)impl_;

    pia_timer_t *t = impl->timers_.head();
    pia_timer_t *t2;
    unsigned tc = impl->timer_count_++;

    while(t)
    {
        t2=impl->timers_.next(t);

        if(!t->period_)
        {
            delete t;
            t=t2;
            continue;
        }

        if((t->bias_+tc)%(t->period_)==0)
        {
            try
            {
                t->cb_(t->ctx_);
            }
            CATCHLOG()
        }

        t=t2;
    }
}

void *pia::manager_t::impl_t::add_timer(void (*cb)(void *), void *ctx,unsigned period)
{
    pia_timer_t *t = new pia_timer_t;
    t->cb_=cb;
    t->ctx_=ctx;
    t->period_=period;
    t->bias_=((((unsigned long long)t)>>9)&0xff);
    timers_.append(t);
    return (void *)t;
}

void pia::manager_t::impl_t::del_timer(void *hnd)
{
    ((pia_timer_t *)hnd)->period_=0;;
}

pia::context_t::context_t(): impl_(0)
{
}

pia::context_t::context_t(impl_t *i): impl_(i)
{
}

pia::context_t::context_t(const context_t &c): impl_(c.impl_)
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->weak_inc();
	}
}

pia::context_t &pia::context_t::operator=(const pia::context_t &c)
{
    release();

	impl_=c.impl_;

	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->weak_inc();
	}

	return *this;
}

pia::context_t::~context_t()
{
    release();
}

void pia::context_t::release()
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->weak_dec();
		impl_=0;
	}
}

void pia::context_t::lock()
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->lock();
	}
}

void pia::context_t::unlock()
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->unlock();
	}
}

bool pia::context_t::inuse()
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        return impl_->inuse();
	}

    return false;
}

void pia::context_t::trigger()
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->strong_inc();
        impl_->strong_dec();
	}
}

void pia::context_t::kill()
{
	if(impl_)
	{
        pia_mainguard_t guard(impl_->glue());
        impl_->kill();
	}
}

bct_entity_t pia::context_t::entity() const
{
	return impl_?impl_->api():0;
}
