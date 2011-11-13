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

#ifndef __PIA_SRC_GLUE__
#define __PIA_SRC_GLUE__

#include <stdlib.h>

#include <pibelcanto/plugin.h>
#include <picross/pic_functor.h>
#include <picross/pic_time.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_thread.h>
#include <picross/pic_atomic.h>

#include "pia_manager.h"
#include "pia_proxy.h"
#include "pia_local.h"
#include "pia_index.h"
#include "pia_glue.h"
#include "pia_thing.h"
#include "pia_data.h"
#include "pia_clock.h"
#include "pia_fastdata.h"
#include "pia_rpc.h"
#include "pia_error.h"
#include "pia_eventq.h"
#include "pia_dataqueue.h"
#include "pia_window.h"

struct pia_ctx_t;
struct pia_timer_t;

class pia::manager_t::impl_t: pic::nocopy_t, public pic::logger_t, virtual public pic::lckobject_t
{
    public:
        impl_t(pia::controller_t *h, pic::nballocator_t *a, pia::network_t *n, const pic::f_string_t &log, const pic::f_string_t &winch, void *winctx);
        ~impl_t();

        void kill(const pia_ctx_t &e);
        void dump(const pia_ctx_t &e);
        void dump_killed();
        unsigned random() { return (unsigned)rand(); }

        void create_clocksource(const pia_ctx_t &e, const pia_data_t &n, unsigned bs, unsigned sr, bct_clocksource_t *s);
        void create_clockdomain(const pia_ctx_t &e, bct_clockdomain_t *d);
        void create_client(const pia_ctx_t &e, const pia_data_t &addr, bct_client_t *c, bool fast);
        void create_server(const pia_ctx_t &e, const pia_data_t &addr, bct_server_t *s);
        void create_thing(const pia_ctx_t &e, bct_thing_t *i);
        void create_window(const pia_ctx_t &e, bct_window_t *i);
        void create_fastdata(const pia_ctx_t &e, bct_fastdata_t *i);
        void create_index(const pia_ctx_t &e, const pia_data_t &addr, bct_index_t *i);
        void create_rpcserver(const pia_ctx_t &e, bct_rpcserver_t *i, const pia_data_t &id);
        void create_rpcclient(const pia_ctx_t &e, bct_rpcclient_t *i, const pia_data_t &id, const pia_data_t &p, const pia_data_t &n, const pia_data_t &val, unsigned long t);
        unsigned window_count() { return window_.window_count(); }
        std::string window_title(unsigned w) { return window_.window_title(w); }
        void set_window_state(unsigned w,bool o) { window_.set_window_state(w,o); }
        bool window_state(unsigned w) { return window_.window_state(w); }

        pia_dataqueue_t allocate_dataqueue(unsigned len) { return pia_dataqueue_t::alloc(len); }

        bct_data_t allocate_host_raw(unsigned nb, unsigned long long ts, float u, float l, float rst,unsigned t, unsigned sl, unsigned char **hp, unsigned vl, float **vp) { return ::allocate_host_raw(allocator(),nb,ts,u,l,rst,t,sl,hp,vl,vp); }
        bct_data_t allocate_wire_raw(unsigned nb, unsigned wl, const unsigned char *wp) { return ::allocate_wire_raw(allocator(),nb,wl,wp); }

        pia_data_t allocate_host(unsigned nb,unsigned long long ts, float u, float l, float r, unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp) const { return pia_data_t::allocate_host(allocator(),nb,ts,u,l,r,t,dl,dp,vl,vp); }
        pia_data_t allocate_wire(unsigned nb,unsigned dl, const unsigned char *dp) const { return pia_data_t::allocate_wire(allocator(),nb,dl,dp); }
        pia_data_t allocate_path(unsigned pl, const unsigned char *p, unsigned long long ts=0ULL) const { return pia_data_t::allocate_path(allocator(),pl,p,ts); }
        pia_data_t path_append(const pia_data_t &p, unsigned e) const { return pia_data_t::path_append(allocator(),p,e); }
        pia_data_t allocate_string(const char *s, unsigned sl,unsigned long long ts = 0ULL) const { return pia_data_t::allocate_string(allocator(),s,sl,ts); }
        pia_data_t allocate_bool(bool v,unsigned long long ts = 0ULL) const { return pia_data_t::allocate_bool(allocator(),v,ts); }
        pia_data_t allocate_cstring(const char *s, unsigned long long ts = 0ULL) const { return pia_data_t::allocate_cstring(allocator(),s,strlen(s),ts); }
        pia_data_t allocate_cstring_nb(const char *s, unsigned long long ts = 0ULL) const { return pia_data_t::allocate_cstring_nb(allocator(),s,strlen(s),ts); }

        pia_data_nb_t allocate_host_nb(unsigned long long ts, float u, float l, float r, unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp) const { return pia_data_nb_t::allocate_host(allocator(),ts,u,l,r,t,dl,dp,vl,vp); }
        pia_data_nb_t allocate_wire_nb(unsigned dl, const unsigned char *dp) const { return pia_data_nb_t::allocate_wire(allocator(),dl,dp); }

        void advertise(const pia_data_t &index, void *id, const pia_data_t &server, unsigned short cookie) { index_.advertise(id,index,server,cookie); }
        void unadvertise(const pia_data_t &index, void *id, const pia_data_t &server) { index_.unadvertise(id,index,server); }
        void killadvertise(void *id) { index_.killadvertise(id); }
        unsigned long long time() { return pic_microtime(); }
        pia_eventq_t *mainq() { return &mainq_; }
        pia_eventq_nb_t *fastq() { return &fastq_; }
        pia_eventq_t *auxq() { return &auxq_; }
        pic::nballocator_t *allocator() const { return allocator_; }
        pia::network_t *network() { return network_; }
        void *addclocknotify(bct_clocksink_t *c, void (*callback)(void *), void *arg) { return clock_.addnotify(c,callback,arg); }
        void cancelclocknotify(void *n) { clock_.cancelnotify(n); }
        int setdownstreamclock(void *u, void *d) { return clock_.setdownstream(u,d); }
        int cleardownstreamclock(void *u, void *d) { return clock_.cleardownstream(u,d); }
        void context_add(pia::context_t::impl_t *);
        void context_del(pia::context_t::impl_t *);
        int fastcall(int (*cb)(void *, void *,void *, void *), void *, void *,void *, void *);
        pia::controller_t *handle() { return handle_; }
        void main_lock() { lock_.lock(); }
        void main_unlock() { lock_.unlock(); }
        bool global_lock();
        void global_unlock();
        bool fast_active() { return fastactive_; }
        void fast_pause();
        void fast_resume();
        void log(const pia_data_t &msg);
        void log(const char *msg);
        void winch(const char *msg);
        pia_data_t unique() const;
        void *winctx() const { return winctx_; }

        void *add_timer(void (*cb)(void *arg), void *arg, unsigned sec);
        void del_timer(void *hnd);

        void service_ctx(int group);
        void service_fast();
        void service_main();
        void service_aux();

        void process_ctx(int grp, unsigned long long now, bool *activity);
        void process_fast(unsigned long long now, unsigned long long *timer, bool *activity);
        void process_main(unsigned long long now, unsigned long long *timer, bool *activity);

    private:
        friend class pia::context_t::impl_t;

        static void idle_callback(void *g_, const pia_data_t &d);
        static void timer_callback(void *);
        static void log_callback(void *e_, const pia_data_t &d);
        static void winch_callback(void *e_, const pia_data_t &d);

        pia::controller_t *handle_;
        unsigned int seed_;
        unsigned long long chuff_;

        pia::network_t *network_;
        pic::nballocator_t *allocator_;

        pia_eventq_t auxq_;
        pia_eventq_nb_t fastq_;
        pia_eventq_t mainq_;
        pic::mutex_t lock_;

        pia_proxylist_t proxy_;
        pia_locallist_t local_;
        pia_indexlist_t index_;
        pia_thinglist_t thing_;
        pia_fastdatalist_t fast_;
        pia_clocklist_t clock_;
        pia_rpclist_t rpc_;
        pia_windowlist_t window_;

        pic::ilist_t<pia::context_t::impl_t> contexts_;

        pic::f_string_t log_;
        pic_atomic_t auxflag_;
        bool auxbusy_;
        pia_cref_t cpoint_;

        pic::ilist_t<pia_timer_t> timers_;
        pia_cref_t timer_cpoint_;
        unsigned timer_count_;
		void *winctx_;
        pic::f_string_t winch_;
        pic::rwmutex_t global_lock_;

        pic::mutex_t fast_lock_;
        pic_atomic_t fastactive_;
};

class pia_logguard_t: pic::nocopy_t
{
    public:
        pia_logguard_t(pic::logger_t *l, pic::nballocator_t *a): logger_(pic::logger_t::tsd_getlogger()), allocator_(pic::nballocator_t::tsd_getnballocator())
        {
            pic::logger_t::tsd_setlogger(l);
            pic::nballocator_t::tsd_setnballocator(a);
        }

        pia_logguard_t(pia::manager_t::impl_t *glue): logger_(pic::logger_t::tsd_getlogger()), allocator_(pic::nballocator_t::tsd_getnballocator())
        {
            pic::logger_t::tsd_setlogger(glue);
            pic::nballocator_t::tsd_setnballocator(glue->allocator());
        }

        ~pia_logguard_t()
        {
            pic::logger_t::tsd_setlogger(logger_);
            pic::nballocator_t::tsd_setnballocator(allocator_);
        }

    private:
        pic::logger_t *logger_;
        pic::nballocator_t *allocator_;
};

class pia_logprotect_t: pic::nocopy_t
{
    public:
        pia_logprotect_t(): logger_(pic::logger_t::tsd_getlogger()), allocator_(pic::nballocator_t::tsd_getnballocator())
        {
        }

        ~pia_logprotect_t()
        {
            pic::logger_t::tsd_setlogger(logger_);
            pic::nballocator_t::tsd_setnballocator(allocator_);
        }

    private:
        pic::logger_t *logger_;
        pic::nballocator_t *allocator_;
};

class pia_mainguard_t: pia_logguard_t
{
    public:
        pia_mainguard_t(pia::manager_t::impl_t *glue): pia_logguard_t(glue), glue_(glue) { glue_->main_lock(); }
        ~pia_mainguard_t() { glue_->main_unlock(); }
    private:
        pia::manager_t::impl_t *glue_;
};

class pia::context_t::impl_t: pic::nocopy_t, public pic::element_t<>, virtual public pic::lckobject_t
{
    public:
        impl_t(pia::manager_t::impl_t *, const char *user, int, const pic::status_t &, const pic::f_string_t &, const char *, bool);
        ~impl_t();

        void kill();
        const char *tag() const { return (const char *)(tag_.hostdata()); }
        pia::manager_t::impl_t *glue() { return glue_; }
        pia_data_t user() { return user_; }

        pia_data_t expand_address(const char *addr);
        pia_data_t expand_address_relative(const char *addr, const pia_data_t &user);
        pia_data_t collapse_address(const pia_data_t &addr);
        pia_data_t collapse_address_relative(const pia_data_t &addr, const pia_data_t &user);
        unsigned weak_count() { return weak_; }
        unsigned strong_count() { return strong_; }
        void weak_inc() { weak_++; }
        void strong_inc() { strong_++; weak_inc(); }
        bool inuse() { return strong_!=0; }
        void log(const pia_data_t &msg);
        void strong_dec() { if(--strong_==0) { glue_->auxq()->idle(cpoint_,idle_callback,this,pia_data_t()); return; } weak_dec(); }
        void weak_dec() { if(--weak_==0) { glue_->context_del(this); delete this; } }
        void cancel() { log_.clear(); gone_.clear(); }
        pia_eventq_t *appq() { return &appq_; }
        void service_ctx();
        unsigned weak() { return weak_; }
        void exit();

        static pia::context_t::impl_t *from_entity(bct_entity_t e);
        bct_entity_t api() { return &ops_; }

        bool enter() { weak_inc(); if(!busy_.trylock()) { weak_dec(); return false; } return true; }
        void leave() { busy_.unlock(); weak_dec(); }
        void lock() { weak_inc(); busy_.lock();  }
        void unlock() { busy_.unlock();  service_ctx(); weak_dec(); }
        bool killed() { return killed_; }

    private:
        friend class pia::manager_t::impl_t;

        static void idle_callback(void *e_, const pia_data_t &d);
        static void log_callback(void *e_, const pia_data_t &d);

        pic::status_t gone_;
        pic::f_string_t log_;
        pia::manager_t::impl_t *glue_;
        pia_eventq_t appq_;

        pia_data_t tag_;
        unsigned strong_;
        unsigned weak_;
        pic::mutex_t busy_;

        bct_entity_ops_t *ops_;
        static bct_entity_ops_t dispatch__;

        pic_atomic_t queued_;
        bool killed_;
        bool exited_;
        int group_;
        pia_cref_t cpoint_;
        pia_data_t user_;
};

class pia_ctx_t
{
	public:
		pia_ctx_t() { entity_=0; }
		pia_ctx_t(pia::context_t::impl_t *e): entity_(e) { if(entity_) entity_->strong_inc(); }
        pia_ctx_t(const pia_ctx_t &e): entity_(e.entity_) { if(entity_) entity_->strong_inc(); }
        pia_ctx_t(pia::manager_t::impl_t *g, const char *u, int grp, const pic::status_t &n, const pic::f_string_t &l, const char *tag): entity_(new pia::context_t::impl_t(g,u,grp,n,l,tag,true)) {}
		~pia_ctx_t() { release(); }

        pia_ctx_t &operator=(const pia_ctx_t &e) { release(); entity_=e.entity_; if(entity_) entity_->strong_inc(); return *this; }
        pia_ctx_t &operator=(pia::context_t::impl_t *e) { release(); entity_=e; if(entity_) entity_->strong_inc(); return *this; }
        void release() { if(entity_) entity_->strong_dec(); entity_=0; }
        bool valid() { return entity_!=0; }

        pia::context_t::impl_t *operator->() const { PIC_ASSERT(entity_); return entity_; }

		bool matches(pia::context_t::impl_t *e) const { if(!entity_) return false; return e==0 || e==entity_; }
		bool matches(const pia_ctx_t &e) const { if(!entity_) return false; return e.entity_==0 || e.entity_==entity_; }

	private:
		pia::context_t::impl_t *entity_;
};

class pia_timerhnd_t
{
    public:
        pia_timerhnd_t(pia::manager_t::impl_t *glue): glue_(glue), hnd_(0) {}
        ~pia_timerhnd_t() { cancel(); }
        void cancel() { if(hnd_) { glue_->del_timer(hnd_); hnd_=0; } }
        void start(void (*cb)(void *ctx), void (*ctx), unsigned sec) { cancel(); hnd_=glue_->add_timer(cb,ctx,sec); }
    private:
        pia::manager_t::impl_t *glue_;
    public:
        void *hnd_;
};

#endif
