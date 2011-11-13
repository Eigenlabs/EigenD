
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

#include <piw/piw_tsd.h>
#include <piw/piw_data.h>
#include <picross/pic_log.h>
#include <picross/pic_fastalloc.h>
#include <pibelcanto/plugin.h>

namespace piw
{
    piw_allocator_t allocator__;
    piw_logger_t logger__;
};

static void writelog(bct_entity_t e, const char *msg)
{
    unsigned msglen=strlen(msg);

    while(msglen>0)
    {
        unsigned seglen=msglen;

        if(seglen > BCTLIMIT_DATA-100)
        {
            seglen=BCTLIMIT_DATA-100;
        }

        unsigned char *dp;
        float *v;
        bct_data_t d = bct_entity_allocate_host(e,PIC_ALLOC_NB,0,1,-1,0,BCTVTYPE_STRING,seglen,&dp,1,&v);

        *v=0;
        memcpy(dp,msg,seglen);
        bct_entity_log(e,d);
        piw::piw_data_decref_fast(d);

        msg+=seglen;
        msglen-=seglen;
    }
}

void *piw::piw_allocator_t::allocator_xmalloc(unsigned nb,size_t size, deallocator_t *d, void **da)
{
    return piw::tsd_alloc(nb,size,d,da);
}

void piw::piw_logger_t::log(const char *msg)
{
    piw::tsd_log(msg);
}

void piw::tsd_log(const char *msg)
{
    bct_entity_t e = piw::tsd_getcontext();

    if(!e)
    {
        printf("%s\n",msg);
        return;
    }

    writelog(e,msg);
}

piw::dataqueue_t piw::tsd_dataqueue(unsigned size)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    bct_dataqueue_t q = bct_entity_dataqueue(e,size);
    PIC_ASSERT(q);
    return dataqueue_t::from_given(q);
}

void piw::tsd_clocksource(const piw::data_t &name, unsigned bs, unsigned sr, bct_clocksource_t *s)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_clocksource(e, name.lend(), bs, sr, s)<0)
    {
        PIC_THROW("can't create source");
    }
}

void piw::tsd_clockdomain(bct_clockdomain_t *d)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_clockdomain(e, d)<0)
    {
        PIC_THROW("can't create domain");
    }
}

void piw::tsd_server(const char *n, bct_server_t *s)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_server(e,n,s)<0)
    {
        pic::msg() << "can't create server " << n << pic::hurl;
    }
}

void piw::tsd_rpcserver(bct_rpcserver_t *s, const char *id)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_rpcserver(e,s,id)<0)
    {
        PIC_THROW("can't create rpc server");
    }
}

void piw::tsd_rpcclient(bct_rpcclient_t *s, const char *id, const piw::data_t &p, const piw::data_t &n, const piw::data_t &v, unsigned long t)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_rpcclient(e,s,id,p.lend(),n.lend(),v.lend(),t)<0)
    {
        PIC_THROW("can't create rpc client");
    }
}

void piw::tsd_client(const char *n, bct_client_t *s, bool fast)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_client(e,n,s, fast?1:0)<0)
    {
        PIC_THROW("can't create client");
    }
}

void piw::tsd_index(const char *n, bct_index_t *s)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_index(e,n,s)<0)
    {
        PIC_THROW("can't create index");
    }
}

void piw::tsd_fastdata(bct_fastdata_t *s)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_fastdata(e,s)<0)
    {
        PIC_THROW("can't create fastdata");
    }
}

void piw::tsd_window(bct_window_t *s)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_window(e,s)<0)
    {
        PIC_THROW("can't create window");
    }
}

void piw::tsd_thing(bct_thing_t *s)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    if(bct_entity_thing(e,s)<0)
    {
        PIC_THROW("can't create thing");
    }
}

std::string piw::tsd_user()
{
    bct_entity_t e = tsd_getcontext();
    piw::data_nb_t d = piw::data_nb_t::from_given(bct_entity_user(e));
    return d.as_string();
}

unsigned long long piw::tsd_time()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    return bct_entity_time(e);
}

void *piw::tsd_alloc(unsigned nb,unsigned size, void (**dealloc)(void *,void *), void **dealloc_arg)
{
    bct_entity_t e = tsd_getcontext();
    void *m = bct_entity_alloc(e,nb,size,dealloc,dealloc_arg);
    PIC_ASSERT(m);
    return m;
}

void *piw::tsd_winctx()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    return bct_entity_winctx(e);
}

void piw::tsd_dump()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    bct_entity_dump(e);
}

void piw::tsd_exit()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    bct_entity_exit(e);
}

void piw::tsd_lock()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    bct_entity_lock(e);
}

void piw::tsd_unlock()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    bct_entity_unlock(e);
}

bool piw::tsd_killed()
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    return bct_entity_killed(e)==1;
}

struct fastcall_t
{
    fastcall_t(void *a1, void *a2, void *a3, void *a4): arg1_(a1), arg2_(a2), arg3_(a3), arg4_(a4) {}

    void *arg1_;
    void *arg2_;
    void *arg3_;
    void *arg4_;
};

static int fastcaller2__(bct_entity_t e, void *call_, void *args_)
{
    piw::tsd_setcontext(e);
    fastcall_t *args = (fastcall_t *)args_;
    int (*call)(void *,void *) = (int (*)(void *,void *))call_;

    try
    {
        return (call)(args->arg1_,args->arg2_);
    }
    CATCHLOG()

    return -1;
}

static int fastcaller3__(bct_entity_t e, void *call_, void *args_)
{
    piw::tsd_setcontext(e);
    fastcall_t *args = (fastcall_t *)args_;
    int (*call)(void *,void *, void *) = (int (*)(void *,void *,void *))call_;

    try
    {
        return (call)(args->arg1_,args->arg2_,args->arg3_);
    }
    CATCHLOG()

    return -1;
}

static int fastcaller4__(bct_entity_t e, void *call_, void *args_)
{
    piw::tsd_setcontext(e);
    fastcall_t *args = (fastcall_t *)args_;
    int (*call)(void *,void *,void *,void *) = (int (*)(void *,void *,void *,void *))call_;

    try
    {
        return (call)(args->arg1_,args->arg2_,args->arg3_,args->arg4_);
    }
    CATCHLOG()

    return -1;
}

piw::data_nb_t piw::tsd_snapshot_t::allocate_host(unsigned long long ts,float u,float l,float r,unsigned t, unsigned dl, unsigned char **dp,unsigned vl,float **vp)
{
    bct_data_t d = bct_entity_allocate_host(entity_,PIC_ALLOC_NB,ts,u,l,r,t,dl,dp,vl,vp);
    PIC_ASSERT(d);
    return data_nb_t::from_given(d);
}

piw::data_nb_t piw::tsd_snapshot_t::allocate_wire(unsigned dl, const unsigned char *dp)
{
    bct_data_t d = bct_entity_allocate_wire(entity_,PIC_ALLOC_NB,dl,dp);
    PIC_ASSERT(d);
    return data_nb_t::from_given(d);
}

int piw::tsd_fastcall(int (*cb)(void *arg1, void *arg2), void *arg1, void *arg2)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    fastcall_t call(arg1,arg2,0,0);
    return bct_entity_fastcall(e,fastcaller2__,(void *)cb,&call);
}

int piw::tsd_fastcall3(int (*cb)(void *arg1, void *arg2, void *arg3), void *arg1, void *arg2, void *arg3)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    fastcall_t call(arg1,arg2,arg3,0);
    return bct_entity_fastcall(e,fastcaller3__,(void *)cb,&call);
}

int piw::tsd_fastcall4(int (*cb)(void *arg1, void *arg2, void *arg3, void *arg4), void *arg1, void *arg2, void *arg3, void *arg4)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);

    fastcall_t call(arg1,arg2,arg3,arg4);
    return bct_entity_fastcall(e,fastcaller4__,(void *)cb,&call);
}

void piw::tsd_alert(const char *k, const char *l, const char *m)
{
    bct_entity_t e = tsd_getcontext();
    PIC_ASSERT(e);
    pic::msg_t text;
    text << k << ":" << l << ":" << m;
    return bct_entity_winch(e,text.str().c_str());
}

void piw::tsd_snapshot_t::log(const char *msg)
{
    writelog(entity_,msg);
}

piw::tsd_subcontext_t::tsd_subcontext_t(bool gui, const char *user, const char *tag)
{
    entity_ = bct_entity_new(tsd_getcontext(),gui,user,tag);
}

void piw::tsd_subcontext_t::install()
{
    if(entity_)
    {
        tsd_setcontext(entity_);
    }
    else
    {
        tsd_clearcontext();
    }
}

void piw::tsd_subcontext_t::kill()
{
    if(entity_)
    {
        bct_entity_kill(entity_);
    }
}

void piw::tsd_subcontext_t::clear()
{
    if(entity_)
    {
        bct_entity_decref(entity_);
        entity_=0;
    }
}

piw::tsd_subcontext_t::~tsd_subcontext_t()
{
    clear();
}

int piw::tsd_snapshot_t::fastcall(int (*cb)(void *arg1, void *arg2), void *arg1, void *arg2)
{
    fastcall_t call(arg1,arg2,0,0);
    return bct_entity_fastcall(entity_,fastcaller2__,(void *)cb,&call);
}

int piw::tsd_snapshot_t::fastcall3(int (*cb)(void *arg1, void *arg2, void *arg3), void *arg1, void *arg2, void *arg3)
{
    fastcall_t call(arg1,arg2,arg3,0);
    return bct_entity_fastcall(entity_,fastcaller3__,(void *)cb,&call);
}

int piw::tsd_snapshot_t::fastcall4(int (*cb)(void *arg1, void *arg2, void *arg3, void *arg4), void *arg1, void *arg2, void *arg3, void *arg4)
{
    fastcall_t call(arg1,arg2,arg3,arg4);
    return bct_entity_fastcall(entity_,fastcaller4__,(void *)cb,&call);
}

