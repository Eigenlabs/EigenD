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

#ifndef __PIA_SRC_PROXY__
#define __PIA_SRC_PROXY__

#include <pibelcanto/plugin.h>

struct pia_ctx_t;
class pia_data_t;

struct pia_proxylist_t
{
    public:
        struct impl_t;

    public:
        pia_proxylist_t();
        ~pia_proxylist_t();

        void dump(const pia_ctx_t &e);
        void kill(const pia_ctx_t &e);
        void killserver(const pia_data_t &name);
        void client(const pia_data_t &name, const pia_ctx_t &e, bct_client_t *c,bool fast);

    private:
        impl_t *impl_;
};

#endif
