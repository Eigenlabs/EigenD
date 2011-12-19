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

#ifndef __PIW_TSD__
#define __PIW_TSD__

#include "piw_exports.h"
#include <pibelcanto/plugin.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_log.h>
#include "piw_data.h"
#include "piw_dataqueue.h"

namespace piw
{
    struct PIW_DECLSPEC_CLASS piw_logger_t: pic::logger_t
    {
        void log(const char *msg);
    };

    struct PIW_DECLSPEC_CLASS piw_allocator_t: pic::nballocator_t
    {
        void *allocator_xmalloc(unsigned nb,size_t size, deallocator_t *d, void **da);
    };

    extern PIW_DECLSPEC_FUNC(piw_logger_t) logger__;
    extern PIW_DECLSPEC_FUNC(piw_allocator_t) allocator__;

    inline bct_entity_t tsd_getcontext() { return (bct_entity_t)pic::thread_t::tsd_getcontext(); }
    inline void tsd_setcontext(bct_entity_t c) { pic::thread_t::tsd_setcontext((void *)c); pic::nballocator_t::tsd_setnballocator(&allocator__); pic::logger_t::tsd_setlogger(&logger__); }
    inline void tsd_clearcontext() { pic::thread_t::tsd_setcontext(0); pic::logger_t::tsd_clearlogger(); pic::nballocator_t::tsd_clearnballocator(); }
    PIW_DECLSPEC_FUNC(void) tsd_server(const char *n, bct_server_t *s);
    PIW_DECLSPEC_FUNC(void) tsd_client(const char *n, bct_client_t *s, bool fast = true);
    PIW_DECLSPEC_FUNC(void) tsd_index(const char *n, bct_index_t *s);
    PIW_DECLSPEC_FUNC(void) tsd_fastdata(bct_fastdata_t *f);
    PIW_DECLSPEC_FUNC(void) tsd_clocksource(const piw::data_t &name,unsigned bs, unsigned sr, bct_clocksource_t *s);
    PIW_DECLSPEC_FUNC(void) tsd_clockdomain(bct_clockdomain_t *d);
    PIW_DECLSPEC_FUNC(void) tsd_thing(bct_thing_t *t);
    PIW_DECLSPEC_FUNC(void) tsd_window(bct_window_t *t);
    PIW_DECLSPEC_FUNC(void) tsd_rpcserver(bct_rpcserver_t *, const char *id);
    PIW_DECLSPEC_FUNC(void) tsd_rpcclient(bct_rpcclient_t *, const char *id, const piw::data_t &p, const piw::data_t &name, const piw::data_t &val, unsigned long);
    PIW_DECLSPEC_FUNC(void) tsd_rpcasync(const char *id, const piw::data_t &p, const piw::data_t &name, const piw::data_t &val, unsigned long);
    PIW_DECLSPEC_FUNC(unsigned long long) tsd_time();
    PIW_DECLSPEC_FUNC(void) *tsd_alloc(unsigned nb,unsigned size, void (**dealloc)(void *, void *), void **dealloc_arg);
    PIW_DECLSPEC_FUNC(void) tsd_lock();
    PIW_DECLSPEC_FUNC(void) tsd_unlock();
    PIW_DECLSPEC_FUNC(int) tsd_fastcall(int (*cb)(void *arg1, void *arg2), void *arg1, void *arg2);
    PIW_DECLSPEC_FUNC(int) tsd_fastcall3(int (*cb)(void *arg1, void *arg2, void *arg3), void *arg1, void *arg2, void *arg3);
    PIW_DECLSPEC_FUNC(int) tsd_fastcall4(int (*cb)(void *arg1, void *arg2, void *arg3, void *arg4), void *arg1, void *arg2, void *arg3, void *arg4);
    PIW_DECLSPEC_FUNC(int) tsd_fastcall5(int (*cb)(void *arg1, void *arg2, void *arg3, void *arg4, void *arg5), void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);
    PIW_DECLSPEC_FUNC(void) tsd_log(const char *);
    PIW_DECLSPEC_FUNC(bool) tsd_killed();
    PIW_DECLSPEC_FUNC(void) tsd_exit();
    PIW_DECLSPEC_FUNC(dataqueue_t) tsd_dataqueue(unsigned size);
    PIW_DECLSPEC_FUNC(void) tsd_dump();
    PIW_DECLSPEC_FUNC(void) *tsd_winctx();
    PIW_DECLSPEC_FUNC(void) tsd_alert(const char *,const char *,const char *);

    PIW_DECLSPEC_FUNC(std::string) tsd_scope();

    class PIW_DECLSPEC_CLASS tsd_subcontext_t: public pic::nocopy_t
    {
        public:
            tsd_subcontext_t(bool,const char *,const char *);
            ~tsd_subcontext_t();
            void kill();
            void install();
            void clear();
            bct_entity_t entity() { return entity_; }
        private:
            bct_entity_t entity_;
    };

    class PIW_DECLSPEC_CLASS tsd_snapshot_t: public pic::nocopy_t
    {
        public:
            tsd_snapshot_t(): entity_(tsd_getcontext()) {}
            void save() { entity_=tsd_getcontext(); }
            void install() { tsd_setcontext(entity_); }
            void log(const char *);
            int fastcall(int (*cb)(void *arg1, void *arg2), void *arg1, void *arg2);
            int fastcall3(int (*cb)(void *arg1, void *arg2, void *arg3), void *arg1, void *arg2, void *arg3);
            int fastcall4(int (*cb)(void *arg1, void *arg2, void *arg3, void *arg4), void *arg1, void *arg2, void *arg3, void *arg4);
            int fastcall5(int (*cb)(void *arg1, void *arg2, void *arg3, void *arg4, void *arg5), void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);
            unsigned long long time() { return bct_entity_time(entity_); }
            piw::data_nb_t allocate_host(unsigned long long ts,float u,float l,float r,unsigned t,unsigned dl, unsigned char **dp,unsigned vl, float **vp);
            piw::data_nb_t allocate_wire(unsigned dl, const unsigned char *dp);
        private:
            bct_entity_t entity_;
    };

    class PIW_DECLSPEC_CLASS tsd_protect_t: public pic::nocopy_t
    {
        public:
            tsd_protect_t(): entity_(tsd_getcontext()) {}
            ~tsd_protect_t() { tsd_setcontext(entity_); }
        private:
            bct_entity_t entity_;
    };
};

#endif
