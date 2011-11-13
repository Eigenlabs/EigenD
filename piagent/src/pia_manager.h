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

#ifndef __PIA_MANAGER__
#define __PIA_MANAGER__

#include <pibelcanto/plugin.h>

#include <piagent/pia_network.h>
#include <piagent/pia_scaffold.h>

#include <picross/pic_fastalloc.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_functor.h>
#include <picross/pic_fastalloc.h>

namespace pia
{
	class context_t;

    class controller_t: virtual public pic::lckobject_t
    {
        public:
            virtual ~controller_t() {}
            virtual void service_fast() = 0;
            virtual void service_main() = 0;
            virtual void service_ctx(int grp) = 0;
            virtual void service_gone() = 0;
            virtual bool service_isfast() = 0;
            virtual pic::f_string_t service_context(bool isgui, const char *tag, int *grp) = 0;
    };

	class manager_t: public pic::nocopy_t, virtual public pic::lckobject_t
	{
		public:
            class impl_t;
		public:
			manager_t(controller_t *, pic::nballocator_t *, pia::network_t *, const pic::f_string_t &log, const pic::f_string_t &winch, void *winctx = 0);
			virtual ~manager_t();

            void process_fast(unsigned long long, unsigned long long *timer, bool *activity);
            void process_main(unsigned long long, unsigned long long *timer, bool *activity);
            void process_ctx(int grp,unsigned long long, bool *activity);
            unsigned window_count();
            std::string window_title(unsigned);
            void set_window_state(unsigned,bool);
            bool window_state(unsigned);
            bool global_lock();
            void global_unlock();
            void fast_pause();
            void fast_resume();

			context_t context(int grp, const char *user, const pic::status_t &gone, const pic::f_string_t &log, const char *tag = ""); 

            impl_t *impl() { return impl_; }
		private:
			impl_t *impl_;
	};


};

#endif
/* vim: set filetype=cpp : */
