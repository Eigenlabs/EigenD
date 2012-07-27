
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

/*
 * midi_clock.cpp: converts Belcanto metronome to MIDI clock
 *
 *
 */

#include <piw/piw_cfilter.h>
#include <piw/piw_clock.h>
#include <piw/piw_phase.h>
#include <piw/piw_tsd.h>
#include <plg_midi/midi_clock.h>
#include <cmath>

#define IN_RUNNING 1
#define IN_BEAT 2
#define IN_TEMPO 3


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi clock filter class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace
{

    struct midi_clock_filter_t: piw::cfilterfunc_t
    {
        midi_clock_filter_t(pi_midi::midi_clock_t::impl_t *p):
            impl_(p), running_(false), inclock_ratio_(0.0), inclock_zero_(1e30), on_(false), tempo_(120),
            pulse_time_(0), pulse_period_(0), pulse_count_(0), pulse_(0), pulse_beat_(0), delay_(0LL)
        {
            // establish when clock pulses happen within a beat
            for(unsigned i=0; i<24; i++)
                clock_pulses_beats_[i] = double(i)/24.0;

        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id);
        bool cfilterfunc_process(piw::cfilterenv_t *env,unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs);
        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time);
        void tick_running(unsigned long long t, double running_value, piw::cfilterenv_t *env);
        void tick_beat(unsigned long long t, double beat_value);
        double interpolate_in(unsigned long long t);
        void send_midi_byte(unsigned char midi_byte, unsigned long long t, piw::cfilterenv_t *env);

        pi_midi::midi_clock_t::impl_t *impl_;
        bool running_;
        double lastfloor_;
        double lastbeat_;
        double inclock_ratio_;
        double inclock_zero_;
        unsigned long long inclock_offset_;
        bool on_;
        double tempo_;
        unsigned long long pulse_time_;
        unsigned long long pulse_period_;
        unsigned pulse_count_;
        unsigned pulse_;
        double pulse_beat_;

        double clock_pulses_beats_[24];
        long long delay_;
    };


} // namespace

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi clock implementation class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace pi_midi
{
    struct midi_clock_t::impl_t: piw::cfilterctl_t, piw::cfilter_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d): piw::cfilter_t(this,o,d), target_delay_(0LL)
        {
        }

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &)
        {
            return new midi_clock_filter_t(this);
        }

        void cfilterctl_delete(piw::cfilterfunc_t *f)
        {
            delete f;
        }

        unsigned cfilterctl_latency() { return 0; }

        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return (1ULL<<3)-1; }
        unsigned long long cfilterctl_outputs() { return 1; }

        static int __set_delay(void *i_, void *ms_)
        {
            impl_t *i = (impl_t *)i_;
            float ms = *(float *)ms_;
            i->target_delay_ = (long long)(1000.0*ms);
            pic::logmsg() << "target delay set to " << i->target_delay_;
            return 0;
        }

        long long target_delay_;

    };

} // namespace pi_midi


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi clock filter functions
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool midi_clock_filter_t::cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
{
    running_=false;
    inclock_ratio_=0.0;
    inclock_zero_=1e30;
    on_=true;

    env->cfilterenv_reset(IN_RUNNING,id.time());
    env->cfilterenv_reset(IN_BEAT,id.time());
    env->cfilterenv_reset(IN_TEMPO,id.time());
    return true;
}

bool midi_clock_filter_t::cfilterfunc_process(piw::cfilterenv_t *env,unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs)
{
    unsigned i;
    piw::data_nb_t d;

    if(!on_)
    {
        return false;
    }

    double beat = 0;

    while(env->cfilterenv_next(i, d, to))
    {
        if(d.time() < from)
            continue;

        switch(i)
        {
            case IN_RUNNING:
                tick_running(d.time(),d.as_norm(),env);
                break;
            case IN_BEAT:
                // the metronome only sends one beat value per process call
                // so can pick it up here
                beat = d.as_renorm_float(BCTUNIT_BEATS,0,96000,0);
                tick_beat(d.time(),beat);
                break;
            case IN_TEMPO:
                tempo_ = d.as_renorm_float(BCTUNIT_BPM,0,100000,0);
                break;
        }
    }

    // find the beat time
    if(inclock_ratio_)
    {
        // when there is a ratio after starting the clock, then ready to interpolate beat

        // beat value at the start of the clock tick
        //double from_beat = interpolate_in(from);
        // beat value at the end of the clock tick
        double to_beat = interpolate_in(to);

        //pic::logmsg() << "from: t=" << from << " beat=" << from_beat << "  to: t=" << to << " beat=" << to_beat;



        while(true)
        {
            // current pulse in beats
            double pulse_beat_offset = clock_pulses_beats_[pulse_];

            // calculate time of pulse
            unsigned long long pulse_time = to - (unsigned long long)((to_beat-(pulse_beat_+pulse_beat_offset))/inclock_ratio_);

            if(delay_ != impl_->target_delay_)
            {
                // adjust the compensation delay by at most 10% of the pulse interval each pulse
                long long adj = impl_->target_delay_-delay_;
                long long pulse_interval = (long long)(1.0/(inclock_ratio_*24));
                if(adj>pulse_interval/10) adj=pulse_interval/10;
                if(adj<-pulse_interval/10) adj=-pulse_interval/10;
                delay_ += adj;
                //pic::logmsg() << "delay adjusted to " << delay_ << " target " << impl_->target_delay_;
            }

            pulse_time += delay_;

            if(pulse_time>=to)
                break;

            //pic::logmsg() << "pulse: num=" << pulse_ << " pulse_beat=" << pulse_beat_+pulse_beat_offset << " time=" << pulse_time;

            // send pulse, if pulse time is either during this clock tick or before
            send_midi_byte(0xf8, pulse_time, env);

            // next pulse
            pulse_++;
            if(pulse_==24)
            {
                pulse_ = 0;
                pulse_beat_++;
            }
        }
    }

    return true;
}

bool midi_clock_filter_t::cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long time)
{
    tick_running(time,0,env);
    on_ = false;
    return false;
}

void midi_clock_filter_t::tick_running(unsigned long long t, double running_value, piw::cfilterenv_t *env)
{
    // handle the running signal changing
    bool running = (running_value>0.0);

    if(running != running_)
    {
        running_ = running;

        if(!running)
        {
            // start running, initialize
            inclock_zero_ = 1e30;
            inclock_ratio_=0.0;
            pulse_ = 0;
            pulse_beat_ = 0;
        }

        // send start/stop message
        send_midi_byte(running?((unsigned char)(0xfa)):((unsigned char)(0xfc)), t, env);

        pic::msg() << "midi clock transport has " << (running?"started":"stopped") << " at time t=" << t << pic::log;
    }
}

void midi_clock_filter_t::tick_beat(unsigned long long t, double beat_value)
{
//    pic::logmsg() << beat_value;

    // from
    if(running_)
    {
        if(beat_value > inclock_zero_)
        {
            inclock_ratio_ = (beat_value-inclock_zero_)/(t-inclock_offset_);
        }

        inclock_offset_ = t;
        inclock_zero_ = beat_value;
    }
}

double midi_clock_filter_t::interpolate_in(unsigned long long t)
{
    // return the beat value interpolated at a given time
    return (inclock_zero_+inclock_ratio_*((double)t-(double)inclock_offset_));
}

void midi_clock_filter_t::send_midi_byte(unsigned char midi_byte, unsigned long long t, piw::cfilterenv_t *env)
{
    unsigned char *midi_data = 0;
    piw::data_nb_t midi_output_buffer = piw::makeblob_nb(t, 1, &midi_data);

    if(midi_data)
    {
        midi_data[0] = midi_byte;
        env->cfilterenv_output(1, midi_output_buffer);
    }
    else
    {
        pic::logmsg() << "warning: midi byte not sent";
    }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi clock interface class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace pi_midi
{

    midi_clock_t::midi_clock_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *d): impl_(new impl_t(c,d))
    {
    }

    midi_clock_t::~midi_clock_t()
    {
        delete impl_;
    }

    piw::cookie_t midi_clock_t::cookie()
    {
        return impl_->cookie();
    }

    void midi_clock_t::set_delay(float ms)
    {
        piw::tsd_fastcall(&impl_t::__set_delay,impl_,&ms);
    }

} // namespace pi_midi
