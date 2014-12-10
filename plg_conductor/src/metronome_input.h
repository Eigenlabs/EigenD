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

#ifndef __CDTR_METRONOME_INPUT_H__
#define __CDTR_METRONOME_INPUT_H__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>

#include "conductor.h"

#define CLK_SONGBEAT 0
#define CLK_BARBEAT 1 

namespace cdtr
{
    class metronome_input_t: public piw::root_t, public piw::wire_t, public piw::event_data_sink_t, public virtual pic::lckobject_t 
    {
        public:
            metronome_input_t(cdtr::conductor_t::impl_t *root);
            ~metronome_input_t();
    
            void invalidate();
    
            piw::wire_t *root_wire(const piw::event_data_source_t &);
    
            void root_closed();
            void root_opened();
            void root_clock();
            void root_latency();
    
            void wire_closed();
    
            void event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
            void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
            bool event_end(unsigned long long t);
    
            void ticked(unsigned long long t);

            bool is_running();
    
        private:
            cdtr::conductor_t::impl_t * const root_;
            bct_clocksink_t *clock_;
            bool running_;
    
            piw::xevent_data_buffer_t::iter_t iterator_;
    };
};

#endif
