
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

#include <piagent/pia_scaffold.h>
#include <piagent/pia_fastalloc.h>
#include <piagent/pia_realnet.h>
#include <picross/pic_thread.h>
#include <picross/pic_time.h>
#include <picross/pic_ref.h>
#include <picross/pic_safeq.h>
#include <picross/pic_mlock.h>
#include "pia_glue.h"

#include <vector>

namespace
{
    struct usage_t: virtual public pic::lckobject_t
    {
        usage_t(unsigned *u, pic::xgate_t *g): usage_(u), ousage_(0), gate_(g) {}

        void set(unsigned u)
        {
            if(u!=ousage_)
            {
                ousage_=u;
                *usage_=u;
                gate_->open();
            }
        }

        unsigned *usage_;
        unsigned ousage_;
        pic::xgate_t *gate_;
    };

    class pingerthread_t: public pic::nocopy_t, public pic::thread_t, virtual public pic::lckobject_t
    {
        public:
            pingerthread_t(unsigned priority, const pic::notify_t &service): pic::thread_t(priority), quit_(false), service_(service)
            {
            }

            virtual ~pingerthread_t()
            {
            }

            void quit()
            {
                quit_=true;
                gate_.open();
                wait();
            }

            void ping()
            {
                gate_.open();
            }

            void thread_main()
            {
#ifdef DEBUG_DATA_ATOMICITY
                std::cout << "Started pinger thread with ID " << pic_current_threadid() << std::endl;
#endif
                while(!quit_)
                {
                    if(gate_.pass_and_shut_timed(1000))
                    {
                        service_();
                    }
                }
            }

        private:
            pic::xgate_t gate_;
            bool quit_;
            pic::notify_t service_;
    };

    struct fastthread_t: pic::thread_t, virtual public pic::lckobject_t
    {
        fastthread_t(unsigned pri,pia::manager_t *m,usage_t *u=0);
        ~fastthread_t();

        bool isfast() PIC_FASTCODE;
        void service() PIC_FASTCODE;
        void thread_main() PIC_FASTCODE;
        void start();
        void shutdown(bool w);
        void thread_init();
        void thread_term();

        pia::manager_t *manager_;
        pic::xgate_t gate_;
        pic::tsd_t marker_;
        volatile bool shutdown_;
        usage_t *usage;
    };

    struct ctxthread_t: pic::thread_t, pic::counted_t, virtual public pic::lckobject_t
    {
        ctxthread_t(unsigned pri,int grp,pia::manager_t *m);
        ~ctxthread_t();

        void service();
        void shutdown(bool w);
        void thread_init();
        void thread_main();

        pia::manager_t *manager_;
        pic::xgate_t gate_;
        volatile bool shutdown_;
        int group_;
    };

    struct mainthread_t: pic::thread_t, virtual public pic::lckobject_t
    {
        mainthread_t(unsigned pri,pia::manager_t *m, pia::realnet_t *n);
        ~mainthread_t();

        void service();
        void shutdown(bool w);
        void thread_init();
        void thread_main();

        pia::manager_t *manager_;
        pia::realnet_t *network_;
        volatile bool shutdown_;
        unsigned usage_;
        pic::xgate_t gate_;
    };

    struct mtscaffold_t: pia::controller_t, virtual pic::lckobject_t
    {
        mtscaffold_t(const char *user,pic::nballocator_t *a, unsigned mt, const pic::f_string_t &l,const pic::f_string_t &winch,bool ck, bool rt);
        ~mtscaffold_t();

        pia::context_t context(const pic::status_t &gone, const pic::f_string_t &log, const char *tag);
        void wait();
        void service_fast();
        void service_main();
        void service_ctx(int group);
        bool service_isfast();
        void service_gone();

        pia::realnet_t network_;
        fastthread_t fast_;
        mainthread_t main_;
        pia::manager_t manager_;
        bool shutdown_;
        pic::lckvector_t<pic::ref_t<ctxthread_t> >::lcktype context_;
        unsigned mt_;
    };

    struct guiscaffold_t: pia::controller_t
    {
        guiscaffold_t(const char *user,pic::nballocator_t *a, const pic::notify_t &s, const pic::notify_t &g,const pic::f_string_t &l,const pic::f_string_t &winch, bool ck,bool rt);
        ~guiscaffold_t();

        pia::context_t context(int grp,const pic::status_t &gone, const pic::f_string_t &log, const char *tag);

        void process_ctx();
        void service_fast();
        void service_main();
        bool service_isfast();
        void service_gone();
        unsigned cpu_usage() { return cpu_usage_; }

        void service_ctx(int group);

        pic::notify_t gone_;

        unsigned cpu_usage_;
        pic::xgate_t gate_;
        pia::realnet_t network_;
        mainthread_t main_;
        usage_t usage_;
        fastthread_t fast_;
        ctxthread_t ctx0_;
        pia::manager_t manager_;
        pingerthread_t pinger_;

    };
};

struct pia::scaffold_mt_t::impl_t
{
    impl_t(const char *user, unsigned mt,const pic::f_string_t &l,const pic::f_string_t &w,bool ck,bool rt)
    {
        pia_logguard_t guard(0,&allocator_);
        pic_set_foreground(rt);
        scaffold_=new mtscaffold_t(user,&allocator_,mt,l,w,ck,rt);
    }

    ~impl_t()
    {
        pia_logguard_t g(0,&allocator_);
        delete scaffold_;
    }

    pia::fastalloc_t allocator_;
    mtscaffold_t *scaffold_;
};

struct pia::scaffold_gui_t::impl_t
{
    impl_t(const char *user, const pic::notify_t &s, const pic::notify_t &g,const pic::f_string_t &l,const pic::f_string_t &w,bool ck,bool rt)
    {
        pia_logguard_t guard(0,&allocator_);
        pic_set_foreground(rt);
        scaffold_=new guiscaffold_t(user,&allocator_,s,g,l,w,ck,rt);
    }

    ~impl_t()
    {
        pia_logguard_t g(0,&allocator_);
        delete scaffold_;
    }

    pia::fastalloc_t allocator_;
    guiscaffold_t *scaffold_;
};

void fastthread_t::service()
{
    gate_.open();
}

pia::scaffold_mt_t::scaffold_mt_t(const char *user, unsigned mt,const pic::f_string_t &l,const pic::f_string_t &winch,bool ck,bool rt)
{
    impl_=new impl_t(user,(mt==0)?1:mt,l,winch,ck,rt);
}

pia::scaffold_mt_t::~scaffold_mt_t()
{
    delete impl_;
}

void pia::scaffold_mt_t::wait()
{
    impl_->scaffold_->wait();
}

void pia::scaffold_mt_t::global_unlock()
{
    impl_->scaffold_->manager_.global_unlock();
}

bool pia::scaffold_mt_t::global_lock()
{
    return impl_->scaffold_->manager_.global_lock();
}

pia::context_t pia::scaffold_mt_t::context(const pic::status_t &gone, const pic::f_string_t &log, const char *tag)
{
    return impl_->scaffold_->context(gone,log,tag);
}

pia::scaffold_gui_t::scaffold_gui_t(const char *user, const pic::notify_t &svc, const pic::notify_t &gone,const pic::f_string_t &l,const pic::f_string_t &winch,bool ck,bool rt)
{
    impl_=new impl_t(user,svc,gone,l,winch,ck,rt);
}

unsigned pia::scaffold_gui_t::window_count()
{
    return impl_->scaffold_->manager_.window_count();
}

std::string pia::scaffold_gui_t::window_title(unsigned w)
{
    return impl_->scaffold_->manager_.window_title(w);
}

bool pia::scaffold_gui_t::window_state(unsigned w)
{
    return impl_->scaffold_->manager_.window_state(w);
}

void pia::scaffold_gui_t::global_unlock()
{
    impl_->scaffold_->manager_.global_unlock();
}

bool pia::scaffold_gui_t::global_lock()
{
    return impl_->scaffold_->manager_.global_lock();
}

void pia::scaffold_gui_t::fast_pause()
{
    impl_->scaffold_->manager_.fast_pause();
    impl_->scaffold_->fast_.shutdown(true);
}

void pia::scaffold_gui_t::fast_resume()
{
    impl_->scaffold_->fast_.start();
    impl_->scaffold_->manager_.fast_resume();
}

void pia::scaffold_gui_t::set_window_state(unsigned w,bool o)
{
    impl_->scaffold_->manager_.set_window_state(w,o);
}

pia::scaffold_gui_t::~scaffold_gui_t()
{
    delete impl_;
}

unsigned pia::scaffold_gui_t::cpu_usage()
{
    return impl_->scaffold_->cpu_usage();
}

pia::context_t pia::scaffold_gui_t::context(const pic::status_t &gone, const pic::f_string_t &log, const char *tag)
{
    return impl_->scaffold_->context(1,gone,log,tag);
}

pia::context_t pia::scaffold_gui_t::bgcontext(const pic::status_t &gone, const pic::f_string_t &log, const char *tag)
{
    return impl_->scaffold_->context(0,gone,log,tag);
}

void pia::scaffold_gui_t::process_ctx()
{
    impl_->scaffold_->process_ctx();
}

fastthread_t::fastthread_t(unsigned pri,pia::manager_t *m, usage_t *u): pic::thread_t(pri), manager_(m), shutdown_(false), usage(u)
{
}

fastthread_t::~fastthread_t()
{
    shutdown(true);
}

bool fastthread_t::isfast()
{
    return (marker_.get()==this);
}

void fastthread_t::start()
{
    if(isrunning()) return;

    shutdown_ = false;
    run();
}

void fastthread_t::shutdown(bool w)
{
    if(!isrunning()) return;

    shutdown_ = true;
    gate_.open();
    if(w) wait();
}

void fastthread_t::thread_init()
{
    marker_.set(this);
    pic_set_fpu();
}

void fastthread_t::thread_term()
{
    marker_.set(0);
}

void fastthread_t::thread_main()
{
#ifdef DEBUG_DATA_ATOMICITY
    std::cout << "Started fast thread with ID " << pic_current_threadid() << std::endl;
#endif

    unsigned long long t1 = pic_microtime();
    unsigned long long tn = 0;
    unsigned u = 0;
    unsigned uc = 0;

    while(!shutdown_)
    {
        unsigned long long next_timer;
        unsigned long long now = pic_microtime();
        unsigned long long last = now;

        for(;;)
        {
            bool a = false;
            next_timer = now+5000000ULL;

            manager_->process_fast(now,&next_timer,&a);

            if(!a) break;

            last = now;
            now = pic_microtime();
            tn += (now-last);
        }

        now = pic_microtime();

        if(usage)
        {
            unsigned long long n = now-t1;
            if(n>=50000)
            {
                u = std::max(u,(unsigned)((tn*100)/n));
                t1=now;
                tn=0;
                if(++uc==10)
                {
                    usage->set(u);
                    uc=0;
                    u=0;
                }
            }
        }

        if(now < next_timer)
        {
            gate_.pass_and_shut_timed(next_timer-now);
        }
    }
}

ctxthread_t::ctxthread_t(unsigned pri,int grp,pia::manager_t *m): pic::thread_t(pri), manager_(m), shutdown_(false), group_(grp)
{
}

ctxthread_t::~ctxthread_t()
{
    shutdown(true);
}

void ctxthread_t::service()
{
    gate_.open();
}

void ctxthread_t::shutdown(bool w)
{
    shutdown_ = true;
    gate_.open();
    if(w) wait();
}

void ctxthread_t::thread_init()
{

}

void ctxthread_t::thread_main()
{
#ifdef DEBUG_DATA_ATOMICITY
    std::cout << "Started context thread with ID " << pic_current_threadid() << std::endl;
#endif

    while(!shutdown_)
    {
        for(;;)
        {
            bool a = false;

            manager_->process_ctx(group_,pic_microtime(),&a);

            if(!a) break;
        }

        gate_.pass_and_shut_timed(5000000);

    }
}

mainthread_t::mainthread_t(unsigned pri,pia::manager_t *m, pia::realnet_t *n): pic::thread_t(pri), manager_(m), network_(n), shutdown_(false), usage_(0)
{
}

mainthread_t::~mainthread_t()
{
    shutdown(true);
}

void mainthread_t::service()
{
    gate_.open();
}

void mainthread_t::shutdown(bool w)
{
    shutdown_ = true;
    gate_.open();
    if(w) wait();
}

void mainthread_t::thread_init()
{
}

void mainthread_t::thread_main()
{
#ifdef DEBUG_DATA_ATOMICITY
    std::cout << "Started main thread with ID " << pic_current_threadid() << std::endl;
#endif

    for(;;)
    {
        unsigned long long next_timer;
        unsigned long long now = pic_microtime();

        if(shutdown_) return;

        for(;;)
        {
            bool a = false;
            next_timer = now+1000000ULL;

            if(shutdown_) return;

            manager_->process_main(now,&next_timer,&a);
            now = pic_microtime();

            if(!a) break;
        }

        if(shutdown_) return;

        if(now<next_timer)
        {
            gate_.pass_and_shut_timed(next_timer-now);
        }

        if(usage_)
        {
            fprintf(stderr,"cpu: %u\n",usage_);
            usage_=0;
        }
    }
}

mtscaffold_t::mtscaffold_t(const char *user,pic::nballocator_t *a, unsigned mt, const pic::f_string_t &l,const pic::f_string_t &winch,bool ck,bool rt): network_(a,ck), fast_(rt?19:0,&manager_), main_(1,&manager_,&network_), manager_(user,this,a,&network_,l,winch), context_(mt), mt_(mt)
{
    fast_.run();

    for(unsigned i=0;i<mt_;i++)
    {
        context_[i] = pic::ref(new ctxthread_t(0,0,&manager_));
        context_[i]->run();
    }
}

mtscaffold_t::~mtscaffold_t()
{
    for(unsigned i=0;i<mt_;i++)
    {
        context_[i]->shutdown(true);
    }

    fast_.shutdown(true);
}

pia::context_t mtscaffold_t::context(const pic::status_t &gone, const pic::f_string_t &log, const char *tag)
{
    return manager_.context(0,gone,log,tag);
}

void mtscaffold_t::wait()
{
    main_.run();
    main_.wait();
}

void mtscaffold_t::service_fast() { fast_.service(); }
void mtscaffold_t::service_main() { main_.service(); }

void mtscaffold_t::service_ctx(int group)
{
    for(unsigned i=0;i<mt_;i++)
    {
        context_[i]->service();
    }
}

bool mtscaffold_t::service_isfast() { return fast_.isfast(); }
void mtscaffold_t::service_gone() { main_.shutdown(false); }

guiscaffold_t::guiscaffold_t(const char *user,pic::nballocator_t *a, const pic::notify_t &s, const pic::notify_t &g, const pic::f_string_t &l,const pic::f_string_t &winch,bool ck, bool rt): gone_(g), network_(a,ck), main_(2,&manager_,&network_), usage_(&cpu_usage_,&gate_), fast_(rt?19:0,&manager_,&usage_),  ctx0_(0,0,&manager_), manager_(user,this,a,&network_,l,winch), pinger_(1,s)
{
    cpu_usage_ = 0;
    fast_.run();
    main_.run();
    ctx0_.run();
    pinger_.run();
}

guiscaffold_t::~guiscaffold_t()
{
    ctx0_.shutdown(true);
    main_.shutdown(true);
    fast_.shutdown(true);
    pinger_.quit();
}

pia::context_t guiscaffold_t::context(int grp,const pic::status_t &gone, const pic::f_string_t &log, const char *tag)
{
    return manager_.context(grp,gone,log,tag);
}

void guiscaffold_t::process_ctx()
{
    unsigned long long now = pic_microtime();
    bool a;
    manager_.process_ctx(1,now,&a);
}

void guiscaffold_t::service_fast() { fast_.service(); }
void guiscaffold_t::service_main() { main_.service(); }
bool guiscaffold_t::service_isfast() { return fast_.isfast(); }
void guiscaffold_t::service_gone() { gone_(); }

void guiscaffold_t::service_ctx(int group)
{
    if(group!=0)
    {
        pinger_.ping();
        return;
    }

    ctx0_.service();
}
