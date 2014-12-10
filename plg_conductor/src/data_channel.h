/*
 Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __CDTR_DATA_CHANNEL_H__
#define __CDTR_DATA_CHANNEL_H__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>

#include "conductor_impl.h"

namespace cdtr
{
    class data_wire_t;

    class data_channel_t: public piw::root_t, public pic::element_t<3>, public virtual pic::lckobject_t 
    {
        public:
            data_channel_t(cdtr::conductor_t::impl_t *root, const unsigned index);
            virtual ~data_channel_t();
    
            cdtr::conductor_t::impl_t *get_conductor();
            unsigned get_index();
            unsigned long long get_recording_start();
    
            void invalidate();
    
            piw::wire_t *root_wire(const piw::event_data_source_t &es);
    
            void root_closed();
            void root_opened();
            void root_clock();
            void root_latency();
    
            void add_wire(const piw::data_t &path, cdtr::data_wire_t *child);
            void remove_wire(const piw::data_t &path);
            void activate_wire(cdtr::data_wire_t *wire);
            void deactivate_wire(cdtr::data_wire_t *wire);
    
            void ticked(unsigned long long f, unsigned long long t);
    
            void start_recording(unsigned long long t);
            void stop_recordings();

            void clear_controller();
        
        protected:
            virtual cdtr::data_wire_t *create_wire(unsigned index, const piw::data_t &path, const piw::event_data_source_t &es) = 0;
            virtual void delete_wire(cdtr::data_wire_t *wire) = 0;
            virtual void remove() = 0;
    
        private:
            cdtr::conductor_t::impl_t * const root_;
            const unsigned index_;
            volatile unsigned long long recording_start_;
            bct_clocksink_t *clock_;
    
            unsigned wire_index_;
    
            pic::flipflop_t<std::map<piw::data_t,cdtr::data_wire_t *> > wires_;
            pic::ilist_t<cdtr::data_wire_t,1> active_wires_;
    };
};

#endif
