
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

#include "pia_rpc.h"
#include "pia_glue.h"
#include "pia_data.h"
#include "pia_error.h"
#include "pia_network.h"

#include <pibelcanto/link.h>
#include <pibelcanto/plugin.h>
#include <piembedded/pie_message.h>

#include <piagent/pia_network.h>
#include <picross/pic_time.h>
#include <picross/pic_ilist.h>
#include <picross/pic_log.h>
#include <picross/pic_strbase.h>
#include <picross/pic_ref.h>
#include <picross/pic_weak.h>

#include <string.h>
#include <stdlib.h>
#include <map>
#include <set>

#define ACK_TIMER 1
#define RSP_TIMER 5
#define ACK_FAIL  10
#define RSP_FAIL  10

namespace
{
    struct invocation_t
    {
        invocation_t(const pia_data_t &p, const pia_data_t &n, const pia_data_t &d);

        pia_data_t request() { return request_; }
        pia_data_t response() { return response_; }
        pia_data_t name() { return name_; }
        pia_data_t key() { return key_; }
        int status() { return status_; }

        void set_cancel(void (*cb)(void *, invocation_t *), void *ctx);
        bool cancelled();
        void cancel();
        virtual ~invocation_t();
        void complete(int s,const pia_data_t &v);
        virtual void completed() = 0;

        pia_data_t key_,name_,request_,response_;
        int status_;
        void (*cancelcb_)(void *, invocation_t *);
        void *cancelctx_;
        bool cancelled_;
    };

    struct sproxy_t;
    struct cproxy_t;
    struct snode_t;
    struct cnode_t;

    struct network_t: virtual pic::counted_t, virtual pic::tracked_t
    {
        network_t(pia::manager_t::impl_t *glue, const pia_data_t &addr);
        ~network_t();

        void decode_req(const pia_data_t &key, const pia_data_t &n, const pia_data_t &dp);
        void decode_rsp(const pia_data_t &key, int st, const pia_data_t &dp);
        void decode_ack(const pia_data_t &key);
        void decode_get(const pia_data_t &key);
        void decode_dun(const pia_data_t &key);
        void send_get(const pia_data_t &key);
        void send_dun(const pia_data_t &key);
        void send_ack(const pia_data_t &key);
        void send_request(const pia_data_t &key, const pia_data_t &name, const pia_data_t &val);
        void send_response(const pia_data_t &key, int st, const pia_data_t &val);
        void decode(const unsigned char *b, unsigned bl);
        static void network__(void *ctx,const pia_data_t &data);
        void counted_deallocate();
        bool set_server(snode_t *server);
        void clear_server(snode_t *server);
        void add_cproxy(const pia_data_t &key,cproxy_t *p);
        void remove_cproxy(const pia_data_t &key);
        void add_sproxy(const pia_data_t &key, sproxy_t *p);
        void remove_sproxy(const pia_data_t &key);
        void invoke(invocation_t *invocation);

        pia::manager_t::impl_t *glue_;
        snode_t *server_;
        std::map<pia_data_t,cproxy_t *> cproxies_;
        std::map<pia_data_t,sproxy_t *> sproxies_;
        pia_sockref_t socket_;
    };

    typedef pic::ref_t<network_t> netref_t;
    typedef pic::weak_t<network_t> netwref_t;

    // client side of a remote rpc
    struct cproxy_t
    {
        cproxy_t(const netref_t &n, invocation_t *i);
        ~cproxy_t();
        static void timer__(void *ctx);
        void net_ack();
        void net_response(int st, const pia_data_t &dp);
        static void cancel__(void *ctx, invocation_t *i);

        netref_t network_;
        invocation_t *invocation_;
        bool acked_;
        unsigned count_;
        pia_timerhnd_t job_timer_;
    };

    // server side of a remote rpc
    struct sproxy_t: invocation_t
    {
        sproxy_t(snode_t *server, const netref_t &n, const pia_data_t &path, const pia_data_t &name, const pia_data_t &v);
        ~sproxy_t();
        void handle_get();
        void handle_dun();
        void handle_req();
        static void timer__(void *ctx);
        void completed();

        netref_t network_;
        pia_data_t id_;
        bool completed_;
        pia_timerhnd_t job_timer_;
        bool timing_;
    };

    struct blob_t
    {
        invocation_t *invocation;
    };

    struct snode_t: pic::element_t<0>
    {
        snode_t(pia_rpclist_t::impl_t *l, const pia_ctx_t &e, bct_rpcserver_t *i, const pia_data_t &id);
        ~snode_t();
        void detach(int e);
        static void close_callback(void *t_, const pia_data_t & d);
        void close();
        void complete(void *c, bool s, const pia_data_t &v);
        static void api_close(bct_rpcserver_host_ops_t **t_);
        static void api_complete(bct_rpcserver_host_ops_t **t_, void *c, int s, bct_data_t v);
        static void invoke__(void *s_, const pia_data_t &d);
        static void cancel__(void *ctx, invocation_t *i);
        bool invoke(invocation_t *i);

        bct_rpcserver_host_ops_t *host_ops_;
        static bct_rpcserver_host_ops_t dispatch__;

        pia_rpclist_t::impl_t *list_;
        bool open_;
        pia_ctx_t entity_;
        std::set<invocation_t *> invocations_;
        bct_rpcserver_t *rpcserver_;
        pia_job_t job_close_;
        pia_cref_t cpoint_;
        netref_t network_;
    };

    struct cnode_t: invocation_t, pic::element_t<0>
    {
        cnode_t(pia_rpclist_t::impl_t *l, const pia_ctx_t &e, bct_rpcclient_t *i, const pia_data_t &id, const pia_data_t &path, const pia_data_t &n, const pia_data_t &v, unsigned long t);
        static void api_close(bct_rpcclient_host_ops_t **t_);
        static void close_callback(void *t_, const pia_data_t & d);
        void detach(int e);
        void close();
        void completed();

        pia_rpclist_t::impl_t *list_;
        bool open_;
        pia_ctx_t entity_;
        bct_rpcclient_host_ops_t *host_ops_;
        static bct_rpcclient_host_ops_t dispatch__;
        bct_rpcclient_t *rpcclient_;
        pia_job_t job_close_;
        netref_t network_;
    };
};

struct pia_rpclist_t::impl_t
{
    impl_t(pia::manager_t::impl_t *g): glue_(g) {}

    netref_t get_network(const pia_data_t &addr);
    void dump(const pia_ctx_t &e);
    void kill(const pia_ctx_t &e);

    pia::manager_t::impl_t *glue_;
    std::map<pia_data_t,netwref_t,pia_notime_less> networks_;
    pic::ilist_t<snode_t,0> servers_;
    pic::ilist_t<cnode_t,0> clients_;
};

void invocation_t::set_cancel(void (*cb)(void *, invocation_t *), void *ctx)
{
    cancelcb_ = cb;
    cancelctx_ = ctx;
}

bool invocation_t::cancelled()
{
    return cancelcb_!=0;
}

void invocation_t::cancel()
{
    if(!cancelled_)
    {
        cancelled_ = true;
        status_ = 0;

        if(cancelcb_)
        {
            cancelcb_(cancelctx_,this);
            cancelcb_=0;
        }
    }
}

invocation_t::invocation_t(const pia_data_t &p, const pia_data_t &n, const pia_data_t &d): key_(p), name_(n), request_(d), cancelled_(false)
{
}

invocation_t::~invocation_t()
{
}

void invocation_t::complete(int s,const pia_data_t &v)
{
    if(!cancelled_)
    {
        cancelled_=true;
        response_=v;
        status_=s;
        completed();
    }
}

sproxy_t::sproxy_t(snode_t *server, const netref_t &n, const pia_data_t &path, const pia_data_t &name, const pia_data_t &v): invocation_t(path,name,v), network_(n), completed_(false), job_timer_(n->glue_)
{
    network_->add_sproxy(key(),this);
    server->invoke(this);

    if(!completed_)
    {
        network_->send_ack(key());
    }
}

snode_t::snode_t(pia_rpclist_t::impl_t *l, const pia_ctx_t &e, bct_rpcserver_t *i, const pia_data_t &id): list_(l), open_(true), entity_(e), rpcserver_(i)
{
    cpoint_=pia_make_cpoint();
    host_ops_ = &dispatch__;
    rpcserver_->host_ops=&host_ops_;
    rpcserver_->plg_state=PLG_STATE_OPENED;
    l->servers_.append(this);
    network_ = l->get_network(id);
    network_->set_server(this);
}

cnode_t::cnode_t(pia_rpclist_t::impl_t *l, const pia_ctx_t &e, bct_rpcclient_t *i, const pia_data_t &id, const pia_data_t &path, const pia_data_t &n, const pia_data_t &v, unsigned long t): invocation_t(path,n,v), list_(l), open_(true), entity_(e), rpcclient_(i)
{
    host_ops_ = &dispatch__;
    rpcclient_->host_ops=&host_ops_;
    l->clients_.append(this);
    network_ = l->get_network(id);
    network_->invoke(this);
}

void network_t::invoke(invocation_t *invocation)
{
    if(server_)
    {
        server_->invoke(invocation);
        return;
    }

    new cproxy_t(netref_t::from_lent(this),invocation);
}

void network_t::decode_req(const pia_data_t &path, const pia_data_t &np, const pia_data_t &dp)
{
    if(!server_)
    {
        //send_response(path,0,pia_data_t());
        return;
    }

    std::map<pia_data_t,sproxy_t *>::iterator i;

    if((i=sproxies_.find(path))==sproxies_.end())
    {
        new sproxy_t(server_,netref_t::from_lent(this), path, np, dp);
    }
    else
    {
        i->second->handle_req();
    }

}

void network_t::decode_dun(const pia_data_t &key)
{
    std::map<pia_data_t,sproxy_t *>::iterator i;

    if((i=sproxies_.find(key))!=sproxies_.end())
    {
        i->second->handle_dun();
        return;
    }
}

void network_t::decode_get(const pia_data_t &key)
{
    std::map<pia_data_t,sproxy_t *>::iterator i;

    if((i=sproxies_.find(key))!=sproxies_.end())
    {
        i->second->handle_get();
        return;
    }

    send_response(key,-1,pia_data_t());
}

void network_t::decode_ack(const pia_data_t &key)
{
    std::map<pia_data_t,cproxy_t *>::iterator i;

    if((i=cproxies_.find(key))!=cproxies_.end())
    {
        i->second->net_ack();
    }
}

void network_t::decode_rsp(const pia_data_t &key, int st, const pia_data_t &dp)
{
    std::map<pia_data_t,cproxy_t *>::iterator i;

    if((i=cproxies_.find(key))!=cproxies_.end())
    {
        i->second->net_response(st,dp);
    }
}

netref_t pia_rpclist_t::impl_t::get_network(const pia_data_t &addr)
{
    pia_data_t eaddr = glue_->expand_address(addr);
    std::map<pia_data_t,netwref_t,pia_notime_less>::iterator i;

    if((i=networks_.find(eaddr))==networks_.end())
    {
        network_t *n = new network_t(glue_,eaddr);
        networks_.insert(std::make_pair(eaddr,n));
        return netref_t::from_given(n);
    }

    if(!i->second.isvalid())
    {
        network_t *n = new network_t(glue_,eaddr);
        i->second.assign(n);
        return netref_t::from_given(n);
    }

    return netref_t::from_lent(i->second.ptr());
}

void pia_rpclist_t::impl_t::dump(const pia_ctx_t &e)
{
    snode_t *s;

    s=servers_.head();

    while(s)
    {
        if(s->entity_.matches(e))
        {
            pic::logmsg() << "rpc server in " << s->entity_->tag();
        }

        s = servers_.next(s);
    }

    cnode_t *c;

    c=clients_.head();

    while(c)
    {
        if(c->entity_.matches(e))
        {
            pic::logmsg() << "rpc client in " << c->entity_->tag();
        }

        c = clients_.next(c);
    }
}

void pia_rpclist_t::impl_t::kill(const pia_ctx_t &e)
{
    snode_t *s,*sn;

    s=servers_.head();

    while(s)
    {
        sn = servers_.next(s);

        if(s->entity_.matches(e))
        {
            s->detach(1);
        }

        s=sn;
    }

    cnode_t *c,*cn;

    c=clients_.head();

    while(c)
    {
        cn = clients_.next(c);

        if(c->entity_.matches(e))
        {
            c->detach(1);
        }

        c=cn;
    }
}

pia_rpclist_t::pia_rpclist_t(pia::manager_t::impl_t *g)
{
    impl_= new impl_t(g);
}

pia_rpclist_t::~pia_rpclist_t()
{
    delete impl_;
}

void pia_rpclist_t::server(const pia_ctx_t &e, bct_rpcserver_t *i, const pia_data_t &id)
{
    new snode_t(impl_,e,i,id);
}

void pia_rpclist_t::client(const pia_ctx_t &e, bct_rpcclient_t *i, const pia_data_t &id, const pia_data_t &path, const pia_data_t &n, const pia_data_t &v, unsigned long t)
{
    unsigned long long cookie = e->glue()->random();
    cookie <<=32;
    cookie += rand();

    pia_data_t key = e->glue()->allocate_path(path.aspathlen(),path.aspath(),cookie);

    new cnode_t(impl_,e,i,id,key,n,v,t);
}

void pia_rpclist_t::dump(const pia_ctx_t &e)
{
    impl_->dump(e);
}

void pia_rpclist_t::kill(const pia_ctx_t &e)
{
    impl_->kill(e);
}

void network_t::send_get(const pia_data_t &key)
{
    uint64_t cookie = key.time();
    unsigned char buffer[BCTLINK_MAXPAYLOAD];
    int l;

    if((l=pie_setrpc(buffer,sizeof(buffer),key.aspath(),key.aspathlen(),BCTMTYPE_RPC_GET,&cookie,0,0,0,0,0))>0)
    {
        socket_.write(buffer,l);
    }
}

void network_t::send_dun(const pia_data_t &key)
{
    uint64_t cookie = key.time();
    unsigned char buffer[BCTLINK_MAXPAYLOAD];
    int l;

    if((l=pie_setrpc(buffer,sizeof(buffer),key.aspath(),key.aspathlen(),BCTMTYPE_RPC_DUN,&cookie,0,0,0,0,0))>0)
    {
        socket_.write(buffer,l);
    }
}

void network_t::send_ack(const pia_data_t &key)
{
    uint64_t cookie = key.time();
    unsigned char buffer[BCTLINK_MAXPAYLOAD];
    int l;

    if((l=pie_setrpc(buffer,sizeof(buffer),key.aspath(),key.aspathlen(),BCTMTYPE_RPC_ACK,&cookie,0,0,0,0,0))>0)
    {
        socket_.write(buffer,l);
    }
}

void network_t::send_request(const pia_data_t &key, const pia_data_t &name, const pia_data_t &val)
{
    uint64_t cookie = key.time();
    unsigned char buffer[BCTLINK_MAXPAYLOAD];
    int l;

    if((l=pie_setrpc(buffer,sizeof(buffer),key.aspath(),key.aspathlen(),BCTMTYPE_RPC_REQ,&cookie,name.hostlen(),name.hostdata(),0,val.wirelen(),val.wiredata()))>0)
    {
        socket_.write(buffer,l);
    }
}

void network_t::send_response(const pia_data_t &key, int st, const pia_data_t &val)
{
    uint64_t cookie = key.time();
    unsigned char buffer[BCTLINK_MAXPAYLOAD];
    int l;

    if((l=pie_setrpc(buffer,sizeof(buffer),key.aspath(),key.aspathlen(),BCTMTYPE_RPC_RSP,&cookie,0,0,st,val.wirelen(),val.wiredata()))>0)
    {
        socket_.write(buffer,l);
    }
}

void network_t::decode(const unsigned char *b, unsigned bl)
{
    const unsigned char *p,*dp,*np;
    unsigned pl,bt,nl;
    unsigned short dl;
    uint64_t cookie;
    int st;

    if(pie_getrpc(b,bl,&p,&pl,&bt,&cookie,&nl,&np,&st,&dl,&dp)>0)
    {
        pia_data_t xp = glue_->allocate_path(pl,p,cookie);
        pia_data_t xn = glue_->allocate_string((const char *)np,nl);
        pia_data_t xd = glue_->allocate_wire(PIC_ALLOC_NORMAL,dl,dp);
        //pic::logmsg() << "received message " << xp << ":" << xn << ':' << bt << ':' << xd << ':' << cookie;

        switch(bt)
        {
            case BCTMTYPE_RPC_ACK: decode_ack(xp); break;
            case BCTMTYPE_RPC_REQ: decode_req(xp,xn,xd); break;
            case BCTMTYPE_RPC_RSP: decode_rsp(xp,st,xd); break;
            case BCTMTYPE_RPC_GET: decode_get(xp); break;
            case BCTMTYPE_RPC_DUN: decode_dun(xp); break;
        }
    }
    else
    {
        pic::logmsg() << "garbled rpc packet";
    }
}

void network_t::network__(void *ctx,const pia_data_t &data)
{
    network_t *n = (network_t *)ctx;
    n->decode(data.hostdata(),data.hostlen());
}

void network_t::counted_deallocate()
{
    delete this;
}

bool network_t::set_server(snode_t *server)
{
    if(!server_)
    {
        server_=server;
        return true;
    }

    return false;
}

void network_t::clear_server(snode_t *server)
{
    if(server_==server)
    {
        server_=0;
    }
}

void network_t::add_cproxy(const pia_data_t &key,cproxy_t *p)
{
    cproxies_.insert(std::make_pair(key,p));
}

void network_t::remove_cproxy(const pia_data_t &key)
{
    cproxies_.erase(key);
}

void network_t::add_sproxy(const pia_data_t &key, sproxy_t *p)
{
    sproxies_.insert(std::make_pair(key,p));
}

void network_t::remove_sproxy(const pia_data_t &key)
{
    sproxies_.erase(key);
}

network_t::network_t(pia::manager_t::impl_t *glue, const pia_data_t &addr): glue_(glue), server_(0), socket_(glue->allocator())
{
    socket_.open(glue->network(),BCTLINK_NAMESPACE_RPC,addr);
    socket_.callback(glue->mainq(),network__,this);
}

network_t::~network_t()
{
    tracked_invalidate();
    socket_.close();
}

cproxy_t::cproxy_t(const netref_t &n, invocation_t *i): network_(n), invocation_(i), acked_(false), count_(0), job_timer_(n->glue_)
{
    network_->add_cproxy(i->key(),this);
    invocation_->set_cancel(cancel__,this);
    network_->send_request(i->key(),i->name(),i->request());
    job_timer_.start(timer__, this,ACK_TIMER);
}

cproxy_t::~cproxy_t()
{
    job_timer_.cancel();
    network_->remove_cproxy(invocation_->key());
}

void cproxy_t::timer__(void *ctx)
{
    cproxy_t *c = (cproxy_t *)ctx;

    if(c->acked_)
    {
        if(++c->count_ > RSP_FAIL)
        {
            pic::logmsg() << "rpc timeout for rsp";
            c->net_response(0,pia_data_t());
            return;
        }

        pic::logmsg() << "retransmit get " << c->invocation_->name() << c->invocation_->request() << " count=" << c->count_ << ' ' << (void *)c;
        c->network_->send_get(c->invocation_->key());
        return;
    }

    if(++c->count_ > ACK_FAIL)
    {
        pic::logmsg() << "rpc timeout for ack";
        c->net_response(0,pia_data_t());
        return;
    }

    pic::logmsg() << "retransmit ack";
    c->network_->send_request(c->invocation_->key(),c->invocation_->name(),c->invocation_->request());
}

void cproxy_t::net_ack()
{
    if(!acked_)
    {
        job_timer_.start(timer__, this,RSP_TIMER);
        acked_=true;
    }

    count_=0;
}

void cproxy_t::net_response(int st, const pia_data_t &dp)
{
    invocation_->complete(st,dp);
    network_->send_dun(invocation_->key());
    delete this;
}

void cproxy_t::cancel__(void *ctx, invocation_t *i)
{
    cproxy_t *c = (cproxy_t *)ctx;
    delete c;
}

void sproxy_t::handle_get()
{
    if(completed_)
    {
        timing_=false;
        job_timer_.start(timer__, this,60);
        network_->send_response(key(),status(),response());
    }
    else
    {
        network_->send_ack(key());
    }
}

void sproxy_t::handle_dun()
{
    job_timer_.start(timer__, this,1);
}

void sproxy_t::handle_req()
{
    if(!completed_)
    {
        network_->send_ack(key());
    }
    else
    {
        timing_=false;
        job_timer_.start(timer__, this,60);
        network_->send_response(key(),status(),response());
    }
}

void sproxy_t::timer__(void *ctx)
{
    sproxy_t *s = (sproxy_t *)ctx;

    if(!s->timing_)
    {
        s->timing_=true;
        return;
    }

    delete s;
}

sproxy_t::~sproxy_t()
{
    job_timer_.cancel();
    cancel();
    network_->remove_sproxy(key());
}

void sproxy_t::completed()
{
    completed_=true;
    timing_=false;
    network_->send_response(key(),status(),response());
    job_timer_.start(timer__, this,60);
}

snode_t::~snode_t()
{
    network_->clear_server(this);
}

void snode_t::detach(int e)
{
    if(!open_)
    {
        return;
    }

    remove();
    open_=false;

    std::set<invocation_t *>::iterator i;

    while((i=invocations_.begin())!=invocations_.end())
    {
        invocation_t *ii = (*i);
        invocations_.erase(i);
        ii->complete(0,pia_data_t());
    }

    cpoint_->disable();
    rpcserver_->plg_state = PLG_STATE_DETACHED;

    job_close_.cancel();

    if(e)
    {
        job_close_.idle(entity_->appq(),close_callback,this,pia_data_t());
    }
}

void snode_t::close_callback(void *t_, const pia_data_t & d)
{
    snode_t *t = (snode_t *)t_;
    bct_rpcserver_plug_closed(t->rpcserver_,t->entity_->api());
}

void snode_t::close()
{
    detach(0);
    rpcserver_->plg_state = PLG_STATE_CLOSED;
    delete this;
}

void snode_t::complete(void *c, bool s, const pia_data_t &v)
{
    std::set<invocation_t *>::iterator i;
    invocation_t *ii = (invocation_t *)c;

    if((i=invocations_.find(ii))==invocations_.end())
    {
        return;
    }

    invocations_.erase(i);
    ii->complete(s?1:-1,v);
}

void snode_t::api_close(bct_rpcserver_host_ops_t **t_)
{
    snode_t *t = PIC_STRBASE(snode_t,t_,host_ops_);

    try
    {
        pia_mainguard_t g(t->entity_->glue());
        t->close();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void snode_t::api_complete(bct_rpcserver_host_ops_t **t_, void *c, int s, bct_data_t v)
{
    snode_t *t = PIC_STRBASE(snode_t,t_,host_ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());
        t->complete(c,s?true:false,pia_data_t::from_lent(v));
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void snode_t::invoke__(void *s_, const pia_data_t &d)
{
    snode_t *s = (snode_t *)s_;
    const blob_t *b = (const blob_t *)d.asblob();
    invocation_t *i = b->invocation;
    pia_data_t v,n,k;


    {
        pia_mainguard_t g(s->entity_->glue());

        if(s->invocations_.find(i)==s->invocations_.end())
        {
            return;
        }

        v = i->request();
        n = i->name();
        k = i->key();
    }

    bct_rpcserver_plug_invoke(s->rpcserver_,s->entity_->api(),i,k.lend(),n.lend(),v.lend());
}

void snode_t::cancel__(void *ctx, invocation_t *i)
{
    snode_t *s = (snode_t *)ctx;
    std::set<invocation_t *>::iterator ii;

    if((ii=s->invocations_.find(i))!=s->invocations_.end())
    {
        s->invocations_.erase(ii);
    }
}

bool snode_t::invoke(invocation_t *i)
{
    if(!open_)
    {
        return false;
    }

    unsigned char *b;
    invocations_.insert(i);
    pia_data_t blobd = entity_->glue()->allocate_host(PIC_ALLOC_NORMAL,0,1,-1,0,BCTVTYPE_BLOB,sizeof(blob_t),&b,0,0);

    ((blob_t *)b)->invocation = i;
    i->set_cancel(cancel__,this);

    entity_->appq()->idle(cpoint_,invoke__,this,blobd);

    return true;
}

void cnode_t::api_close(bct_rpcclient_host_ops_t **t_)
{
    cnode_t *t = PIC_STRBASE(cnode_t,t_,host_ops_);

    try
    {
        pia_mainguard_t g(t->entity_->glue());
        t->close();
    }
    PIA_CATCHLOG_EREF(t->entity_)
}

void cnode_t::close_callback(void *t_, const pia_data_t & d)
{
    cnode_t *t = (cnode_t *)t_;
    bct_rpcclient_plug_complete(t->rpcclient_,t->entity_->api(), t->status(), t->response().lend());
}

void cnode_t::detach(int e)
{
    if(!open_)
    {
        return;
    }

    remove();
    open_=false;
    cancel();
    
    job_close_.cancel();

    if(e)
    {
        job_close_.idle(entity_->appq(),close_callback,this,pia_data_t());
    }
}

void cnode_t::close()
{
    detach(0);
    delete this;
}

void cnode_t::completed()
{
    detach(1);
}

bct_rpcserver_host_ops_t snode_t::dispatch__ =
{
    api_close,
    api_complete,
};

bct_rpcclient_host_ops_t cnode_t::dispatch__ =
{
    api_close,
};

