
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

#include <piw/piw_clock.h>
#include <piw/piw_cfilter.h>
#include <picross/pic_stl.h>

#include <lib_midi/midi_decoder.h>

#include <plg_midi/midi_processor.h>

namespace
{
    typedef pic::lckmap_t<unsigned,unsigned>::lcktype mapping_t;
}

namespace pi_midi
{
    struct midi_processor_t::impl_t: piw::cfilterctl_t, piw::cfilter_t 
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &);
        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return SIG2(1,2); }
        unsigned long long cfilterctl_outputs() { return SIG2(1,2); }

        void closed(bool v);
        static int __closed(void *r_, void *c_);
        void noteon_enabled(bool v);
        static int __noteon_enabled(void *r_, void *c_);
        void noteoff_enabled(bool v);
        static int __noteoff_enabled(void *r_, void *c_);
        void polypressure_enabled(bool v);
        static int __polypressure_enabled(void *r_, void *c_);
        void cc_enabled(bool v);
        static int __cc_enabled(void *r_, void *c_);
        void programchange_enabled(bool v);
        static int __programchange_enabled(void *r_, void *c_);
        void channelpressure_enabled(bool v);
        static int __channelpressure_enabled(void *r_, void *c_);
        void pitchbend_enabled(bool v);
        static int __pitchbend_enabled(void *r_, void *c_);
        void messages_enabled(bool v);
        static int __messages_enabled(void *r_, void *c_);

        void clear_channel_mapping();
        void set_channel_mapping(unsigned from, unsigned to);
        void activate_channel_mapping();

        bool closed_;
        bool noteon_enabled_;
        bool noteoff_enabled_;
        bool polypressure_enabled_;
        bool cc_enabled_;
        bool programchange_enabled_;
        bool channelpressure_enabled_;
        bool pitchbend_enabled_;
        bool messages_enabled_;

        pic::flipflop_t<mapping_t> channel_mapping_;
    };
}

namespace
{
    struct midiprocessorfunc_t: piw::cfilterfunc_t, midi::mididecoder_t
    {
        midiprocessorfunc_t(pi_midi::midi_processor_t::impl_t *root) : root_(root), time_(0), env_(0)
        {
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            env->cfilterenv_reset(1,id.time());
            env->cfilterenv_reset(2,id.time());
            return true;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to, unsigned long samplerate, unsigned buffersize)
        {
            env_ = env;

            time_ = std::max(time_, from);

            if(root_->closed_)
            {
                if(!notes_.empty())
                {
                    pic::lckset_t<unsigned>::nbtype::iterator it;
                    unsigned long long t = from;
                    while((it = notes_.begin()) != notes_.end())
                    {
                        unsigned channel;
                        unsigned number;
                        decode_note_entry(*it, &channel, &number);

                        unsigned char *blob = 0;
                        piw::data_nb_t d = piw::makeblob_nb(t++, 3, &blob);

                        blob[0] = (unsigned char)0x80+channel;
                        blob[1] = (unsigned char)number;
                        blob[2] = (unsigned char)0x40;

                        env->cfilterenv_output(1, d);

                        notes_.erase(it);
                    }
                }
            }
            else
            {
                piw::data_nb_t d;
                if(env->cfilterenv_nextsig(1,d,to))
                {
                    if(d.is_blob())
                    {
                        time_ = std::max(time_, d.time());

                        unsigned length = d.as_bloblen();
                        const unsigned char *buffer = (const unsigned char *)(d.as_blob());
                        decoder_input(buffer, length);
                    }
                }
            }

            env_ = 0;

            return true;
        }

        void send(unsigned signal, unsigned char b1)
        {
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(time_++, 1, &blob);
            blob[0] = b1;
            env_->cfilterenv_output(signal, d);
        }

        void send(unsigned signal, unsigned char b1, unsigned char b2)
        {
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(time_++, 2, &blob);
            blob[0] = b1;
            blob[1] = b2;
            env_->cfilterenv_output(signal, d);
        }

        void send(unsigned signal, unsigned char b1, unsigned char b2, unsigned char b3)
        {
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(time_++, 3, &blob);
            blob[0] = b1;
            blob[1] = b2;
            blob[2] = b3;
            env_->cfilterenv_output(signal, d);
        }

        unsigned encode_note_entry(unsigned channel, unsigned number)
        {
            return ((number & 0x7f) << 4) | (channel & 0xf);
        }

        void decode_note_entry(unsigned entry, unsigned *channel, unsigned *number)
        {
            *channel = entry & 0xf;
            *number = entry >> 4;
        }

        unsigned mapped_channel(unsigned channel)
        {
            pic::flipflop_t<mapping_t>::guard_t guard_channel(root_->channel_mapping_);
            mapping_t::const_iterator it = guard_channel.value().find(channel+1);
            if(it != guard_channel.value().end())
            {
                return it->second-1;
            }
            return channel;
        }

        void decoder_noteon(unsigned channel, unsigned number, unsigned velocity)
        {
            if(root_->noteon_enabled_)
            {
                channel = mapped_channel(channel);
                notes_.insert(encode_note_entry(channel, number));
                send(1, 0x90|channel, number, velocity);
            }
            else
            {
                send(2, 0x90|channel, number, velocity);
            }
        }

        void decoder_noteoff(unsigned channel, unsigned number, unsigned velocity)
        {
            if(root_->noteoff_enabled_)
            {
                channel = mapped_channel(channel);
                notes_.erase(encode_note_entry(channel, number));
                send(1, 0x80|channel, number, velocity);
            }
            else
            {
                send(2, 0x80|channel, number, velocity);
            }
        }

        void decoder_polypressure(unsigned channel, unsigned number, unsigned value)
        {
            if(root_->polypressure_enabled_)
            {
                send(1, 0xa0|mapped_channel(channel), number, value);
            }
            else
            {
                send(2, 0xa0|channel, number, value);
            }
        }

        void decoder_cc(unsigned channel, unsigned number, unsigned value)
        {
            if(root_->cc_enabled_)
            {
                send(1, 0xb0|mapped_channel(channel), number, value);
            }
            else
            {
                send(2, 0xb0|channel, number, value);
            }
        }

        void decoder_programchange(unsigned channel, unsigned value)
        {
            if(root_->programchange_enabled_)
            {
                send(1, 0xc0|mapped_channel(channel), value);
            }
            else
            {
                send(2, 0xc0|channel, value);
            }
        }

        void decoder_channelpressure(unsigned channel, unsigned value)
        {
            if(root_->channelpressure_enabled_)
            {
                send(1, 0xd0|mapped_channel(channel), value);
            }
            else
            {
                send(2, 0xd0|channel, value);
            }
        }

        void decoder_pitchbend(unsigned channel, unsigned value)
        {
            if(root_->pitchbend_enabled_)
            {
                send(1, 0xe0|mapped_channel(channel), value&0x7f, (value&0x3f80)>>7);
            }
            else
            {
                send(2, 0xe0|channel, value&0x7f, (value&0x3f80)>>7);
            }
        }

        void decoder_generic1(bool handled, unsigned char b1)
        {
            if(!handled)
            {
                if(root_->messages_enabled_)
                {
                    send(1, b1);
                }
                else
                {
                    send(2, b1);
                }
            }
        }

        void decoder_generic2(bool handled, unsigned char b1, unsigned char b2)
        {
            if(!handled)
            {
                if(root_->messages_enabled_)
                {
                    send(1, b1, b2);
                }
                else
                {
                    send(2, b1, b2);
                }
            }
        }

        void decoder_generic3(bool handled, unsigned char b1, unsigned char b2, unsigned char b3)
        {
            if(!handled)
            {
                if(root_->messages_enabled_)
                {
                    send(1, b1, b2, b3);
                }
                else
                {
                    send(2, b1, b2, b3);
                }
            }
        }

        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long to)
        {
            return false;
        }

        pi_midi::midi_processor_t::impl_t *root_;
        unsigned long long time_;
        piw::cfilterenv_t *env_;
        pic::lckset_t<unsigned>::nbtype notes_;
    };
};

pi_midi::midi_processor_t::impl_t::impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this, o, d),
    closed_(false), noteon_enabled_(true), noteoff_enabled_(true), polypressure_enabled_(true), cc_enabled_(true),
    programchange_enabled_(true), channelpressure_enabled_(true), pitchbend_enabled_(true), messages_enabled_(true)
{
}

piw::cfilterfunc_t *pi_midi::midi_processor_t::impl_t::cfilterctl_create(const piw::data_t &)
{
    return new midiprocessorfunc_t(this);
}

void pi_midi::midi_processor_t::impl_t::closed(bool v)
{
    piw::tsd_fastcall(__closed, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__closed(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->closed_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::noteon_enabled(bool v)
{
    piw::tsd_fastcall(__noteon_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__noteon_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->noteon_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::noteoff_enabled(bool v)
{
    piw::tsd_fastcall(__noteoff_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__noteoff_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->noteoff_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::polypressure_enabled(bool v)
{
    piw::tsd_fastcall(__polypressure_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__polypressure_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->polypressure_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::cc_enabled(bool v)
{
    piw::tsd_fastcall(__cc_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__cc_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->cc_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::programchange_enabled(bool v)
{
    piw::tsd_fastcall(__programchange_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__programchange_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->programchange_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::channelpressure_enabled(bool v)
{
    piw::tsd_fastcall(__channelpressure_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__channelpressure_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->channelpressure_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::pitchbend_enabled(bool v)
{
    piw::tsd_fastcall(__pitchbend_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__pitchbend_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->pitchbend_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::messages_enabled(bool v)
{
    piw::tsd_fastcall(__messages_enabled, this, &v);
}

int pi_midi::midi_processor_t::impl_t::__messages_enabled(void *r_, void *v_)
{
    midi_processor_t::impl_t *r = (midi_processor_t::impl_t *)r_;
    r->messages_enabled_ = *(bool *)v_;
    return 0;
}

void pi_midi::midi_processor_t::impl_t::clear_channel_mapping()
{
    channel_mapping_.alternate().clear();
}

void pi_midi::midi_processor_t::impl_t::set_channel_mapping(unsigned from, unsigned to)
{
    channel_mapping_.alternate().insert(std::make_pair(from,to));
}

void pi_midi::midi_processor_t::impl_t::activate_channel_mapping()
{
    channel_mapping_.exchange();
}

pi_midi::midi_processor_t::midi_processor_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
piw::cookie_t pi_midi::midi_processor_t::cookie() { return impl_->cookie(); }
pi_midi::midi_processor_t::~midi_processor_t() { delete impl_; }
void pi_midi::midi_processor_t::closed(bool v) { impl_->closed(v); }
void pi_midi::midi_processor_t::noteon_enabled(bool v) { impl_->noteon_enabled(v); }
void pi_midi::midi_processor_t::noteoff_enabled(bool v) { impl_->noteoff_enabled(v); }
void pi_midi::midi_processor_t::polypressure_enabled(bool v) { impl_->polypressure_enabled(v); }
void pi_midi::midi_processor_t::cc_enabled(bool v) { impl_->cc_enabled(v); }
void pi_midi::midi_processor_t::programchange_enabled(bool v) { impl_->programchange_enabled(v); }
void pi_midi::midi_processor_t::channelpressure_enabled(bool v) { impl_->channelpressure_enabled(v); }
void pi_midi::midi_processor_t::pitchbend_enabled(bool v) { impl_->pitchbend_enabled(v); }
void pi_midi::midi_processor_t::messages_enabled(bool v) { impl_->messages_enabled(v); }
void pi_midi::midi_processor_t::clear_channel_mapping() { impl_->clear_channel_mapping(); }
void pi_midi::midi_processor_t::set_channel_mapping(unsigned from, unsigned to) { impl_->set_channel_mapping(from, to); }
void pi_midi::midi_processor_t::activate_channel_mapping() { impl_->activate_channel_mapping(); }
