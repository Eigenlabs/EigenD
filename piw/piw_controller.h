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

#ifndef __PIW_CONTROLLER__
#define __PIW_CONTROLLER__

#include "piw_exports.h"
#include <piw/piw_bundle.h>
#include <piw/piw_fastdata.h>
#include <picross/pic_weak.h>

namespace piw
{
    class xcontrolled_t;

    class PIW_DECLSPEC_CLASS controller_t: public pic::nocopy_t
    {
        public:
            class impl_t;
            class control_t;
            class ctlsignal_t;

        public:
            controller_t(const cookie_t &cookie,const std::string &sigmap);
            virtual ~controller_t();
            virtual void controller_clock(bct_clocksink_t *) {}
            virtual void controller_latency(unsigned) {}

            cookie_t event_cookie();
            cookie_t control_cookie();

            impl_t *impl_;
            control_t *control_;
    };

    class PIW_DECLSPEC_CLASS xxcontrolled_t: public pic::nocopy_t
    {
        friend class controller_t::impl_t;
        friend class controller_t::ctlsignal_t;

        public:
            class impl_t;

        public:
            xxcontrolled_t();
            virtual ~xxcontrolled_t();
            void attach(controller_t *);
            bool attach_to(controller_t *,unsigned n);
            void detach();
            int ordinal();
            void set_light(unsigned index, unsigned value);
            void save_lights();
            void restore_lights();
            void set_key(const piw::data_t &key);
            fastdata_t *fastdata();

            virtual void control_init() {}
            virtual void control_start(unsigned long long t) {}
            virtual void control_receive(unsigned s,const piw::data_nb_t &value) {}
            virtual void control_term(unsigned long long t) {}

            change_nb_t sender();
            void send(const piw::data_nb_t &d);
            void start_event(unsigned long long t);
            void end_event(unsigned long long t);

        private:
            static int __destruct(void *, void *);

            controller_t *controller_;
            unsigned index_;
            bool active_;

            impl_t *xximpl_;
    };

    class PIW_DECLSPEC_CLASS xcontrolled_t: public xxcontrolled_t
    {
        protected:
            // starts and stops output events to correspond to input events
            void control_start(unsigned long long t)  { start_event(t); }
            void control_term(unsigned long long t)   { end_event(t); }
    };

    class PIW_DECLSPEC_CLASS controlled_t: public xxcontrolled_t
    {
        public:
            controlled_t(bool);
            ~controlled_t();
            virtual void control_enable() {}
            virtual void control_disable() {}
            void enable();
            void disable();
            virtual void control_init();
            virtual void control_receive(unsigned s,const piw::data_nb_t &value);

        private:
            static int __enable_direct(void *,void *);
            static int __disable_direct(void *,void *);

            bool enabled_;
    };

    class PIW_DECLSPEC_CLASS fasttrigger_t: public xxcontrolled_t, public pic::tracked_t
    {
        public:
            fasttrigger_t(unsigned color);
            void ping();
            change_t trigger() { return change_t::method(this,&fasttrigger_t::trigger__); }
            virtual void control_init();
            virtual void control_receive(unsigned s,const piw::data_nb_t &value);
            virtual void control_start(unsigned long long t);
            virtual void control_term(unsigned long long t);
        private:
            static int __ping_direct(void *c_, void *_);
            void trigger__(const piw::data_t &d);
            unsigned color_;
            int active_;
    };
};

#endif
