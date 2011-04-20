
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
 * loop_pinger.cpp: metronome with tap tempo and midi clock control
 *
 */


#include <picross/pic_config.h>
#include <picross/pic_mlock.h>
#include <piw/piw_clock.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_bundle.h>

#include "loop_pinger.h"

#include <math.h>

#include <iostream>
#include <iomanip>

#define STATE_IDLE              0
#define STATE_STARTING          1
#define STATE_PLAYING           2
#define STATE_STOPPING          3

#define MIDI_CLK_STATE_IDLE     0
#define MIDI_CLK_STATE_SONG_POS 1
#define MIDI_CLK_STATE_BEAT_POS 2
#define MIDI_CLK_STATE_WAIT     3
#define MIDI_CLK_STATE_CONTINUE 4
#define MIDI_CLK_STATE_RUN      5
#define MIDI_CLK_DATA_PERIOD    100
#define MIDI_CLK_DEBUG          0

#define INPUT_MASK SIG4(1,2,3,4)
#define OUTPUT_MASK SIG4(1,2,3,4)
#define TEMPO_MASK SIG1(5)
#define STATUS_MASK SIG1(1)

#define STATUS_OFF 2
#define STATUS_RUN 1
#define STATUS_RUN_FLASHING 0
#define STATUS_BEAT 3
#define STATUS_BAR 1

namespace
{

#ifdef PI_WINDOWS
    double rint( double x) 
    // Copyright (C) 2001 Tor M. Aamodt, University of Toronto 
    // Permisssion to use for all purposes commercial and otherwise granted. 
    // THIS MATERIAL IS PROVIDED "AS IS" WITHOUT WARRANTY, OR ANY CONDITION OR 
    // OTHER TERM OF ANY KIND INCLUDING, WITHOUT LIMITATION, ANY WARRANTY 
    // OF MERCHANTABILITY, SATISFACTORY QUALITY, OR FITNESS FOR A PARTICULAR 
    // PURPOSE. 
    { 
        if( x > 0 ) { 
            __int64 xint = (__int64) (x+0.5); 
            if( xint % 2 ) { 
                // then we might have an even number... 
                double diff = x - (double)xint; 
                if( diff == -0.5 ) 
                    return double(xint-1); 
            } 
            return double(xint); 
        } else { 
            __int64 xint = (__int64) (x-0.5); 
            if( xint % 2 ) { 
                // then we might have an even number... 
                double diff = x - (double)xint; 
                if( diff == 0.5 ) 
                    return double(xint+1); 
            } 
            return double(xint); 
        } 
    } 
#endif

    static inline double time2tempo(double i) { return 60000000.0/i; }
    static inline double tempo2time(double i) { return 60000000.0/i; }

    // tap controlled tempo input wire
    struct tap_wire_t : piw::wire_t, piw::event_data_sink_t
    {
        tap_wire_t(loop::pinger_t::impl_t *pinger, const piw::event_data_source_t &es);
        ~tap_wire_t();

        void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &data);
        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n);
        bool event_end(unsigned long long);

        void wire_closed()
        {
            unsubscribe();
        }

        void ticked(unsigned long long from,unsigned long long t);

        piw::xevent_data_buffer_t::iter_t iterator_;
        // the parent pinger
        loop::pinger_t::impl_t *pinger_;
    };

    // midi clock controlled tempo input wire
    struct midi_clock_wire_t : piw::wire_t, piw::event_data_sink_t
    {
        midi_clock_wire_t(loop::pinger_t::impl_t *pinger, const piw::event_data_source_t &es);
        ~midi_clock_wire_t();

        void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &data);
        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n);
        bool event_end(unsigned long long);

        void wire_closed()
        {
            unsubscribe();
        }

        void ticked(unsigned long long from,unsigned long long t);

        piw::xevent_data_buffer_t::iter_t iterator_;
        // the parent pinger
        loop::pinger_t::impl_t *pinger_;
    };

    // status output source
    struct status_output_t : piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        status_output_t(const piw::cookie_t &c, piw::clocksink_t *clk_sink):
            piw::event_data_source_real_t(piw::pathnull(0)), buffer_(STATUS_MASK,PIW_DATAQUEUE_SIZE_TINY), status_(-1.0)
        {
            connect(c);
            connect_wire(this,source());
            set_clock(clk_sink);

            piw::tsd_fastcall(init__,this,0);
        }

        ~status_output_t()
        {
            source_end(piw::tsd_time());
            wire_ctl_t::disconnect();
            root_ctl_t::disconnect();
        }

        static int init__(void *self_, void *arg_)
        {
            status_output_t *self = (status_output_t *)self_;

            unsigned long long t = piw::tsd_time();
            self->buffer_.add_value(1,piw::makefloat_bounded_nb(5.0,0.0,0.0,STATUS_OFF,t));
            self->source_start(0,piw::pathnull_nb(t),self->buffer_);

            return 1;
        }

        void set_status(float status, unsigned long long t)
        {
            if(status != status_)
            {
                buffer_.add_value(1,piw::makefloat_bounded(5.0,0.0,0.0,status,t));
                status_ = status;
            }
        }

        piw::xevent_data_buffer_t buffer_;
        float status_;
    };

    // tempo output source
    struct tempo_output_t : piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        tempo_output_t(const piw::cookie_t &c, piw::clocksink_t *clk_sink, float tempo) :
            piw::event_data_source_real_t(piw::pathnull(0)), buffer_(TEMPO_MASK,PIW_DATAQUEUE_SIZE_TINY), tempo_(120)
        {
            connect(c);
            connect_wire(this,source());
            set_clock(clk_sink);

            piw::tsd_fastcall(init__,this,&tempo);
        }

        ~tempo_output_t()
        {
            source_end(piw::tsd_time());
            wire_ctl_t::disconnect();
            root_ctl_t::disconnect();
        }

        static int init__(void *self_, void *arg_)
        {
            tempo_output_t *self = (tempo_output_t *)self_;
            float tempo = *(float *)arg_;

            unsigned long long t = piw::tsd_time();
            self->buffer_.add_value(5,piw::makedouble_bounded_units_nb(BCTUNIT_BPM,100000,0,0,tempo,t));
            self->source_start(0,piw::pathnull_nb(t),self->buffer_);

            return 1;
        }

        void set_tempo(float tempo, unsigned long long t)
        {
            if(tempo_ != tempo)
            {
                buffer_.add_value(5,piw::makedouble_bounded_units(BCTUNIT_BPM,100000,0,0,tempo,t));
                tempo_ = tempo;
            }
        }

        piw::xevent_data_buffer_t buffer_;

        float tempo_;
    };


}


namespace loop
{

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // pinger impl class
    //
    // implements the metronome tempo and beat calculation and output
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct pinger_t::impl_t : piw::root_t, piw::clocksink_t, piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
    {
        impl_t(const piw::cookie_t &c, const piw::cookie_t &c1, const piw::cookie_t &c3, piw::clockdomain_ctl_t *d, const piw::change_t &tc, const piw::change_t &bc,const piw::change_t &pc):
            root_t(0), piw::event_data_source_real_t(piw::pathnull(0)),
            state_(STATE_IDLE), midi_clock_enabled_(false), output_started_(false), tempo_(120), last_tempo_(120), lower_(30.0), upper_(240.0),
            last_beat_(0), last_tap_time_(0), beats_per_bar_(4.0), last_bar_beat_(0),
            persistence_time_(0),
            preroll_(0), adjust_target_time_(0), adjust_target_tempo_(0),
            midi_clk_state_(MIDI_CLK_STATE_IDLE), midi_clk_last_time_(0), midi_clk_song_pos_(0), midi_clk_avg_tempo_1_(120), midi_clk_first_tick_(false), midi_clk_latency_(0),
            tchange_(tc), bchange_(bc), pchange_(pc), upstream_clk_(0), active_tap_wire_(0), active_midi_clock_wire_(0)
        {

            // connect to beat output
            connect(c);
            connect_wire(this,source());
            // setup clock
            d->sink(this,"pinger");
            set_clock(this);
            // enable clock and do not supress ticking
            tick_enable(false);
            // connect to tempo output
            tempo_output_ = new tempo_output_t(c1, this, tempo_);
            status_output_ = new status_output_t(c3, this);
        }

        ~impl_t()
        {
            source_shutdown();
            delete tempo_output_;
            delete status_output_;
        }

        // ------------------------------------------------------------------------------
        // root virtual functions
        // ------------------------------------------------------------------------------
        piw::wire_t *root_wire(const piw::event_data_source_t &s)
        {
            if(s.path().as_pathlen()>0)
            {
                unsigned t = s.path().as_path()[0];

                if(t==1)
                {
                    return new tap_wire_t(this,s);
                }

                if(t==2)
                {
                    return new midi_clock_wire_t(this,s);
                }

            }

            return 0;
        }

        void root_closed() {}

        void root_clock()
        {
            bct_clocksink_t *s(get_clock());

            if(upstream_clk_)
            {
                remove_upstream(upstream_clk_);
            }

            if(s)
            {
                upstream_clk_ = s;
                add_upstream(s);
            }

        }

        void root_opened()
        {
            root_clock();
            root_latency();
        }

        void root_latency()
        {
            this->set_sink_latency(get_latency());
        }

        // TODO: ?
        void source_ended(unsigned seq)
        {
        }

        // ------------------------------------------------------------------------------
        // clocksink_virtual_functions
        // ------------------------------------------------------------------------------

        void clocksink_ticked(unsigned long long from,unsigned long long t)
        {
            if(state_==STATE_IDLE)
            {
                return;
            }

            //pic::logmsg() << "pinger tick " << from << "-" << t;

            if(state_==STATE_STARTING)
            {
                state_=STATE_PLAYING;

                // reset to start of song
                last_beat_=0;
                beat_origin_time_=t-1;
                beat_origin_=0;
                bar_tap_int_beat_phase_=0;
                bar_tap_beat_=0;
                adjust_target_time_=0;

                // if no midi clock then metronome can start the output
                if(!midi_clock_enabled_)
                    pinger_prime(from, t, beat_origin_, beat_origin_time_);

                status_output_->set_status(persistence_time_ ? STATUS_RUN_FLASHING : STATUS_RUN,t);
                return;
            }

            if(state_==STATE_PLAYING)
            {
                double beat;

                // calculate beat

                if(!midi_clock_enabled_)
                {
                    // read the tapping inputs
                    if(active_tap_wire_)
                        active_tap_wire_->ticked(from,t);

                    // calculate beat at time of end of clock tick from beat phase
                    if(adjust_target_time_)
                    {
                        if(t<adjust_target_time_)
                        {
                            beat = get_beat(t,adjust_target_tempo_);
                            pic::logmsg() << "adjustment " << beat << " " << adjust_target_tempo_;
                        }
                        else
                        {
                            beat = get_beat(t,tempo_);
                            pic::logmsg() << "adjustment complete " << beat << " " << tempo_;
                            adjust_target_time_=0;
                        }
                    }
                    else
                    {
                        beat = get_beat(t,tempo_);
                    }

                }
                else
                {
                    // process midi clock
                    if(active_midi_clock_wire_)
                        active_midi_clock_wire_->ticked(from,t);

                    if(midi_clk_state_==MIDI_CLK_STATE_RUN)
                    {

                        if(midi_clk_exp_time_<t && midi_clk_last_time_<from)
                        {
                            // midi clock tick is late, should have fallen in this clock tick
                            // hold beat at last beat
                            beat = last_beat_;

                            //pic::logmsg() << "midi clk tick is late, expected time=" << midi_clk_exp_time_;
                        }
                        else
                        {
                            beat = get_beat(t,tempo_);
                          //pic::logmsg() << "      beat=" << beat << " time=" << t << " beat origin=" << beat_origin_ << " time=" << beat_origin_time_ << " last beat=" << last_beat_;
                        }

                    }
                    else
                        beat = last_beat_;
                }


                if(!midi_clock_enabled_ || (midi_clock_enabled_ && midi_clk_state_==MIDI_CLK_STATE_RUN))
                {
                    // output the beat at the time of the end of the clock tick

                    if(beat<last_beat_)
                    {
                        pic::logmsg() << "discontinuity, beat running too fast " << last_beat_ << "->" << beat;
                        beat = last_beat_;
                    }

                    if((beat-last_beat_)>0.5)
                    {
                        pic::logmsg() << "discontinuity, the beat is jumping " << last_beat_ << "->" << beat;
                    }

#if MIDI_CLK_DEBUG>1
                    pic::logmsg() << beat <<" " <<  t << "   " << beat_origin_ << " " << beat_origin_time_;
#endif // MIDI_CLK_DEBUG>1

                    if(tempo_!=last_tempo_)
                    {
                        tempo_output_->set_tempo(tempo_,t);
                        last_tempo_=tempo_;
                    }

                    // song beat output at time t
                    //if(beat!=last_beat_)
                    output_.add_value(2,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,beat,t));

                    // calculate beat bar
                    double bb = fmod((double)beat-bar_tap_beat_,(double)beats_per_bar_);
                    if(bb<0.0)
                        bb += beats_per_bar_;
                    // beats per bar output
                    output_.add_value(1,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,beats_per_bar_,0,0,bb,t));

                    // song beat output at time t
                    // why is the bar beat phase added to the bar beat output?
                    output_.add_value(4,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,bar_tap_int_beat_phase_+(beat-bar_tap_beat_)/beats_per_bar_,t));

                    if(persistence_time_)
                    {
                        int xb = int(bb);

                        if(xb!=last_bar_beat_)
                        {
                            float c;

                            if(xb==0)
                            {
                                c=STATUS_BAR;
                            }
                            else
                            {
                                c=STATUS_BEAT;
                            }

                            persistence_ = t;
                            status_output_->set_status(c,t);
                        }
                        else
                        {
                            if(t-persistence_ > persistence_time_*1000)
                            {
                                status_output_->set_status(STATUS_RUN_FLASHING,t);
                            }
                        }

                        last_bar_beat_ = bb;
                    }
                    else if(persistence_)
                    {
                        persistence_=0;
                        status_output_->set_status(STATUS_RUN,t);
                    }

                    // history of beat value and time of that value
                    last_beat_ = beat;
                    last_beat_time_=t;
                }

                return;
            }

            if(state_==STATE_STOPPING)
            {
                pinger_stop(t);
                status_output_->set_status(STATUS_OFF,t);
                state_=STATE_IDLE;
                return;
            }
        }

        // ------------------------------------------------------------------------------
        // tap tempo calculation functions
        // ------------------------------------------------------------------------------

        void pinger_prime(unsigned long long from, unsigned long long to, double beat, unsigned long long time)
        {
            //pic::logmsg() << "pinger start at "<<time << " started="<<output_started_;

            double past_beat = get_beat(from+1,tempo_);

            if(!output_started_)
                output_ = piw::xevent_data_buffer_t(OUTPUT_MASK,PIW_DATAQUEUE_SIZE_NORM);

            output_.add_value(2,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,past_beat,from+1));
            output_.add_value(1,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,beats_per_bar_,0,0,fmod((double)past_beat,(double)beats_per_bar_),from+1));
            output_.add_value(4,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,past_beat/beats_per_bar_,from+1));

            if(!output_started_)
            {
                source_start(0,piw::pathnull_nb(from+1),output_);
                output_started_ = true;
            }

            output_.add_value(2,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,beat,time));
            output_.add_value(1,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,beats_per_bar_,0,0,fmod((double)beat,(double)beats_per_bar_),time));
            output_.add_value(4,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,beat/beats_per_bar_,time));

            past_beat = get_beat(to,tempo_);

            // song beat output
            output_.add_value(2,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,past_beat,to));
            // bar beat output
            output_.add_value(1,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,beats_per_bar_,0,0,fmod((double)past_beat,(double)beats_per_bar_),to));
            // bar output
            output_.add_value(4,piw::makedouble_bounded_units_nb(BCTUNIT_BEATS,100000,0,0,past_beat/beats_per_bar_,to));

            // running output
            pic::logmsg() << "running   from="<<from<<" to="<<to<<" beat="<<beat<<" time="<<time;
            output_.add_value(3,piw::makefloat_bounded_nb(1,0,0,1,to));

            tempo_output_->set_tempo(tempo_,to);
            last_tempo_ = tempo_;

        }

        void pinger_stop(unsigned long long t)
        {
            //pic::logmsg() << "pinger stop at " << t << " started="<<output_started_; //output_.dump(false);

            if(output_started_)
            {
                output_.add_value(3,piw::makefloat_bounded_nb(1,0,0,0,t-1));
                output_started_ = false;
                source_end(t);
            }
        }


        double get_beat(unsigned long long t, double tempo)
        {
            // get beat value at time t for a given tempo from the beat phase (reference point from which a beat marked)
            if(t>beat_origin_time_)
            {
                return beat_origin_+(t-beat_origin_time_)*tempo/60000000.0;
            }
            else
            {
                return beat_origin_-(beat_origin_time_-t)*tempo/60000000.0;
            }
        }

        void tap(unsigned long long tap_time, bool bar)
        {
            // respond to a tap at time t

            // calc last tempo from last beat period
            double last_tempo = time2tempo(tap_time-last_tap_time_);
            unsigned beats_per_bar = (unsigned)beats_per_bar_;
            double avg_tempo;

            // tempo too high
            if(last_tempo > upper_)
            {
                pic::logmsg() << "ignored key " << last_tempo << ' ' << upper_;
                return;
            }

            // tempo too low
            if(last_tempo < lower_)
            {
                avg_tempo = tempo_;
                beat_count_from_bar_=0;
                avg_tap_time_start_=tap_time;
                avg_tap_cnt_=0;
            }
            else
            {

                // average the tempo over a number of taps
                avg_tap_cnt_++;
                avg_tempo = time2tempo(((double)(tap_time-avg_tap_time_start_))/avg_tap_cnt_);
                double r = avg_tempo/last_tempo;

                // average tempo differing from tempo from current period so reset point to calculate average from
                if(r < 0.90 || r >1.10)
                {
                    pic::logmsg() << "reset average " << avg_tempo << ' ' << last_tempo << ' ' << r;
                    avg_tempo=last_tempo;
                    avg_tap_time_start_=tap_time;
                    avg_tap_cnt_=0;
                }
            }

            pic::logmsg() << "tap " << last_tempo << " avg " << avg_tempo;

            last_tap_time_=tap_time;

            if(preroll_)
            {
                if(beat_count_from_bar_+1<preroll_)
                {
                    beat_count_from_bar_++;
                    // do not start beat until preroll finished
                    return;
                }

                // reset preroll after starting
                preroll_=0;
                beat_count_from_bar_=0;
                pchange_(piw::makebool(false,tap_time));
            }

            double ob = get_beat(tap_time,avg_tempo);
            double beat_at_tap_time = ob;
            // round to nearest integer beat
            double beat_int = rint(beat_at_tap_time);
            unsigned iters = 0;

            // advance rounded beat to the next beat value after last beat
            while(beat_int<last_beat_)
            {
                beat_at_tap_time++;
                beat_int++;
                // inc tap time by 1 beat period according to average tempo
                tap_time+=((unsigned long long)(60000000.0/avg_tempo));
                if(++iters>1000)
                {
                    pic::logmsg() << "warning, cannot sync to beat " << rint(ob) << " (time=" << tap_time << " tempo=" << avg_tempo << ")";
                    return;
                }
            }

            // offset from last beat value to the next int beat
            double beatoffset = beat_int-last_beat_;
            unsigned long long timeoffset = (unsigned long long)(beatoffset*60000000.0/avg_tempo);

            // time that next int beat should fall on
            adjust_target_time_ = last_beat_time_+timeoffset;
            // target tempo, proportional to slope between last beat and the next integer beat
            adjust_target_tempo_ = (beat_int-last_beat_)*60000000.0/((double)(adjust_target_time_-last_beat_time_));

            // reset int and int time - reference points for beat slope
            // prevents a discontinuity if the slope has changed because the slope is calculated from the next int beat
            beat_origin_time_ = adjust_target_time_;
            beat_origin_ = beat_int;

            // process bar tap
            if(bar)
            {
                double bar_int = rint(fmod((double)beat_at_tap_time-bar_tap_beat_,(double)beats_per_bar_));
                bar_tap_beat_ = ob;
                bar_tap_int_beat_phase_ = bar_int;

                if(beat_count_from_bar_>0)
                {
                    beats_per_bar = beat_count_from_bar_+1;
                    beat_count_from_bar_=0;
                }
            }
            else
            {
                beat_count_from_bar_++;
            }

            pic::logmsg() << "bar_beat="<<bar_tap_beat_ << " bar_phase=" << bar_tap_int_beat_phase_ << " beat=" << beats_per_bar_ << " counter=" << beat_count_from_bar_;

            if(avg_tempo != tempo_)
            {
                tempo_=avg_tempo;
                // set tempo atom
                tchange_(piw::makefloat(avg_tempo,0));
            }

            if(beats_per_bar != beats_per_bar_)
            {
                // number of beats in a bar
                beats_per_bar_=beats_per_bar;
                // set beat atom
                bchange_(piw::makefloat(beats_per_bar,0));
            }
        }

        // ------------------------------------------------------------------------------
        // midi clock tempo calculation functions
        // ------------------------------------------------------------------------------

        void midi_clock_start()
        {
#if MIDI_CLK_DEBUG>0
            pic::logmsg() << "------- midi clock start -------";
#endif // MIDI_CLK_DEBUG>0
            midi_clk_state_=MIDI_CLK_STATE_WAIT;

        }

        void midi_clock_continue(unsigned long long t)
        {
#if MIDI_CLK_DEBUG>0
            pic::logmsg() << "------- midi clock continue ------- " << last_beat_;
#endif // MIDI_CLK_DEBUG>0
            midi_clk_state_=MIDI_CLK_STATE_CONTINUE;
        }

        void midi_clock_stop(unsigned long long t)
        {
#if MIDI_CLK_DEBUG>0
            pic::logmsg() << "------- midi clock stop ------- " << last_beat_;
#endif // MIDI_CLK_DEBUG>0
            midi_clk_state_=MIDI_CLK_STATE_IDLE;
            //midi_clk_tick_num_=0;

            pinger_stop(t);

        }

        void midi_clock_song_position(const unsigned char *data, unsigned long long time)
        {
            unsigned msb = (unsigned)data[2];
            unsigned lsb = (unsigned)data[1];
            midi_clk_song_pos_ = (double)((msb<<7)+lsb);
#if MIDI_CLK_DEBUG>0
            pic::logmsg() << "------- midi clock song position ------- 1/16=" << midi_clk_song_pos_ << " 1/4=" << (midi_clk_song_pos_/4);
#endif // MIDI_CLK_DEBUG>0
            if(midi_clk_state_==MIDI_CLK_STATE_RUN)
            {
                // if already running then stop first
                pinger_stop(time);
            }

            midi_clk_tick_num_=0;

            midi_clk_state_=MIDI_CLK_STATE_SONG_POS;
            midi_clk_last_time_ = time;


        }

        void midi_clock_beat_position(const unsigned char *data, unsigned long long time)
        {
            unsigned msb = (unsigned)data[2];
            unsigned lsb = (unsigned)data[1];
            midi_clk_song_pos_ = (double)((msb<<7)+lsb);
#if MIDI_CLK_DEBUG>0
            pic::logmsg() << "------- midi clock beat position ------- 1/16=" << midi_clk_song_pos_ << " 1/4=" << (midi_clk_song_pos_/4);
#endif // MIDI_CLK_DEBUG>0
            midi_clk_tick_num_=0;

            midi_clk_state_=MIDI_CLK_STATE_BEAT_POS;
            midi_clk_last_time_ = time;
        }

        void midi_clock_tick(unsigned long long time, unsigned long long from, unsigned long long to)
        {
            // received midi clock tick
            //pic::logmsg() << "midi clock tick  state = " << midi_clk_state_;

            // calc tempo from clock tick period
            double avg_tempo = 0;
            double last_tempo = time2tempo((time-midi_clk_last_time_)*24);
            if(midi_clk_first_tick_)
            {
                midi_clk_first_tick_ = false;
                midi_clk_avg_tempo_1_ = last_tempo;

                midi_clk_tempo_sum_ = 0;
                midi_clk_avg_cnt_ = 0;
                midi_clk_avg_time_start_ = time;
                avg_tempo = tempo_;
            }

            // filter to average the tempo signal
/*
            if(last_tempo <= upper_ && last_tempo >= lower_)
            {
                avg_tempo = midi_clk_avg_tempo_1_ + 0.5*(last_tempo-midi_clk_avg_tempo_1_);
                midi_clk_avg_tempo_1_ = avg_tempo;
            }
            else
            {
                // tempo out of range so stick to the last good tempo
                avg_tempo = midi_clk_avg_tempo_1_;
            }
*/

            if(!midi_clk_first_tick_)
            {
                midi_clk_avg_cnt_++;
                midi_clk_tempo_sum_ += last_tempo;
                avg_tempo = midi_clk_tempo_sum_/midi_clk_avg_cnt_;

                double r = avg_tempo/last_tempo;

                // average tempo differing from tempo from current period so reset point to calculate average from
                if(r < 0.90 || r >1.10)
                {
#if MIDI_CLK_DEBUG>0
                    pic::logmsg() << "midi clock reset average tempo " << avg_tempo << ' ' << last_tempo << ' ' << r;
#endif // MIDI_CLK_DEBUG>0
                    avg_tempo=last_tempo;
                    midi_clk_avg_time_start_=time;
                    midi_clk_tempo_sum_ = 0;
                    midi_clk_avg_cnt_=0;
                }
            }

#if MIDI_CLK_DEBUG>0
            pic::logmsg() << "midiclk tick: n="<<midi_clk_tick_num_<<" t="<<time<<" lt="<<midi_clk_last_time_<<" dt="<<(time-midi_clk_last_time_)<<" tem="<<last_tempo<<" run="<<midi_clk_state_ << " atem=" << avg_tempo;
            if(time<from || time>to)
                pic::logmsg() << "tick out of clock bounds";
#endif // MIDI_CLK_DEBUG>0

            if(midi_clk_state_==MIDI_CLK_STATE_WAIT)
            {
                // waiting after start message, then run now on this tick
                // this tick is the actual starting beat

                // also start if ticked when idle,
                // not inline to the MIDI spec, but some clock sources seem to do this!

                midi_clk_tick_num_ = 0;
                midi_clk_beat_ = 0;
                beat_origin_ = midi_clk_beat_;
                beat_origin_time_ = time;

#if MIDI_CLK_DEBUG>0
                pic::logmsg() << "starting running at beat origin=" << beat_origin_ << " time=" << beat_origin_time_ << " midi clk tick="<<midi_clk_tick_num_ <<" tempo="<< tempo_;
#endif // MIDI_CLK_DEBUG>0

                midi_clk_state_ = MIDI_CLK_STATE_RUN;

                last_beat_ = beat_origin_;
                last_beat_time_=time;

                midi_clk_first_tick_ = true;

                pinger_prime(from, to, beat_origin_, time);

                midi_clk_tick_num_++;
                midi_clk_beat_ += 1/24.0;
                midi_clk_last_time_ = time;

                midi_clk_exp_time_ = midi_clk_last_time_ + tempo2time(tempo_)/24;

                return;
            }

            if(midi_clk_state_==MIDI_CLK_STATE_CONTINUE)
            {
                // waiting after the continue message, can follow a song position
                // this tick is the actual starting beat

                // calculate the starting beat

                // starting tick number
                midi_clk_tick_num_ = (unsigned)(fmod((midi_clk_song_pos_*6), 24.0));
                // starting beat
                midi_clk_beat_ = midi_clk_song_pos_/4;

                beat_origin_ = midi_clk_beat_;
                beat_origin_time_ = time;

#if MIDI_CLK_DEBUG>0
                pic::logmsg() << "continue running at beat origin=" << beat_origin_ << " time=" << beat_origin_time_ << " midi clk tick="<<midi_clk_tick_num_ <<" tempo="<< tempo_;
#endif // MIDI_CLK_DEBUG>0

                midi_clk_state_ = MIDI_CLK_STATE_RUN;

                last_beat_ = beat_origin_;
                last_beat_time_=time;

                midi_clk_first_tick_ = true;

                pinger_prime(from, to, beat_origin_, time);

                midi_clk_tick_num_++;
                midi_clk_beat_ += 1/24.0;
                midi_clk_last_time_ = time;

                midi_clk_exp_time_ = midi_clk_last_time_ + tempo2time(tempo_)/24;

                return;
            }

            if(midi_clk_state_==MIDI_CLK_STATE_SONG_POS || midi_clk_state_==MIDI_CLK_STATE_BEAT_POS)
            {
                // after a song position, calculate the starting beat value

                if(midi_clk_tick_num_>0 && (time-midi_clk_last_time_)>MIDI_CLK_DATA_PERIOD)
                {
                    // calculate the starting beat

                    // fraction into a beat at start, from counting the ticks rapidly issued after a song pos
                    double beat_frac = ((double)(midi_clk_tick_num_))/24.0;
                    // starting tick number
                    midi_clk_tick_num_ = (unsigned)(fmod((midi_clk_song_pos_*6 + (double)(midi_clk_tick_num_)), 24.0));
                    // starting beat
                    midi_clk_beat_ = midi_clk_song_pos_/4 + beat_frac;

                    beat_origin_ = midi_clk_beat_;
                    // adjust for desired latency
                    beat_origin_time_ = midi_clk_latency_>=0?(time + (unsigned long long)midi_clk_latency_):(time - (unsigned long long)(abs(midi_clk_latency_)));
#if MIDI_CLK_DEBUG>0
                    pic::logmsg() << "starting running at beat origin=" << beat_origin_ << " time=" << beat_origin_time_ << " midi clk tick="<<midi_clk_tick_num_ <<" tempo="<< tempo_;
#endif // MIDI_CLK_DEBUG>0

                    midi_clk_state_ = MIDI_CLK_STATE_RUN;

                    last_beat_ = beat_origin_;
                    last_beat_time_=time;

                    // reset the tempo averaging filter
                    midi_clk_first_tick_ = true;

                    pinger_prime(from, to, beat_origin_, time);

                    midi_clk_tick_num_++;
                    midi_clk_beat_ += 1/24.0;
                    midi_clk_last_time_ = time;

                    midi_clk_exp_time_ = midi_clk_last_time_ + tempo2time(tempo_)/24;

                    return;

                }

                midi_clk_tick_num_++;
                midi_clk_last_time_ = time;

                return;

            }

            if(midi_clk_state_==MIDI_CLK_STATE_RUN)
            {
                beat_origin_ = midi_clk_beat_;
                beat_origin_time_ = midi_clk_latency_>=0?(time + (unsigned long long)midi_clk_latency_):(time - (unsigned long long)(abs(midi_clk_latency_)));
#if MIDI_CLK_DEBUG>0
                pic::logmsg() << "  midi clk set beat origin = "<<beat_origin_ << " bo time=" << beat_origin_time_ << " n="<<midi_clk_tick_num_;
#endif // MIDI_CLK_DEBUG>0
                midi_clk_tick_num_++;

                midi_clk_beat_ += 1/24.0;

                if(midi_clk_tick_num_==24)
                {
#if MIDI_CLK_DEBUG>0
                    pic::logmsg() << "midi clk end of beat, starting new beat";
#endif // MIDI_CLK_DEBUG>0
                    midi_clk_tick_num_ = 0;
                }

                if(avg_tempo > upper_ || avg_tempo < lower_)
                {
                    // tempo out of range
                    if(last_tempo > upper_)
                        pic::logmsg() << "warning: midi clock tempo out of range, ignored tempo=" << last_tempo << ">" << upper_;
                    if(last_tempo < lower_)
                        pic::logmsg() << "warning: midi clock tempo out of range, ignored tempo=" << last_tempo << "<" << lower_;
                }
                else
                {
                    if(avg_tempo != tempo_)
                    {
                        tempo_=avg_tempo;
                    }
                }

                midi_clk_exp_time_ = 2*time-midi_clk_last_time_;

                midi_clk_last_time_ = time;

                return;

            }


        }



        // ------------------------------------------------------------------------------
        // interface functions
        // ------------------------------------------------------------------------------

        void start()
        {
            if(state_==STATE_IDLE)
                state_=STATE_STARTING;
        }

        void stop()
        {
            if(state_==STATE_PLAYING)
                state_=STATE_STOPPING;
        }

        void toggle()
        {
            if(state_==STATE_IDLE)
            {
                state_=STATE_STARTING;
            }
            else
            {
                if(state_==STATE_PLAYING)
                    state_=STATE_STOPPING;
            }
        }

        void set_tempo(float t)
        {
            // check tempo in set tempo range
            if(t>=lower_ && t<=upper_)
            {
                pic::logmsg() << "set tempo " << t << " from " << tempo_;

                if(t!=tempo_)
                {
                    beat_origin_ = last_beat_;
                    beat_origin_time_ = last_beat_time_;
                    tempo_=t;
                    // set the agent tempo atom
                    tchange_(piw::makefloat(t,0));
                    // if not running then make sure tempo output is set
                    if(!(state_==STATE_STARTING || state_==STATE_PLAYING))
                        tempo_output_->set_tempo(tempo_, piw::tsd_time());
                }
            }
        }

        void set_beats(float b)
        {
            if(b!=beats_per_bar_)
            {
                beats_per_bar_=b;
                // set the agent beat atom
                bchange_(piw::makefloat(b,0));
            }
        }

        void set_range(float u, float l)
        {
            // set the tempo range
            pic::logmsg() << "tempo range " << l << "->" << u;
            lower_=l;
            upper_=u;
        }

        void start_preroll(unsigned p)
        {
            pic::logmsg() << "preroll: " << p;

            if(p)
            {
                if(!preroll_)
                {
                    // set the agent preroll atom to the preroll count
                    pchange_(piw::makebool(true,0));
                    preroll_=p;
                }
            }
            else
            {
                if(preroll_)
                {
                    // set the agent preroll atom to no preroll
                    pchange_(piw::makebool(false,0));
                    preroll_=0;
                }
            }
        }

        void midi_clock_enable(bool b)
        {
            if(!midi_clock_enabled_ && b)
            {
                midi_clk_state_ = MIDI_CLK_STATE_IDLE;
                pinger_stop(piw::tsd_time());                
            }

            midi_clock_enabled_=b;
        }

        void midi_clock_set_latency(float latency)
        {
            // set the latency given in ms
            midi_clk_latency_ = (int)(latency*1000);
        }

        void set_beat_flash_persistence(unsigned time_)
        {
            persistence_time_=time_;
        }

        // ------------------------------------------------------------------------------

        // running state
        int state_;
        // midi clock sync state
        bool midi_clock_enabled_;
        // output buffer status
        bool output_started_;
        // current tempo value
        float tempo_;
        // last tempo value
        float last_tempo_;
        // tempo lower limit
        float lower_;
        // tempo upper limit
        float upper_;

        // last beat value
        double last_beat_;
        // time of last beat value
        unsigned long long last_beat_time_;
        // time of last tap
        unsigned long long last_tap_time_;
        // the number of beats per bar (~time signature)
        float beats_per_bar_;
        // integer beat value
        double beat_origin_;
        // time of integer beat value
        unsigned long long beat_origin_time_;
        // beat value the bar was tapped at
        double bar_tap_beat_;
        // int beat value of the beat within the bar that the bar was tapped at (what is this for?)
        double bar_tap_int_beat_phase_;
        // beats counted from bar tap
        unsigned beat_count_from_bar_;
        // last bar beat
        int last_bar_beat_;
        // beat flash persistence
        unsigned long long persistence_;
        // beat flash persistence time
        unsigned persistence_time_;
        // preroll beats
        unsigned preroll_;

        // time that next int beat should fall on
        unsigned long long adjust_target_time_;
        // new tempo to adjust to
        float adjust_target_tempo_;

        // start time of tapping averaging to calculate average tempo
        unsigned long long avg_tap_time_start_;
        // number of taps to average tempo over
        unsigned avg_tap_cnt_;

        // midi clock running state
        unsigned midi_clk_state_;
        // previous time a midi clk event received
        unsigned long long midi_clk_last_time_;
        // current midi clock tick number in beat (0-23)
        unsigned midi_clk_tick_num_;
        // midi clk song position
        double midi_clk_song_pos_;
        // state variable of midi clk tempo averaging filter
        double midi_clk_avg_tempo_1_;
        // flag to indicate first tick after starting midi clk for calculating the starting tempo
        bool midi_clk_first_tick_;
        // latency adjustment for midi clock timing
        int midi_clk_latency_;
        // expected time of next midi clk tick
        unsigned long long midi_clk_exp_time_;
        // beat value from midi clock
        double midi_clk_beat_;


        unsigned midi_clk_avg_cnt_;
        unsigned long long midi_clk_avg_time_start_;
        double midi_clk_tempo_sum_;

        // functors for setting agent atoms
        pic::flipflop_functor_t<piw::change_t> tchange_;
        pic::flipflop_functor_t<piw::change_t> bchange_;
        pic::flipflop_functor_t<piw::change_t> pchange_;

        // bundle stuff
        bct_clocksink_t *upstream_clk_;
        piw::xevent_data_buffer_t output_;
        tempo_output_t *tempo_output_;
        tap_wire_t *active_tap_wire_;
        midi_clock_wire_t *active_midi_clock_wire_;
        status_output_t *status_output_;

    };



    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // pinger static functions for slow->fast thread calls
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    static int __set_tempo(void *impl_, void *f_)
    {
        ((pinger_t::impl_t *)impl_)->set_tempo(*(float *)f_);
        return 0;
    }

    static int __set_beats(void *impl_, void *f_)
    {
        ((pinger_t::impl_t *)impl_)->set_beats(*(float *)f_);
        return 0;
    }

    static int __set_range(void *impl_, void *u_, void *l_)
    {
        ((pinger_t::impl_t *)impl_)->set_range(*(float *)u_,*(float *)l_);
        return 0;
    }

    static int __start_preroll(void *impl_, void *f_)
    {
        ((pinger_t::impl_t *)impl_)->start_preroll(*(unsigned *)f_);
        return 0;
    }

    static int __start(void *impl_, void *f_)
    {
        ((pinger_t::impl_t *)impl_)->start();
        return 0;
    }

    static int __stop(void *impl_, void *f_)
    {
        ((pinger_t::impl_t *)impl_)->stop();
        return 0;
    }
    
    static int __toggle(void *impl_, void *f_)
    {
        ((pinger_t::impl_t *)impl_)->toggle();
        return 0;
    }

    static int __midi_clock_enable(void *impl_, void *e_)
    {
        ((pinger_t::impl_t *)impl_)->midi_clock_enable(*(bool *)e_);
        return 0;
    }

    static int __midi_clock_set_latency(void *impl_, void *latency_)
    {
        ((pinger_t::impl_t *)impl_)->midi_clock_set_latency(*(float *)latency_);
        return 0;
    }

    static int __set_beat_flash_persistence(void *impl_, void *time_)
    {
        ((pinger_t::impl_t *)impl_)->set_beat_flash_persistence(*(unsigned *)time_);
        return 0;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // pinger interface class member functions
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    pinger_t::pinger_t(const piw::cookie_t &c, const piw::cookie_t &c1, const piw::cookie_t &c3, piw::clockdomain_ctl_t *d,const piw::change_t &tc,const piw::change_t &bc,const piw::change_t &pr) : impl_(new impl_t(c,c1,c3,d,tc,bc,pr)) {}
    pinger_t::~pinger_t() { delete impl_; }
    void pinger_t::set_tempo(float t) { piw::tsd_fastcall(__set_tempo,impl_,&t); }
    void pinger_t::set_beats(float b) { piw::tsd_fastcall(__set_beats,impl_,&b); }
    void pinger_t::set_range(float ub, float lb) { piw::tsd_fastcall3(__set_range,impl_,&ub,&lb); }
    void pinger_t::play() { piw::tsd_fastcall(__start,impl_,0); }
    void pinger_t::stop() { piw::tsd_fastcall(__stop,impl_,0); }
    void pinger_t::toggle() { piw::tsd_fastcall(__toggle,impl_,0); }
    void pinger_t::start_preroll(unsigned p) { piw::tsd_fastcall(__start_preroll,impl_,&p); }
    void pinger_t::midi_clock_enable(bool e) { piw::tsd_fastcall(__midi_clock_enable,impl_,&e); }
    void pinger_t::midi_clock_set_latency(float l) { piw::tsd_fastcall(__midi_clock_set_latency,impl_,&l);  }
    void pinger_t::set_beat_flash_persistence(unsigned t) { piw::tsd_fastcall(__set_beat_flash_persistence,impl_,&t);  }
    piw::cookie_t pinger_t::cookie() { return piw::cookie_t(impl_); }

    int pinger_t::gc_traverse(void *v, void *a) const
    {
        int r;
        if((r=impl_->tchange_.gc_traverse(v,a))!=0) return r;
        if((r=impl_->pchange_.gc_traverse(v,a))!=0) return r;
        if((r=impl_->bchange_.gc_traverse(v,a))!=0) return r;
        return 0;
    }

    int pinger_t::gc_clear()
    {
        impl_->tchange_.gc_clear();
        impl_->pchange_.gc_clear();
        impl_->bchange_.gc_clear();
        return 0;
    }
}



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// tap input class member functions
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

tap_wire_t::tap_wire_t(loop::pinger_t::impl_t *pinger, const piw::event_data_source_t &es) : pinger_(pinger)
{
    subscribe(es);
}

tap_wire_t::~tap_wire_t()
{
    unsubscribe();
}

void tap_wire_t::event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &data)
{
    if(!iterator_.isvalid())
    {
        iterator_ = pic::ref(new piw::evtiterator_t(pic::ref(new piw::event_data_t)));
    }

    iterator_->set_signal(1,data.signal(1));
    iterator_->set_signal(2,data.signal(2));
    iterator_->reset(1,id.time());
    iterator_->reset(2,id.time());

    pinger_->active_tap_wire_ = this;
}

void tap_wire_t::event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    iterator_->set_signal(sig,n);
    iterator_->reset(sig,t);
}

bool tap_wire_t::event_end(unsigned long long)
{
    iterator_->clear_signal(1);
    iterator_->clear_signal(2);

    return true;
}

void tap_wire_t::ticked(unsigned long long from,unsigned long long t)
{
    unsigned s;
    piw::data_nb_t d;
    while(iterator_->next(3,s,d,t))
    {
        switch(s)
        {
            case 1:
                // get beat tapping
                pinger_->tap(d.time(),false);
                break;

            case 2:
                // get bar tapping
                pinger_->tap(d.time(),true);
                break;
        }
    }
}



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi clock input class member functions
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

midi_clock_wire_t::midi_clock_wire_t(loop::pinger_t::impl_t *pinger, const piw::event_data_source_t &es) : pinger_(pinger)
{
    //pic::logmsg() << "creating midi clock wire";
    subscribe(es);
}

midi_clock_wire_t::~midi_clock_wire_t()
{
    unsubscribe();
}

void midi_clock_wire_t::event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &data)
{
    //pic::logmsg() << "midi_clock_wire event_start";

    if(!iterator_.isvalid())
    {
        iterator_ = pic::ref(new piw::evtiterator_t(pic::ref(new piw::event_data_t)));
    }

    iterator_->set_signal(1,data.signal(1));
    iterator_->reset(1,id.time());

    pinger_->active_midi_clock_wire_ = this;
}

void midi_clock_wire_t::event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    iterator_->set_signal(sig,n);
    iterator_->reset(sig,t);
}

bool midi_clock_wire_t::event_end(unsigned long long)
{
    iterator_->clear_signal(1);

    return true;
}

void midi_clock_wire_t::ticked(unsigned long long from,unsigned long long t)
{

    unsigned s;
    piw::data_nb_t d;
    while(iterator_->next(1,s,d,t))
    {
        if(s==1)
        {
            unsigned char *data = (unsigned char *)d.as_blob();
            unsigned len = d.as_bloblen();

            //pic::logmsg() << "midi_clock_wire_t::ticked f=" << from << " t=" << t << " d=" << std::hex << (unsigned)data[0] << " l=" << len;

            // decode 1 byte messages
            if(len==1)
            {
                switch(data[0])
                {
                case 0xf8:
                    // clock tick
                    pinger_->midi_clock_tick(d.time(), from, t);
                    break;
                case 0xfa:
                    // start the metronome
                    pinger_->midi_clock_start();
                    break;
                case 0xfb:
                    // continue
                    pinger_->midi_clock_continue(d.time());
                    break;
                case 0xfc:
                    // stop the metronome
                    pinger_->midi_clock_stop(d.time());
                    break;
                }
            }
            // decode 3 byte messages
            if(len==3)
            {
                switch(data[0])
                {
                case 0xf8:
                    // clock tick with MIDI beat position
                    pinger_->midi_clock_beat_position(data, d.time());
                    break;
                case 0xf2:
                    // song position, can indicate a midi clock start
                    pinger_->midi_clock_song_position(data, d.time());
                    break;
                }

            }
        }
    }
}







