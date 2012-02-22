
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


#include "pia_fastdata.h"

#include "pia_glue.h"
#include "pia_error.h"
#include "pia_dataqueue.h"

#include <picross/pic_ilist.h>
#include <picross/pic_strbase.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_log.h>
#include <picross/pic_stl.h>

#include <stdlib.h>

struct slavelist_t: virtual public pic::lckobject_t
{
    slavelist_t() { list_.reserve(4); }
    slavelist_t(const slavelist_t &m): list_(m.list_) {}
    pic::lckvector_t<struct fnode_t *>::lcktype list_;
    typedef pic::lckvector_t<struct fnode_t *>::lcktype::const_iterator iter_t;
};


struct fnode_t: pic::element_t<0>, pic::element_t<1>, pia_dataqueue_t::subscriber_t, virtual public pic::lckobject_t
{
    bct_fastdata_host_ops_t *ops_;
    pia_ctx_t entity_;
    pia_fastdatalist_t::impl_t *list_;
    bct_fastdata_t *client_;
    bool enabled_;
    pia_job_t job_close_;
    unsigned short flags_;
    pic::ilist_t<fnode_t,1> downstream_;
    fnode_t *upstream_;
    void *upstream_tag_;
    pic::flipflop_t<slavelist_t> index_;
    pic::flipflop_t<fnode_t *> head_;
    pia_dataholder_nb_t current_id_;
    pia_dataqueuedrop_t current_queue_;
    bool id_suppressed_;

    fnode_t(pia_fastdatalist_t::impl_t *l, const pia_ctx_t &e, bct_fastdata_t *s);
    ~fnode_t();

    void detach(int e);
    void close();
	int send(const pia_data_t &d,const pia_dataqueue_t &);
	int send_fast(const pia_data_nb_t &d,const pia_dataqueue_t &);
    int set_upstream(bct_fastdata_t *f, void *);
    void enable(bool e,bool *s,bool p);
    void suppress(bool s,bool p);
    pia_data_nb_t current_nb(bool id);
    pia_data_t current_normal(pic::nballocator_t *a, bool id);
    pia_dataqueue_t current_queue();
    void rebuild();
    void set_head(fnode_t *);
    void clear_downstream(void *tag);
    void subscribe();
    void unsubscribe();

    virtual bool receive_data(const pia_data_nb_t &d)
    {
        pia_logprotect_t p;
        return bct_fastdata_plug_receive_data(client_,entity_->api(),d.lend())!=0;
    }

    static int close__(void *t_, void *, void *, void *);
    static void closer__(void *t_, const pia_data_t & d);
    static int evt_sender__(void *f_, void *d_, void *q_, void *);
    static int pinger__(void *f_, void *, void *, void *);

    static int api_close(bct_fastdata_host_ops_t **);
    static int api_send(bct_fastdata_host_ops_t **,bct_data_t,bct_dataqueue_t);
    static int api_set_upstream(bct_fastdata_host_ops_t **, bct_fastdata_t *);
    static int api_enable(bct_fastdata_host_ops_t **,int,int *,int);
    static int api_suppress(bct_fastdata_host_ops_t **,int,int);
    static bct_data_t api_current(bct_fastdata_host_ops_t **,int);
    static bct_dataqueue_t api_current_queue(bct_fastdata_host_ops_t **);
    static int api_send_fast(bct_fastdata_host_ops_t **,bct_data_t,bct_dataqueue_t);
    static int api_subscribe(bct_fastdata_host_ops_t **);

    static bct_fastdata_host_ops_t dispatch__;
};

struct pia_fastdatalist_t::impl_t
{
    pic::ilist_t<fnode_t,0> nodes_;

    fnode_t *find(bct_fastdata_t *f);
};

bct_fastdata_host_ops_t fnode_t::dispatch__ =
{
    api_close,
    api_send,
    api_set_upstream,
    api_enable,
    api_suppress,
    api_current,
    api_current_queue,
    api_send_fast,
    api_subscribe
};

pia_fastdatalist_t::pia_fastdatalist_t()
{
    impl_ = new impl_t;
}

pia_fastdatalist_t::~pia_fastdatalist_t()
{
    try
    {
        kill(0);
    }
    PIA_CATCHLOG_PRINT()
}

void pia_fastdatalist_t::fastdata(const pia_ctx_t &e, bct_fastdata_t *s)
{
    new fnode_t(impl_,e,s);
}

fnode_t *pia_fastdatalist_t::impl_t::find(bct_fastdata_t *s)
{
    if(s && s->host_ops)
    {
        fnode_t *sn = PIC_STRBASE(fnode_t,s->host_ops,ops_);

        if(sn->client_ == s)
        {
            return sn;
        }
    }

    return 0;
}

void pia_fastdatalist_t::dump(const pia_ctx_t &e)
{
	fnode_t *i;

	i=impl_->nodes_.head();

    while(i)
    {
		if(i->entity_.matches(e))
		{
            pic::logmsg() << "fastdata " << i->entity_->tag();
        }

        i = impl_->nodes_.next(i);
    }
}

void pia_fastdatalist_t::kill(const pia_ctx_t &e)
{
    fnode_t *i,*n;

    i=impl_->nodes_.head();

    while(i)
    {
        n = impl_->nodes_.next(i);

        if(i->entity_.matches(e))
        {
            i->detach(1);
        }

        i=n;
    }
}

fnode_t::fnode_t(pia_fastdatalist_t::impl_t *l, const pia_ctx_t &e, bct_fastdata_t *s): entity_(e), current_id_()
{
    current_queue_.set(pia_dataqueue_t());

    list_=l;
    client_=s;
    enabled_=false;
    id_suppressed_=true;
    ops_=&dispatch__;
    flags_=s->plug_flags;
    client_->host_ops=&ops_;
    upstream_=0;

    l->nodes_.append(this);
    rebuild();

    if(client_->plug_ops && client_->plug_ops->fastdata_attached)
    {
        bct_fastdata_plug_attached(client_);
    }
}

fnode_t::~fnode_t()
{
    detach(0);
}

void fnode_t::closer__(void *t_, const pia_data_t & d)
{
    fnode_t *t = (fnode_t *)t_;
    bct_fastdata_plug_closed(t->client_,t->entity_->api());
}

void fnode_t::set_head(fnode_t *head)
{
    fnode_t *f;

    head_.set(head);

    if(head && enabled_ && client_->plug_ops && client_->plug_ops->fastdata_receive_event)
    {
        head->index_.alternate().list_.push_back(this);
    }

    for(f=downstream_.head(); f!=0; f=downstream_.next(f))
    {
        f->set_head(head);
    }
}

void fnode_t::rebuild()
{
    fnode_t *f = this;

    while(f->upstream_)
    {
        f=f->upstream_;
    }

    if((f->flags_&PLG_FASTDATA_SENDER)!=0)
    {
        f->index_.alternate().list_.clear();
        f->set_head(f);
        f->index_.exchange();
    }
    else
    {
        f->set_head(0);
    }
}

void fnode_t::detach(int e)
{
    if(!list_)
    {
        return;
    }

    list_->nodes_.remove(this);
    list_=0;
    job_close_.cancel();

    if(upstream_)
    {
        upstream_->downstream_.remove(this);
        upstream_->rebuild();
        upstream_=0;
    }

    clear_downstream(0);

    if(e && client_->plug_ops && client_->plug_ops->fastdata_closed)
    {
        job_close_.idle(entity_->appq(),closer__,this,pia_data_t());
    }
}

void fnode_t::clear_downstream(void *tag)
{
    fnode_t *d;

restart:

    for(d=downstream_.head(); d!=0; d=downstream_.next(d))
    {
        if(!tag || d->upstream_tag_ == tag)
        {
            //pic::logmsg() << "clearing " << d->client_ << " from downstream of " << client_;
            downstream_.remove(d);
            d->upstream_=0;
            d->rebuild();
            goto restart;
        }
    }

    rebuild();
}

void fnode_t::close()
{
    entity_->glue()->fastcall(close__,this,0,0,0);
}

int fnode_t::close__(void *f_, void *, void *, void *)
{
    fnode_t *f = (fnode_t *)f_;
    
    f->detach(0);
    delete f;

    return 0;
}

pia_data_nb_t fnode_t::current_nb(bool id)
{
    pic::flipflop_t<fnode_t *>::guard_t g(head_);

    if(g.value())
    {
        if(id)
        {
            return g.value()->current_id_.get_nb();
        }
        else
        {
            return current_queue().current_nb();
        }
    }

    return pia_data_nb_t();
}

pia_data_t fnode_t::current_normal(pic::nballocator_t *a, bool id)
{
    pic::flipflop_t<fnode_t *>::guard_t g(head_);

    if(g.value())
    {
        if(id)
        {
            return g.value()->current_id_.get_normal(a);
        }
        else
        {
            return current_queue().current_normal(a);
        }
    }

    return pia_data_t();
}

pia_dataqueue_t fnode_t::current_queue()
{
    pic::flipflop_t<fnode_t *>::guard_t g(head_);

    if(g.value())
    {
        return g.value()->current_queue_.get();
    }

    return pia_dataqueue_t();
}

int fnode_t::evt_sender__(void *f_, void *d_, void *q_, void *)
{
    fnode_t *f = (fnode_t *)f_;
    const pia_data_nb_t d = pia_data_nb_t::from_given((bct_data_t)d_);
    const pia_dataqueue_t *q = (const pia_dataqueue_t *)q_;

    f->current_id_.set(d);
    f->current_queue_.set(*q);

    if(f->index_.current().list_.empty())
    {
        return 0;
    }

    pic::flipflop_t<slavelist_t>::guard_t g(f->index_);
    slavelist_t::iter_t b,e,i;

    b=g.value().list_.begin(); e=g.value().list_.end();

    for(i=b; i!=e; i++)
    {
        fnode_t *f2=const_cast<fnode_t *>(*i);
        if(!f2->id_suppressed_)
        {
            f2->unsubscribe();

            int rc;
            {
                pia_logprotect_t p;
                bct_data_t d2 = d.lend();
                rc=bct_fastdata_plug_receive_event(f2->client_,f2->entity_->api(),d2,(*q).lend());
            }

            if(rc)
            {
                if(!d.is_null())
                {
                    f2->subscribe();
                }
            }
        }
    }

    return 0;
}

void fnode_t::unsubscribe()
{
    pic::element_t<DATAQUEUE_SUBSCRIBER>::remove();
}

void fnode_t::subscribe()
{
    if(current_queue().isvalid())
    {
        current_queue().subscribe(this);
    }
}

int fnode_t::send_fast(const pia_data_nb_t &d, const pia_dataqueue_t &q)
{
    if((flags_&PLG_FASTDATA_SENDER) == 0)
    {
        return -1;
    }

    evt_sender__(this,(void *)d.give(),(void *)&q,0);

    return 0;
}

int fnode_t::send(const pia_data_t &d, const pia_dataqueue_t &q)
{
    if((flags_&PLG_FASTDATA_SENDER) == 0)
    {
        return -1;
    }

    entity_->glue()->fastcall(evt_sender__,this,(void *)d.give_copy(entity_->glue()->allocator(), PIC_ALLOC_NB),(void *)&q,0);
    return 0;
}

int fnode_t::set_upstream(bct_fastdata_t *s, void *tag)
{
    //pic::logmsg() << (void *)client_ << " set_upstream " << s;

    if((flags_ & PLG_FASTDATA_SENDER)!=0)
    {
        return -1;
    }

    if(upstream_)
    {
        upstream_->downstream_.remove(this);
        upstream_->rebuild();
        upstream_=0;
    }

    if(!s)
    {
        rebuild();
        return 0;
    }

    if(!(upstream_=list_->find(s)))
    {
        return 0;
    }

    upstream_tag_=tag;
    upstream_->downstream_.append(this);
    rebuild();
    return 0;
}

int fnode_t::pinger__(void *f_, void *s_, void *p_, void *)
{
    fnode_t *f = (fnode_t *)f_;
    bool *s = (bool *)s_;
    bool p = *(bool *)p_;

    if(s)
    {
        f->id_suppressed_ = *s;
    }

    if(p && !f->id_suppressed_)
    {
        pia_data_nb_t id(f->current_nb(true));
        pia_dataqueue_t q(f->current_queue());

        if(!id.is_null() && q.isvalid())
        {
            int rc;
            {
                pia_logprotect_t p;
                rc=bct_fastdata_plug_receive_event(f->client_,f->entity_->api(),id.lend(),q.lend());
            }

            if(rc)
            {
                f->subscribe();
            }
        }
    }

    return 0;
}

void fnode_t::enable(bool e, bool *s, bool p)
{
    //pic::logmsg() << (void *)client_ << " enable? " << e;
    bool oe = enabled_;

    if(oe==e)
    {
        entity_->glue()->fastcall(pinger__,this,s,&p,0);
    }
    else
    {
        if(oe)
        {
            enabled_=false;
            rebuild();

            if(s) id_suppressed_=*s;
        }
        else
        {
            enabled_=true;

            if(s) id_suppressed_=*s;

            rebuild();

            if(p)
            {
                entity_->glue()->fastcall(pinger__,this,s,&p,0);
            }
        }
    }
}

void fnode_t::suppress(bool s, bool p)
{
    if(enabled_)
    {
        id_suppressed_=s;

        if(!id_suppressed_ && enabled_ && p)
        {
            bct_fastdata_plug_receive_event(client_,entity_->api(),current_nb(true).lend(),current_queue().lend());
        }
    }
}

int fnode_t::api_close(bct_fastdata_host_ops_t **t_)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());
        t->close();
        return 0;
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

int fnode_t::api_send_fast(bct_fastdata_host_ops_t **t_, bct_data_t d, bct_dataqueue_t q)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        if(t->list_)
        {
            return t->send_fast(pia_data_nb_t::from_lent(d),pia_dataqueue_t::from_lent(q));
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

int fnode_t::api_subscribe(bct_fastdata_host_ops_t **t_)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        t->subscribe();
        return 1;
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

int fnode_t::api_send(bct_fastdata_host_ops_t **t_, bct_data_t d, bct_dataqueue_t q)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        if(t->list_)
        {
            return t->send(pia_data_t::from_given(d),pia_dataqueue_t::from_lent(q));
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

int fnode_t::api_set_upstream(bct_fastdata_host_ops_t **t_, bct_fastdata_t *f)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());

        if(t->list_)
        {
            return t->set_upstream(f,0);
        }

        if(!f)
        {
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

int fnode_t::api_suppress(bct_fastdata_host_ops_t **t_, int s, int p)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        if(t->list_)
        {
            t->suppress(s?true:false,p?true:false);
            return 0;
        }

        if(s)
        {
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

int fnode_t::api_enable(bct_fastdata_host_ops_t **t_,int e, int *s, int p)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_mainguard_t guard(t->entity_->glue());
        bool bs = s?((*s)?true:false):false;

        if(t->list_)
        {
            t->enable(e?true:false,s?(&bs):0,p?true:false);
            return 0;
        }

        if(!e)
        {
            return 0;
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return -1;
}

bct_data_t fnode_t::api_current(bct_fastdata_host_ops_t **t_, int id)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        if(t->list_)
        {
            return t->current_nb(id?true:false).give();
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return (bct_data_t)0;
}

bct_dataqueue_t fnode_t::api_current_queue(bct_fastdata_host_ops_t **t_)
{
    fnode_t *t = PIC_STRBASE(fnode_t,t_,ops_);

    try
    {
        pia_logguard_t guard(t->entity_->glue());

        if(t->list_)
        {
            return t->current_queue().give();
        }
    }
    PIA_CATCHLOG_EREF(t->entity_)

    return (bct_dataqueue_t)0;

}

pia_fastdata_t::pia_fastdata_t(const pia_ctx_t &e, unsigned flags, int (*rcv)(void *,bct_entity_t,bct_data_t,bct_dataqueue_t), int (*rcv2)(void *,bct_entity_t,bct_data_t),void *rcvctx)
{
    plug_flags = flags;
    plug_ops = &dispatch_;
    dispatch_.fastdata_attached = 0;
    dispatch_.fastdata_closed = 0;
    dispatch_.fastdata_receive_event = rcv;
    dispatch_.fastdata_receive_data = rcv2;
    dispatch_.fastdata_receive_context = rcvctx;
    e->glue()->create_fastdata(e,this);
}

pia_fastdata_t::~pia_fastdata_t()
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);
    t->close();
}

void pia_fastdata_t::fastdata_clear_downstream(void *tag)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);

    if(t->list_)
    {
        t->clear_downstream(tag);
    }
}

void pia_fastdata_t::fastdata_add_downstream(bct_fastdata_t *d, void *tag)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);
    fnode_t *dt = PIC_STRBASE(fnode_t,d->host_ops,ops_);

    if(dt->list_ && t->list_)
    {
        dt->set_upstream(this,0);
    }
}

void pia_fastdata_t::fastdata_set_upstream(bct_fastdata_t *u, void *tag)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);

    if(t->list_)
    {
        t->set_upstream(u,tag);
    }
}

void pia_fastdata_t::fastdata_enable(bool f,bool p)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);
    bool s = false;

    if(t->list_)
    {
        t->enable(f,&s,p);
    }
}

pia_data_nb_t pia_fastdata_t::fastdata_current_nb(bool id)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);

    if(t->list_)
    {
        return t->current_nb(id);
    }

    return pia_data_nb_t();
}

pia_data_t pia_fastdata_t::fastdata_current_normal(pic::nballocator_t *a, bool id)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);

    if(t->list_)
    {
        return t->current_normal(a, id);
    }

    return pia_data_t();
}

void pia_fastdata_t::fastdata_send(const pia_data_t &d,const pia_dataqueue_t &q)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);

    if(t->list_)
    {
        t->send(d,q);
    }
}

void pia_fastdata_t::fastdata_send(const pia_data_nb_t &d,const pia_dataqueue_t &q)
{
    fnode_t *t = PIC_STRBASE(fnode_t,host_ops,ops_);

    if(t->list_)
    {
        t->send_fast(d,q);
    }
}
