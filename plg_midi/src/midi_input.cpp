
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
#include <piw/piw_bundle.h>
#include <piw/piw_keys.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_config.h>

#include <lib_midi/midi_decoder.h>
#include <lib_midi/midi_input_port.h>

#include <plg_midi/midi_input.h>

#include <iostream>
#include <iomanip>

#define MIDI_INPUT_DEBUG 0
#define WIRES 128
#define KBD_LATENCY 5000

namespace
{
    struct keyblob_t
    {
        unsigned key,ch,val;
    };
    struct ccblob_t
    {
        unsigned cc,ch,val;
    };
    struct pcblob_t
    {
        unsigned ch,val;
    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // wire class
    //
    // send data converted from midi
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct keyboard_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
    {
        keyboard_wire_t(unsigned k): event_data_source_real_t(piw::pathone(k,0)), active_(false), id_(k)
        {
            output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        }

        ~keyboard_wire_t()
        {
            source_shutdown();
        }

        void source_ended(unsigned seq)
        {
        }

        void sendkey(unsigned channel, unsigned m, unsigned long long t)
        {
            piw::hardness_t hardness = (m==0)?piw::KEY_LIGHT:((m>63)?piw::KEY_HARD:piw::KEY_SOFT);

            piw::data_nb_t d = piw::makekey(1,id_,1,id_,hardness,t);

            output_.add_value(1,d);
            if(m)
            {
                if(!active_)
                {
                    source_start(0,piw::pathone_nb(id_,t),output_);
#if MIDI_INPUT_DEBUG
                    pic::logmsg() << "midi key start " << id_ << " " << d;
#endif
                    active_ = true;
                }
#if MIDI_INPUT_DEBUG
                else
                {
                    pic::logmsg() << "midi key continues " << " " << id_ << " " << d;
                }
#endif
            }
            else
            {
                if(active_)
                {
                    source_end(t+1);
#if MIDI_INPUT_DEBUG
                    pic::logmsg() << "midi key end " << id_ << " " << d;
#endif
                    active_ = false;
                }
            }
        }

        piw::xevent_data_buffer_t output_;
        bool active_;
        unsigned id_;
    };

    struct cc_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
    {
        cc_wire_t(unsigned k): event_data_source_real_t(piw::pathone(k,0)), active_(false), id_(k), min_(0), max_(127), invert_(false), current_(0)
        {
            output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        }

        ~cc_wire_t()
        {
            source_shutdown();
        }

        void source_ended(unsigned seq)
        {
        }

        void sendcc(unsigned channel, unsigned om, unsigned long long t)
        {
            current_ = om;
            unsigned m = om;

            if(m<min_) m=min_;
            if(m>max_) m=max_;

            m = m-min_;

            if(invert_) m=max_-min_-m;

            piw::data_nb_t d = piw::makefloat_bounded_nb(1,0,0,((float)m)/(float)(max_-min_),t);

            output_.add_value(1,d);
            if(m)
            {
                if(!active_)
                {
                    source_start(0,piw::pathone_nb(id_,t),output_);
#if MIDI_INPUT_DEBUG
                    pic::logmsg() << "midi cc start " << id_ << " " << d;
#endif
                    active_ = true;
                }
#if MIDI_INPUT_DEBUG
                else
                {
                    pic::logmsg() << "midi cc continues " << id_ << " " << d;
                }
#endif
            }
            else
            {
                if(active_)
                {
                    source_end(t+1);
#if MIDI_INPUT_DEBUG
                    pic::logmsg() << "midi cc end " << id_ << " " << d;
#endif
                    active_ = false;
                }
            }

        }

        piw::xevent_data_buffer_t output_;
        bool active_;
        unsigned id_;
        float min_;
        float max_;
        bool invert_;
        unsigned current_;
    };

    struct programchange_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
    {
        programchange_wire_t(unsigned k): event_data_source_real_t(piw::pathone(k,0)), id_(k)
        {
            output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        }

        ~programchange_wire_t()
        {
            source_shutdown();
        }

        void source_ended(unsigned seq)
        {
        }

        void sendprogramchange(unsigned channel, unsigned long long t)
        {
            piw::data_nb_t d = piw::makekey(1,id_,1,id_,piw::KEY_HARD,t);

            output_.add_value(1,d);
            source_start(0,piw::pathone_nb(id_,t),output_);
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "midi program change " << id_ << " " << d;
#endif
            source_end(t+1);
        }

        piw::xevent_data_buffer_t output_;
        unsigned id_;
    };

    struct trigger_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
    {
        trigger_wire_t(unsigned k): event_data_source_real_t(piw::pathone(k,0)), id_(k)
        {
            output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        }

        ~trigger_wire_t()
        {
            source_shutdown();
        }

        void source_ended(unsigned seq)
        {
        }

        void sendtrigger(unsigned channel, unsigned value, unsigned long long t)
        {
            if(0 == value)
            {
                piw::data_nb_t d = piw::makekey(0,id_,0,id_,piw::KEY_HARD,t);

                output_.add_value(1,d);
                source_start(0,piw::pathone_nb(id_,t),output_);
#if MIDI_INPUT_DEBUG
                pic::logmsg() << "midi trigger off " << id_ << " " << d;
#endif
                source_end(t+1);
            }
            else if(127 == value)
            {
                piw::data_nb_t d = piw::makekey(1,id_,1,id_,piw::KEY_HARD,t);

                output_.add_value(1,d);
                source_start(0,piw::pathone_nb(id_,t),output_);
#if MIDI_INPUT_DEBUG
                pic::logmsg() << "midi trigger on " << id_ << " " << d;
#endif
                source_end(t+1);
            }
        }

        piw::xevent_data_buffer_t output_;
        unsigned id_;
    };


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi handling classes
    //
    // Have a midiwire for each key/controller
    // They're a midi receivers and decoders
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct belcanto_from_midi_t: piw::root_ctl_t, piw::thing_t, midi::mididecoder_t
    {
        belcanto_from_midi_t()
        {
            piw::tsd_thing(this);

            set_latency(KBD_LATENCY);
        }

        ~belcanto_from_midi_t()
        {
            tracked_invalidate();
            unplumb();
        }

        virtual void plumb() = 0;
        virtual void unplumb() {}

        void midi_receive(const unsigned char *buffer, unsigned length)
        {
            // pass buffer to the piw midi decoder
            decoder_input(buffer,length);
        }
    };

    struct keyboard_t: belcanto_from_midi_t
    {
        keyboard_t()
        {
            for(unsigned i=0;i<WIRES;i++)
            {
                wires_[i] = pic::ref(new keyboard_wire_t(i));
            }
        }

        void plumb()
        {
            for(unsigned k=0; k<WIRES; k++)
            {
                connect_wire(wires_[k].ptr(),wires_[k]->source());
            }
        }

        void decoder_noteon(unsigned channel, unsigned number, unsigned velocity)
        {
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "note on " << channel << "," << number << "," << velocity;
#endif
            unsigned long long t = piw::tsd_time();
            kbd_enqueue(t, channel, number, velocity);
        }

        void decoder_noteoff(unsigned channel, unsigned number, unsigned velocity)
        {
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "note off " << channel << "," << number << "," << velocity;
#endif
            kbd_enqueue(piw::tsd_time(), channel, number, 0);
        }

        void kbd_enqueue(unsigned long long t, unsigned channel, unsigned key, unsigned val)
        {
            keyblob_t *blob;
            piw::data_nb_t data = piw::makeblob_nb(t, sizeof(keyblob_t), (unsigned char **)&blob);

            blob->ch = channel;
            blob->key = key+1;
            blob->val = val;

            enqueue_fast(data);
        }

        void thing_dequeue_fast(const piw::data_nb_t &d)
        {
            if(d.is_blob())
            {
                keyblob_t *blob = (keyblob_t *)d.as_blob();

                if(blob->key && blob->key <= WIRES)
                {
                    wires_[blob->key-1]->sendkey(blob->ch, blob->val, d.time());
                }
            }
        }

        pic::ref_t<keyboard_wire_t> wires_[WIRES];
    };

    struct continouscontrol_t: belcanto_from_midi_t
    {
        continouscontrol_t()
        {
            for(unsigned i=0;i<WIRES;i++)
            {
                wires_[i] = pic::ref(new cc_wire_t(i));
            }
        }

        void plumb()
        {
            for(unsigned k=0; k<WIRES; k++)
            {
                connect_wire(wires_[k].ptr(),wires_[k]->source());
            }
        }

        void decoder_cc(unsigned channel, unsigned number, unsigned value)
        {
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "cc " << channel << "," << number << "," << value;
#endif
            ccblob_t *blob;
            piw::data_nb_t data = piw::makeblob_nb(piw::tsd_time(), sizeof(ccblob_t), (unsigned char **)&blob);

            blob->ch = channel;
            blob->cc = number+1;
            blob->val = value;

            enqueue_fast(data);
        }

        void thing_dequeue_fast(const piw::data_nb_t &d)
        {
            if(d.is_blob())
            {
                ccblob_t *blob = (ccblob_t *)d.as_blob();

                if(blob->cc && blob->cc <= WIRES)
                {
                    wires_[blob->cc-1]->sendcc(blob->ch, blob->val, d.time());
                }
            }
        }

        unsigned current(unsigned cc)
        {
            if(cc>=0 && cc<WIRES)
            {
                return wires_[cc]->current_;
            }

            return 0;
        }

        void set_trim(unsigned cc, float min, float max, bool inv)
        {
            if(cc>=0 && cc<WIRES)
            {
                wires_[cc]->min_=min;
                wires_[cc]->max_=max;
                wires_[cc]->invert_=inv;
            }
        }

        void clear_trim()
        {
            for(unsigned i=0;i<WIRES;i++)
            {
                wires_[i]->invert_=false;
                wires_[i]->min_=0;
                wires_[i]->max_=127;
            }
        }

        pic::ref_t<cc_wire_t> wires_[WIRES];
    };

    struct programchange_t: belcanto_from_midi_t
    {
        programchange_t()
        {
            for(unsigned i=0;i<WIRES;i++)
            {
                wires_[i] = pic::ref(new programchange_wire_t(i));
            }
        }

        void plumb()
        {
            for(unsigned k=0; k<WIRES; k++)
            {
                connect_wire(wires_[k].ptr(),wires_[k]->source());
            }
        }

        void decoder_programchange(unsigned channel, unsigned value)
        {
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "program change " << channel << "," << value;
#endif

            pcblob_t *blob;
            piw::data_nb_t data = piw::makeblob_nb(piw::tsd_time(), sizeof(pcblob_t), (unsigned char **)&blob);

            blob->ch=channel;
            blob->val=value;

            enqueue_fast(data);
        }

        void thing_dequeue_fast(const piw::data_nb_t &d)
        {
            if(d.is_blob())
            {
                pcblob_t *blob = (pcblob_t *)d.as_blob();

                if(blob->val >= 0 && blob->val < WIRES)
                {
                    wires_[blob->val]->sendprogramchange(blob->ch, d.time());
                }
            }
        }

        pic::ref_t<programchange_wire_t> wires_[WIRES];
    };

    struct trigger_t: belcanto_from_midi_t
    {
        trigger_t()
        {
            for(unsigned i=0;i<WIRES;i++)
            {
                wires_[i] = pic::ref(new trigger_wire_t(i));
            }
        }

        void plumb()
        {
            for(unsigned k=0; k<WIRES; k++)
            {
                connect_wire(wires_[k].ptr(),wires_[k]->source());
            }
        }

        void decoder_cc(unsigned channel, unsigned number, unsigned value)
        {
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "trigger " << channel << "," << number << "," << value;
#endif
            ccblob_t *blob;
            piw::data_nb_t data = piw::makeblob_nb(piw::tsd_time(), sizeof(ccblob_t), (unsigned char **)&blob);

            blob->ch = channel;
            blob->cc = number+1;
            blob->val = value;

            enqueue_fast(data);
        }

        void thing_dequeue_fast(const piw::data_nb_t &d)
        {
            if(d.is_blob())
            {
                ccblob_t *blob = (ccblob_t *)d.as_blob();

                if(blob->cc && blob->cc <= WIRES)
                {
                    wires_[blob->cc-1]->sendtrigger(blob->ch, blob->val, d.time());
                }
            }
        }

        pic::ref_t<trigger_wire_t> wires_[WIRES];
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

    struct midi_stream_t: piw::root_ctl_t, piw::thing_t, midi::mididecoder_t
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
#if MIDI_INPUT_DEBUG
            pic::logmsg() << "midi_receive l="<<length<<" b="<<(void *)buffer << " d0="<<std::hex<<(unsigned)buffer[0];
#endif

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

#if MIDI_INPUT_DEBUG
                pic::logmsg() << "midi dequeue l="<<length<<" b="<<(void *)buffer;
#endif

                // filter out midi clock messages and send out midi clock output
                filter_midi_clock(buffer, length, d.time());

#if MIDI_INPUT_DEBUG
                pic::logmsg() << "midi_input l=" << length << " t=" << d.time();
                for(unsigned i=0; i<length; i++)
                    pic::logmsg() << "d"<<i<<"=" << std::hex << (unsigned)buffer[i];
#endif

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

struct pi_midi::midi_input_t::impl_t: pic::tracked_t, midi::midi_input_port_t
{
    impl_t(pi_midi::midi_input_t *d, const piw::cookie_t &key_cookie, const piw::cookie_t &cc_cookie, const piw::cookie_t &pc_cookie, const piw::cookie_t &trig_cookie, const piw::cookie_t &midi_cookie): midi_input_port_t(piw::change_nb_t::method(this,&impl_t::receive__)), delegate_(d)
    {
        keyboard.connect(key_cookie);
        keyboard.plumb();

        continouscontrol.connect(cc_cookie);
        continouscontrol.plumb();

        programchange.connect(pc_cookie);
        programchange.plumb();

        trigger.connect(trig_cookie);
        trigger.plumb();

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
        midistream.midi_receive(buf,len);
        keyboard.midi_receive(buf,len);
        continouscontrol.midi_receive(buf,len);
        programchange.midi_receive(buf,len);
        trigger.midi_receive(buf,len);
    }

    ~impl_t()
    {
        stop();
        tracked_invalidate();
    }

    midi_input_t *delegate_;
    midi_stream_t midistream;
    keyboard_t keyboard;
    continouscontrol_t continouscontrol;
    trigger_t trigger;
    programchange_t programchange;
};

pi_midi::midi_input_t::midi_input_t(const piw::cookie_t &key_cookie, const piw::cookie_t &cc_cookie, const piw::cookie_t &pc_cookie, const piw::cookie_t &trig_cookie, const piw::cookie_t &midi_cookie): root_(new impl_t(this, key_cookie, cc_cookie, pc_cookie, trig_cookie, midi_cookie))
{
}

unsigned pi_midi::midi_input_t::current(unsigned cc)
{
    return root_->continouscontrol.current(cc);
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
    root_->continouscontrol.clear_trim();
}

void pi_midi::midi_input_t::set_trim(unsigned cc, float min, float max, bool invert)
{
    root_->continouscontrol.set_trim(cc,min,max,invert);
}

long pi_midi::midi_input_t::get_port(void)
{
    return root_->get_port();
}

bool pi_midi::midi_input_t::set_port(long uid)
{
    return root_->set_port(uid);
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
    if((r=root_->continouscontrol.gc_traverse(v,a))!=0) return r;
    if((r=root_->programchange.gc_traverse(v,a))!=0) return r;
    if((r=root_->trigger.gc_traverse(v,a))!=0) return r;
    return 0;
}

int pi_midi::midi_input_t::gc_clear()
{
    root_->keyboard.gc_clear();
    root_->continouscontrol.gc_clear();
    root_->programchange.gc_clear();
    root_->trigger.gc_clear();
    return 0;
}
