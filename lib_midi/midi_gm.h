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

#ifndef __MIDI_GM__
#define __MIDI_GM__

#define MIDI_CC_MAX 128
#define MIDI_STATUS_MAX 4 
#define MIDI_EIGEND_MAX 1 

namespace midi
{
    enum midi_eigend_t {LEGATO_TRIGGER};

    static const std::string midi_eigend[MIDI_EIGEND_MAX] = {
        "Legato Trigger"
    };

    enum midi_status_t {POLY_AFTERTOUCH, PROGRAM_CHANGE, CHANNEL_AFTERTOUCH, PITCH_WHEEL};

    static const std::string midi_status[MIDI_STATUS_MAX] = {
        "Polyphonic Aftertouch",
        "Program Change",
        "Channel Aftertouch",
        "Pitch Wheel"
    };

    static const std::string midi_cc[MIDI_CC_MAX] = {
        "Bank Select",
        "Modulation Wheel",
        "Breath Controller",
        "Undefined",
        "Foot Controller",
        "Portamento Time",
        "Data Entry MSB",
        "Channel Volume",
        "Balance",
        "Undefined",
        "Pan",
        "Expression Controller",
        "Effect Control 1",
        "Effect Control 2",
        "Undefined",
        "Undefined",
        "General Purpose Controller 1",
        "General Purpose Controller 2",
        "General Purpose Controller 3",
        "General Purpose Controller 4",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "LSB for 0 (Bank Select)",
        "LSB for 1 (Modulation Wheel)",
        "LSB for 2 (Breath Controller)",
        "LSB for 3 (Undefined)",
        "LSB for 4 (Foot Controller)",
        "LSB for 5 (Portamento Time)",
        "LSB for 6 (Data Entry)",
        "LSB for 7 (Channel Volume)",
        "LSB for 8 (Balance)",
        "LSB for 9 (Undefined)",
        "LSB for 10 (Pan)",
        "LSB for 11 (Expression Controller)",
        "LSB for 12 (Effect control 1)",
        "LSB for 13 (Effect control 2)",
        "LSB for 14 (Undefined)",
        "LSB for 15 (Undefined)",
        "LSB for 16 (General Purpose Controller 1)",
        "LSB for 17 (General Purpose Controller 2)",
        "LSB for 18 (General Purpose Controller 3)",
        "LSB for 19 (General Purpose Controller 4)",
        "LSB for 20 (Undefined)",
        "LSB for 21 (Undefined)",
        "LSB for 22 (Undefined)",
        "LSB for 23 (Undefined)",
        "LSB for 24 (Undefined)",
        "LSB for 25 (Undefined)",
        "LSB for 26 (Undefined)",
        "LSB for 27 (Undefined)",
        "LSB for 28 (Undefined)",
        "LSB for 29 (Undefined)",
        "LSB for 30 (Undefined)",
        "LSB for 31 (Undefined)",
        "Damper Pedal on/off (Sustain)",
        "Portamento On/Off",
        "Sostenuto On/Off",
        "Soft Pedal On/Off",
        "Legato Footswitch",
        "Hold 2",
        "Sound Controller 1 (Sound Variation)",
        "Sound Controller 2 (Timbre/Harmonic Intens.)",
        "Sound Controller 3 (Release Time)",
        "Sound Controller 4 (Attack Time)",
        "Sound Controller 5 (Brightness)",
        "Sound Controller 6 (Decay Time)",
        "Sound Controller 7 (Vibrato Rate)",
        "Sound Controller 8 (Vibrato Depth)",
        "Sound Controller 9 (Vibrato Delay)",
        "Sound Controller 10 (undefined)",
        "General Purpose Controller 5",
        "General Purpose Controller 6",
        "General Purpose Controller 7",
        "General Purpose Controller 8",
        "Portamento Control",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Effects 1 Depth",
        "Effects 2 Depth",
        "Effects 3 Depth",
        "Effects 4 Depth",
        "Effects 5 Depth",
        "Data Increment (Data Entry +1)",
        "Data Decrement (Data Entry -1)",
        "Non-Registered Parameter Number - LSB",
        "Non-Registered Parameter Number - MSB",
        "Registered Parameter Number - LSB",
        "Registered Parameter Number - MSB",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "All Sound Off",
        "Reset All Controllers",
        "Local Control On/Off",
        "All Notes Off",
        "Omni Mode Off (+ all notes off)",
        "Omni Mode On (+ all notes off)",
        "Mono Mode On (+ poly off, + all notes off)",
        "Poly Mode On (+ mono off, +all notes off)"
    };
} // namespace midi


#endif /* __MIDI_GM__ */
