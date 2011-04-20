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

#ifndef __PIA_SRC_GENERIC__
#define __PIA_SRC_GENERIC__

#include <pibelcanto/plugin.h>

#include <picross/pic_ref.h>
#include <picross/pic_ilist.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_stl.h>

#include <map>

#include "pia_glue.h"

#define CNODE_CLIENTS 0
#define CNODE_SYNCS 1
#define CNODE_RECEIVE 2
#define CNODE_CHILDREN 3
#define CNODE_CLOCKS 4
#define CNODE_ROOTS 5
#define CNODE_SCLIENTS 6

struct cnode_t;
struct pia_serverlocal_t;
struct pia_serverglobal_t;


class pia_server_t
{
    public:
        typedef pic::lckmap_t<unsigned char, pia_server_t *>::type children_t;

        struct fast_receiver_t: pic::atomic_counted_t, virtual pic::lckobject_t
        {
            virtual bool evt_received(const pia_data_nb_t &,const pia_dataqueue_t &) = 0;
            virtual bool data_received(const pia_data_nb_t &) = 0;
        };

    public:
        pia_server_t(const pia_data_t &addr, const pia_ctx_t &e, const pia_data_t & data, unsigned long seq);
        pia_server_t(pia_server_t *parent, unsigned char last, const pia_data_t & data, unsigned long seq);

        virtual ~pia_server_t();

        virtual void server_closed() {}
        virtual void server_empty() {}
        virtual void server_childgone() {}
        virtual void server_changed(const pia_data_t &) {}
        virtual void server_childadded() {}
        virtual pic::ref_t<fast_receiver_t> server_fast() = 0;
        virtual unsigned server_fastflags() = 0;

        void close();
        int isopen();

        void filter(void *arg, int (*func)(void *, unsigned char, pia_server_t *));
        int visit(void *arg, int (*func)(void *, unsigned char, pia_server_t *));
        unsigned long dseq() { return dseq_; }
        unsigned long tseq() { return tseq_; }
        unsigned long nseq() { return nseq_; }
        void sync(bool);
        bool insync();
        void change_slow(const pia_data_t &, unsigned long seq);
        void set_nseq(unsigned long seq);
        void kill(const pia_ctx_t &e);
        void dump(const pia_ctx_t &e);
        void add(const pia_ctx_t &e, bct_client_t *c);
        unsigned char enum_visible(unsigned char name);
        pia_server_t *find(const unsigned char *, unsigned, pia_server_t **);
        pia_server_t *findvisible(const unsigned char *, unsigned);
        pia_server_t *root() { return root_; }
        pia_server_t *parent() { return parent_; }
        bool isroot() { return parent_==0; }
        bool isfast() { return fast_!=0; }
        const pia_data_t &current() { return current_slow_; }
        const pia_data_t &path();
        const pia_data_t &addr();
        const unsigned char *pathdata();
        void setcookie(unsigned short);
        unsigned short getcookie();
        void setclock(bct_clocksink_t *);
        unsigned visible_children();
        bool isvisible() { return visible_; }
        void setvisible(bool);
        unsigned flags() { return flags_; }
        void set_flags(unsigned);

        const pia_ctx_t &entity();
        pia::manager_t::impl_t *glue() { return entity()->glue(); }
        pia_fastdata_t *fastdata() { PIC_ASSERT(fast_); return fast_; }

    private:
        friend struct cnode_t;

        void childadded(pia_server_t *);
        void childgone(pia_server_t *);
        void close0(bool);
        void dropclients();
        void init(const pia_data_t &,unsigned long);
        void recalc_tseq();
        static void clockgone(void *);
        void schedule_setclock();
        void dirty();

        pia_fastdata_t *fast_;
        pic::flipflop_t<bool> open_;
        pic::flipflop_t<children_t> children_;
        pic::ilist_t<cnode_t,CNODE_CLIENTS> clients_;
        pic::ilist_t<cnode_t,CNODE_SCLIENTS> sclients_;
        pia_data_t current_slow_;
        unsigned long tseq_,nseq_,dseq_;
        pia_server_t *parent_;
        pic::ref_t<pia_serverlocal_t> local_;
        pia_server_t *root_;
        unsigned visible_children_;
        unsigned flags_;
        bool visible_;
};


#endif
