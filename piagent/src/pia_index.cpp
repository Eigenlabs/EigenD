
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

#include "pia_index.h"
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

#include <string.h>
#include <stdlib.h>

#define ISERVER_TICK_INTERVAL 5000
#define ISERVER_TICKS_DEATH   10

struct inode_t;
struct irecord_t;
struct iserver_t;

struct pia_indexlist_t::impl_t
{
    impl_t(pia::manager_t::impl_t *g): glue_(g) {}

    pia::manager_t::impl_t *glue_;
    pic::ilist_t<iserver_t> servers_;
};

struct iserver_t: pic::element_t<>
{
    iserver_t(pia_indexlist_t::impl_t *,const pia_data_t &);
    ~iserver_t();

    void unadvertise(void *id, const pia_data_t & a);
    void advertise(void *id, const pia_data_t & a, unsigned short cookie);
    void kill(const pia_ctx_t &e);
    void dump(const pia_ctx_t &e);
    void receive(const unsigned char *msg, unsigned len);
    void refresh(const pia_data_t &a, unsigned short cookie);
    void add(const pia_ctx_t &e, bct_index_t *i);
    void killadvertise(void *id);
    void close();
    void changed();
    int tick();
    void transmit();
    void timer();

    static void job_network(void *s_, const pia_data_t &);

    pia_data_t name_;
    pia_ctx_t entity_;
    pia_indexlist_t::impl_t *list_;

    pia_sockref_t socket_;
    pia_job_t job_timer_;

    pic::ilist_t<inode_t> nodes_;
    pic::ilist_t<irecord_t> records_;
};

struct inode_t: pic::element_t<>
{
    inode_t(iserver_t *s, const pia_ctx_t &e, bct_index_t *i);
    ~inode_t();

    void detach(bool e);

    int member_count();
    void member_died(const pia_data_t & name, unsigned short cookie);
    unsigned short member_cookie(int c);
    pia_data_t name();
    void close();
    pia_data_t member_name(int c);

    static int api_member_count(bct_index_host_ops_t **o);
    static void api_member_died(bct_index_host_ops_t **o, bct_data_t name, unsigned short cookie);
    static unsigned short api_member_cookie(bct_index_host_ops_t **o, int c);
    static bct_data_t api_name(bct_index_host_ops_t **o);
    static void api_close(bct_index_host_ops_t **o);
    static bct_data_t api_member_name(bct_index_host_ops_t **o, int c);

    bct_index_host_ops_t *ops_;

    bct_index_t *index_;
    iserver_t *server_;
    pia_ctx_t entity_;

    pia_job_t job_opened_;
    pia_job_t job_changed_;
    pia_job_t job_closed_;

    pia_data_t name_;

    static bct_index_host_ops_t dispatch__;
};

struct irecord_t: pic::element_t<>
{
    irecord_t(const pia_data_t &n, void *i, unsigned short c);

    pia_data_t name_;
    void *id_;
    unsigned short cookie_;
    int death_;
    long hash_;
};

void iserver_t::killadvertise(void *id)
{
    irecord_t *r,*n;

    r=records_.head();

    while(r)
    {
        n=records_.next(r);

        if(r->id_==id)
        {
            r->remove();
            delete r;
        }

        r=n;
    }

    changed();
}

void inode_job_closed(void *h_, const pia_data_t &d)
{
    inode_t *h = (inode_t *)h_;
    if(h->index_->plug_ops->index_closed) bct_index_plug_closed(h->index_,h->entity_->api());
}

void inode_t::detach(bool e)
{
    if(server_)
    {
        remove();
        server_->killadvertise(this);
        server_=0;

        job_closed_.cancel();
        job_changed_.cancel();
        job_opened_.cancel();

        index_->plg_state = PLG_STATE_DETACHED;

        if(e)
        {
            job_closed_.idle(entity_->appq(), inode_job_closed, this, pia_data_t());
        }
    }
}

void iserver_t::close()
{
    inode_t *h;
    irecord_t *r;

    remove();
    
    while((h=nodes_.head())!=0)
    {
        h->detach(true);
    }

    while((r=records_.head())!=0)
    {
        delete r;
    }

    job_timer_.cancel();
    socket_.close();

    delete this;
}

void inode_job_changed(void *h_, const pia_data_t & d)
{
    inode_t *h = (inode_t *)h_;
    if(h->index_->plug_ops->index_changed) bct_index_plug_changed(h->index_,h->entity_->api());
}

void iserver_t::changed()
{
    inode_t *h;

    for(h=nodes_.head(); h!=0; h=nodes_.next(h))
    {
        h->job_changed_.idle(h->entity_->appq(),inode_job_changed,h,pia_data_t());
    }

}

int iserver_t::tick()
{
    int chg=0, busy=0;
    irecord_t *r, *n;

    for(r=records_.head(); r!=0; r=records_.next(r))
    {
        if(r->id_)
        {
            busy=1;
        }
        else
        {
            r->death_++;
        }
    }

    r=records_.head();
    while(r)
    {
        n=records_.next(r);

        if(!r->id_ && r->death_ > ISERVER_TICKS_DEATH)
        {
            delete r;
            chg=1;
        }

        r=n;
    }

    if(chg)
    {
        changed();
    }

    if(nodes_.head())
    {
        busy=1;
    }

    return busy;
}

void iserver_t::transmit()
{
    int len;
    unsigned char msg[BCTLINK_SMALLPAYLOAD];
    unsigned char *buf;

    int used;

    used=0;
    len=sizeof(msg);
    buf=msg;

    for(irecord_t *r=records_.head(); r!=0; r=records_.next(r))
    {

    restart:

        if(r->id_)
        {
            int x;

            if((x=pie_setindex(buf,len,r->cookie_,r->name_.wirelen(),r->name_.wiredata()))<0)
            {
                socket_.write(msg,used);
                used=0;
                len=sizeof(msg);
                buf=msg;
                goto restart;
            }

            buf+=x; len-=x; used+=x;
        }
    }

    if(used>0)
    {
        socket_.write(msg,used);
    }

    if(records_.head())
    {
        records_.append(records_.head());
    }
}

void iserver_job_timer(void *e)
{
    iserver_t *s = (iserver_t *)e;
    s->timer();
}

void iserver_t::timer()
{
    if(tick())
    {
        transmit();
    }
    else
    {
        close();
    }
}

void inode_job_opened(void *h_, const pia_data_t & d)
{
    inode_t *h = (inode_t *)h_;

    if(h->index_->plug_ops->index_opened)
    {
        h->index_->plg_state = PLG_STATE_OPENED;
        bct_index_plug_opened(h->index_,h->entity_->api());
    }
}

void inode_t::close()
{
	iserver_t *s = server_;

    detach(false);
    job_closed_.cancel();

    if(s)
    {
        s->changed();
        s->timer();
    }

    index_->plg_state = PLG_STATE_CLOSED;
    delete this;
}

void inode_t::api_close(bct_index_host_ops_t **o)
{
    inode_t *h = PIC_STRBASE(inode_t,o,ops_);

    try
    {
        pia_mainguard_t guard(h->entity_->glue());

        h->close();
    }
    PIA_CATCHLOG_EREF(h->entity_)
}

pia_data_t inode_t::name()
{
    return name_;
}

bct_data_t inode_t::api_name(bct_index_host_ops_t **o)
{
    inode_t *h = PIC_STRBASE(inode_t,o,ops_);

    try
    {
        pia_mainguard_t guard(h->entity_->glue());

        return h->name().give_copy(h->entity_->glue()->allocator(), PIC_ALLOC_NORMAL);
    }
    PIA_CATCHLOG_EREF(h->entity_)

    return 0;
}

void inode_t::member_died(const pia_data_t & name, unsigned short cookie)
{
    irecord_t *r;

    if(server_)
    {
        for(r=server_->records_.head(); r!=0; r=server_->records_.next(r))
        {
            if(r->cookie_==cookie && r->name_==name)
            {
                delete r;
                break;
            }
        }

        server_->changed();
    }
}

void inode_t::api_member_died(bct_index_host_ops_t **o, bct_data_t name, unsigned short cookie)
{
    inode_t *h = PIC_STRBASE(inode_t,o,ops_);

    try
    {
        pia_mainguard_t guard(h->entity_->glue());

        h->member_died(pia_data_t::from_lent(name),cookie);
    }
    PIA_CATCHLOG_EREF(h->entity_)
}

unsigned short inode_t::member_cookie(int c)
{
    irecord_t *r;

    if(server_)
    {
        for(r=server_->records_.head(); r!=0; r=server_->records_.next(r))
        {
            if(c==0)
            {
                return r->cookie_;
            }

            c--;
        }
    }

    return 0;
}

unsigned short inode_t::api_member_cookie(bct_index_host_ops_t **o, int c)
{
    inode_t *h = PIC_STRBASE(inode_t,o,ops_);

    try
    {
        pia_mainguard_t guard(h->entity_->glue());

        return h->member_cookie(c);
    }
    PIA_CATCHLOG_EREF(h->entity_)

    return 0;
}

pia_data_t inode_t::member_name(int c)
{
    irecord_t *r;

    if(server_)
    {
        for(r=server_->records_.head(); r!=0; r=server_->records_.next(r))
        {
            if(c==0)
            {
                return r->name_;
            }

            c--;
        }
    }

    return pia_data_t();
}

bct_data_t inode_t::api_member_name(bct_index_host_ops_t **o, int c)
{
    inode_t *h = PIC_STRBASE(inode_t,o,ops_);

    try
    {
        pia_mainguard_t guard(h->entity_->glue());

        return h->member_name(c).give_copy(h->entity_->glue()->allocator(), PIC_ALLOC_NORMAL);
    }
    PIA_CATCHLOG_EREF(h->entity_)

    return 0;
}

int inode_t::member_count()
{
    irecord_t *r;
    unsigned c=0;

    if(server_)
    {
        for(r=server_->records_.head(); r!=0; r=server_->records_.next(r))
        {
            c++;
        }
    }

    return c;
}

int inode_t::api_member_count(bct_index_host_ops_t **o)
{
    inode_t *h = PIC_STRBASE(inode_t,o,ops_);

    try
    {
        pia_mainguard_t guard(h->entity_->glue());

        return h->member_count();
    }
    PIA_CATCHLOG_EREF(h->entity_)

    return 0;
}

bct_index_host_ops_t inode_t::dispatch__ = 
{
    api_close,
    api_name,
    api_member_name,
    api_member_count,
    api_member_cookie,
    api_member_died
};

inode_t::inode_t(iserver_t *s, const pia_ctx_t &e, bct_index_t *i): index_(i), server_(s), entity_(e), name_(s->name_)
{
    ops_=&dispatch__;
    i->host_ops=&ops_;
    index_->plg_state = PLG_STATE_ATTACHED;
    job_opened_.idle(entity_->appq(), inode_job_opened, this, pia_data_t());
    s->nodes_.append(this);
    bct_index_plug_attached(i);
}

inode_t::~inode_t()
{
}

void iserver_t::add(const pia_ctx_t &e, bct_index_t *i)
{
    new inode_t(this,e,i);
}

void iserver_t::refresh(const pia_data_t &a, unsigned short cookie)
{
    long hash = a.hash();

    irecord_t *r;

    for(r=records_.head(); r!=0; r=records_.next(r))
    {
        if(r->hash_==hash && r->name_==a && r->cookie_==cookie)
        {
            r->death_=0;
            return;
        }
    }

    r = new irecord_t(a,0,cookie);
    records_.append(r);
    changed();
}

void iserver_t::receive(const unsigned char *msg, unsigned len)
{
    int x;
    unsigned short cookie;
    const unsigned char *dp;
    unsigned short dl;

    while(len>0)
    {
        if((x=pie_getindex(msg,len,&cookie,&dl,&dp))<0)
        {
            break;
        }

        msg+=x; len-=x;
        refresh(entity_->glue()->allocate_wire(PIC_ALLOC_NORMAL,dl,dp),cookie);
    }
}

void iserver_t::dump(const pia_ctx_t &e)
{
    inode_t *h;

    h = nodes_.head();

    while(h!=0)
    {
        if(h->entity_.matches(e))
        {
            pic::logmsg() << "index server";
        }

        h=nodes_.next(h);
    }
}

void iserver_t::kill(const pia_ctx_t &e)
{
    inode_t *h,*n;

    h = nodes_.head();

    while(h!=0)
    {
        n=nodes_.next(h);

        if(h->entity_.matches(e))
        {
            h->detach(true);
        }

        h=n;
    }
}

void iserver_t::job_network(void *s_, const pia_data_t &d)
{
    iserver_t *s = (iserver_t *)s_;
    s->receive(d.hostdata(),d.hostlen());
}

irecord_t::irecord_t(const pia_data_t &n, void *i, unsigned short c): name_(n), id_(i), cookie_(c), death_(0)
{
    hash_ = n.hash();
}

void iserver_t::advertise(void *id, const pia_data_t &a, unsigned short cookie)
{
    long hash = a.hash();
    irecord_t *r;

    for(r=records_.head(); r!=0; r=records_.next(r))
    {
        if(r->id_==id && r->hash_==hash && r->name_==a)
        {
            r->cookie_=cookie;
            timer();
            changed();
            return;
        }
    }

    r = new irecord_t(a.copy(entity_->glue()->allocator(), PIC_ALLOC_NORMAL),id,cookie);
    records_.append(r);
    timer();
    changed();
}

void iserver_t::unadvertise(void *id, const pia_data_t &a)
{
    long hash = a.hash();
    irecord_t *r,*n;

    r=records_.head();

    while(r)
    {
        n=records_.next(r);

        if(r->id_==id && r->hash_==hash && r->name_==a)
        {
            delete r;
        }

        r=n;
    }

    changed();
}

iserver_t::~iserver_t()
{
}

iserver_t::iserver_t(pia_indexlist_t::impl_t *l, const pia_data_t &n): name_(n), list_(l), socket_(list_->glue_->allocator())
{
    entity_ = pia_ctx_t(list_->glue_,0,pic::status_t(),pic::f_string_t(), "index server");

    socket_.open(entity_->glue()->network(), BCTLINK_NAMESPACE_INDEX, entity_->glue()->expand_address(name_));
    socket_.callback(entity_->glue()->mainq(),job_network,this);
    job_timer_.timer(entity_->glue()->mainq(),iserver_job_timer,this,ISERVER_TICK_INTERVAL);

    l->servers_.append(this);
}

pia_indexlist_t::pia_indexlist_t(pia::manager_t::impl_t *g)
{
    impl_= new impl_t(g);
}

pia_indexlist_t::~pia_indexlist_t()
{
    try
    {
        iserver_t *s;

        while((s=impl_->servers_.head())!=0)
        {
            s->close();
        }
    }
    PIA_CATCHLOG_PRINT()

    delete impl_;
}

void pia_indexlist_t::index(const pia_data_t &n, const pia_ctx_t &e, bct_index_t *w)
{
    iserver_t *s;

    for(s=impl_->servers_.head(); s!=0; s=impl_->servers_.next(s))
    {
        if(s->name_==n)
        {
            goto found;
        }
    }

    s=new iserver_t(impl_,n);

found:

    s->add(e,w);
}

void pia_indexlist_t::dump(const pia_ctx_t &e)
{
    iserver_t *s;

    for(s=impl_->servers_.head(); s!=0; s=impl_->servers_.next(s))
    {
        s->dump(e);
    }
}

void pia_indexlist_t::kill(const pia_ctx_t &e)
{
    iserver_t *s;

    for(s=impl_->servers_.head(); s!=0; s=impl_->servers_.next(s))
    {
        s->kill(e);
    }
}

void pia_indexlist_t::advertise(void *id, const pia_data_t &index, const pia_data_t &name, unsigned short cookie)
{
    iserver_t *s;

    for(s=impl_->servers_.head(); s!=0; s=impl_->servers_.next(s))
    {
        if(s->name_==index)
        {
            goto found;
        }
    }

    s=new iserver_t(impl_,index);

found:

    s->advertise(id,name,cookie);
}

void pia_indexlist_t::unadvertise(void *id, const pia_data_t &index, const pia_data_t &name)
{
    iserver_t *s;

    for(s=impl_->servers_.head(); s!=0; s=impl_->servers_.next(s))
    {
        if(s->name_==index)
        {
            s->unadvertise(id,name);
            return;
        }
    }
}

void pia_indexlist_t::killadvertise(void *id)
{
    iserver_t *s;

    for(s=impl_->servers_.head(); s!=0; s=impl_->servers_.next(s))
    {
        s->killadvertise(id);
    }
}
