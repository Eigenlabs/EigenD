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

#ifndef __PIA_SRC_WINDOW__
#define __PIA_SRC_WINDOW__

#include <pibelcanto/plugin.h>
#include <string>

struct pia_ctx_t;

class pia_windowlist_t
{
    public:
        struct impl_t;

    public:
        pia_windowlist_t();
        ~pia_windowlist_t();

        void kill(const pia_ctx_t &);
        void dump(const pia_ctx_t &);
        void window(const pia_ctx_t &, bct_window_t *);
        unsigned window_count();
        std::string window_title(unsigned w);
        void set_window_state(unsigned w, bool o);
        bool window_state(unsigned w);

    private:
        impl_t *impl_;
};

#endif
