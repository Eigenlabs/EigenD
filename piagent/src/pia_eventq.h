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

#ifndef __PIA_SRC_EVENTQ__
#define __PIA_SRC_EVENTQ__

#include <picross/pic_nocopy.h>
#include <picross/pic_ref.h>
#include <picross/pic_mlock.h>

#include "pia_data.h"

class pia_cpoint_t: public pic::atomic_counted_t, virtual public pic::lckobject_t
{
    public:
        pia_cpoint_t() {}
        virtual ~pia_cpoint_t() {}
        virtual const bool *lock() = 0;
        virtual void unlock(const bool *) = 0;
        virtual void disable() = 0;
};

class pia_sync_cpoint_t: public pia_cpoint_t
{
    public:
        pia_sync_cpoint_t(): enabled_(true) {}
        const bool *lock() { return enabled_.acquire(); }
        void unlock(const bool *v) { enabled_.release(v); }
        void disable() { enabled_.set(false); }

    private:
        pic::flipflop_t<bool> enabled_;
};

class pia_async_cpoint_t: public pia_cpoint_t
{
    public:
        pia_async_cpoint_t(): enabled_(true) {}
        const bool *lock() { return &enabled_; }
        void unlock(const bool *) {}
        void disable() { enabled_=false; }

    private:
        bool enabled_;
};

typedef pic::ref_t<pia_cpoint_t> pia_cref_t;

inline pia_cref_t pia_make_cpoint() { return pic::ref(new pia_async_cpoint_t); }
inline pia_cref_t pia_make_sync_cpoint() { return pic::ref(new pia_sync_cpoint_t); }

class pia_cguard_t
{
    public:
        pia_cguard_t(const pia_cref_t &c): c_(c), v_(c_->lock()) {}
        ~pia_cguard_t() { c_->unlock(v_); }
        bool enabled() { return *v_; }

    private:
        pia_cref_t c_;
        const bool *v_;
};

class pia_eventq_t: pic::nocopy_t, virtual public pic::lckobject_t
{
    public:
        class impl_t;

    public:
        pia_eventq_t(pic::nballocator_t *,void (*pinger)(void *), void *);
        ~pia_eventq_t();

        void idlecall(const pia_cref_t &cp, void (*cb)(void *), void *ctx);
        void idle(const pia_cref_t &cp, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t &d);
        void fastjob(const pia_cref_t &cp, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t &d);
        void slowjob(const pia_cref_t &cp, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t &d);
        void timer(const pia_cref_t &cp, void (*cb)(void *), void *ctx, unsigned long ms, long us=0);

        bool run(unsigned long long now);
        unsigned long long next();

    private:
        friend class pia_job_t;
        impl_t *impl_;
};

class pia_eventq_nb_t: pic::nocopy_t, virtual public pic::lckobject_t
{
    public:
        class impl_nb_t;

    public:
        pia_eventq_nb_t(pic::nballocator_t *,void (*pinger)(void *), void *);
        ~pia_eventq_nb_t();

        void idlecall(const pia_cref_t &cp, void (*cb)(void *), void *ctx);
        void idle(const pia_cref_t &cp, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t &d);
        void fastjob(const pia_cref_t &cp, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t &d);
        void slowjob(const pia_cref_t &cp, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t &d);
        void timer(const pia_cref_t &cp, void (*cb)(void *), void *ctx, unsigned long ms, long us=0);

        bool run(unsigned long long now);
        unsigned long long next();

    private:
        friend class pia_job_nb_t;
        impl_nb_t *impl_;
};

class pia_job_t: pic::nocopy_t, virtual public pic::lckobject_t
{
    public:
        pia_job_t() {}
        ~pia_job_t() { cancel(); }

        void cancel() { if(cpoint_.isvalid()) { cpoint_->disable(); cpoint_.clear(); } }

        void slowjob(pia_eventq_t *l, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t &d);
        void idlecall(pia_eventq_t *l, void (*cb)(void *), void *ctx);
        void idle(pia_eventq_t *l, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t &d);
        void fastjob(pia_eventq_t *l, void (*cb)(void *, const pia_data_t &), void *ctx, const pia_data_t &d);
        void timer(pia_eventq_t *l, void (*cb)(void *), void *ctx, unsigned long ms, long us=0);
    
    private:
        pia_cref_t cpoint_;
};

class pia_job_nb_t: pic::nocopy_t, virtual public pic::lckobject_t
{
    public:
        pia_job_nb_t() {}
        ~pia_job_nb_t() { cancel(); }

        void cancel() { if(cpoint_.isvalid()) { cpoint_->disable(); cpoint_.clear(); } }

        void slowjob(pia_eventq_nb_t *l, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t &d);
        void idlecall(pia_eventq_nb_t *l, void (*cb)(void *), void *ctx);
        void idle(pia_eventq_nb_t *l, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t &d);
        void fastjob(pia_eventq_nb_t *l, void (*cb)(void *, const pia_data_nb_t &), void *ctx, const pia_data_nb_t &d);
        void timer(pia_eventq_nb_t *l, void (*cb)(void *), void *ctx, unsigned long ms, long us=0);
    
    private:
        pia_cref_t cpoint_;
};

#endif
