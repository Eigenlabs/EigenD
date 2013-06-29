
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

#include "pia_proxy.h"
#include "pia_server.h"
#include "pia_glue.h"
#include "pia_buffer.h"
#include "pia_data.h"
#include "pia_error.h"

#include <string.h>
#include <stdlib.h>

#include <pibelcanto/link.h>

#include <piembedded/pie_message.h>
#include <piembedded/pie_print.h>
#include <piagent/pia_network.h>
#include <picross/pic_log.h>
#include <picross/pic_error.h>
#include <picross/pic_ilist.h>
#include <picross/pic_strbase.h>

#define PNODE_SERVERS 1
#define PNODE_OUTSTANDING 2

#define PIA_TIMER_INTERVAL 1
#define PIA_TIMER_SLOW_TICKS_QUERY 3
#define PIA_TIMER_SLOW_TICKS_CLOSE 15
#define PIA_TIMER_SLOW_TICKS_START 4
#define PIA_TIMER_FAST_TICKS_QUERY 2
#define PIA_TIMER_FAST_TICKS_START 2

#define PIA_OUTSTANDING_TREE 1
#define PIA_OUTSTANDING_DATA 4
#define PIA_OUTSTANDING_FAST_DATA 2
#define PIA_OUTSTANDING_FAST_ID 8

#define PIA_DATAQUEUE_SIZE 12

struct pia_proot_t;
struct pia_pnode_t;

struct pia_proxylist_t::impl_t
{
    pic::ilist_t<pia_proot_t,PNODE_SERVERS> fastservers_;
    pic::ilist_t<pia_proot_t,PNODE_SERVERS> slowservers_;
};

struct pia_pnode_t: pia_server_t, pic::element_t<PNODE_OUTSTANDING>
{
    pia_pnode_t(pia_pnode_t *parent, unsigned char last, unsigned flags, const pia_data_t &data,unsigned long dseq);
    pia_pnode_t(pia_pnode_t *parent, unsigned char last);
    pia_pnode_t(const pia_data_t &addr, const pia_ctx_t &e);
    ~pia_pnode_t();

    static void receive_firstfast(void *n_, const pia_data_t & d);
    static void receive_firstid(void *n_, const pia_data_t & d);
    static void receive_transient_fast(void *n_, const pia_data_nb_t & d);
    static void receive_transient_id(void *n_, const pia_data_nb_t & d);
    static int clear_marker(void *x, unsigned char n, pia_server_t *s);
    static int wipe_marker(void *x, unsigned char n, pia_server_t *s);

    bool setoutstanding(unsigned f);
    void clearoutstanding(unsigned f);
    void transmit_ireq(int original);
    void transmit_freq(int original);
    void transmit_treq(int original);
    void transmit_dreq(int original);
    void retransmit();

    void check(unsigned char path, uint32_t tseq, uint32_t gseq);

    virtual void server_closed();
    void server_empty();
    void server_changed(const pia_data_t &d);
    pic::ref_t<pia_server_t::fast_receiver_t> server_fast() { return pic::ref_t<pia_server_t::fast_receiver_t>(); }
    unsigned server_fastflags() { return PLG_FASTDATA_SENDER; }

    pia_proot_t *root();

    int isoutstanding_;
    int mark_;
    bool got_fast_data_;
    bool got_fast_id_;
    pia_cref_t firstfast_cpoint_;
    pia_cref_t transient_cpoint_;
    pia_dataqueue_t queue_;
    uint32_t last_good_;
};

struct pia_proot_t: pia_buffer_t, pia_pnode_t, pic::element_t<PNODE_SERVERS>
{
    pia_proot_t(pia_proxylist_t::impl_t *list, const pia_data_t &name, const pia_ctx_t &e, bool fast);
    ~pia_proot_t();

    void retransmit();
    int receive_data(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen);
    int receive_fast_data(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen);
    int receive_fast_id(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen);
    int receive_tree(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen);
    void server_closed();

    void buffer_receive_fast(const unsigned char *msg, unsigned len);
    void buffer_fixup_fast(unsigned char *msg, unsigned len);
    void buffer_receive_slow(const unsigned char *msg, unsigned len);
    void buffer_fixup_slow(unsigned char *msg, unsigned len);

    static void job_timer(void *r_);
    void check_sync(bool retransmit);

    pic::ilist_t<pia_pnode_t,PNODE_OUTSTANDING> outstanding_;

    pia_proxylist_t::impl_t *list_;
    bool fast_;

    int slowtick_;
    int fasttick_;

    uint32_t mtseq_, mdseq_, msseq_, mnseq_, mgseq_, mltseq_;

    pia_timerhnd_t job_timer_;
};

pia_proot_t *pia_pnode_t::root()
{
    return static_cast<pia_proot_t *>(pia_server_t::root());
}

bool pia_pnode_t::setoutstanding(unsigned f)
{
    if((isoutstanding_&f)==0)
    {
        root()->outstanding_.append(this);
        isoutstanding_ |= f;
        return true;
    }

    return false;
}

void pia_pnode_t::clearoutstanding(unsigned f)
{
    if(isoutstanding_&f)
    {
        isoutstanding_ &= ~f;

        if(!isoutstanding_)
        {
            root()->outstanding_.remove(this);
        }
    }
}

void pia_pnode_t::transmit_ireq(int original)
{
    if(root()->buffer_enabled())
    {
        pia_data_t p = path();
        unsigned l = pie_stanzalen_req(p.aspathlen(), 0);
        unsigned char *b = root()->buffer_begin_transmit_slow(l);
        if(b)
        {
            pie_setstanza(b,l,BCTMTYPE_IDNT_REQ,(const unsigned char *)p.aspath(),p.aspathlen(), 0);
        }
        root()->buffer_end_transmit_slow();
    }
}

void pia_pnode_t::transmit_freq(int original)
{
    if(root()->buffer_enabled())
    {
        pia_data_t p = path();
        unsigned l = pie_stanzalen_req(p.aspathlen(), 0);
        unsigned char *b = root()->buffer_begin_transmit_slow(l);
        if(b)
        {
            pie_setstanza(b,l,BCTMTYPE_FAST_REQ,(const unsigned char *)p.aspath(),p.aspathlen(), 0);
        }
        root()->buffer_end_transmit_slow();
    }
}

void pia_pnode_t::transmit_treq(int original)
{
    if(original)
    {
        if(!setoutstanding(PIA_OUTSTANDING_TREE))
        {
            return;
        }
    }

    if(root()->buffer_enabled())
    {
        pia_data_t p = path();
        unsigned l = pie_stanzalen_req(p.aspathlen(), 0);
        unsigned char *b = root()->buffer_begin_transmit_slow(l);

        if(b)
        {
            pie_setstanza(b,l,BCTMTYPE_TREE_REQ,(const unsigned char *)p.aspath(),p.aspathlen(), 0);
        }
        root()->buffer_end_transmit_slow();
    }
}

void pia_pnode_t::transmit_dreq(int original)
{
    if(original)
    {
        if(!setoutstanding(PIA_OUTSTANDING_DATA))
        {
            return;
        }
    }

    if(root()->buffer_enabled())
    {
        pia_data_t p = path();
        unsigned l = pie_stanzalen_req(p.aspathlen(), 0);
        unsigned char *b = root()->buffer_begin_transmit_slow(l);
        pie_setstanza(b,l,BCTMTYPE_DATA_REQ,(const unsigned char *)p.aspath(),p.aspathlen(), 0);
        root()->buffer_end_transmit_slow();
    }
}

void pia_pnode_t::retransmit()
{
    if(isoutstanding_&PIA_OUTSTANDING_TREE)
    {
        transmit_treq(0);
    }

    if(isoutstanding_&PIA_OUTSTANDING_DATA)
    {
        transmit_dreq(0);
    }

    if(isoutstanding_&PIA_OUTSTANDING_FAST_DATA)
    {
        got_fast_data_=false;
        transmit_freq(0);
    }

    if(isoutstanding_&PIA_OUTSTANDING_FAST_ID)
    {
        got_fast_id_=false;
        transmit_ireq(0);
    }
}

void pia_proot_t::retransmit()
{
    for(pia_pnode_t *n = outstanding_.head(); n!=0; n=outstanding_.next(n))
    {
        n->retransmit();
    }
}

void pia_pnode_t::receive_firstid(void *n_, const pia_data_t &d)
{
    pia_pnode_t *n = (pia_pnode_t *)n_;
    n->clearoutstanding(PIA_OUTSTANDING_FAST_ID);

    if((n->isoutstanding_&(PIA_OUTSTANDING_FAST_DATA|PIA_OUTSTANDING_FAST_ID))==0)
    {
        n->setvisible(true);
        n->root()->check_sync(false);
    }
}

void pia_pnode_t::receive_firstfast(void *n_, const pia_data_t &d)
{
    pia_pnode_t *n = (pia_pnode_t *)n_;
    n->clearoutstanding(PIA_OUTSTANDING_FAST_DATA);

    if((n->isoutstanding_&(PIA_OUTSTANDING_FAST_DATA|PIA_OUTSTANDING_FAST_ID))==0)
    {
        n->setvisible(true);
        n->root()->check_sync(false);
    }
}

void pia_pnode_t::receive_transient_id(void *n_, const pia_data_nb_t &d)
{
    pia_pnode_t *n = (pia_pnode_t *)n_;
    if(!d.is_null())
    {
        if(!n->queue_.isvalid())
        {
            n->queue_ = n->glue()->allocate_dataqueue(PIA_DATAQUEUE_SIZE);
        }
        n->fastdata()->fastdata_send(d,n->queue_);
    }
    else
    {
        n->queue_ = pia_dataqueue_t();
        n->fastdata()->fastdata_send(d,n->queue_);
    }
}

void pia_pnode_t::receive_transient_fast(void *n_, const pia_data_nb_t &d)
{
    pia_pnode_t *n = (pia_pnode_t *)n_;
    if(!d.is_null())
    {
        if(!n->queue_.isvalid())
        {
            n->queue_ = n->glue()->allocate_dataqueue(PIA_DATAQUEUE_SIZE);
        }
        n->queue_.write(d);
    }
}

pia_proot_t::~pia_proot_t()
{
}

pia_pnode_t::~pia_pnode_t()
{
}

void pia_proot_t::server_closed()
{
    buffer_disable();

    if(fast_)
        list_->fastservers_.remove(this);
    else
        list_->slowservers_.remove(this);

    firstfast_cpoint_->disable();
    transient_cpoint_->disable();
    job_timer_.cancel();
    clearoutstanding(PIA_OUTSTANDING_DATA|PIA_OUTSTANDING_TREE);

    delete this;
}

void pia_pnode_t::server_closed()
{
    firstfast_cpoint_->disable();
    transient_cpoint_->disable();
    clearoutstanding(PIA_OUTSTANDING_DATA|PIA_OUTSTANDING_TREE);

    delete this;
}

void pia_pnode_t::server_empty()
{
    if(isroot())
    {
        close();
    }
}

int pia_proot_t::receive_data(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen)
{
    pia_pnode_t *s;
    int x,y;
    unsigned df;
    unsigned short dl;
    const unsigned char *dp;
    pia_server_t *good;
    unsigned oflags,nflags;
    uint32_t dseq,nseq,tseq;

    if((y=pie_getevthdr(msg,msglen,&dseq,&nseq,&tseq))<0)
    {
        return -1;
    }

    if((x=pie_getdata(msg+y,msglen-y,&df,&dl,&dp))<0)
    {
        return -1;
    }

    x=x+y;

    pia_data_t d = glue()->allocate_wire(PIC_ALLOC_NORMAL,dl,dp);
    nflags=df;

    if(!fast_)
    {
        nflags &= ~(PLG_SERVER_FAST);
    }

    if((s=(pia_pnode_t *)find(name, namelen, &good)))
    {
        s->clearoutstanding(PIA_OUTSTANDING_DATA);
        s->change_slow(d,dseq);
        oflags=s->flags();
    }
    else
    {
        if(good->path().aspathlen() != namelen-1)
        {
            ((pia_pnode_t *)good)->transmit_treq(1);
            return x;
        }

        oflags=nflags&~(PLG_SERVER_FAST);
        s=new pia_pnode_t((pia_pnode_t *)good,name[namelen-1],oflags,d,dseq);
        s->setoutstanding(PIA_OUTSTANDING_TREE);
        s->clearoutstanding(PIA_OUTSTANDING_DATA);
    }

    if(oflags != nflags)
    {
        s->set_flags(nflags);
    }

    if((oflags&~PLG_SERVER_RO)!=(nflags&~PLG_SERVER_RO))
    {
        s->setvisible(false);
        bool v = true;

        if((nflags&PLG_SERVER_FAST))
        {
            s->setoutstanding(PIA_OUTSTANDING_FAST_DATA);
            s->setoutstanding(PIA_OUTSTANDING_FAST_ID);
            s->transmit_freq(1);
            s->transmit_ireq(1);
            s->got_fast_id_=false;
            s->got_fast_data_=false;
            v = false;
        }

        if(v)
        {
            s->clearoutstanding(PIA_OUTSTANDING_FAST_DATA);
            s->clearoutstanding(PIA_OUTSTANDING_FAST_ID);
            s->setvisible(true);
        }
    }

    return x;
}

void pia_pnode_t::check(unsigned char last, uint32_t tseq, uint32_t gseq)
{
    pia_pnode_t *c;
    pia_server_t *cb;

    cb=find(&last,1, 0);

    if(cb)
    {
        c=(pia_pnode_t *)cb;
        c->mark_=1;

        if(tseq != c->tseq() || c->tseq() > gseq )
        {
            c->transmit_dreq(1);
            c->transmit_treq(1);
        }

        if(!c->isvisible())
        {
            c->transmit_freq(1);
            c->transmit_ireq(1);
        }

        return;
    }

    c = new pia_pnode_t(this,last);
    c->transmit_dreq(1);
    c->transmit_treq(1);
}

int pia_pnode_t::clear_marker(void *x, unsigned char n, pia_server_t *s)
{
    pia_pnode_t *p = (pia_pnode_t *)s;
    p->mark_=0;
    return 0;
}

int pia_pnode_t::wipe_marker(void *x, unsigned char n, pia_server_t *s)
{
    pia_pnode_t *p = (pia_pnode_t *)s;
    return p->mark_==0;
}

int pia_proot_t::receive_tree(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen)
{
    int x,used;
    unsigned char path;
    uint32_t dseq,tseq,nseq,tseq2;
    pia_pnode_t *n;
    pia_server_t *s;

    if(!(s=find(name, namelen, 0)))
    {
        return pie_skipstanza(msg,msglen, BCTMTYPE_TREE_EVT);
    }

    n=(pia_pnode_t *)s;

    n->sync(false);
    n->visit(0,clear_marker);

    used=0;

    if((x=pie_getevthdr(msg,msglen,&dseq,&nseq,&tseq))<0)
    {
        return -1;
    }

    msg+=x; msglen-=x; used+=x;

    for(;;)
    {
        if((x=pie_gettevtpath(msg,msglen,&path,&tseq2))<0)
        {
            return -1;
        }

        msg+=x; msglen-=x; used+=x;
        
        if(!path) break;

        n->check(path,tseq2,mgseq_);
    }

    n->filter(0,wipe_marker);
    n->set_nseq(nseq);
    n->clearoutstanding(PIA_OUTSTANDING_TREE);

    return used;
}

void pia_proot_t::buffer_receive_slow(const unsigned char *msg, unsigned len)
{
    int x;
    unsigned bt,pl;
    const unsigned char *p;
    unsigned short cookie;
    uint32_t msseq,mdseq,mnseq,mtseq,ltseq;

    if((x=pie_getheader(msg,len,&cookie,&msseq,&mdseq,&mnseq,&mtseq,&ltseq))<0 || cookie==0)
    {
        return;
    }

    msg+=x; len-=x;

    if(getcookie())
    {
        if(getcookie() != cookie)
        {
            pic::logmsg() << addr() << " new server cookie";
            close();
            return;
        }
    }
    else
    {
        setcookie(cookie);
    }

    if(!len)
    {
        pic::logmsg() << addr() << " exited";
        close();
        return;
    }

    slowtick_=0;

    while((x=pie_getstanza(msg,len,&bt,&p,&pl))>0)
    {
        msg+=x; len-=x;

        switch(bt)
        {
            case BCTMTYPE_TREE_EVT:
            {
                if((x=receive_tree(p,pl,msg,len))<0)
                {
                    return;
                }

                msg+=x; len-=x;
                continue;
            }

            case BCTMTYPE_DATA_EVT:
            {
                if((x=receive_data(p,pl,msg,len))<0)
                {
                    return;
                }

                msg+=x; len-=x;
                continue;
            }

            case BCTMTYPE_IDNT_EVT:
            {
                if((x=receive_fast_id(p,pl,msg,len))<0)
                {
                    return;
                }

                msg+=x; len-=x;
                continue;
            }

            case BCTMTYPE_FAST_EVT:
            {
                if((x=receive_fast_data(p,pl,msg,len))<0)
                {
                    return;
                }

                msg+=x; len-=x;
                continue;
            }

            case BCTMTYPE_FAST_REQ:
            {
                fasttick_=0;
            }
            // fallthrough

            case BCTMTYPE_IDNT_REQ:
            {
                fasttick_=0;
            }
            // fallthrough
        }

        if((x=pie_skipstanza(msg,len,bt))<0)
        {
            return;
        }

        msg+=x; len-=x;
    }

    msseq_ = msseq;
    mdseq_ = mdseq;
    mnseq_ = mnseq;
    mtseq_ = mtseq;

    if(ltseq != mltseq_)
    {
        if(mltseq_)
        {
            pic::logmsg() << addr() << " missed packet " << mltseq_ << ' ' << ltseq;
        }

        transmit_treq(1);
    }
    else
    {
        if(ltseq == mgseq_)
        {
            mgseq_ = mtseq_;
        }

        check_sync(false);
    }

    mltseq_ = mtseq_;
}

void pia_proot_t::check_sync(bool rt)
{
    if(!outstanding_.head())
    {
        int flag=1;

        if((mtseq_ != tseq()) || (mnseq_ != nseq()))
        {
            if(rt)
            {
                transmit_treq(1);
            }

            flag=0;
        }

        if(mdseq_ != dseq())
        {
            if(rt) transmit_dreq(1);
            flag=0;
        }

        if(msseq_ != tseq())
        {
            flag=0;
        }

        if(flag)
        {
            mgseq_ = mtseq_;
            sync(true);
        }
    }
}

void pia_proot_t::buffer_fixup_fast(unsigned char *msg, unsigned len)
{
}

int pia_proot_t::receive_fast_id(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen)
{
    int x;
    unsigned df;
    unsigned short dl;
    const unsigned char *dp;

    if((x=pie_getdata(msg,msglen,&df,&dl,&dp))<0)
    {
        return -1;
    }

    pia_server_t *g;
    pia_pnode_t *n = (pia_pnode_t *)find(name,namelen,&g);

    if(n && n->isfast())
    {
        n->glue()->fastq()->idle(n->transient_cpoint_,receive_transient_id, n, glue()->allocate_wire_nb(dl,dp));

        if(!n->got_fast_id_)
        {
            n->got_fast_id_=true;
            n->glue()->mainq()->idle(n->firstfast_cpoint_,receive_firstid, n, glue()->allocate_wire(PIC_ALLOC_NB, dl,dp));
        }
    }

    return x;
}

int pia_proot_t::receive_fast_data(const unsigned char *name, unsigned namelen, const unsigned char *msg, unsigned msglen)
{
    int x;
    unsigned df;
    unsigned short dl;
    const unsigned char *dp;

    if((x=pie_getdata(msg,msglen,&df,&dl,&dp))<0)
    {
        return -1;
    }

    pia_server_t *g;
    pia_pnode_t *n = (pia_pnode_t *)find(name,namelen,&g);

    if(n && n->isfast())
    {
        n->glue()->fastq()->idle(n->transient_cpoint_,receive_transient_fast, n, glue()->allocate_wire_nb(dl,dp));

        if(!n->got_fast_data_)
        {
            n->got_fast_data_=true;
            n->glue()->mainq()->idle(n->firstfast_cpoint_,receive_firstfast, n, glue()->allocate_wire(PIC_ALLOC_NB, dl,dp));
        }
    }

    return x;
}

void pia_proot_t::buffer_receive_fast(const unsigned char *msg, unsigned len)
{
    int x;
    unsigned bt,pl;
    const unsigned char *p;
    unsigned short cookie;
    uint32_t sseq,tseq,dseq,nseq,tseq2;

    if((x=pie_getheader(msg,len,&cookie,&sseq,&dseq,&nseq,&tseq,&tseq2))<0 || cookie==0)
    {
        return;
    }

    msg+=x; len-=x;

    if(getcookie() != cookie)
    {
        return;
    }

    while((x=pie_getstanza(msg,len,&bt,&p,&pl))>0)
    {
        msg+=x; len-=x;

        switch(bt)
        {
            case BCTMTYPE_FAST_EVT:
                if((x=receive_fast_data(p,pl,msg,len))<0)
                {
                    return;
                }

                msg+=x; len-=x;
                continue;

            case BCTMTYPE_IDNT_EVT:
                if((x=receive_fast_id(p,pl,msg,len))<0)
                {
                    return;
                }

                msg+=x; len-=x;
                continue;
        }

        if((x=pie_skipstanza(msg,len,bt))<0)
        {
            return;
        }

        msg+=x; len-=x;
    }

}

void pia_pnode_t::server_changed(const pia_data_t &d)
{
    sync(false);
}

void pia_proot_t::buffer_fixup_slow(unsigned char *buffer, unsigned len)
{
    pie_setheader(buffer,len,0,0,0,0,0,0);
}

void pia_proot_t::job_timer(void *r_)
{
    pia_proot_t *r = (pia_proot_t *)r_;

    r->fasttick_++;

    if(r->fast_)
    {
        if(r->fasttick_>PIA_TIMER_FAST_TICKS_QUERY)
        {
            r->fasttick_=0;
            r->transmit_freq(1);
        }
    }

    r->slowtick_++;

    if(r->slowtick_>=PIA_TIMER_SLOW_TICKS_CLOSE)
    {
        pic::logmsg() << r->addr() << " proxy closing down";
        r->close();
        return;
    }

    if(r->slowtick_>=PIA_TIMER_SLOW_TICKS_QUERY)
    {
        r->transmit_treq(0);
    }

    if(r->outstanding_.head())
    {
        r->retransmit();
    }
    else
    {
        r->check_sync(true);
    }
}

pia_proxylist_t::pia_proxylist_t()
{
    impl_ = new impl_t;
}

pia_proxylist_t::~pia_proxylist_t()
{
    pia_proot_t *r;

    while((r=impl_->fastservers_.head())!=0)
    {
        r->close();
    }

    while((r=impl_->slowservers_.head())!=0)
    {
        r->close();
    }

    delete impl_;
}

void pia_proxylist_t::dump(const pia_ctx_t &e)
{
    pia_proot_t *r;

    for(r=impl_->fastservers_.head(); r!=0; r=impl_->fastservers_.next(r))
    {
        r->dump(e);
    }

    for(r=impl_->slowservers_.head(); r!=0; r=impl_->slowservers_.next(r))
    {
        r->dump(e);
    }
}

void pia_proxylist_t::kill(const pia_ctx_t &e)
{
    pia_proot_t *r,*n;

    r=impl_->fastservers_.head();

    while(r!=0)
    {
        n=impl_->fastservers_.next(r);
        r->kill(e);
        r=n;
    }

    r=impl_->slowservers_.head();

    while(r!=0)
    {
        n=impl_->slowservers_.next(r);
        r->kill(e);
        r=n;
    }
}

void pia_proxylist_t::killserver(const pia_data_t &name)
{
    pia_proot_t *r;

    for(r=impl_->fastservers_.head(); r!=0; r=impl_->fastservers_.next(r))
    {
        if(r->addr() == name)
        {
            r->close();
            break;
        }
    }

    for(r=impl_->slowservers_.head(); r!=0; r=impl_->slowservers_.next(r))
    {
        if(r->addr() == name)
        {
            r->close();
            break;
        }
    }
}

void pia_proxylist_t::client(const pia_data_t &name, const pia_ctx_t &e, bct_client_t *c, bool fast)
{
    pia_proot_t *r;

    if(fast)
    {
        for(r=impl_->fastservers_.head(); r!=0; r=impl_->fastservers_.next(r))
        {
            if(r->addr()==name)
            {
                r->add(e,c);
                return;
            }
        }
    }
    else
    {
        for(r=impl_->slowservers_.head(); r!=0; r=impl_->slowservers_.next(r))
        {
            if(r->addr()==name)
            {
                r->add(e,c);
                return;
            }
        }
    }

    pia_ctx_t e2(e->glue(),e->scope().asstring(),0,pic::status_t(),pic::f_string_t(), "proxy");
    pia_pnode_t *n = new pia_proot_t(impl_,name,e2,fast);
    n->add(e,c);
}

pia_pnode_t::pia_pnode_t(pia_pnode_t *p, unsigned char last): pia_server_t(p,last,pia_data_t(),0)
{
    firstfast_cpoint_=pia_make_cpoint();
    transient_cpoint_=pia_make_sync_cpoint();
    mark_=1;
    isoutstanding_=0;
    got_fast_data_=false;
    got_fast_id_=false;
    sync(false);
    set_flags(~0);
    setvisible(false);
}

pia_pnode_t::pia_pnode_t(pia_pnode_t *p, unsigned char last, unsigned f, const pia_data_t &data, unsigned long dseq): pia_server_t(p,last,data,dseq)
{
    firstfast_cpoint_=pia_make_cpoint();
    transient_cpoint_=pia_make_sync_cpoint();
    mark_=1;
    isoutstanding_=0;
    got_fast_data_=false;
    got_fast_id_=false;
    sync(false);
    set_flags(f);
    setvisible((flags()&(PLG_SERVER_FAST))==0);
}

pia_pnode_t::pia_pnode_t(const pia_data_t &addr, const pia_ctx_t &e): pia_server_t(addr,e,pia_data_t(),0)
{
    firstfast_cpoint_=pia_make_cpoint();
    transient_cpoint_=pia_make_sync_cpoint();
    mark_=1;
    isoutstanding_=0;
    got_fast_data_=false;
    got_fast_id_=false;
    sync(false);
    set_flags(PLG_SERVER_RO);
    setvisible(false);
}

pia_proot_t::pia_proot_t(pia_proxylist_t::impl_t *l, const pia_data_t &a, const pia_ctx_t &e, bool fast): pia_buffer_t(e->glue(),a,pie_headerlen()), pia_pnode_t(a, e), job_timer_(e->glue())
{
    list_=l;
    slowtick_=PIA_TIMER_SLOW_TICKS_START;
    fasttick_=PIA_TIMER_FAST_TICKS_START;
    mgseq_=0;
    mltseq_=0;

    fast_=fast;

    job_timer_.start(job_timer,this,PIA_TIMER_INTERVAL);

    if(fast_)
        l->fastservers_.append(this);
    else
        l->slowservers_.append(this);

    setvisible(true);
    transmit_treq(1);
    transmit_dreq(1);

    buffer_enable();
}
