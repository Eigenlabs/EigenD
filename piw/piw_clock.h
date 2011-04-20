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

#ifndef __PIW_CLOCK__
#define __PIW_CLOCK__

#include "piw_exports.h"

#include <pibelcanto/plugin.h>
#include <picross/pic_error.h>

#include "piw_data.h"


#include <picross/pic_weak.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_log.h>
#include <picross/pic_xref.h>
#include <picross/pic_functor.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS clocksource_t: public pic::nocopy_t, public bct_clocksource_t, virtual public pic::tracked_t
    {
        public:
            clocksource_t();
            virtual ~clocksource_t();
            void tick(unsigned long long time);
            void tick_slow(unsigned long long time);
            void set_details(unsigned bs, unsigned long sr);
            void close_source();
            bool open() { return host_ops != 0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

        protected:
            virtual void clocksource_closed();

        private:
            static void closed_thunk(bct_clocksource_t *, bct_entity_t);
            static bct_clocksource_plug_ops_t dispatch__;
    };

    class PIW_DECLSPEC_CLASS clocksink_t: public pic::nocopy_t, public bct_clocksink_t, virtual public pic::tracked_t
    {
        public:
            clocksink_t();
            virtual ~clocksink_t();
            void add_upstream(bct_clocksink_t *up);
            void remove_upstream(bct_clocksink_t *up);
            void add_downstream(bct_clocksink_t *down);
            void remove_downstream(bct_clocksink_t *down);
            void close_sink();
            bool open() { return host_ops != 0; }
            void tick_enable(bool s);
            void tick_disable();
            void tick_suppress(bool s);
            void set_sink_latency(unsigned l) { latency_=l; }
            unsigned get_sink_latency() { return latency_; }
            void tick();
            void tick2(void (*)(unsigned long long,unsigned long long,void *),void *);
            unsigned long long current_tick();
            unsigned get_buffer_size();
            unsigned long get_sample_rate();
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

        protected:
            virtual void clocksink_ticked(unsigned long long f, unsigned long long t) {}
            virtual void clocksink_closed();

        private:
            static void ticked_thunk(bct_clocksink_t *, bct_entity_t, unsigned long long, unsigned long long);
            static void closed_thunk(bct_clocksink_t *, bct_entity_t);

            static bct_clocksink_plug_ops_t dispatch__;
            unsigned latency_;
    };

    class PIW_DECLSPEC_CLASS clockdomain_t: public pic::nocopy_t, public bct_clockdomain_t, virtual public pic::tracked_t
    {
        public:
            clockdomain_t();
            virtual ~clockdomain_t();
            void set_source(const piw::data_t &name);
            piw::data_t source_name();
            unsigned buffer_size();
            unsigned long sample_rate();
            void sink(bct_clocksink_t *s,const char *n);
            void close_domain();
            bool open() { return host_ops != 0; }
            bool opening() { return plg_state==PLG_STATE_ATTACHED; }
            bool closing() { return plg_state==PLG_STATE_DETACHED; }
            bool running() { return open() && !closing(); }

        protected:
            virtual void clockdomain_source_changed();
            virtual void clockdomain_closed();

        private:
            static void source_changed_thunk(bct_clockdomain_t *, bct_entity_t);
            static void closed_thunk(bct_clockdomain_t *, bct_entity_t);
            static bct_clockdomain_plug_ops_t dispatch__;
    };

    class PIW_DECLSPEC_CLASS  clockdomain_ctl_t: public pic::nocopy_t, virtual public pic::tracked_t
    {
        public:
            class impl_t;

        public:
            clockdomain_ctl_t();
            ~clockdomain_ctl_t();
            void set_source(const piw::data_t &src);
            void sink(bct_clocksink_t *s,const char *n);
            piw::data_t get_source();
            unsigned get_buffer_size();
            unsigned long get_sample_rate();

            void add_listener(const pic::notify_t &l);
            void remove_listener(const pic::notify_t &l);

            int gc_traverse(void *, void *) const;
            int gc_clear();

        private:
            impl_t *domain_;
    };
};

#endif
