
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

#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_midi_decoder.h>
#include <piw/piw_bundle.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_config.h>

#include <plg_midi/midi_input.h>

#include <iostream>
#include <iomanip>

#define MIDI_INPUT_DEBUG 0
#define KEYS 128
#define KBD_LATENCY 5000

namespace
{
    // keyboard data struct
    struct blob_t
    {
        unsigned c,k,v;
    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // keyboard wire class
    //
    // sends keyboard data converted from midi
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct keyboard_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
    {
        keyboard_wire_t(unsigned k): event_data_source_real_t(piw::pathone(k,0)), active_(false), id_(k), min_(0), max_(127), invert_(false), current_(0)
        {
        }

        ~keyboard_wire_t()
        {
            source_shutdown();
        }

        void source_ended(unsigned seq)
        {
        }

        void sendcc(unsigned om, unsigned long long t)
        {
            current_=om;
            unsigned m = om;


            if(m<min_) m=min_;
            if(m>max_) m=max_;

            m = m-min_;

            if(invert_) m=max_-min_-m;

            piw::data_nb_t d = piw::makefloat_bounded_nb(1,0,0,((float)m)/(float)(max_-min_),t);

            if(m)
            {
                if(!active_)
                {
                    output_=piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_TINY);
                    output_.add_value(2,d);
                    source_start(0,piw::pathone_nb(id_,t),output_);
                    //pic::logmsg() << "midi start " << 2 << " " << id_ << " " << d;
                    active_=true;
                }
                else
                {
                    output_.add_value(2,d);
                    //pic::logmsg() << "midi continues " << 2 << " " << id_ << " " << d;
                }
            }
            else
            {
                if(active_)
                {
                    output_.add_value(2,d);
                    source_end(t+1);
                    //pic::logmsg() << "midi end " << 2 << " " << id_ << " " << d;
                    active_=false;
                }
            }

        }

        void sendkey(unsigned m, unsigned long long t)
        {
            unsigned act=(m==0)?0:((m>63)?3:2);

            piw::data_nb_t d = piw::makefloat_bounded_nb(3,0,0,act,t);

            if(m)
            {
                if(!active_)
                {
                    output_=piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_TINY);
                    output_.add_value(1,d);
                    source_start(0,piw::pathone_nb(id_,t),output_);
                    //pic::logmsg() << "midi start " << id_ << " " << d;
                    active_=true;
                }
                else
                {
                    output_.add_value(1,d);
                    //pic::logmsg() << "midi continues " << 1 << " " << id_ << " " << d;
                }
            }
            else
            {
                if(active_)
                {
                    output_.add_value(1,d);
                    source_end(t+1);
                    //pic::logmsg() << "midi end " << id_ << " " << d;
                    active_=false;
                }
            }

        }

        bool active_;
        unsigned id_;
        float min_;
        float max_;
        bool invert_;
        unsigned current_;
        piw::xevent_data_buffer_t output_;
    };


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // keyboard class
    //
    // Has a midiwire for each key in the keyboard
    // Is a midi receiver and decoder
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct keyboard_t: piw::root_ctl_t, piw::thing_t, piw::mididecoder_t
    {
        keyboard_t()
        {
            piw::tsd_thing(this);

            set_latency(KBD_LATENCY);

            for(unsigned i=0;i<KEYS;i++)
            {
                wires_[i] = pic::ref(new keyboard_wire_t(i+1));
            }
        }

        ~keyboard_t()
        {
            tracked_invalidate();
            unplumb();
        }


        void plumb()
        {
            for(unsigned k=0; k<KEYS; k++)
            {
                connect_wire(wires_[k].ptr(),wires_[k]->source());
            }
        }

        void unplumb()
        {
        }

        // ------------------------------------------------------------------------------
        // receiver virtual functions
        // ------------------------------------------------------------------------------

        void midi_receive(const unsigned char *buffer, unsigned length)
        {
            // pass buffer to the piw midi decoder
            decoder_input(buffer,length);
        }


        // ------------------------------------------------------------------------------
        // decoder virtual functions
        // ------------------------------------------------------------------------------

        void decoder_noteon(unsigned channel, unsigned number, unsigned velocity)
        {
            //pic::logmsg() << "note on " << channel << "," << number << "," << velocity;
            unsigned long long t = piw::tsd_time();
            kbd_enqueue(t,number,velocity);
        }

        void decoder_noteoff(unsigned channel, unsigned number, unsigned velocity)
        {
            //pic::logmsg() << "note off " << channel << "," << number << "," << velocity;
            kbd_enqueue(piw::tsd_time(),number,0);
        }

        void decoder_cc(unsigned channel, unsigned number, unsigned value)
        {
            //pic::logmsg() << "cc " << channel << "," << number << "," << value;
            cc_enqueue(piw::tsd_time(),number,value);
        }

        void kbd_enqueue(unsigned long long t, unsigned key, unsigned val)
        {
            // enqueue keyboard data
            blob_t *blob;
            piw::data_nb_t data = piw::makeblob_nb(t,sizeof(blob_t),(unsigned char **)&blob);

            blob->k=(key>0)?key:128;
            blob->v=val;
            blob->c=0;

            enqueue_fast(data);
        }

        void cc_enqueue(unsigned long long t, unsigned cc, unsigned val)
        {
            blob_t *blob;
            piw::data_nb_t data = piw::makeblob_nb(t,sizeof(blob_t),(unsigned char **)&blob);

            blob->k=0;
            blob->v=val;
            blob->c=cc;

            enqueue_fast(data);
        }

        void thing_dequeue_fast(const piw::data_nb_t &d)
        {
            if(d.is_blob())
            {
                blob_t *blob = (blob_t *)d.as_blob();

                if(blob->k && blob->k < KEYS)
                    wires_[blob->k-1]->sendkey(blob->v,d.time());

                if(blob->c && blob->c < KEYS)
                    wires_[blob->c]->sendcc(blob->v,d.time());
            }
        }

        unsigned current(unsigned cc)
        {
            if(cc>0 && cc<=KEYS)
            {
                return wires_[cc-1]->current_;
            }

            return 0;
        }

        // set_trim: set key limits, called from agent
        void set_trim(unsigned cc, float min, float max, bool inv)
        {
            if(cc>0 && cc<=KEYS)
            {
                wires_[cc-1]->min_=min;
                wires_[cc-1]->max_=max;
                wires_[cc-1]->invert_=inv;
            }
        }

        // clear_trim: clear key limits, called from agent
        void clear_trim()
        {
            for(unsigned i=0;i<KEYS;i++)
            {
                wires_[i]->invert_=false;
                wires_[i]->min_=0;
                wires_[i]->max_=127;
            }
        }

        pic::ref_t<keyboard_wire_t> wires_[KEYS];
    };






    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi wire class
    //
    // output midi data wire
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct midi_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
    {
        midi_wire_t(): event_data_source_real_t(piw::pathnull(0))
        {
            output_=piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_NORM);
            unsigned long long t = piw::tsd_time();

            start_slow(piw::pathnull(t),output_);

        }

        ~midi_wire_t()
        {
            unsigned long long t = piw::tsd_time();
            end_slow(t);
            // TODO: is this needed?
            source_shutdown();
        }

        void send_midi(unsigned signal, unsigned char *buffer, unsigned length, unsigned long long t)
        {
            unsigned char *output_buffer;
            piw::data_nb_t midi_data = piw::makeblob_nb(t, length, &output_buffer);
            memcpy(output_buffer, buffer, length);

            output_.add_value(signal, midi_data);
        }

        piw::xevent_data_buffer_t output_;
    };


    // midi data buffer blob
    struct midi_buffer_blob_t
    {
        unsigned char buffer[3];
        unsigned length;
    };


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi stream class
    //
    // streams midi outputs from the midi input port, unfiltered and filtered (midi clock)
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct midi_stream_t: piw::root_ctl_t, piw::thing_t, piw::mididecoder_t
    {
        midi_stream_t() : buffer_p_(0), remaining_clock_bytes_(0)
        {
            piw::tsd_thing(this);

            // TODO: what latency?
            set_latency(KBD_LATENCY);

            wire_ = pic::ref(new midi_wire_t());
        }

        ~midi_stream_t()
        {
            tracked_invalidate();
            unplumb();
        }

        void plumb()
        {
            connect_wire(wire_.ptr(),wire_->source());
        }

        void unplumb()
        {
        }

        // ------------------------------------------------------------------------------
        // receiver virtual functions
        // ------------------------------------------------------------------------------
        void midi_receive(const unsigned char *buffer, unsigned length)
        {
#if MIDI_INPUT_DEBUG>1
            pic::logmsg() << "midi_receive l="<<length<<" b="<<(void *)buffer << " d0="<<hex<<(unsigned)buffer[0];
#endif // MIDI_INPUT_DEBUG>1

            if(length>3)
            {
                pic::logmsg() << "warning: MIDI packet dropped from stream, too long";
                return;
            }

            // pass buffer to the fast thread
            midi_buffer_blob_t *midi_buffer;

            unsigned long long t = piw::tsd_time();
            piw::data_nb_t d = piw::makeblob_nb(t,sizeof(midi_buffer_blob_t),(unsigned char **)&midi_buffer);
            midi_buffer->length = length;
            memcpy(midi_buffer->buffer, buffer, length);

            enqueue_fast(d);

        }

        // ------------------------------------------------------------------------------
        // thing virtual functions
        // ------------------------------------------------------------------------------
        void thing_dequeue_fast(const piw::data_nb_t &d)
        {
            // dequeue midi clock data into the output wire
            if(d.is_blob())
            {
                midi_buffer_blob_t *midi_buffer = (midi_buffer_blob_t *)d.as_blob();
                unsigned char *buffer = (unsigned char *)midi_buffer->buffer;
                unsigned length = midi_buffer->length;

#if MIDI_INPUT_DEBUG>1
//                pic::logmsg() << "midi dequeue l="<<length<<" b="<<(void *)buffer;
#endif // MIDI_INPUT_DEBUG>1

                // filter out midi clock messages and send out midi clock output
                filter_midi_clock(buffer, length, d.time());

#if MIDI_INPUT_DEBUG>1
                pic::logmsg() << "midi_input l=" << length << " t=" << d.time();
                for(unsigned i=0; i<length; i++)
                    pic::logmsg() << "d"<<i<<"=" << hex << (unsigned)buffer[i];
#endif // MIDI_INPUT_DEBUG>1

                wire_->send_midi(1, buffer, length, d.time());

            }
        }

        void filter_midi_clock(unsigned char *buffer, unsigned length, unsigned long long t)
        {
            // decode midi clock status bytes
            switch(buffer[0])
            {
            case 0xfa:
                // MIDI start
            case 0xfb:
                // MIDI continue
            case 0xfc:
                // MIDI stop
            case 0xf8:
                // MIDI clock
            case 0xf2:
                // MIDI song position pointer
                wire_->send_midi(2, buffer, length, t);
                break;
            }

        }

        unsigned buffer_p_;
        unsigned remaining_clock_bytes_;
        unsigned char buffer_[3];
        pic::ref_t<midi_wire_t> wire_;
    };

} // namespace


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi input implementation class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct pi_midi::midi_input_t::impl_t: pic::tracked_t, midi_input_port_t
{
    impl_t(pi_midi::midi_input_t *d,const piw::cookie_t &keyboard_cookie,const piw::cookie_t &midi_cookie): midi_input_port_t(piw::change_nb_t::method(this,&impl_t::receive__)), delegate_(d)
    {
        keyboard.connect(keyboard_cookie);
        keyboard.plumb();

        midistream.connect(midi_cookie);
        midistream.plumb();
    }

    void source_added(long uid, const std::string &name)
    {
        delegate_->source_added(uid,name);
    }

    void source_removed(long uid)
    {
        delegate_->source_removed(uid);
    }

    void receive__(const piw::data_nb_t &d)
    {
        unsigned len = d.as_bloblen();
        const unsigned char *buf = (const unsigned char *)(d.as_blob());
        keyboard.midi_receive(buf,len);
        midistream.midi_receive(buf,len);
    }

    ~impl_t()
    {
        stop();
        tracked_invalidate();
    }

    midi_input_t *delegate_;
    keyboard_t keyboard;
    midi_stream_t midistream;
};

pi_midi::midi_input_t::midi_input_t(const piw::cookie_t &keyboard_cookie,const piw::cookie_t &midi_cookie): root_(new impl_t(this,keyboard_cookie,midi_cookie))
{
}

unsigned pi_midi::midi_input_t::current(unsigned cc)
{
    return root_->keyboard.current(cc);
}

void pi_midi::midi_input_t::run()
{
    root_->run();
}

void pi_midi::midi_input_t::stop()
{
    root_->stop();
}

void pi_midi::midi_input_t::clear_trim()
{
    root_->keyboard.clear_trim();
}

void pi_midi::midi_input_t::set_trim(unsigned cc, float min, float max, bool invert)
{
    root_->keyboard.set_trim(cc,min,max,invert);
}

long pi_midi::midi_input_t::getport(void)
{
    return root_->getport();
}

bool pi_midi::midi_input_t::setport(long uid)
{
    return root_->setport(uid);
}

void pi_midi::midi_input_t::set_destination(const std::string &name)
{
    root_->set_destination(name);
}

pi_midi::midi_input_t::~midi_input_t()
{
    stop();
    delete root_;
}

int pi_midi::midi_input_t::gc_traverse(void *v, void *a) const
{
    int r;
    if((r=root_->keyboard.gc_traverse(v,a))!=0) return r;
    return 0;
}

int pi_midi::midi_input_t::gc_clear()
{
    root_->keyboard.gc_clear();
    return 0;
}
