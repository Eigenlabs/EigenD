
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

#include "pia_server.h"
#include "pia_glue.h"
#include "pia_data.h"
#include "pia_error.h"

#include <pibelcanto/plugin.h>
#include <picross/pic_strbase.h>
#include <picross/pic_log.h>
#include <piembedded/pie_message.h>

#include <stdlib.h>
#include <string.h>

struct cnode_t;
struct pia_serverlocal_t;
struct pia_serverglobal_t;

typedef pic::ref_t<pia_serverlocal_t> lref_t;
typedef pic::ref_t<pia_serverglobal_t> gref_t;

struct pia_serverglobal_t: pic::counted_t
{
    pia_serverglobal_t(const pia_data_t &a, const pia_ctx_t &e);

    pia::manager_t::impl_t *glue() { return entity_->glue(); }

    pia_data_t addr_;
    pia_ctx_t entity_;
    pic::ilist_t<cnode_t,CNODE_ROOTS> roots_;
    unsigned short cookie_;
    bool insync_;
};

struct pia_serverlocal_t: pic::counted_t
{
    pia_serverlocal_t(const pia_data_t &, const pia_ctx_t &e);
    pia_serverlocal_t(lref_t &parent, unsigned char);

    unsigned char last_;
    gref_t global_;
    pia_data_t path_;
    void *clock_;
    pic::ref_t<pia_server_t::fast_receiver_t> fast_;
};

struct cnode_t: pic::element_t<CNODE_CHILDREN>, pic::element_t<CNODE_RECEIVE>, pic::element_t<CNODE_SYNCS>, pic::element_t<CNODE_CLIENTS>, pic::element_t<CNODE_ROOTS>, pic::element_t<CNODE_SCLIENTS>
{
    cnode_t(pia_server_t *s, const pia_ctx_t &e, cnode_t *p, bct_client_t *cl);

    static void api_close(bct_client_host_ops_t **);
    static bct_data_t api_servername(bct_client_host_ops_t **);
    static bct_data_t api_path(bct_client_host_ops_t **);
    static int api_child_add(bct_client_host_ops_t **, const unsigned char *, unsigned, bct_client_t *);
    static int api_child_remove(bct_client_host_ops_t **, const unsigned char *, unsigned, bct_client_t *);
    static unsigned char api_child_enum(bct_client_host_ops_t **, const unsigned char *, unsigned, unsigned char);
    static bct_client_t *api_child_get(bct_client_host_ops_t **, const unsigned char *, unsigned);
    static bct_data_t api_current_slow(bct_client_host_ops_t **);
    static int api_clone(bct_client_host_ops_t **, bct_client_t *);
    static bct_data_t api_child_data(bct_client_host_ops_t **, const unsigned char *, unsigned, unsigned *);
    static int api_child_exists(bct_client_host_ops_t **, const unsigned char *, unsigned);
    static int api_cookie(bct_client_host_ops_t **);
    static int api_shutdown(bct_client_host_ops_t **);
    static int api_set_downstream(bct_client_host_ops_t **, bct_clocksink_t *);
    static int api_set_sink(bct_client_host_ops_t **, bct_fastdata_t *);
    static long api_child_dcrc(bct_client_host_ops_t **, const unsigned char *, unsigned);
    static long api_child_tcrc(bct_client_host_ops_t **, const unsigned char *, unsigned);
    static long api_child_ncrc(bct_client_host_ops_t **, const unsigned char *, unsigned);
    static int api_sync(bct_client_host_ops_t **);

    static void job_close(void *c_, const pia_data_t & d);
    static void job_sync(void *c_, const pia_data_t & d);
    static void job_rsync(void *c_, const pia_data_t & d);
    static void job_child(void *c_, const pia_data_t & d);
    static void job_tree(void *c_, const pia_data_t & d);
    static void job_data(void *c_, const pia_data_t & d);
    static void job_setclock(void *c_, const pia_data_t & d);
    static void job_open(void *c_, const pia_data_t & d);

    static void clockgone(void *);

    void schedule_child();
    void schedule_tree();
    void schedule_data(const pia_data_t & d);
    void schedule_sync();
    void schedule_rsync();
    void schedule_setclock();
    void close();

    void dirty();

    static bct_client_host_ops_t dispatch__;
    bct_client_host_ops_t *client_ops_;

    lref_t local_;
    bct_client_t *client_;
    pia_server_t *server_;
    cnode_t *parent_;
    cnode_t *root_;
    pia_ctx_t entity_;
    pia_data_t current_slow_;
    void *clock_;
    bool needsync_;
    pia_cref_t cpoint_;

    pic::ilist_t<cnode_t, CNODE_CHILDREN> children_;
    pic::ilist_t<cnode_t, CNODE_SYNCS> syncs_;

    pia_job_t job_open_;
    pia_job_t job_child_;
    pia_job_t job_tree_;
    pia_job_t job_sync_;
    pia_job_t job_close_;
    pia_job_t job_clock_;
    pia_job_t job_rsync_;
};

pia_serverglobal_t::pia_serverglobal_t(const pia_data_t &addr, const pia_ctx_t &e): addr_(addr), entity_(e), cookie_(0), insync_(true)
{
}

pia_serverlocal_t::pia_serverlocal_t(const pia_data_t &addr, const pia_ctx_t &e): last_(0), global_(pic::ref(new pia_serverglobal_t(addr,e))), clock_(0)
{
    path_ = global_->glue()->allocate_path(0,0);
}

pia_serverlocal_t::pia_serverlocal_t(lref_t &p, unsigned char last): last_(last), global_(p->global_), clock_(0)
{
    path_ = global_->glue()->path_append(p->path_,last);
}

void cnode_t::job_close(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    bct_client_plug_closed(c->client_,c->entity_->api());
}

void cnode_t::job_sync(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    bct_client_plug_sync(c->client_,c->entity_->api());
}

void cnode_t::job_child(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    bct_client_plug_child(c->client_,c->entity_->api());
}

void cnode_t::job_tree(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    bct_client_plug_tree(c->client_,c->entity_->api());
}

void cnode_t::job_data(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    bct_client_plug_data(c->client_,c->entity_->api(),d.lend());
}

void cnode_t::job_setclock(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    bct_client_plug_clock(c->client_,c->entity_->api());
}

void cnode_t::schedule_child()
{
    if(server_)
        job_child_.idle(entity_->appq(),job_child,this,pia_data_t());
}

void cnode_t::schedule_tree()
{
    if(server_)
        job_tree_.idle(entity_->appq(),job_tree,this,pia_data_t());
}

void cnode_t::schedule_data(const pia_data_t & d)
{
    if(server_)
    {
        entity_->appq()->idle(cpoint_,job_data, this, d);
    }
}

void cnode_t::schedule_rsync()
{
    if(root_->server_ && local_->global_->insync_)
    {
        root_->job_rsync_.idle(entity_->appq(),job_rsync,root_,pia_data_t());
    }
}

void cnode_t::schedule_sync()
{
    if(server_)
        job_sync_.slowjob(entity_->appq(),job_sync,this,pia_data_t());
}

void cnode_t::schedule_setclock()
{
    if(server_)
        job_clock_.idle(entity_->appq(),job_setclock,this,pia_data_t());
}

void cnode_t::job_open(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;

    {   pia_mainguard_t guard(c->entity_->glue());
        c->schedule_tree();

        if(c->client_->plug_flags & PLG_CLIENT_SYNC)
        {
            c->dirty();
            c->schedule_rsync();
        }
    }

    c->client_->plg_state=PLG_STATE_OPENED;
    bct_client_plug_opened(c->client_,c->entity_->api());
}

void cnode_t::clockgone(void *c_)
{
    cnode_t *c = (cnode_t *)c_;
    c->clock_=0;
}

int cnode_t::api_cookie(bct_client_host_ops_t **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_logguard_t guard(c->entity_->glue());
        return c->local_->global_->cookie_;
    }
    PIA_CATCHLOG_EREF(c->entity_)
    return -1;
}

bct_data_t cnode_t::api_path(bct_client_host_ops_t  **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_logguard_t guard(c->entity_->glue());
        return c->local_->path_.give_copy(c->entity_->glue()->allocator(), PIC_ALLOC_NORMAL);
    }
    PIA_CATCHLOG_EREF(c->entity_)
    return 0;
}

bct_data_t cnode_t::api_servername(bct_client_host_ops_t  **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_logguard_t guard(c->entity_->glue());
        return c->local_->global_->addr_.give_copy(c->entity_->glue()->allocator(), PIC_ALLOC_NORMAL);
    }
    PIA_CATCHLOG_EREF(c->entity_)
    return 0;
}

int cnode_t::api_set_downstream(bct_client_host_ops_t **co, bct_clocksink_t *dn)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());

        if(c->clock_)
        {
            c->entity_->glue()->cleardownstreamclock(c->local_->clock_,c->clock_);
            c->entity_->glue()->cancelclocknotify(c->clock_);
        }

        if(!dn)
        {
            return 1;
        }

        c->clock_ = c->entity_->glue()->addclocknotify(dn, clockgone, c);
        return c->entity_->glue()->setdownstreamclock(c->local_->clock_,c->clock_);
    }
    PIA_CATCHLOG_EREF(c->entity_)
    return -1;
}

pia_server_t *pia_server_t::find(const unsigned char *path, unsigned len, pia_server_t **good)
{
    pia_server_t *n = this;

restart:

    while(len>0)
    {
        const children_t &cc(n->children_.current());
        children_t::const_iterator iter(cc.find(*path));

        if(iter!=cc.end())
        {
            n=iter->second;
            len--;
            path++;
            goto restart;
        }

        if(good) *good=n;
        return 0;
    }

    return n;
}

pia_server_t *pia_server_t::findvisible(const unsigned char *path, unsigned len)
{
    pia_server_t *n = this;

restart:

    while(len>0)
    {
        const children_t &cc(n->children_.current());
        children_t::const_iterator iter(cc.find(*path));

        if(iter!=cc.end())
        {
            if(iter->second->isvisible())
            {
                n=iter->second;
                len--;
                path++;
                goto restart;
            }
        }

        return 0;
    }

    return n;
}

int cnode_t::api_child_exists(bct_client_host_ops_t **co, const unsigned char *path, unsigned pathlen)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t;

        if(s)
        {
            if((t=s->findvisible(path,pathlen)) != 0)
            {
                return 1;
            }
        }

        return 0;
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

bct_data_t cnode_t::api_child_data(bct_client_host_ops_t **co, const unsigned char *path, unsigned pathlen, unsigned *flags)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t;

        if(s)
        {
            if((t=s->findvisible(path,pathlen)) != 0)
            {
                if(flags) *flags=t->flags();
                return t->current_slow_.give();
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    pic::msg() << "no child " << path[0] << pic::log;
    return 0;
}

void cnode_t::close()
{
    pia_server_t *s = server_;
    cnode_t *c;

    if(s)
    {
        s->clients_.remove(this);
        s->sclients_.remove(this);
        server_=0; 

        if(s->fast_)
            s->fast_->fastdata_clear_downstream(this);

        if(parent_)
        {
            parent_->children_.remove(this);
            parent_->schedule_child();
        }
        else
        {
            local_->global_->roots_.remove(this);
        }

        if(client_->plug_flags & PLG_CLIENT_SYNC)
        {
            root_->syncs_.remove(this);
        }

        if(clock_)
        {
            entity_->glue()->cancelclocknotify(clock_);
            clock_ = 0;
        }

        job_close_.idle(entity_->appq(),job_close,this,pia_data_t());
        client_->plg_state = PLG_STATE_DETACHED;

        while((c=children_.head())!=0)
        {
            c->close();
        }

        job_open_.cancel();
        job_tree_.cancel();
        job_child_.cancel();
        job_clock_.cancel();
        job_sync_.cancel();
        job_rsync_.cancel();

        cpoint_->disable();
    }
}

void pia_server_t::dropclients()
{
    cnode_t *c;

    while((c=clients_.head())!=0)
    {
        c->close();
    }
}

int cnode_t::api_shutdown(bct_client_host_ops_t **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_;

        if(s)
        {
            c->close();

            if(!s->clients_.head())
            {
                s->server_empty();
            }

            return 0;
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)
    return -1;
}

void cnode_t::api_close(bct_client_host_ops_t **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_;

        if(s)
        {
            c->close();

            if(!s->clients_.head())
            {
                s->server_empty();
            }
        }

        c->client_->host_ops=0;
        c->client_->plg_state=PLG_STATE_CLOSED;
        c->job_close_.cancel();

        delete c;
    }
    PIA_CATCHLOG_EREF(c->entity_)
}

int cnode_t::api_child_add(bct_client_host_ops_t  **co, const unsigned char *o, unsigned l, bct_client_t *ch)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t = 0;

        if(s)
        {
            if((t=s->findvisible(o,l))!=0)
            {
                new cnode_t(t,c->entity_,c,ch);
                return 1;
            }
            else
                pic::msg() << "child add to invisible node" << pic::log;
        }
        else
            pic::msg() << "child add to closed node" << pic::log;
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

int cnode_t::api_child_remove(bct_client_host_ops_t  **co, const unsigned char *o, unsigned l, bct_client_t *ch)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t = 0;
        cnode_t *c2;

        if(!s || (t=s->findvisible(o,l))==0)
        {
            return -1;
        }

        for(c2=t->clients_.head(); c2!=0; c2=t->clients_.next(c2))
        {
            if(c2->client_==ch)
            {
                c2->close();

                if(!t->clients_.head())
                {
                    t->server_empty();
                }

                return 1;
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

int cnode_t::api_clone(bct_client_host_ops_t  **co, bct_client_t *ch)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_;

        bct_client_host_ops_t **oco = ch->host_ops;
        if(oco)
        {
            return -1;
        }

        if(s)
        {
            new cnode_t(s,c->entity_,c,ch);
            return 1;
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

unsigned char cnode_t::api_child_enum(bct_client_host_ops_t **co, const unsigned char *o, unsigned l, unsigned char ch)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t;

        if(s)
        {
            if((t=s->findvisible(o,l))!=0)
            {
                return t->enum_visible(ch);
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return 0;
}

bct_client_t *cnode_t::api_child_get(bct_client_host_ops_t **co, const unsigned char *o, unsigned l)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t = 0;
        cnode_t *c2;

        if(!s || (t=s->findvisible(o,l))==0)
        {
            return 0;
        }

        for(c2=t->clients_.head(); c2!=0; c2=t->clients_.next(c2))
        {
            if(c2->parent_==c)
            {
                return c2->client_;
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return 0;
}

bct_data_t cnode_t::api_current_slow(bct_client_host_ops_t **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        return c->current_slow_.give_copy(c->entity_->glue()->allocator(), PIC_ALLOC_NORMAL);
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return 0;
}

int cnode_t::api_sync(bct_client_host_ops_t **co)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_;

        if((c->client_->plug_flags & PLG_CLIENT_SYNC)!=0 && s)
        {
            c->dirty();
            c->schedule_rsync();
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

long cnode_t::api_child_ncrc(bct_client_host_ops_t **co, const unsigned char *path, unsigned pathlen)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t;

        if(s)
        {
            if((t=s->findvisible(path,pathlen)) != 0)
            {
                return t->nseq_;
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

long cnode_t::api_child_tcrc(bct_client_host_ops_t **co, const unsigned char *path, unsigned pathlen)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t;

        if(s)
        {
            if((t=s->findvisible(path,pathlen)) != 0)
            {
                return t->tseq_;
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

long cnode_t::api_child_dcrc(bct_client_host_ops_t **co, const unsigned char *path, unsigned pathlen)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_, *t;

        if(s)
        {
            if((t=s->findvisible(path,pathlen)) != 0)
            {
                return t->dseq_;
            }
        }
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

int cnode_t::api_set_sink(bct_client_host_ops_t **co, bct_fastdata_t *f)
{
    cnode_t *c = PIC_STRBASE(cnode_t,co,client_ops_);

    try
    {
        pia_mainguard_t guard(c->entity_->glue());
        pia_server_t *s = c->server_;

        if(s)
        {
            pia_fastdata_t *src = s->fastdata();

            if(src)
            {
                src->fastdata_clear_downstream(c);
                if(f)
                {
                    src->fastdata_add_downstream(f,c);
                }
            }
        }

        return 0;
    }
    PIA_CATCHLOG_EREF(c->entity_)

    return -1;
}

unsigned pia_server_t::visible_children()
{
    return visible_children_;
}

int pia_server_t::visit(void *arg, int (*func)(void *, unsigned char, pia_server_t *))
{
    int x;

    children_t::const_iterator iter;
    const children_t &cc(children_.current());

    for(iter=cc.begin(); iter!=cc.end(); iter++)
    {
        if((x=func(arg,iter->first,iter->second))<0)
        {
            return x;
        }
    }

    return 0;
}

void pia_server_t::filter(void *arg, int (*func)(void *, unsigned char, pia_server_t *))
{
    children_t::const_iterator iter;
    const children_t &cc(children_.current());
    children_t dead;

    for(iter=cc.begin(); iter!=cc.end(); iter++)
    {
        if(func(arg, iter->first, iter->second))
        {
            dead.insert(std::make_pair(iter->first,iter->second));
        }
    }

    for(iter=dead.begin(); iter!=dead.end(); iter++)
    {
        iter->second->close();
    }
}

void pia_server_t::childadded(pia_server_t *child)
{
    cnode_t *c2;

    if(child->tseq_>tseq_)
    {
        tseq_=child->tseq_;

        pia_server_t *p = this;

        while(p)
        {
            if(tseq_<p->tseq_)
            {
                break;
            }

            p->tseq_=tseq_;
            p=p->parent_;
        }
    }

    for(c2=clients_.head(); c2!=0; c2=clients_.next(c2))
    {
        c2->schedule_tree();
        //c2->dirty();
    }

    dirty();
    visible_children_++;
    server_childadded();
}

bct_client_host_ops_t cnode_t::dispatch__ =
{
    api_close,
    api_servername,
    api_path,
    api_child_add,
    api_child_remove,
    api_child_enum,
    api_child_get,
    api_current_slow,
    api_clone,
    api_child_data,
    api_child_exists,
    api_cookie,
    api_shutdown,
    api_set_downstream,
    api_set_sink,
    api_child_dcrc,
    api_child_tcrc,
    api_child_ncrc,
    api_sync
};

cnode_t::cnode_t(pia_server_t *s, const pia_ctx_t &e, cnode_t *p, bct_client_t *cl): local_(s->local_), client_(cl), server_(s), parent_(p), entity_(e), current_slow_(s->current_slow_.copy(entity_->glue()->allocator(), PIC_ALLOC_NORMAL)), clock_(0), needsync_(false)
{
    client_ops_ = &dispatch__;
    cpoint_ = pia_make_cpoint();

    if(parent_)
    {
        parent_->children_.append(this);
        root_=parent_->root_;
    }
    else
    {
        local_->global_->roots_.append(this);
        root_=this;
    }

    if(cl->plug_flags & PLG_CLIENT_SYNC)
    {
        root_->syncs_.prepend(this);
        s->sclients_.append(this);
    }

    client_->host_flags=s->flags_;

    s->clients_.append(this);
    job_open_.idle(entity_->appq(),job_open,this,pia_data_t());

    client_->host_ops=&client_ops_;
    client_->plg_state=PLG_STATE_ATTACHED;
    bct_client_plug_attached(client_,current_slow_.give_copy(entity_->glue()->allocator(), PIC_ALLOC_NORMAL));
}

void cnode_t::dirty()
{
    cnode_t *p = this;

    while(p)
    {
        p->needsync_ = true;
        p=p->parent_;
    }
}

void pia_server_t::dirty()
{
    pia_server_t *p = this;
    cnode_t *c;

    while(p)
    {
        for(c=p->sclients_.head(); c!=0; c=p->sclients_.next(c))
        {
            c->dirty();
        }

        p=p->parent_;
    }
}

int pia_server_t::isopen()
{
    return open_.current();
}

void pia_server_t::set_nseq(unsigned long seq)
{
    nseq_ = seq;

    if(nseq_>tseq_)
    {
        tseq_=nseq_;   
        pia_server_t *p = parent_;

        while(p)
        {
            if(tseq_<=p->tseq_)
            {
                break;
            }

            p->tseq_=tseq_;
            p=p->parent_;
        }
    }
}

void pia_server_t::recalc_tseq()
{
    tseq_=0;

    children_t::const_iterator iter;
    const children_t &cc(children_.current());

    for(iter=cc.begin(); iter!=cc.end(); iter++)
    {
        if(iter->second->isvisible())
        {
            if(iter->second->tseq_>tseq_)
            {
                tseq_=iter->second->tseq_;
            }
        }
    }
}

void pia_server_t::childgone(pia_server_t *child)
{
    unsigned long old_tseq = child->tseq_;
    pia_server_t *p = this;

    while(p)
    {
        if(old_tseq!=p->tseq_)
        {
            break;
        }

        old_tseq = p->tseq_;
        p->recalc_tseq();
        p=p->parent_;
    }

    cnode_t *c2;

    for(c2=clients_.head(); c2!=0; c2=clients_.next(c2))
    {
        c2->schedule_tree();
        //c2->dirty();
    }

    dirty();
    visible_children_--;
    server_childgone();
}

void pia_server_t::close0(bool calladj)
{
    if(!isopen())
    {
        return;
    }

    if(fast_)
    {
        delete fast_;
        fast_=0;
    }

    local_->fast_.clear();
    open_.set(false);

    if(parent_)
    {
        parent_->children_.alternate().erase(local_->last_);
        parent_->children_.exchange();
    }

    if(local_->clock_)
    {
        local_->global_->entity_->glue()->cancelclocknotify(local_->clock_);
        local_->clock_=0;
    }

    dropclients();

    children_t::const_iterator iter;

    while((iter=children_.current().begin()) != children_.current().end())
    {
        iter->second->close();
    }

    if(isvisible())
    {
        if(parent_)
        {
            parent_->childgone(this);
        }
    }

    if(calladj)
    {
        server_closed();
    }
}

void pia_server_t::close()
{
    close0(1);
}

pia_server_t::~pia_server_t()
{
    close0(0);
}

void cnode_t::job_rsync(void *c_, const pia_data_t & d)
{
    cnode_t *c = (cnode_t *)c_;
    cnode_t *s;

    pia_mainguard_t guard(c->entity_->glue());

    for(s=c->syncs_.head(); s!=0; s=c->syncs_.next(s))
    {
        if(s->needsync_)
        {
            s->needsync_=false;
            s->schedule_sync();
        }
    }
}

void pia_server_t::sync(bool insync)
{
    if(root_ != this)
    {
        root_->sync(insync);
        return;
    }

    if(local_->global_->insync_ == insync)
    {
        return;
    }

    local_->global_->insync_=insync;

    if(insync)
    {
        cnode_t *c;

        for(c=local_->global_->roots_.head(); c!=0; c=local_->global_->roots_.next(c))
        {
            c->schedule_rsync();
        }
    }
}

void pia_server_t::schedule_setclock()
{
    for(cnode_t *c2=clients_.head(); c2!=0; c2=clients_.next(c2))
    {
        if(c2->client_->plug_flags & PLG_CLIENT_CLOCK)
        {
            c2->schedule_setclock();
        }
    }
}

void pia_server_t::clockgone(void *s_)
{
    pia_server_t *s = (pia_server_t *)s_;

    s->local_->clock_=0;
    s->schedule_setclock();
}

void pia_server_t::setclock(bct_clocksink_t *c)
{
    if(local_->clock_)
    {
        for(cnode_t *c2=clients_.head(); c2!=0; c2=clients_.next(c2))
        {
            if(c2->clock_)
            {
                local_->global_->entity_->glue()->cleardownstreamclock(local_->clock_,c2->clock_);
            }
        }
        local_->global_->entity_->glue()->cancelclocknotify(local_->clock_);
    }

    if(c)
    {
        local_->clock_ = local_->global_->entity_->glue()->addclocknotify(c, clockgone, this);

        for(cnode_t *c2=clients_.head(); c2!=0; c2=clients_.next(c2))
        {
            if(c2->clock_)
            {
                local_->global_->entity_->glue()->setdownstreamclock(local_->clock_,c2->clock_);
            }
        }
    }
    else
    {
        local_->clock_ = 0;
    }

    schedule_setclock();
}

void pia_server_t::dump(const pia_ctx_t &e)
{
    cnode_t *c;

    c=clients_.head();

    while(c)
    {
        if(c->entity_.matches(e))
        {
            pic::logmsg() << "client of " << c->local_->global_->addr_ << " in " << c->entity_->tag();
        }

        c=clients_.next(c);
    }
}

void pia_server_t::kill(const pia_ctx_t &e)
{
    cnode_t *c,*i;

    c=clients_.head();

    while(c)
    {
        i=c; c=clients_.next(c);

        if(i->entity_.matches(e))
        {
            i->close();
        }
    }

    if(!clients_.head())
    {
        server_empty();
    }
}

void pia_server_t::add(const pia_ctx_t &e, bct_client_t *c)
{
    new cnode_t(this,e,0,c);
}

void pia_server_t::change_slow(const pia_data_t &d, unsigned long seq)
{
    if(current_slow_==d && seq==dseq_)
    {
        return;
    }

    current_slow_=d;
    dseq_=seq;

    if(dseq_>tseq_)
    {
        tseq_=dseq_;   
        pia_server_t *p = parent_;

        while(p)
        {
            if(tseq_<=p->tseq_)
            {
                break;
            }

            p->tseq_=tseq_;
            p=p->parent_;
        }
    }

    if(visible_)
        server_changed(d);

    for(cnode_t *c2=clients_.head(); c2!=0; c2=clients_.next(c2))
    {
        c2->schedule_data(d);
        //c2->dirty();
    }

    dirty();
}

void pia_server_t::setvisible(bool visible)
{
    if(visible==visible_)
    {
        return;
    }

    visible_=visible;

    if(visible)
    {
        if(parent_)
        {
            parent_->childadded(this);
        }
        server_changed(current_slow_);
        return;
    }

    dropclients();

    if(parent_)
    {
        parent_->childgone(this);
    }

    if(!clients_.head())
    {
        server_empty();
    }
}

unsigned char pia_server_t::enum_visible(unsigned char name)
{
    const children_t &cc(children_.current());
    children_t::const_iterator iter;

    for(iter=cc.upper_bound(name); iter!=cc.end(); iter++)
    {
        if(iter->first>name && iter->second->isvisible())
        {
            return iter->first;
        }
    }

    return 0;
}

const pia_data_t &pia_server_t::path()
{
    return local_->path_;
}

const pia_data_t &pia_server_t::addr()
{
    return local_->global_->addr_;
}

bool pia_server_t::insync()
{
    return local_->global_->insync_;
}

void pia_server_t::setcookie(unsigned short c)
{
    local_->global_->cookie_ = c;
}

unsigned short pia_server_t::getcookie()
{
    return local_->global_->cookie_;
}

const pia_ctx_t &pia_server_t::entity()
{
    return local_->global_->entity_;
}

void pia_server_t::init(const pia_data_t &data, unsigned long dseq)
{
    nseq_=0;
    tseq_=dseq;
    dseq_=dseq;
    visible_children_=0;
    flags_=0;
    visible_=false;
    current_slow_=data;
    open_.set(true);
}

static int event_rcv__(void *ctx, bct_entity_t e, bct_data_t d, bct_dataqueue_t q)
{
    return ((pia_server_t::fast_receiver_t *)ctx)->evt_received(pia_data_nb_t::from_lent(d),pia_dataqueue_t::from_lent(q))?1:0;
}

static int data_rcv__(void *ctx, bct_entity_t e, bct_data_t d)
{
    return (((pia_server_t::fast_receiver_t *)ctx)->data_received(pia_data_nb_t::from_lent(d)))?1:0;
}

void pia_server_t::set_flags(unsigned flags)
{
    flags_=flags;

    if((flags&PLG_SERVER_FAST)!=0)
    {
        if(!local_->fast_.isvalid())
        {
            local_->fast_ = server_fast();
        }

        if(!fast_)
        {
            unsigned fflags = server_fastflags();
            pia_server_t::fast_receiver_t *r = local_->fast_.ptr();
            fast_ = new pia_fastdata_t(entity(),fflags,event_rcv__,data_rcv__,r);
            if((fflags&PLG_FASTDATA_SENDER)==0)
            {
                PIC_ASSERT(r);
                fast_->fastdata_enable(true,true);
            }
        }
    }
    else
    {
        if(fast_)
        {
            delete fast_;
            fast_=0;
        }

        local_->fast_.clear();
    }
}

pia_server_t::pia_server_t(const pia_data_t &addr, const pia_ctx_t &e, const pia_data_t &data, unsigned long dseq)
{
    local_ = pic::ref(new pia_serverlocal_t(addr,e));
    root_ = this;
    parent_ = 0;
    fast_= 0;

    init(data,dseq);
}

pia_server_t::pia_server_t(pia_server_t *parent, unsigned char last, const pia_data_t &data, unsigned long dseq)
{
    local_ = pic::ref(new pia_serverlocal_t(parent->local_, last));
    parent_ = parent;
    root_ = parent->root_;
    fast_= 0;

    children_t &cc(parent->children_.alternate());

    PIC_ASSERT(cc.find(last)==cc.end());
    cc.insert(std::make_pair(last,this));
    parent->children_.exchange();

    init(data,dseq);

    while(parent)
    {
        if(tseq_<=parent->tseq_)
        {
            break;
        }

        parent->tseq_=tseq_;
        parent=parent->parent_;
    }

}

