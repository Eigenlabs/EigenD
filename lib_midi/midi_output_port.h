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

#ifndef __MIDI_OUTPUT_PORT_H__
#define __MIDI_OUTPUT_PORT_H__

#include <piw/piw_data.h>
#include <midilib_exports.h>

namespace midi
{
    class MIDILIB_DECLSPEC_CLASS midi_output_port_t
    {
        public:
            class impl_t;

        public:
            midi_output_port_t();
            virtual ~midi_output_port_t();
            
            bool set_port(long);
            long get_port(void);
            piw::change_nb_t get_midi_output_functor();
            
            void set_source(const std::string &name);
            virtual void sink_added(long uid, const std::string &name) {}
            virtual void sink_removed(long uid) {}
            
            void run();
            void stop();

        private:
            impl_t *impl_;
    };
};

#endif /* __MIDI_OUTPUT_PORT_H__ */
