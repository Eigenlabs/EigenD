
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
#include <piw/piw_delay.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_clockclient.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>
#include <piw/piw_address.h>
#include <vector>

#include <stdio.h>


// define the default delay line length
#define DEFAULT_MAX_DELAY_TIME (0.1)
#define DEFAULT_DELAY_LENGTH ((unsigned)(DEFAULT_MAX_DELAY_TIME*48000))
#define DELAY_SEG_ADDR_WIDTH 14
#define DELAY_SEG_SIZE 16384             // = 2^DELAY_SEG_ADDR_WIDTH
#define DELAY_SEG_MASK (DELAY_SEG_SIZE-1)

#define DELAY_RUNNING 1
#define DELAY_IDLE 2
#define DELAY_ENABLE_FADEDOWN 3
#define DELAY_ENABLE_FADEUP 4
#define DELAY_ENABLE_FADE_TIME (0.1f)

#define DELAY_INTERP_SAMPS 48000

// debug level
// 0: off
// 1: lo
// 2: hi
#define DELAY_DEBUG 0

// TODO: tasks
// 1. filter set/unset                  done
// 2. filter cutoff                     done
// 3. feedback limiting                 done
// 4. tap create interval               done
// 5. make delay length unlimited       done
// 6. beat matching delay time          done
// 7. extend to surround                done (to be integrated)
// 8. wet/dry mix                       done
// 9. slaving channels/taps together    done
//10. testing scripts                   done
//11. panner on feedback                done
//12. test state recreation             done
//13. effect send

// sound improvements
// 1. dezipper master feedback          done
//    (and other parameters?)
// 2. sweep delay time changes
// 3. adjustable curve on mix parameter
// 4. fractional delays

namespace
{
    struct interp_param_t
    {
        interp_param_t(float interp_samps) : interp_samps_(interp_samps), current_(0), target_(0), step_(0) {}
        interp_param_t() : interp_samps_(DELAY_INTERP_SAMPS), current_(0), target_(0), step_(0) {}

        void set(float target)
        {
            target_ = target;
            step_ = (target_-current_)/interp_samps_;
        }

        void set_immediately(float value)
        {
            target_ = value;
            current_ = value;
            step_ = 0;
        }

        float get()
        {
            if(current_ != target_)
            {
                if(current_ < target_)
                {
                    current_ += step_;
                    if(current_ > target_)
                        current_ = target_;
                }
                else
                {
                    current_ += step_;
                    if(current_ < target_)
                        current_ = target_;
                }
            }
            return current_;
        }

        void set_interp_samples(float interp_samps)
        {
            if(interp_samps_==0)
                interp_samps_=1;
            interp_samps_ = interp_samps;
        }

        float interp_samps_;
        float current_;
        float target_;
        float step_;

    };

    enum                            // delay line state
    {
        delay_norm = 0,             // normal state
        delay_inc,                  // increase in size state
        delay_dec                   // decrease in size state
    };

    struct delay_line_seg_t : pic::lckobject_t, pic::atomic_counted_t
    {
        delay_line_seg_t()
        {
#if DELAY_DEBUG>1
            pic::logmsg() << "delay_line_seg_t constructor";
#endif // DELAY_DEBUG>1
            memset((void *)data,0,sizeof(float)*DELAY_SEG_SIZE);
        }
        ~delay_line_seg_t() {}
        float data[DELAY_SEG_SIZE];
    };

    struct delay_line_t
    {
        delay_line_t() : num_segs_(1)
        {
#if DELAY_DEBUG>1
            pic::logmsg() << "delay_line_t constructor";
#endif // DELAY_DEBUG>1
            // initialize with one segment
            seg_ptrs_.resize(num_segs_);
            seg_ptrs_[0] = pic::ref(new delay_line_seg_t);
        }
        ~delay_line_t() {}

        void resize(unsigned &size)
        {
            // takes size in samples, and returns size rounded up to
            // the total size of the segments

            // new size
            unsigned num_segs = size>>DELAY_SEG_ADDR_WIDTH;
            if(size&DELAY_SEG_MASK)
                num_segs++;

            if(num_segs != num_segs_)
            {
#if DELAY_DEBUG>1
                pic::logmsg() << "delay_line_t::resize num_segs=" << num_segs << " size=" << size << " inc=" << num_segs-num_segs_ << " new size=" << (num_segs*DELAY_SEG_SIZE);
#endif // DELAY_DEBUG>1
                seg_ptrs_.resize(num_segs);
                if(num_segs>num_segs_)
                    for(unsigned i=num_segs_; i<num_segs; i++)
                        seg_ptrs_[i] = pic::ref(new delay_line_seg_t);

                // set size to new size
                num_segs_ = num_segs;
                // set the size in samples to total size of segments
                size = num_segs_*DELAY_SEG_SIZE;
            }
        }

        inline float &operator[](unsigned index)
        {
            return seg_ptrs_[index>>DELAY_SEG_ADDR_WIDTH].ptr()->data[(index&DELAY_SEG_MASK)];
        }

        unsigned num_segs_;
        pic::lckvector_t< pic::ref_t<delay_line_seg_t> > :: nbtype seg_ptrs_;

    };

    struct tap_channel_t                            // single channel of tap
    {
        unsigned        read_ptr_;                  // current read pointer
        unsigned        wrap_pos_;                  // pointer wrap position (changes with delay line length)
        bool            wrap_start_;                // change wrap pos when pointer at start
        interp_param_t  time_samples_;              // delay time in samples
        float           time_seconds_;              // delay time in seconds
        float           time_beats_;                // delay time in beats
        bool            time_is_beats_;             // delay time is in beats/seconds (true/false)
        float           feedback_gain_;             // feedback gain of tap
        bool            filter_enabled_;            // filter on/off
        float           filter_g_;                  // cutoff coefficient
        float           filter_cutoff_;             // cutoff frequency
        float           filter_state_1_;            // filter pole 1 y(n-1) state
        float           filter_state_2_;            // filter pole 2 y(n-1) state
        float           cross_feedback_gain_[2];    // cross feedback gain to all channels
        float           output_gain_[2];            // output gain to all channels
    };

    struct tap_data_t                                               // single tap
    {
        unsigned tap_num_;                                          // user tap number
        pic::lckvector_t< tap_channel_t >::nbtype channels_;        // tap channels
    };

    typedef pic::lckvector_t< tap_data_t >::nbtype delay_data_t;    // all taps

    bool latest(unsigned sig, piw::data_nb_t &d, piw::cfilterenv_t *e, unsigned long long t)
    {
        bool got = false;
        while(e->cfilterenv_nextsig(sig,d,t))
        {
            got = true;
        }
        return got;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // delay function (cfilterfunc) class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct delayfunc_t : piw::cfilterfunc_t
    {
        delayfunc_t() : interp_(2), default_tap_interval_(0),
                        wet_dry_mix_(512), master_feedback_(512), master_feedback_offset_(512), fade_gain_(512),
                        num_taps_(0), write_ptr_(0), delay_line_len_(DEFAULT_DELAY_LENGTH),
                        tempo_(120), last_tempo_(120), enabled_(true), sample_rate_(48000),
                        shutdown_(false), idle_(true), lingering_(false)
        {
            // initialize delay lines

            // TODO: add more channels
            // set number of channels
            delay_line_.resize(2);
            // first delay_line_t constructor called by vector.resize so start at 1!
            for(unsigned i=1; i<2; i++)
                delay_line_[i] = delay_line_t();

            for(unsigned c=0; c<2; c++)
            {
                // set delay line sizes
                delay_line_[c].resize(delay_line_len_);
            }

            // clear the silence buffer
            memset(silence_,0,PLG_CLOCK_BUFFER_SIZE*sizeof(float));

            state_ = DELAY_RUNNING;
            fade_samples_ = PLG_CLOCK_BUFFER_SIZE;
            fade_gain_.set(1.0f);

            // initialize power variables
            power_ = 0;
            power_ptr_ = 0;

        }

        unsigned long long cfilterfunc_thru()
        {
            return 0;
        }

        // prime is called when an event starts, e.g. a key press
        // usually reset state in prime because processing a new event
        // env: filter environment, stores data about , e.g. sampling frequency
        // id: event id
        //
        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {

#if DELAY_DEBUG>1
            pic::logmsg() << "delay prime";
            pic::logmsg() << "num taps " << num_taps_;
            pic::logmsg() << "write ptr " << write_ptr_;
            pic::logmsg() << "delay length " << delay_line_len_;
            pic::logmsg() << "wet/dry mix " << wet_dry_mix_.get();
            pic::logmsg() << "master feedback " << (master_feedback_.get()+master_feedback_offset_.get());

            for(unsigned t=0; t<num_taps_; t++)
            {
                pic::logmsg() << "tap " << t << ":";
                pic::logmsg() << "ind " << t << "  num " << delay_data_[t].tap_num_;
                pic::logmsg() << "l ptr " << delay_data_[t].channels_[0].read_ptr_ << "  r ptr " << delay_data_[t].channels_[1].read_ptr_;
                pic::logmsg() << "l time " << delay_data_[t].channels_[0].time_seconds_ << "  r time " << delay_data_[t].channels_[1].time_seconds_;
                pic::logmsg() << "l pos " << delay_data_[t].channels_[0].wrap_pos_ << "  r pos " << delay_data_[t].channels_[1].wrap_pos_;
                pic::logmsg() << "l start " << delay_data_[t].channels_[0].wrap_start_ << "  r start " << delay_data_[t].channels_[1].wrap_start_;
                pic::logmsg() << "l gain " << delay_data_[t].channels_[0].feedback_gain_ << "  r gain " << pic::logmsg() << delay_data_[t].channels_[1].feedback_gain_;
                pic::logmsg() << "l filter " << delay_data_[t].channels_[0].filter_enabled_ << "  r filter " << delay_data_[t].channels_[1].filter_enabled_;
                pic::logmsg() << "l cutoff " << delay_data_[t].channels_[0].filter_g_ << "  r cutoff " << delay_data_[t].channels_[1].filter_g_;
                pic::logmsg() << "";
            }
#endif // DELAY_DEBUG>1

            input(env, id.time());

            lingering_ = false;

#if DELAY_DEBUG>1
            pic::logmsg() << "state_="<<state_<<" enabled_="<<enabled_<<" idle_="<<idle_<<" gain="<<fade_gain_.current_;
#endif // DELAY_DEBUG>1
            return true;
        }

        void input(piw::cfilterenv_t *env, unsigned long long t)
        {
            piw::data_nb_t d;

            // audio input
            // TODO: add more channels
            for(unsigned i=0; i<2; i++)
            {
                if(latest(i+1,d,env,t))		// pull latest data for signal i+1 (channel i) out of the buffer into d
                {
                    last_audio_[i].set_nb(d);     // store data object
                }
            }

            // wet/dry mix
            if(latest(3,d,env,t))
            {
                //pic::logmsg() << "wet/dry " << d;
                wet_dry_mix_.set(d.as_renorm(0,1,0));
            }

            // master feedback
            if(latest(4,d,env,t))
            {
                //pic::logmsg() << "master feedback " << d;
                master_feedback_.set(d.as_renorm(0,2,0));
            }

            // tempo
            if(latest(5,d,env,t))
            {
                //pic::logmsg() << "tempo  " << d;
                tempo_ = d.as_renorm_float(BCTUNIT_BPM,0,100000,0);
            }

            // master feedback
            if(latest(6,d,env,t))
            {
                //pic::logmsg() << "feedback offset  " << d;
                master_feedback_offset_.set(d.as_renorm(-2,2,0));
            }

        }

        // Process here!
        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
//            static unsigned ticked = 0;
//            pic::logmsg() << "ticked " << ticked;
//            ticked++;
            double sample_rate = (double)(e->cfilterenv_clock()->get_sample_rate());

            input(e, t);

            shutdown_ = false;
            idle_ = false;

            // if the tempo changes, move the taps!
            if(tempo_!=last_tempo_)
            {
#if DELAY_DEBUG>1
                pic::logmsg() << "tempo changed to " << tempo_;
#endif // DELAY_DEBUG>1
                last_tempo_=tempo_;

                set_taps_to_tempo(sample_rate);
            }
            else
            {
                if(sample_rate!=sample_rate_)
                {
                    reset_delay_lines__(this, 0);
                    set_taps_to_samplerate(sample_rate);
                    sample_rate_=sample_rate;
                }
            }

            // create output buffer
            float *buffer_out[2],*scalar[2];
            const float *buffer_in[2];
            piw::data_nb_t audio_out[2];

            // TODO: add more channels
            for(unsigned i=0; i<2; i++)
            {
                // get audio input pointers
                if(last_audio_[i].get().is_array())
                    buffer_in[i] = last_audio_[i].get().as_array();
                else
                    buffer_in[i] = silence_;

                // get audio output pointers
                audio_out[i] = piw::makenorm_nb(t, bs,&buffer_out[i], &scalar[i]);

            }

            // TODO: currently assume we must have stereo channels so do nothing until stereo arrives
            // this will be resolved when more channels added and cfilter API will change soon
            // so handle this whichever happens first!
//            if(&buffer_in[0][0] == 0 || &buffer_in[1][0] == 0)
//            {
//                //pic::logmsg() << "delay ticked: missing buffer bailing out!";
//                return true;
//            }

            float ch_feedback[2];
            float ch_wet[2];

            // ------- process audio -------
            for(unsigned i=0; i<bs; i++)
            {
                for(unsigned c=0; c<2; c++)
                {
                    ch_feedback[c] = 0;
                    ch_wet[c] = 0;
                }

                // ------- read from delay line -------
                for(unsigned c=0; c<2; c++)
                {
                    for(unsigned t=0; t<num_taps_; t++)
                    {
                        tap_channel_t &tap_ch = delay_data_[t].channels_[c];

                        // interpolate from the delay line
                        float delay = tap_ch.time_samples_.get();
                        float delay_floor = floor(delay);

                        // interpolation coeffs
                        float alpha = delay-delay_floor;
                        float om_alpha = 1-alpha;

                        // sample pointers
                        int read_ptr = (int)write_ptr_ - (int)delay_floor;
                        if(read_ptr<0)
                            read_ptr += (int)tap_ch.wrap_pos_;
                        
                        int read_ptr_1 = read_ptr-1;
                        if(read_ptr_1<0)
                            read_ptr_1 += (int)tap_ch.wrap_pos_;

                        float tap_output = delay_line_[c][read_ptr] * om_alpha + delay_line_[c][read_ptr_1] * alpha;

                        tap_ch.read_ptr_ = (unsigned)read_ptr_1;

                        // update wrap position
                        if(tap_ch.wrap_start_)
                        {
                            // read ptr has crossed end of delay line
                            if(read_ptr_1 == 0)
                            {
                                // the delay line length has increased so
                                // increase the tap channel wrap pos when just wrapped
                                tap_ch.wrap_pos_ = delay_line_len_;
                                tap_ch.wrap_start_ = false;
                            }
                        }


                        if(tap_ch.filter_enabled_)
                        {
                            // apply tap 2-pole low-pass filter, + 1e-18 - 1e-18 is some magic to stop Intel denormal problems!
                            float &filter_state_1 = tap_ch.filter_state_1_;
                            float &filter_state_2 = tap_ch.filter_state_2_;
                            float &filter_g = tap_ch.filter_g_;
                            tap_output = filter_state_1 + filter_g * (tap_output - filter_state_1) + 1e-18 - 1e-18;
                            filter_state_1 = tap_output;
                            tap_output = filter_state_2 + filter_g * (tap_output - filter_state_2) + 1e-18 - 1e-18;
                            filter_state_2 = tap_output;
                        }

                        // apply tap feedback gain
                        tap_output = tap_ch.feedback_gain_*tap_output;

                        // mix tap output to feedback and output
                        for(unsigned c1=0; c1<2; c1++)
                        {
                            ch_feedback[c1] += tap_ch.cross_feedback_gain_[c1]*tap_output;
                            ch_wet[c1] += tap_ch.output_gain_[c1]*tap_output;
                        }

                    } // tap

                } // channel

                // ------- write to delay line -------
                float out = 0;
                for(unsigned c=0; c<2; c++)
                {

                    // apply master feedback gain
                    ch_feedback[c] *= (master_feedback_.get()+master_feedback_offset_.get());

                    // add input to delay line input
                    float dry = buffer_in[c][i];
                    float delay_line_in = ch_feedback[c] + dry;

                    // saturate delay line input
                    delay_line_in = pic::approx::tanh(delay_line_in);
                    // TODO: test if hard limit needed
                    // if(accum>1) accum=1;

                    delay_line_[c][write_ptr_] = delay_line_in;

                    // ------- write to output buffers -------

                    if(state_!=DELAY_IDLE)
                    {
                        // not disabled so hear wet and dry audio
                        // (1-wet_dry_mix_)*dry + wet_dry_mix_*fade_gain_*wet;
                        buffer_out[c][i] = dry + wet_dry_mix_.get() * (fade_gain_.get() * ch_wet[c] - dry);
                    }
                    else
                    {
                        // disabled so only hear dry level audio
                        buffer_out[c][i] = (1 - wet_dry_mix_.get() ) * dry;
                    }

                    /*
                    // ------- update read pointers -------
                    for(unsigned t=0; t<num_taps_; t++)
                    {
                        tap_channel_t &tap_ch = delay_data_[t].channels_[c];
                        unsigned &read_ptr = tap_ch.read_ptr_;
                        read_ptr++;
                        read_ptr = read_ptr % tap_ch.wrap_pos_;

                        if(tap_ch.wrap_start_)
                        {
                            // read ptr has crossed end of delay line
                            if(read_ptr == 0)
                            {
                                // the delay line length has increased so
                                // increase the tap channel wrap pos when just wrapped
                                tap_ch.wrap_pos_ = delay_line_len_;
                                tap_ch.wrap_start_ = false;
#if DELAY_DEBUG>1           
                                pic::logmsg() << "wrap read pointer now tap " << delay_data_[t].tap_num_ << " ch " << c;
#endif // DELAY_DEBUG>1           
                            }
                        }
                    } // tap
*/

                    // sum the signals across the delay lines for peak power calculation
                    out += delay_line_[c][power_ptr_];


                } // channel

                // calculate peak power in signal
#ifdef PI_WINDOWS
                if (power_<(out*out))
					power_ = (out*out);
#else

                power_ = fmaxf(power_, out*out);
#endif
                power_ptr_++;
                if(power_ptr_==delay_line_len_)
                {
                    power_ptr_ = 0;
                    //pic::logmsg() << "power = " << power_;
                    // reached end of buffer, evaluate the power
                    if(power_<1e-6)
                    {
                        // if the power is effectively 0
                        // then allow shutdown
                        shutdown_ = true;
                    }
                    power_ = 0;
                }


                // ------- update write pointers -------
                write_ptr_++;
                write_ptr_ = write_ptr_ % delay_line_len_;

            } // sample

            if(state_==DELAY_ENABLE_FADEUP)
            {
#if DELAY_DEBUG>1
                pic::logmsg() << "fade up " << fade_count_;
#endif // DELAY_DEBUG>1
                fade_count_ += bs;

                if(fade_count_ >= fade_samples_)
                {
                    state_=DELAY_RUNNING;
                    fade_count_ = 0;
                 }
            }

            if(state_==DELAY_ENABLE_FADEDOWN)
            {
#if DELAY_DEBUG>1
                pic::logmsg() << "fade down " << fade_count_;
#endif // DELAY_DEBUG>1
                fade_count_ += bs;

                if(fade_count_ >= fade_samples_)
                {
                    state_=DELAY_IDLE;
                    enabled_ = false;
                    fade_count_ = 0;
                }
            }



            // TODO: add more channels
            for(unsigned i=0; i<2; i++)
            {
                // set scalar to last value
                *scalar[i]=buffer_out[i][bs-1];
                // output processed audio
                e->cfilterenv_output(i+1, audio_out[i]);
                // clear the last_audio_ inputs to prevent the input buffer cycling
                // when the input event stop
                last_audio_[i].clear_nb();
            }

            if(lingering_)
            {
                // if no power in output and not fading then go idle
                if(shutdown_ && (state_==DELAY_RUNNING || state_==DELAY_IDLE))
                {
#if DELAY_DEBUG>1
                    pic::logmsg() << "delay: going idle";
#endif // DELAY_DEBUG>1
                    idle_ = true;
                    lingering_ = false;

                    return false;
                }
            }

            return true;
        }

        void set_enable(bool enable)
        {
            if(enable!=enabled_)
            {
                if(enable)
                {
                    if(!idle_)
                    {
#if DELAY_DEBUG>1
                        pic::logmsg() << "enable fade";
#endif // DELAY_DEBUG>1
                        fade_gain_.set_interp_samples((float)fade_samples_);
                        fade_gain_.set(1.0f);
                        state_ = DELAY_ENABLE_FADEUP;
                        fade_count_=0;
                    }
                    else
                    {
                        // process is idle, so make enable change happen immediately
#if DELAY_DEBUG>1
                        pic::logmsg() << "enable set";
#endif // DELAY_DEBUG>1
                        state_ = DELAY_RUNNING;
                        fade_gain_.set_immediately(1.0f);
                    }
                    enabled_ = true;
                }
                else
                {
                    if(!idle_)
                    {
#if DELAY_DEBUG>1
                        pic::logmsg() << "disable fade";
#endif // DELAY_DEBUG>1
                        fade_gain_.set_interp_samples((float)fade_samples_);
                        fade_gain_.set(0.0f);
                        state_ = DELAY_ENABLE_FADEDOWN;
                        fade_count_=0;
                    }
                    else
                    {
                        // process is idle, so make enable change happen immediately
#if DELAY_DEBUG>1
                        pic::logmsg() << "disable set";
#endif // DELAY_DEBUG>1
                        state_ = DELAY_IDLE;
                        fade_gain_.set_immediately(0.0f);
                        enabled_ = false;
                    }
                }
            }

        }





        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
        {
#if DELAY_DEBUG>1
            pic::logmsg() << "cfilter_linger";
#endif // DELAY_DEBUG>1

            lingering_ = true;

            return true;
        }

        void find_tap(unsigned tap_num, unsigned &index)
        {
            // find the tap in the tap vector
            for(unsigned i=0; i<num_taps_; i++)
            {
                //pic::logmsg() << "i = " << i << " delay_data_[i].tap_num_=" << delay_data_[i].tap_num_ << " tap_num=" << tap_num;

                if(delay_data_[i].tap_num_ == tap_num)
                {
                    index = i;
                    //pic::logmsg() << "found index = " << index;
                    break;
                }
            }
            // TODO: return error if not found
        }

        bool get_tap_time_beats(unsigned tap_num, unsigned channel, float &time_beats)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);
            if(delay_data_[tap_index].channels_[channel].time_is_beats_)
            {
                time_beats = delay_data_[tap_index].channels_[channel].time_beats_;
                return true;
            }
            else
            {
                return false;
            }
        }

        bool get_tap_time_secs(unsigned tap_num, unsigned channel, float &time_seconds)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);
            if(!delay_data_[tap_index].channels_[channel].time_is_beats_)
            {
                time_seconds = delay_data_[tap_index].channels_[channel].time_seconds_;
                return true;
            }
            else
            {
                return false;
            }
        }

        void get_tap_filter_cutoff(unsigned tap_num, unsigned channel, float &cutoff)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            cutoff = delay_data_[tap_index].channels_[channel].filter_cutoff_;
        }

        void set_tap_time_secs(unsigned tap_num, unsigned channel, float time_seconds, float sample_rate)
        {
#if DELAY_DEBUG>1
            pic::logmsg() << "set_tap_time_secs " << time_seconds;
#endif // DELAY_DEBUG>1

            float time_samples = sample_rate*time_seconds;
            if(time_samples==0)
                time_samples=1;
            set_tap_time_samps(tap_num, channel, time_samples, sample_rate);

            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);
            delay_data_[tap_index].channels_[channel].time_seconds_ = time_seconds;
            delay_data_[tap_index].channels_[channel].time_is_beats_ = false;
        }

        void set_tap_time_beats(unsigned tap_num, unsigned channel, float time_beats, float sample_rate)
        {
#if DELAY_DEBUG>1
            pic::logmsg() << "set_tap_time_beats " << time_beats;
#endif // DELAY_DEBUG>1

            float time_samples = time_beats*(sample_rate/(tempo_/60));
            if(0 == tempo_)
            {
                time_samples = 0;
            }
            else
            {
                time_samples = time_beats*(sample_rate/(tempo_/60));
            }
            set_tap_time_samps(tap_num, channel, time_samples, sample_rate);

            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);
            delay_data_[tap_index].channels_[channel].time_beats_ = time_beats;
            delay_data_[tap_index].channels_[channel].time_is_beats_ = true;
        }

        void set_tap_time_samps(unsigned tap_num, unsigned channel, float time_samples, float sample_rate)
        {
            unsigned time_int_samples = (unsigned)(ceil(time_samples));
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            tap_channel_t &tap = delay_data_[tap_index].channels_[channel];

#if DELAY_DEBUG>1
            pic::logmsg() << "set_tap_time ch " << channel << " tn " << tap_num << " ti " << tap_index << " sam " << time_int_samples;
#endif // DELAY_DEBUG>1

            // do we need to grow the delay lines to allow for the delay time?
            if(time_int_samples > delay_line_len_)
            {
#if DELAY_DEBUG>1
                pic::logmsg() << "increase length from " << delay_line_len_ << " to " << time_int_samples;
#endif // DELAY_DEBUG>1
                delay_line_len_ = time_int_samples;
                // increase delay line length
                for(unsigned c=0; c<2; c++)
                {
                    // TODO: increase all channels, increase only a single channel?
                    delay_line_[c].resize(delay_line_len_);

                    // update all tap read pointer wrap positions
                    for(unsigned t=0; t<num_taps_; t++)
                    {
                        tap_channel_t &tap_ch = delay_data_[t].channels_[c];

                        // find other read ptrs
                        if(!(tap_index==t && channel==c))
                        {
                            if(tap_ch.read_ptr_>=write_ptr_)
                            {
                                // wrap any other read ptrs that are after the write ptr
                                // when they get back to start of the delay line
                                tap_ch.wrap_start_ = true;
#if DELAY_DEBUG>1
                                pic::logmsg() << "wrap other read pointer later " << delay_data_[t].tap_num_ << " ch " << c;
#endif // DELAY_DEBUG>1
                            }
                            else
                            {
                                tap_ch.wrap_start_ = false;
                                tap_ch.wrap_pos_ = delay_line_len_;
#if DELAY_DEBUG>1
                                pic::logmsg() << "wrap other read pointer immediately " << delay_data_[t].tap_num_ << " ch " << c;
#endif // DELAY_DEBUG>1
                            }
                        }
                    }
                }

#if DELAY_DEBUG>1
                pic::logmsg() << "grow with this tap, wrap this read pointer immediately " << delay_data_[tap_index].tap_num_ << " ch " << channel;
#endif // DELAY_DEBUG>1
                tap.wrap_start_ = false;
                tap.wrap_pos_ = delay_line_len_;

            }
            else
            {
#if DELAY_DEBUG>1
                pic::logmsg() << "wrap this read pointer immediately " << delay_data_[tap_index].tap_num_ << " ch " << channel;
#endif // DELAY_DEBUG>1
                tap.wrap_start_ = false;
                tap.wrap_pos_ = delay_line_len_;
            }

            tap.time_samples_.set(time_samples);
            tap.time_samples_.set_interp_samples(sample_rate*0.5);

            // TODO: what is -ve % behaviour? use % instead?
/*
            // don't calculate read ptr position here...
            int ptr = (int)write_ptr_ - (int)time_int_samples;
            if(ptr < 0)
                ptr += (int)delay_line_len_;
            tap.read_ptr_ = ptr;
*/
        }

        void set_taps_to_tempo(double sample_rate)
        {

            for(unsigned t=0; t<num_taps_; t++)
            {
                for(unsigned c=0; c<2; c++)
                {
                    float time_beats = 0;
                    // change delay time if the tap time is set in beats
                    if(get_tap_time_beats(t+1, c, time_beats))
                    {
                        set_tap_time_beats(t+1, c, time_beats, sample_rate);
#if DELAY_DEBUG>1
                        pic::logmsg() << "setting tap " << (t+1) << " ch " << c << " to tempo";
#endif // DELAY_DEBUG>1
                    }
                }
            }

        }

        void set_taps_to_samplerate(double sample_rate)
        {

            for(unsigned t=0; t<num_taps_; t++)
            {
                for(unsigned c=0; c<2; c++)
                {
                    float time_beats = 0;
                    float time_seconds = 0;
                    float cutoff = 0;
                    // change delay time if the tap time is set in beats
                    if(get_tap_time_beats(t+1, c, time_beats))
                    {
                        set_tap_time_beats(t+1, c, time_beats, sample_rate);
#if DELAY_DEBUG>1
                        pic::logmsg() << "setting tap " << (t+1) << " ch " << c << " to sample rate";
#endif // DELAY_DEBUG>1
                    }
                    else
                    {
                        get_tap_time_secs(t+1, c, time_seconds);
                        set_tap_time_secs(t+1, c, time_seconds, sample_rate);
#if DELAY_DEBUG>1
                        pic::logmsg() << "setting tap " << (t+1) << " ch " << c << " to sample rate";
#endif // DELAY_DEBUG>1
                    }
                    get_tap_filter_cutoff(t+1, c, cutoff);
                    set_tap_filter_cutoff(t+1, c, cutoff, sample_rate);
                }
            }

        }

        void set_tap_feedback_gain(unsigned tap_num, unsigned channel, float gain)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            pic::logmsg() << "ch " << channel << " tn " << tap_num << " ti " << tap_index << " fbk " << gain;

            delay_data_[tap_index].channels_[channel].feedback_gain_ = gain;

        }

        void set_tap_cross_feedback_gain(unsigned tap_num, unsigned channel, unsigned to_channel, float gain)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            pic::logmsg() << "ch " << channel << " to ch " << to_channel << " tn " << tap_num << " ti " << tap_index << " fbk " << gain;

            delay_data_[tap_index].channels_[channel].cross_feedback_gain_[to_channel] = gain;

        }

        void set_tap_output_gain(unsigned tap_num, unsigned channel, unsigned to_channel, float gain)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            pic::logmsg() << "ch " << channel << " to ch " << to_channel << " tn " << tap_num << " ti " << tap_index << " fbk " << gain;

            delay_data_[tap_index].channels_[channel].output_gain_[to_channel] = gain;

        }

        void set_tap_enable_filter(unsigned tap_num, unsigned channel, bool enable)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            pic::logmsg() << "ch " << channel << " tn " << tap_num << " ti " << tap_index << " filter " << enable;

            delay_data_[tap_index].channels_[channel].filter_enabled_ = enable;

        }

        void set_tap_filter_cutoff(unsigned tap_num, unsigned channel, float cutoff, float sample_rate)
        {
            unsigned tap_index = 0;
            find_tap(tap_num, tap_index);

            float filter_g = (1.0f-expf(-2.0f*PIC_PI*(cutoff/sample_rate)));

            pic::logmsg() << "ch " << channel << " tn " << tap_num << " ti " << tap_index << " cutoff " << cutoff << " g " << filter_g;

            delay_data_[tap_index].channels_[channel].filter_cutoff_ = cutoff;
            delay_data_[tap_index].channels_[channel].filter_g_ = filter_g;

        }

        void cfilterfunc_changed(piw::cfilterenv_t *, void *p_)
        {
            default_tap_interval_ = *(float *)p_;
        }

        static int add_tap__(void *self_, void *tap_num_)
        {
            // add a tap to the tap data vector, appends to end

            delayfunc_t *self = (delayfunc_t *)self_;
            unsigned *tap_num = (unsigned *)tap_num_;
            // next free tap index = num taps
            unsigned t = self->num_taps_;
            delay_data_t &d = self->delay_data_;

            pic::logmsg() << "add tap tn " << *tap_num << " ti " << t;

            d.resize(self->num_taps_+1);

            d[t].tap_num_ = *tap_num;
            d[t].channels_.resize(2);
            // initialize data
            for(unsigned c=0; c<2; c++)
            {
                d[t].channels_[c].read_ptr_ = 0;
                d[t].channels_[c].wrap_pos_ = self->delay_line_len_;
                d[t].channels_[c].wrap_start_ = false;
                d[t].channels_[c].time_samples_.set_immediately(0);
                d[t].channels_[c].feedback_gain_ = 0;
                d[t].channels_[c].filter_state_1_ = 0;
                d[t].channels_[c].filter_state_2_ = 0;
            }

            self->num_taps_++;

            return 1;
        }

        static int erase_tap__(void *self_, void *tap_num_)
        {
            // erase a tap from the tap data vector, remove in place

            delayfunc_t *self = (delayfunc_t *)self_;
            unsigned *tap_num = (unsigned *)tap_num_;
            unsigned tap_index = 0;
            self->find_tap(*tap_num, tap_index);

            pic::logmsg() << "erase tap tn " << *tap_num << " ti " << tap_index;

            delay_data_t &d = self->delay_data_;
            d.erase(d.begin()+tap_index);

            self->num_taps_--;
            return 1;
        }

        static int reset_delay_lines__(void *self_, void *null_)
        {
            delayfunc_t *self = (delayfunc_t *)self_;
            // TODO: add more channels
            for(unsigned c=0; c<2; c++)
            {
                // reset the delay line
                for(unsigned i=0; i<self->delay_line_len_; i++)
                    self->delay_line_[c][i] = 0.0f;
                // reset filter state
                for(unsigned t=0; t<self->num_taps_; t++)
                {
                    self->delay_data_[t].channels_[c].filter_state_1_ = 0;
                    self->delay_data_[t].channels_[c].filter_state_2_ = 0;
                }

            }
            return 1;
        }



        void set_enable_time(float time)
        {
            // enable fade time in ms
            fade_samples_ = (unsigned)((float)sample_rate_*(time/1000.0f));
        }


        // audio buffers
        // TODO: add more channels
        piw::dataholder_nb_t last_audio_[2];

        // silence buffer when no input
        float silence_[PLG_CLOCK_BUFFER_SIZE];

        // delay lines
        pic::lckvector_t< delay_line_t >::nbtype delay_line_;

        // tap data
        delay_data_t delay_data_;

        // clock interpolator
        piw::clockinterp_t interp_;

        // global parameters
        float default_tap_interval_;
        interp_param_t wet_dry_mix_;
        interp_param_t master_feedback_;
        interp_param_t master_feedback_offset_;
        interp_param_t fade_gain_;

        // global variables
        unsigned num_taps_;
        unsigned write_ptr_;
        unsigned delay_line_len_;
        double   tempo_;
        double   last_tempo_;
        bool     enabled_;
        float    sample_rate_;
        unsigned state_;
        unsigned fade_count_;
        unsigned fade_samples_;
        bool     shutdown_;
        bool     idle_;
        bool     lingering_;

        // power filter variables
        float    power_;
        unsigned power_ptr_;

    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // tap function (cfilterfunc) class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct tapfunc_t: public piw::cfilterfunc_t, public pic::element_t<0>
    {
        // slow thread functions
        tapfunc_t(delayfunc_t *delay, unsigned tap_num, unsigned tap_index): delay_(delay), tap_num_(tap_num), tap_index_(tap_index), is_initialized_(false)
        {
        }
        ~tapfunc_t()
        {
        }

        // fast thread functions
        unsigned long long cfilterfunc_thru() { return 0; }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            // set default parameters
            input(env, id.time());

            return true;
        }

        void input(piw::cfilterenv_t *env, unsigned long long t)
        {
            piw::data_nb_t d;
            float sample_rate = (float)(env->cfilterenv_clock()->get_sample_rate());

            // pic::logmsg() << "tap param";

            // TODO: factor this code!!

            // ------- left channel parameters

            // transfer parameters to audio processing as soon as data arrives
            if(latest(1,d,env,t))
            {
                float time = d.as_renorm(-4,4,0);
                if(time<=0)
                {
                    // time in seconds
                    delay_->set_tap_time_secs(tap_num_, 0, fabsf(time), sample_rate);
                }
                else
                {
                    // time in beats
                    delay_->set_tap_time_beats(tap_num_, 0, time, sample_rate);
                }

            }

            if(latest(2,d,env,t))
            {
                float feedback_gain = d.as_renorm(-2,2,0);
                delay_->set_tap_feedback_gain(tap_num_, 0, feedback_gain);
            }

            if(latest(3,d,env,t))
            {
                bool b = d.as_norm()!=0.0;
                delay_->set_tap_enable_filter(tap_num_, 0, b);
            }

            if(latest(4,d,env,t))
            {
                float cutoff = d.as_renorm(0,20000,0);
                delay_->set_tap_filter_cutoff(tap_num_, 0, cutoff, sample_rate);
            }

            if(latest(5,d,env,t))
            {
                float feedback_gain = d.as_renorm(0,1,0);
                delay_->set_tap_cross_feedback_gain(tap_num_, 0, 0, feedback_gain);
            }

            if(latest(6,d,env,t))
            {
                float feedback_gain = d.as_renorm(0,1,0);
                delay_->set_tap_cross_feedback_gain(tap_num_, 0, 1, feedback_gain);
            }

            if(latest(7,d,env,t))
            {
                float output_gain = d.as_renorm(0,1,0);
                delay_->set_tap_output_gain(tap_num_, 0, 0, output_gain);
            }

            if(latest(8,d,env,t))
            {
                float output_gain = d.as_renorm(0,1,0);
                delay_->set_tap_output_gain(tap_num_, 0, 1, output_gain);
            }

            //------- right channel parameters

            if(latest(9,d,env,t))
            {
                float time = d.as_renorm(-4,4,0);
                if(time<=0)
                {
                    // time in seconds
                    delay_->set_tap_time_secs(tap_num_, 1, fabsf(time), sample_rate);
                }
                else
                {
                    // time in beats
                    delay_->set_tap_time_beats(tap_num_, 1, time, sample_rate);
                }
            }

            if(latest(10,d,env,t))
            {
                float feedback_gain = d.as_renorm(-2,2,0);
                delay_->set_tap_feedback_gain(tap_num_, 1, feedback_gain);
            }

            if(latest(11,d,env,t))
            {
                bool b = d.as_norm()!=0.0;
                delay_->set_tap_enable_filter(tap_num_, 1, b);
            }

            if(latest(12,d,env,t))
            {
                float cutoff = d.as_renorm(0,20000,0);
                delay_->set_tap_filter_cutoff(tap_num_, 1, cutoff, sample_rate);
            }

            if(latest(13,d,env,t))
            {
                float feedback_gain = d.as_renorm(0,1,0);
                delay_->set_tap_cross_feedback_gain(tap_num_, 1, 0, feedback_gain);
            }

            if(latest(14,d,env,t))
            {
                float feedback_gain = d.as_renorm(0,1,0);
                delay_->set_tap_cross_feedback_gain(tap_num_, 1, 1, feedback_gain);
            }

            if(latest(15,d,env,t))
            {
                float output_gain = d.as_renorm(0,1,0);
                delay_->set_tap_output_gain(tap_num_, 1, 0, output_gain);
            }

            if(latest(16,d,env,t))
            {
                float output_gain = d.as_renorm(0,1,0);
                delay_->set_tap_output_gain(tap_num_, 1, 1, output_gain);
            }

        }

        // process
        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs)
        {
            input(e, to);

            return true;
        }

        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
        {
            return true;
        }

        delayfunc_t *delay_;
        // user tap number
        unsigned tap_num_;
        // index of tap in dsp vector
        unsigned tap_index_;
        //
        bool is_initialized_;
    };


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // audio cfilter class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct audio_cfilter_t: piw::cfilterctl_t, piw::cfilter_t
    {
        audio_cfilter_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d), delay_() {}

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &path)
        {
            return &delay_;
        }

        void cfilterctl_delete(piw::cfilterfunc_t *f) { }

        delayfunc_t delay_;
    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // tap cfilter class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    struct tap_cfilter_t: piw::cfilterctl_t, piw::cfilter_t
    {
        tap_cfilter_t(piw::clockdomain_ctl_t *clock_domain, delayfunc_t *delay)
            : cfilter_t(this,piw::cookie_t(0),clock_domain), delay_(delay), num_taps_(0) {}

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &path)
        {
            if(path.as_pathlen()==0)
            {
                return 0;
            }

            unsigned tap_num = path.as_path()[0];
            tapfunc_t *tf = new tapfunc_t(delay_, tap_num, num_taps_);

            piw::tsd_fastcall(delayfunc_t::add_tap__, delay_, &tap_num);
            num_taps_++;
            // return a new tap function
            return tf;
        }

        void cfilterctl_delete(piw::cfilterfunc_t *f)
        {
            tapfunc_t *tap = (tapfunc_t *)f;

            piw::tsd_fastcall(delayfunc_t::erase_tap__, delay_, &tap->tap_num_);
            tap->remove();
            num_taps_--;
            cfilterctl_t::cfilterctl_delete(f);
        }

        // pointer to the audio filterfunc
        delayfunc_t *delay_;
        // number of taps
        unsigned num_taps_;
    };

} // namespace



namespace piw
{

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // delay implementation class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // declare implementation class within delay class
    struct delay_t::impl_t
    {
        // also construct the audio and tap classes
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : audio_(o,d), tap_(d,&audio_.delay_) {}

        audio_cfilter_t audio_;
        tap_cfilter_t tap_;
    };



    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // delay class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // implement outer class that holds the implementation class
    delay_t::delay_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    delay_t::~delay_t() { delete impl_; }
    piw::cookie_t delay_t::audio_cookie() { return impl_->audio_.cookie(); }
    piw::cookie_t delay_t::tap_cookie() { return impl_->tap_.cookie(); }
    void delay_t::reset_delay_lines()
    {
        piw::tsd_fastcall(delayfunc_t::reset_delay_lines__, &impl_->audio_.delay_, 0);
    }
    void delay_t::set_enable(bool enable) { impl_->audio_.delay_.set_enable(enable); }
    void delay_t::set_enable_time(float time) { impl_->audio_.delay_.set_enable_time(time); }

} // namespace piw



