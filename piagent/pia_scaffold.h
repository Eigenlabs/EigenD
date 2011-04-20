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

#ifndef __PIA_SCAFFOLD__
#define __PIA_SCAFFOLD__

#include "pia_exports.h"
#include <picross/pic_functor.h>
#include <picross/pic_config.h>
#include <pibelcanto/plugin.h>

namespace pia
{
	class PIA_DECLSPEC_CLASS context_t
	{
		public:
            class impl_t;

		public:
			context_t();
            context_t(impl_t *);
			context_t(const context_t &);
			context_t &operator=(const context_t &);
			~context_t();

			bct_entity_t entity() const;
            void lock();
            void unlock();

			void kill();
			void release();
            bool inuse();
            void trigger();

            impl_t *impl() { return impl_; }

		private:
			impl_t *impl_;
	};

    class PIA_DECLSPEC_CLASS scaffold_mt_t: pic::nocopy_t
    {
        public:
            scaffold_mt_t(const char *user, unsigned mt, const pic::f_string_t &log, const pic::f_string_t &winch, bool ck, bool rt);
            ~scaffold_mt_t();
			context_t context(const pic::status_t &gone, const pic::f_string_t &log, const char *tag = "");
            void wait();
            bool global_lock();
            void global_unlock();
        public:
            class impl_t;
        private:
            impl_t *impl_;
    };

    class PIA_DECLSPEC_CLASS scaffold_gui_t: pic::nocopy_t
    {
        public:
            scaffold_gui_t(const char *user, const pic::notify_t &service, const pic::notify_t &gone, const pic::f_string_t &log, const pic::f_string_t &winch, bool ck, bool rt);
            ~scaffold_gui_t();
			context_t context(const pic::status_t &gone, const pic::f_string_t &log, const char *tag = "");
			context_t bgcontext(const pic::status_t &gone, const pic::f_string_t &log, const char *tag = "");
            void process_ctx();
            unsigned cpu_usage();
            unsigned window_count();
            std::string window_title(unsigned);
            void set_window_state(unsigned,bool);
            bool window_state(unsigned);
            bool global_lock();
            void global_unlock();
            void fast_pause();
            void fast_resume();

        public:
            class impl_t;
        private:
            impl_t *impl_;
    };
};

#endif

/* vim: set filetype=cpp : */
