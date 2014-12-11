
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

#include <picross/pic_log.h>

//CHANGED 
#include "midi_decoder.h"

midi::mididecoder_t::mididecoder_t()
{
    _state=START;
}

void midi::mididecoder_t::start_state(unsigned char octet)
{
    unsigned hi = (octet&0x70)>>4;
    static state_t __states[] = { GET1OF2, GET1OF2, GET1OF2, GET1OF2, GET1OF1, GET1OF1, GET1OF2 };

    if(hi<7)
    {
        _state = __states[hi];
        return;
    }

    unsigned lo = (octet&0x0f);

    if(lo>=8) { _state=START; return; }
    if(lo==0) { _state=GETSYSEX; return; }
    if(lo==2) { _state=GET1OF2; return; }
    if(lo==3) { _state=GET1OF1; return; }

    _state=START;
}

void midi::mididecoder_t::decode1(unsigned char b1)
{
    decoder_generic1(false,b1);
}

void midi::mididecoder_t::decode2(unsigned char b1,unsigned char b2)
{
    unsigned hi = (b1&0x70)>>4;
    unsigned lo = (b1&0x0f);
    bool h = false;
    switch(hi)
    {
        case 4:
            decoder_programchange(lo,b2);
            h = true;
            break;

        case 5:
            decoder_channelpressure(lo,b2);
            h = true;
            break;
    }
    decoder_generic2(h,b1,b2);
}

void midi::mididecoder_t::decode3(unsigned char b1,unsigned char b2,unsigned char b3)
{
    unsigned hi = (b1&0x70)>>4;
    unsigned lo = (b1&0x0f);
    bool h = false;
    switch(hi)
    {
        case 0:
			//pic::logmsg() << "decoder_input : decode3 noteoff" << hi << " " << lo << " " << (int) b1 << " " << (int) b2 << " " << (int) b3;;
            decoder_noteoff(lo,b2,b3);
            h = true;
            break;

        case 1:
			//pic::logmsg() << "decoder_input : decode3 noteon" << hi << " " << lo << " " << (int) b1 << " " << (int) b2 << " " << (int) b3;;
            if(b3>0)
            {
                decoder_noteon(lo,b2,b3);
                h=true;
                break;
            }
			//pic::logmsg() << "decoder_input : decode3 noteon odd" << hi << " " << lo << " " << (int) b1 << " " << (int) b2 << " " << (int) b3;;
            b3 = 0x7f;
            decoder_noteoff(lo,b2,b3);
			break;

        case 2:
            decoder_polypressure(lo,b2,b3);
            h = true;
            break;

        case 3:
            decoder_cc(lo,b2,b3);
            h = true;
            break;

        case 6:
            unsigned short p=((b3<<7)|(b2));
            decoder_pitchbend(lo,p);
            h = true;
            break;
    }

	//pic::logmsg() << "decoder_input : decode3 generic" << hi << " " << lo << " " << (int) b1 << " " << (int) b2 << " " << (int) b3;;
    decoder_generic3(h,b1,b2,b3);
}

void midi::mididecoder_t::decoder_input(const unsigned char *buffer, unsigned length)
{
    for(unsigned i=0;i<length;i++)
    {
        decoder_input(buffer[i]);
    }
}

void midi::mididecoder_t::decoder_input(unsigned char octet)
{
    bool cmd = (octet&0x80)!=0;
 	//pic::logmsg() << "decoder_input " << (int) octet;

restart:

    switch(_state)
    {
        case STATUS:

 			//pic::logmsg() << "decoder_input : status";
            _state = START;
            if(!cmd) decoder_input(_buffer[0]);
            goto restart;

        case START:

 			//pic::logmsg() << "decoder_input : start";
            if(!cmd) { return; }
            start_state(octet);
            _buffer[0] = octet;
            if(_state == START) { decode1(octet); }
            return;

        case GET1OF1:

 			//pic::logmsg() << "decoder_input : get1of1";
            if(cmd) { _state = START; goto restart; }
            _state = STATUS;
            decode2(_buffer[0],octet);
            return;

        case GET1OF2:

 			//pic::logmsg() << "decoder_input : get1of2";
            if(cmd) { _state = START; goto restart; }
            _buffer[1] = octet;
            _state = GET2OF2;
            return;

        case GET2OF2:

 			//pic::logmsg() << "decoder_input : get2of2";
            if(cmd) { _state = START; goto restart; }
            _state = STATUS;
            decode3(_buffer[0],_buffer[1],octet);
            return;

        case GETSYSEX:

 			//pic::logmsg() << "decoder_input : sysex";
            if((octet&0xf8) == 0xf8) { decode1(octet); return; }
            if(octet == 0xf7) { _state = START; return; }
            if(cmd) { _state = START; goto restart; }
            return;
    }

}
