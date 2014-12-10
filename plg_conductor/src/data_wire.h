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

#ifndef __CDTR_DATA_WIRE_H__
#define __CDTR_DATA_WIRE_H__

#include <piw/piw_bundle.h>

#include "recording.h"

namespace cdtr
{
    class data_channel_t;

    class data_wire_t: public piw::wire_t, public piw::event_data_sink_t, public pic::element_t<1>, public virtual pic::lckobject_t
    {
        public:
            data_wire_t(cdtr::data_channel_t *channel, const unsigned index, const piw::data_t &path, const piw::event_data_source_t &es);
            virtual ~data_wire_t();
    
            void wire_closed();
    
            cdtr::data_channel_t *get_channel();
            unsigned get_index();
    
            void initialize(unsigned long long t);
            void event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
            void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
            bool event_end(unsigned long long t);
    
            bool has_event();
    
            virtual void ticked(unsigned long long f, unsigned long long t);
            void handle_data(const piw::data_nb_t &d);
            void handle_controller(const piw::data_nb_t &d);
    
            void set_recording(cdtr::recording_t *rec);
            void clear_recording();
            cdtr::recording_t *get_recording();
    
        protected:
            virtual void activate();
            virtual void deactivate();
    
        private:
            static int __invalidator(void *w_, void *);
    
            cdtr::data_channel_t * const channel_;
    
            const unsigned index_;
            const piw::data_t path_;
    
            piw::data_nb_t event_id_;
    
            unsigned long long from_;
    
            piw::dataqueue_t data_queue_;
            piw::dataqueue_t controller_queue_;
            unsigned long long data_index_;
            unsigned long long controller_index_;
    
            pic::flipflop_t<cdtr::recording_t *> recording_;
    };
};

#endif
