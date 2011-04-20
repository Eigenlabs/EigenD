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

#ifndef __PISESSION_LOADER__
#define __PISESSION_LOADER__

#include <picross/pic_ref.h>
#include <piagent/pia_scaffold.h>

namespace pisession
{
    class interpreter_it;
    class agent_it;

    typedef pic::ref_t<interpreter_it> interpreter_t;
    typedef pic::ref_t<agent_it> agent_t;

    class interpreter_it: virtual public pic::counted_t
    {
        public:
            virtual agent_t create_agent(const pia::context_t &, const char *module, const char *name) = 0;

        protected:
            virtual ~interpreter_it() {}
            virtual void counted_deallocate() {}
    };

    class agent_it : virtual public pic::counted_t
    {
        public:
            virtual pia::context_t env() = 0;
            virtual std::string unload() = 0;

        protected:
            virtual ~agent_it() {}
            virtual void counted_deallocate() {}
    };

    interpreter_t create_interpreter(const char *zip, const char *boot, const char *setup);
}

#endif
