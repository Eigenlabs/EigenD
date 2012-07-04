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

#ifndef __PIW_CHANNELISER__
#define __PIW_CHANNELISER__

#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS channeliser_t
    {
        public:
            class impl_t;

            class channel_t
            {
                public:
                    virtual ~channel_t() {}
                    virtual cookie_t cookie() = 0;
                    virtual void visited(const void *) {}

                private:
                    cookie_t cookie_;
            };

            class delegate_t
            {
                public:
                    virtual channel_t *create_channel(const cookie_t &output_cookie) = 0;
                    virtual void delete_channel(channel_t *channel) { delete channel; }
            };

        public:
            channeliser_t(piw::clockdomain_ctl_t *cd, delegate_t *delegate, const cookie_t &output_cookie, unsigned initial_poly=2, unsigned poly_minfree=1);
            ~channeliser_t();
            cookie_t cookie();
            void visit_channels(const void *arg);

        private:
            impl_t *impl_;
    };
};

#endif

