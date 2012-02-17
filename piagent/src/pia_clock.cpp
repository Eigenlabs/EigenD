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

#include "pia_glue.h"
#include "pia_error.h"

#include <picross/pic_strbase.h>
#include <picross/pic_error.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_ilist.h>
#include <picross/pic_stl.h>

#include <list>
#include <memory>

#define SRC_INTL_SAMPLE_RATE 48000
#define SRC_INTL_PERIOD      (PLG_CLOCK_BUFFER_SIZE_DEFAULT*1000)/SRC_INTL_SAMPLE_RATE /* millis */
#define CLOCK_DEBUG 0

struct sink_t;
struct domain_t;
struct source_t;
struct notifier_t;

typedef std::list<sink_t *> sinklist_t;
typedef std::list<domain_t *> domainlist_t;
typedef std::list<source_t *> sourcelist_t;
typedef std::list<notifier_t *> notifierlist_t;

typedef pia_clocklist_t::impl_t clockimpl_t;

struct masterlist_t: virtual public pic::lckobject_t
{
    masterlist_t() { list_.reserve(256); }
    masterlist_t(const masterlist_t &m): list_(m.list_) {}

    //pic::lcklist_t<sink_t *>::lcktype list_;
    pic::lckvector_t<sink_t *>::lcktype list_;
    typedef pic::lckvector_t<sink_t *>::lcktype::const_iterator iter_t;
};

struct sink_t: virtual public pic::lckobject_t
{
    sink_t(bct_clocksink_t *c, domain_t *d, const pia_ctx_t &e);

    void ticked(unsigned long long f, unsigned long long t);
    bool add_upstream(sink_t *up);
    void remove_upstream(sink_t *up);
    void dfs(source_t *);
    bool is_upstream(sink_t *);

    void *add_notify(void (*cb)(void *), void *arg);
    void cancel_notify(notifier_t *n);

    void detach(bool notify, bool rebuild);

    static void detach_callback(void *t_, const pia_data_t & d);
    static int api_add_upstream(bct_clocksink_host_ops_t **hops, bct_clocksink_t *up_);
    static int api_remove_upstream(bct_clocksink_host_ops_t **hops, bct_clocksink_t *up_);
    static int api_enable(bct_clocksink_host_ops_t **hops, int f, int s);
    static int api_suppress(bct_clocksink_host_ops_t **hops, int f);
    static void api_close(bct_clocksink_host_ops_t **hops);
    static void api_tick(bct_clocksink_host_ops_t **hops);
    static void api_tick2(bct_clocksink_host_ops_t **hops,void (*cb)(unsigned long long,unsigned long long,void *),void *);
    static unsigned long long api_current_tick(bct_clocksink_host_ops_t **hops);
    static bct_clocksink_host_ops_t dispatch__;
    static unsigned api_buffer_size(bct_clocksink_host_ops_t **hops);
    static unsigned long  api_sample_rate(bct_clocksink_host_ops_t **hops);

    pia_job_t detach_;
    bool attached_;
    bct_clocksink_t *client_;
    bct_clocksink_host_ops_t *ops_;

    notifierlist_t notifiers_;
    sinklist_t up_;
    sinklist_t down_;
    domain_t *domain_;
    bool mark_;
    bool enabled_;
    const char *name_;
    int suppressed_;
    pia_ctx_t env_;
};

struct domain_t: virtual public pic::lckobject_t
{
    domain_t(clockimpl_t *l, const pia_ctx_t &e, bct_clockdomain_t *c);

    void set_source(const pia_data_t &);
    void enroll(bct_clocksink_t *s,const char *n);
    void drop(sink_t *s, bool);

    void detach(bool notify);
    bool kill(const pia_ctx_t &e, bool notify);
    void dump(const pia_ctx_t &e);
    void changed();

    static void detach_callback(void *t_, const pia_data_t & d);
    static void changed_callback(void *t_, const pia_data_t & d);
    static int api_set_source(bct_clockdomain_host_ops_t **hops, bct_data_t n);
    static bct_data_t api_source_name(bct_clockdomain_host_ops_t **hops);
    static unsigned api_buffer_size(bct_clockdomain_host_ops_t **hops);
    static unsigned long  api_sample_rate(bct_clockdomain_host_ops_t **hops);
    static int api_clocksink(bct_clockdomain_host_ops_t **hops, bct_clocksink_t *s,const char *n);
    static void api_close(bct_clockdomain_host_ops_t **hops);
    static bct_clockdomain_host_ops_t dispatch__;

    pia_ctx_t env_;
    pia_job_t detach_;
    pia_job_t changed_;
    bool attached_;
    bct_clockdomain_t *client_;
    bct_clockdomain_host_ops_t *ops_;

    sinklist_t members_;
    clockimpl_t *clocklist_;
    source_t *source_;
    bool iso_;
};


struct source_t: virtual public pic::lckobject_t
{
    source_t(clockimpl_t *l, const pia_data_t &n, unsigned bs, unsigned long sr, const pia_ctx_t &e, bct_clocksource_t *c);

    void tick(unsigned long long t);
    void build();
    void set_details(unsigned bs, unsigned long sr);

    void detach(bool notify);
    bool kill(const pia_ctx_t &e, bool notify);
    void dump(const pia_ctx_t &e);

    static void detach_callback(void *t_, const pia_data_t & d);
    static void api_tick(bct_clocksource_host_ops_t **hops, unsigned long long bt);
    static void api_set_details(bct_clocksource_host_ops_t **hops, unsigned bs, unsigned long sr);
    static void api_close(bct_clocksource_host_ops_t **hops);
    static bct_clocksource_host_ops_t dispatch__;

    pia_ctx_t env_;
    pia_job_t detach_;
    bool attached_;
    bct_clocksource_t *client_;
    bct_clocksource_host_ops_t *ops_;

    clockimpl_t *clocklist_;
    pia_data_t name_;
    unsigned buf_size_;
    unsigned long sample_rate_;
    pic::flipflop_t<masterlist_t> ticks_;
    sink_t *head_;
    unsigned long long last_tick_;
    unsigned long long this_tick_;
};


struct notifier_t
{
    notifier_t(sink_t *s, void (*cb)(void *), void *arg) : sink_(s), cb_(cb), arg_(arg) {}
    bct_clocksink_t *client() { return sink_->client_; }
    void notify() { cb_(arg_); }
    void cancel() { sink_->cancel_notify(this); }

    sink_t *sink_;
    void (*cb_)(void *);
    void *arg_;
};

struct pia_clocklist_t::impl_t
{
    impl_t(pia::manager_t::impl_t *);
    ~impl_t();

    source_t *find_source(const pia_data_t &n);
    void kill(const pia_ctx_t &e, bool notify);
    void dump(const pia_ctx_t &e);
    void sources_dump(const pia_ctx_t &e);
    void domains_dump(const pia_ctx_t &e);
    void sources_kill(const pia_ctx_t &e, bool notify);
    void domains_kill(const pia_ctx_t &e, bool notify);
    void reset_default(source_t *s);
    void build();
    void clearmarks();

    static void internal_tick_fast(void *g_);

    pia::manager_t::impl_t *glue_;
    source_t internalsource_;
    source_t *defaultsource_;
    sourcelist_t sources_;
    domainlist_t domains_;
    pia_cref_t internal_timer_;
};

std::ostream& operator<<(std::ostream &o,sink_t *s)
{
    o << s->name_ << "(" << s->env_->tag() << ")";
    return o;
}

sink_t::sink_t(bct_clocksink_t *c, domain_t *d,const pia_ctx_t &e) : attached_(false), client_(c), ops_(&dispatch__), domain_(d), mark_(false), enabled_(false), suppressed_(0), env_(e)
{
    if(client_)
    {
        attached_ = true;
        client_->host_ops = &ops_;
        client_->plg_state = PLG_STATE_OPENED;
    }
}

void sink_t::ticked(unsigned long long f, unsigned long long t)
{
    if(attached_)
    {
        bct_clocksink_plug_ticked(client_, env_->api(), f, t);
    }
}

bool sink_t::add_upstream(sink_t *up)
{
    if(!attached_)
    {
        return false;
    }

    domain_->clocklist_->clearmarks();

    if(up->is_upstream(this))
    {
        return false;
    }

    if(up->domain_->iso_ && up->domain_->source_!=domain_->source_)
    {
        return false;
    }

    up_.push_front(up);
    up->down_.push_front(this);
    domain_->clocklist_->build();

    return true;
}

void sink_t::remove_upstream(sink_t *up)
{
    if(!attached_)
    {
        return;
    }

    up_.remove(up);
    up->down_.remove(this);
    domain_->clocklist_->build();
}

void sink_t::dfs(source_t *source)
{
    mark_ = true;

    for(sinklist_t::iterator i=up_.begin(); i!=up_.end(); ++i)
    {
        sink_t *s = *i;
        if(!s->mark_)
        {
            s->dfs(source);
        }
    }

    if(attached_ && enabled_)
    {
        source->ticks_.alternate().list_.push_back(this);
    }
}

bool sink_t::is_upstream(sink_t *s)
{
    mark_ = true;

    if(!attached_)
    {
        return false;
    }

    for(sinklist_t::iterator i=up_.begin(); i!=up_.end(); ++i)
    {
        sink_t *up=*i;

        if(up==s)
        {
            return true;
        }

        if(up->mark_)
        {
            continue;
        }

        if(up->is_upstream(s))
        {
            return true;
        }
    }

    return false;
}

void *sink_t::add_notify(void (*cb)(void *), void *arg)
{
    if(!attached_)
    {
        return 0;
    }

    notifier_t *n = new notifier_t(this,cb,arg);
    std::auto_ptr<notifier_t> np(n);
    notifiers_.push_front(n);
    np.release();
    return (void *)n;
}

void sink_t::cancel_notify(notifier_t *n)
{
    if(!attached_)
    {
        return;
    }

    notifiers_.remove(n);
    delete n;
}

void sink_t::detach(bool notify, bool rebuild)
{
    detach_.cancel();

    if(attached_)
    {
        attached_ = false;

        sinklist_t::iterator i=up_.begin();
        sinklist_t::iterator e=up_.end();
        for(; i!=e; ++i)
        {
            (*i)->down_.remove(this);
        }

        i=down_.begin();
        e=down_.end();
        for(; i!=e; ++i)
        {
            (*i)->up_.remove(this);
        }

        domain_->drop(this, rebuild);
    }

    client_->plg_state = PLG_STATE_DETACHED;

    if(notify)
    {
        detach_.idle(env_->appq(), detach_callback, this, pia_data_t());
    }

    notifierlist_t::iterator n;
    while((n=notifiers_.begin())!=notifiers_.end())
    {
        (*n)->notify();
        delete *n;
        notifiers_.erase(n);
    }
}

void sink_t::detach_callback(void *s_, const pia_data_t & _)
{
    sink_t *s = (sink_t *)s_;
    bct_clocksink_plug_closed(s->client_, s->env_->api());
}

domain_t::domain_t(clockimpl_t *l, const pia_ctx_t &e, bct_clockdomain_t *c) : env_(e), attached_(true), client_(c), ops_(&dispatch__), clocklist_(l), iso_(true)
{
    source_ = clocklist_->defaultsource_;
    if(client_)
    {
        client_->host_ops = &ops_;
        client_->plg_state = PLG_STATE_OPENED;
    }
}

void domain_t::set_source(const pia_data_t &name)
{
    if(!attached_)
    {
        return;
    }

    source_t *s;

    if(name.type()==BCTVTYPE_STRING && !strcmp(name.asstring(),"*"))
    {
        s = clocklist_->defaultsource_;
        iso_ = false;
    }
    else
    {
        s = clocklist_->find_source(name);
        if(!s)
        {
            pic::hurlmsg() << "can't find " << name;
        }
        iso_ = true;
    }

    if(s==source_)
    {
        return;
    }

    source_ = s;
    clocklist_->build();
    changed();
}

void domain_t::changed()
{
    changed_.idle(env_->appq(), changed_callback, this, pia_data_t());
}

void domain_t::enroll(bct_clocksink_t *bc,const char *n)
{
    if(!attached_)
    {
        return;
    }

    sink_t *s = new sink_t(bc, this, env_);
    s->name_ = n;
    std::auto_ptr<sink_t> sp(s);
    members_.push_back(s);
    sp.release();
    clocklist_->build();
}

void domain_t::drop(sink_t *s, bool rebuild)
{
    members_.remove(s);

    if(rebuild)
    {
        clocklist_->build();
    }
}

void domain_t::detach(bool notify)
{
    detach_.cancel();

    if(!attached_)
    {
        return;
    }

    attached_ = false;
    clocklist_->domains_.remove(this);

    sinklist_t::iterator m;
    while((m=members_.begin())!=members_.end())
    {
        (*m)->detach(true,false);
    }

    clocklist_->build();
    client_->plg_state = PLG_STATE_DETACHED;

    if(notify)
    {
        detach_.idle(env_->appq(), detach_callback, this, pia_data_t());
    }
}

void domain_t::dump(const pia_ctx_t &e)
{
    if(env_.matches(e))
    {
        pic::logmsg() << "clock domain " << env_->tag();
    }
}

bool domain_t::kill(const pia_ctx_t &e, bool notify)
{
    if(env_.matches(e))
    {
        detach(notify);
        return true;
    }
    return false;
}

void domain_t::detach_callback(void *s_, const pia_data_t & _)
{
    domain_t *s = (domain_t *)s_;
    bct_clockdomain_plug_closed(s->client_, s->env_->api());
}

void domain_t::changed_callback(void *s_, const pia_data_t &_)
{
    domain_t *s = (domain_t *)s_;
    bct_clockdomain_plug_source_changed(s->client_, s->env_->api());
}

source_t::source_t(clockimpl_t *l, const pia_data_t &n, unsigned bs, unsigned long sr, const pia_ctx_t &e, bct_clocksource_t *c) :
env_(e), attached_(true), client_(c), ops_(&dispatch__), clocklist_(l), name_(n), buf_size_(bs), sample_rate_(sr), last_tick_(0), this_tick_(0)
{
    if(client_)
    {
        client_->host_ops = &ops_;
        client_->plg_state = PLG_STATE_OPENED;
    }
}

void source_t::tick(unsigned long long time)
{
    if(!attached_)
    {
        return;
    }

    if(!last_tick_)
    {
        last_tick_ = time;
        return;
    }

    if(!this_tick_)
    {
        this_tick_ = time;
        return;
    }

    this_tick_ = time;

    pic::flipflop_t<masterlist_t>::guard_t g(ticks_);
    sink_t *s;

    masterlist_t::iter_t si=g.value().list_.begin();
    masterlist_t::iter_t se=g.value().list_.end();

    for(; si!=se; ++si)
    {
        s=const_cast<sink_t *>(*si);

        if(!s->suppressed_)
        {
            s->ticked(last_tick_, this_tick_);
        }
    }

    last_tick_ = time;
}

void source_t::build()
{
    if(!attached_)
    {
        return;
    }

    ticks_.alternate().list_.clear();

    domainlist_t::iterator di=clocklist_->domains_.begin();
    domainlist_t::iterator de=clocklist_->domains_.end();

    sink_t bottom(0,0,env_);

    for(; di!=de; ++di)
    {
        domain_t *d=*di;
        if(d->source_==this)
        {
            sinklist_t::iterator si=d->members_.begin();
            sinklist_t::iterator se=d->members_.end();

            for(; si!=se; ++si)
            {
                sink_t *s = *si;
                if(s->down_.empty())
                {
                    bottom.up_.push_back(s);
                }
            }
        }
    }

    bottom.dfs(this);

#if CLOCK_DEBUG
    pic::msg_t m;
    int c=0;
    m << "clocksource " << name_ << ": order is \n";
    for(masterlist_t::iter_t si = ticks_.alternate().list_.begin(); si != ticks_.alternate().list_.end(); ++si)
    {
         m << (c++) << ": " << (*si) << "\n";
    }
    m << pic::log;
#endif

    ticks_.exchange();
}

void source_t::set_details(unsigned bs, unsigned long sr)
{
    buf_size_ = bs;
    sample_rate_ = sr;

    domainlist_t::iterator di=clocklist_->domains_.begin();
    domainlist_t::iterator de=clocklist_->domains_.end();

    for(; di!=de; ++di)
    {
        domain_t *d=*di;
        if(d->source_==this)
        {
            d->changed();
        }
    }
}

void source_t::detach(bool notify)
{
    detach_.cancel();

    if(!attached_)
    {
        return;
    }

    attached_ = false;

    clocklist_->sources_.remove(this);

    domainlist_t::iterator di=clocklist_->domains_.begin();
    domainlist_t::iterator de=clocklist_->domains_.end();

    for(; di!=de; ++di)
    {
        domain_t *d=*di;
        if(d->source_==this)
        {
            d->source_ = &clocklist_->internalsource_;
        }
    }

    if(clocklist_->defaultsource_==this)
    {
        clocklist_->defaultsource_=&clocklist_->internalsource_;
    }

    clocklist_->build();
    client_->plg_state = PLG_STATE_DETACHED;

    if(notify)
    {
        detach_.idle(env_->appq(), detach_callback, this, pia_data_t());
    }
}

void source_t::dump(const pia_ctx_t &e)
{
    if(env_.matches(e))
    {
        pic::logmsg() << "clock source " << env_->tag();
    }
}

bool source_t::kill(const pia_ctx_t &e, bool notify)
{
    if(env_.matches(e))
    {
        detach(notify);
        return true;
    }
    return false;
}

void source_t::detach_callback(void *s_, const pia_data_t & _)
{
    source_t *s = (source_t *)s_;
    bct_clocksource_plug_closed(s->client_, s->env_->api());
}

pia_clocklist_t::impl_t::impl_t(pia::manager_t::impl_t *g):
    glue_(g),
    internalsource_(this, glue_->allocate_cstring("internal"), PLG_CLOCK_BUFFER_SIZE, SRC_INTL_SAMPLE_RATE, 0, 0),
    defaultsource_(&internalsource_)

{
    sources_.push_back(&internalsource_);
    internal_timer_=pia_make_sync_cpoint();
    g->fastq()->timer(internal_timer_,internal_tick_fast, this, SRC_INTL_PERIOD);
}

pia_clocklist_t::impl_t::~impl_t()
{
    internal_timer_->disable();
    kill(0,false);
}

pia_clocklist_t::~pia_clocklist_t()
{
    delete impl_;
}

void pia_clocklist_t::add_source(const pia_data_t &n, unsigned bs, unsigned long sr, const pia_ctx_t &e, bct_clocksource_t *c)
{
    if(impl_->find_source(n))
    {
        throw pic::error("dup source name");
    }
    source_t *s = new source_t(impl_,n,bs,sr,e,c);
    std::auto_ptr<source_t> sp(s);
    impl_->sources_.push_front(sp.get());
    sp.release();
    if(impl_->defaultsource_ == &impl_->internalsource_)
    {
        impl_->reset_default(s);
    }
}

void pia_clocklist_t::impl_t::reset_default(source_t *s)
{
    defaultsource_ = s;

    domainlist_t::iterator di=domains_.begin();
    domainlist_t::iterator de=domains_.end();

    for(; di!=de; ++di)
    {
        domain_t *d=*di;
        if(d->source_==&internalsource_)
        {
            d->source_=defaultsource_;
            d->changed();
        }
    }
    build();
}

void pia_clocklist_t::impl_t::clearmarks()
{
    domainlist_t::iterator di=domains_.begin();
    domainlist_t::iterator de=domains_.end();

    for(; di!=de; ++di)
    {
        domain_t *d=*di;

        sinklist_t::iterator si=d->members_.begin();
        sinklist_t::iterator se=d->members_.end();

        for(; si!=se; ++si)
        {
            sink_t *s=*si;
            s->mark_=false;
        }
    }
}

void pia_clocklist_t::impl_t::build()
{
    clearmarks();

    sourcelist_t::iterator oi=sources_.begin();
    sourcelist_t::iterator oe=sources_.end();

    for(; oi!=oe; ++oi)
    {
        (*oi)->build();
    }
}

void pia_clocklist_t::add_domain(const pia_ctx_t &e, bct_clockdomain_t *c)
{
    std::auto_ptr<domain_t> dp(new domain_t(impl_, e, c));
    impl_->domains_.push_front(dp.get());
    dp.release();
}

source_t *pia_clocklist_t::impl_t::find_source(const pia_data_t &n)
{
    if(n.type()==BCTVTYPE_STRING && !strcmp(n.asstring(),""))
    {
        return defaultsource_;
    }

    for(sourcelist_t::iterator i=sources_.begin(); i!=sources_.end(); ++i)
    {
        if((*i)->name_==n)
        {
            return *i;
        }
    }
    return 0;
}

void pia_clocklist_t::impl_t::sources_dump(const pia_ctx_t &e)
{
    sourcelist_t::iterator s;

    for(s=sources_.begin(); s!=sources_.end(); ++s)
    {
        (*s)->dump(e);
    }
}

void pia_clocklist_t::impl_t::domains_dump(const pia_ctx_t &e)
{
    domainlist_t::iterator d;

    for(d=domains_.begin(); d!=domains_.end(); ++d)
    {
        (*d)->dump(e);
    }
}

void pia_clocklist_t::impl_t::sources_kill(const pia_ctx_t &e, bool notify)
{
    sourcelist_t::iterator s;
restart:
    for(s=sources_.begin(); s!=sources_.end(); ++s)
    {
        if((*s)->kill(e, notify))
        {
            goto restart;
        }
    }
}

void pia_clocklist_t::impl_t::domains_kill(const pia_ctx_t &e, bool notify)
{
    domainlist_t::iterator d;
restart:
    for(d=domains_.begin(); d!=domains_.end(); ++d)
    {
        if((*d)->kill(e, notify))
        {
            goto restart;
        }
    }
}

void pia_clocklist_t::impl_t::dump(const pia_ctx_t &e)
{
    sources_dump(e);
    domains_dump(e);
}

void pia_clocklist_t::impl_t::kill(const pia_ctx_t &e, bool notify)
{
    sources_kill(e,notify);
    //domains_kill(e,notify);
}

void pia_clocklist_t::impl_t::internal_tick_fast(void *g_)
{
    pia_clocklist_t::impl_t *g = (pia_clocklist_t::impl_t *)g_;
    g->internalsource_.tick(g->glue_->time());
}

pia_clocklist_t::pia_clocklist_t(pia::manager_t::impl_t *g)
{
    impl_ = new impl_t(g);
}

int sink_t::api_add_upstream(bct_clocksink_host_ops_t **hops, bct_clocksink_t *up_)
{
    if(!up_->host_ops)
    {
        return -1;
    }

    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(s->env_->glue());
        sink_t *up = PIC_STRBASE(sink_t, up_->host_ops, ops_);
        bool b = s->add_upstream(up);
        return b ? 1 : -1;
    }
    PIA_CATCHLOG_EREF(s->env_)
    return -1;
}

int sink_t::api_remove_upstream(bct_clocksink_host_ops_t **hops, bct_clocksink_t *up_)
{
    if(!up_->host_ops)
    {
        return 0;
    }

    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(s->env_->glue());
        sink_t *up = PIC_STRBASE(sink_t, up_->host_ops, ops_);
        s->remove_upstream(up);
        return 1;
    }
    PIA_CATCHLOG_EREF(s->env_)
    return -1;
}

int sink_t::api_suppress(bct_clocksink_host_ops_t **hops, int f)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(!s->attached_)
    {
        if(f)
        {
            return 0;
        }

        return -1;
    }

    if(f==s->suppressed_)
    {
        return 1;
    }

    s->suppressed_ = f;
    return 1;
}

void sink_t::api_tick(bct_clocksink_host_ops_t **hops)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(s->attached_)
    {
        s->ticked(s->domain_->source_->last_tick_,s->domain_->source_->this_tick_);
    }
}

void sink_t::api_tick2(bct_clocksink_host_ops_t **hops,void (*cb)(unsigned long long,unsigned long long,void *),void *ctx)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(s->attached_)
    {
        cb(s->domain_->source_->last_tick_,s->domain_->source_->this_tick_,ctx);
    }
}

unsigned sink_t::api_buffer_size(bct_clocksink_host_ops_t **hops)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(s->attached_)
    {
        return s->domain_->source_->buf_size_;
    }

    return 0ULL;
}

unsigned long sink_t::api_sample_rate(bct_clocksink_host_ops_t **hops)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(s->attached_)
    {
        return s->domain_->source_->sample_rate_;
    }

    return 0ULL;
}

unsigned long long sink_t::api_current_tick(bct_clocksink_host_ops_t **hops)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(s->attached_)
    {
        return s->domain_->source_->this_tick_;
    }

    return 0ULL;
}

int sink_t::api_enable(bct_clocksink_host_ops_t **hops, int f, int sp)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    if(!s->attached_)
    {
        if(!f)
        {
            return 0;
        }

        return -1;
    }

    try
    {
        pia_mainguard_t guard(s->env_->glue());

        s->enabled_=f;
        s->suppressed_=sp;
        s->domain_->clocklist_->build();

        return 1;
    }
    PIA_CATCHLOG_EREF(s->env_)

    return -1;
}

void sink_t::api_close(bct_clocksink_host_ops_t **hops)
{
    sink_t *s = PIC_STRBASE(sink_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(s->env_->glue());
        s->detach(false,true);
        s->client_->plg_state = PLG_STATE_CLOSED;
        delete s;
    }
    PIA_CATCHLOG_EREF(s->env_)
}

bct_clocksink_host_ops_t sink_t::dispatch__ =
{
    api_add_upstream,
    api_remove_upstream,
    api_enable,
    api_close,
    api_suppress,
    api_tick,
    api_tick2,
    api_current_tick,
    api_buffer_size,
    api_sample_rate
};

int domain_t::api_set_source(bct_clockdomain_host_ops_t **hops, bct_data_t n)
{
    domain_t *d = PIC_STRBASE(domain_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(d->env_->glue());
        d->set_source(pia_data_t::from_lent(n));
        return 1;
    }
    PIA_CATCHLOG_EREF(d->env_)
    return -1;
}

bct_data_t domain_t::api_source_name(bct_clockdomain_host_ops_t **hops)
{
    domain_t *d = PIC_STRBASE(domain_t, hops, ops_);

    try
    {
        pia_logguard_t guard(d->env_->glue());
        if(d->source_)
        {
            return d->source_->name_.give();
        }
        return d->env_->glue()->allocate_cstring("*").give();
    }
    PIA_CATCHLOG_EREF(d->env_)

    return 0;
}

unsigned domain_t::api_buffer_size(bct_clockdomain_host_ops_t **hops)
{
    domain_t *d = PIC_STRBASE(domain_t, hops, ops_);

    try
    {
        pia_logguard_t guard(d->env_->glue());
        if(d->source_)
        {
            return d->source_->buf_size_;
        }
        return 0;
    }
    PIA_CATCHLOG_EREF(d->env_)

    return 0;
}

unsigned long domain_t::api_sample_rate(bct_clockdomain_host_ops_t **hops)
{
    domain_t *d = PIC_STRBASE(domain_t, hops, ops_);

    try
    {
        pia_logguard_t guard(d->env_->glue());
        if(d->source_)
        {
            return d->source_->sample_rate_;
        }
        return 0;
    }
    PIA_CATCHLOG_EREF(d->env_)

    return 0;
}

int domain_t::api_clocksink(bct_clockdomain_host_ops_t **hops, bct_clocksink_t *s,const char *n)
{
    domain_t *d = PIC_STRBASE(domain_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(d->env_->glue());
        d->enroll(s,n ? n : "slartibartfast");
        return 1;
    }
    PIA_CATCHLOG_EREF(d->env_)

    return -1;
}

void domain_t::api_close(bct_clockdomain_host_ops_t **hops)
{
    domain_t *d = PIC_STRBASE(domain_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(d->env_->glue());
        d->detach(false);
        d->client_->plg_state = PLG_STATE_CLOSED;
        delete d;
    }
    PIA_CATCHLOG_EREF(d->env_)
}

bct_clockdomain_host_ops_t domain_t::dispatch__ =
{
    api_set_source,
    api_source_name,
    api_buffer_size,
    api_sample_rate,
    api_clocksink,
    api_close,
};

void source_t::api_tick(bct_clocksource_host_ops_t **hops, unsigned long long bt)
{
    source_t *s = PIC_STRBASE(source_t, hops, ops_);

    try
    {
        pia_logguard_t guard(s->env_->glue());
        s->tick(bt);
    }
    PIA_CATCHLOG_EREF(s->env_)
}

void source_t::api_set_details(bct_clocksource_host_ops_t **hops, unsigned bs, unsigned long sr)
{
    source_t *s = PIC_STRBASE(source_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(s->env_->glue());
        s->set_details(bs,sr);
    }
    PIA_CATCHLOG_EREF(s->env_)
}

void source_t::api_close(bct_clocksource_host_ops_t **hops)
{
    source_t *s = PIC_STRBASE(source_t, hops, ops_);

    try
    {
        pia_mainguard_t guard(s->env_->glue());
        s->detach(false);
        s->client_->plg_state = PLG_STATE_CLOSED;
        delete s;
    }
    PIA_CATCHLOG_EREF(s->env_)
}

bct_clocksource_host_ops_t source_t::dispatch__ =
{
    api_tick,
    api_set_details,
    api_close
};

int pia_clocklist_t::cleardownstream(void *u_, void *d_)
{
    if(!u_ || !d_)
    {
        return -1;
    }

    sink_t *u = ((notifier_t *)u_)->sink_;
    sink_t *d = ((notifier_t *)d_)->sink_;

    d->remove_upstream(u);
    return 1;
}

int pia_clocklist_t::setdownstream(void *u_, void *d_)
{
    if(!u_ || !d_)
    {
        return -1;
    }

    sink_t *u = ((notifier_t *)u_)->sink_;
    sink_t *d = ((notifier_t *)d_)->sink_;

    return d->add_upstream(u) ? 1 : -1;
}

void pia_clocklist_t::dump(const pia_ctx_t &e)
{
    impl_->dump(e);
}

void pia_clocklist_t::kill(const pia_ctx_t &e)
{
    impl_->kill(e,true);
}

void *pia_clocklist_t::addnotify(bct_clocksink_t *s_, void (*cb)(void *), void *arg)
{
    sink_t *s = PIC_STRBASE(sink_t,s_->host_ops,ops_);
    return s->add_notify(cb, arg);
}

void pia_clocklist_t::cancelnotify(void *n_)
{
    if(n_)
    {
        ((notifier_t *)n_)->cancel();
    }
}
