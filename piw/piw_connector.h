
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

#ifndef __PIW_CONNECTOR__
#define __PIW_CONNECTOR__
#include "piw_exports.h"
#include <piw/piw_client.h>
#include <piw/piw_correlator.h>

namespace piw
{
    class backend_t;

    class PIW_DECLSPEC_CLASS connector_t: public client_t
    {
        public:
            connector_t(bool ctl, backend_t *dbackend,backend_t *cbackend, const d2d_nb_t &filter,bool iso);
            virtual ~connector_t();

            int gc_traverse(void *, void *) const;
            int gc_clear();

        protected:
            void client_opened();
            void client_clock();
            void close_client();

        public:
            class impl_t;

        private:
            impl_t *impl_;
    };

    class PIW_DECLSPEC_CLASS backend_t: virtual public pic::tracked_t
    {
        public:
            backend_t(correlator_t *correlator, unsigned id, unsigned signal, int pri, unsigned type, bool clock);

            virtual converter_ref_t create_converter(bool iso) = 0;
            virtual ~backend_t();
            void set_clocked(bool c);
            void set_latency(unsigned l);
            void remove_latency();

        private:
            friend class piw::connector_t::impl_t;

        private:
            correlator_t *correlator_;
            unsigned iid_;
            unsigned signal_;
            int pri_;
            unsigned type_;
            bool clock_;
    };

}

#endif

