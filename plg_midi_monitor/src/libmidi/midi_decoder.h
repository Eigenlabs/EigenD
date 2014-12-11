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

#ifndef __MIDI_DECODER__
#define __MIDI_DECODER__

// CHANGED 
#define MIDILIB_DECLSPEC_CLASS
//#include <midilib_exports.h>

namespace midi
{
    class MIDILIB_DECLSPEC_CLASS mididecoder_t
    {
        private:
            enum state_t { START, STATUS, GET1OF1, GET1OF2, GET2OF2, GETSYSEX };

        public:
            mididecoder_t();
            virtual ~mididecoder_t() {}

            void decoder_input(unsigned char octet);
            void decoder_input(const unsigned char *buffer, unsigned length);

            virtual void decoder_noteoff(unsigned channel, unsigned number, unsigned velocity) {}
            virtual void decoder_noteon(unsigned channel, unsigned number, unsigned velocity) {}
            virtual void decoder_polypressure(unsigned channel, unsigned number, unsigned value) {}
            virtual void decoder_cc(unsigned channel, unsigned number, unsigned value) {}
            virtual void decoder_programchange(unsigned channel, unsigned value) {}
            virtual void decoder_channelpressure(unsigned channel, unsigned value) {}
            virtual void decoder_pitchbend(unsigned channel, unsigned value) {}
            virtual void decoder_generic1(bool, unsigned char) {}
            virtual void decoder_generic2(bool, unsigned char, unsigned char) {}
            virtual void decoder_generic3(bool, unsigned char, unsigned char, unsigned char) {}

        private:
            void decode1(unsigned char);
            void decode2(unsigned char, unsigned char);
            void decode3(unsigned char, unsigned char, unsigned char);
            void start_state(unsigned char);

        private:
            state_t _state;
            unsigned char _buffer[2];
    };

}; // namespace midi

#endif /* __MIDI_DECODER__ */
