
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

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * pi_zita_convolver.cpp : port of the Zita Convolution Engine
 *
 *
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include <piw/piw_tsd.h>
#include <plg_convolver/src/plg_convolver.h>
#include <plg_convolver/src/zita_convolver.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_clockclient.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>
#include <piw/piw_address.h>
#include <lib_samplerate/lib_samplerate.h>
#include <vector>

#include <stdio.h>

// running states
#define CONVOLVER_RUNNING 1
#define CONVOLVER_FADEDOWN 2
#define CONVOLVER_IDLE 3
#define CONVOLVER_FADEUP 4
#define CONVOLVER_ENABLE_FADEDOWN 5
#define CONVOLVER_ENABLE_FADEUP 6

#define CONVOLVER_FADE_TIME (0.5f)
#define CONVOLVER_ENABLE_FADE_TIME (0.1f)

// TODO: change the fade time

// debug level
// 0: off
// 1: lo
// 2: hi
#define CONVOLVER_DEBUG 0

// TODO:
//
//  1. mono mode - convolve a mono or stereo IR with a mono input signal    done
//  2. denormal issue?                                                      done
//  3. fade impulse changes                                                 done
//  4. optimization from vector mode, does this make a difference?          done
//  5. mono mode testing, does this reduce CPU time?                        done
//  6. sample rate notification to resample impulse when idle               done
//  7. normalize impulses for different sample rates

namespace
{

    // TODO: factor these out
    struct interp_param_t
    {
        interp_param_t(float interp_samps) : interp_samps_(interp_samps), current_(0), target_(0), step_(0) {}

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
    // convolver cfilterfunc class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct convolver_cfilterfunc_t : piw::cfilterfunc_t
    {
        convolver_cfilterfunc_t() :
            conv_engine_(0), wet_dry_mix_(512), fade_gain_(512), mono_(false), sample_rate_(48000), buffer_size_(PLG_CLOCK_BUFFER_SIZE),
            imp_resp_(), fade_count_(0), fade_samples_(0), enable_fade_samples_(0), linger_count_(0), linger_num_(1), lingering_(false),
            updating_(false), gain_(1.0f)
        {
#if CONVOLVER_DEBUG>0
            pic::logmsg() << "convolverfunc_t";
#endif // CONVOLVER_DEBUG>0

//            processing_.set(false);

            // clear the silence buffer
            memset(silence_,0,PLG_CLOCK_BUFFER_SIZE*sizeof(float));

            conv_engine_ = new Convlevel();

            // TODO: set partitions according to impulse responses?
            conv_engine_->configure( 0,                      // offset
                                     1,                      // max num pars in impulse response
                                     buffer_size_,  // par size
                                     buffer_size_,  // out step
                                     FFTW_ESTIMATE,          // FFTW planning
                                     0 );                    // vector optimization enable

            // initialize engine
            for(unsigned c=0; c<2; c++)
            {

                // input and output buffer for convolution engines
                // buffers normally created in management class
                inpbuff_[c] = new float [PLG_CLOCK_BUFFER_SIZE*2];
                memset(inpbuff_[c], 0, PLG_CLOCK_BUFFER_SIZE*2 * sizeof (float));
                outbuff_[c] = new float [PLG_CLOCK_BUFFER_SIZE];
                memset(outbuff_[c], 0, PLG_CLOCK_BUFFER_SIZE * sizeof (float));

            }

            for(unsigned c=0; c<2; c++)
            {
                float gain = 1.0;

                // initialize with a valued dirac at t=0, no effect
                // step is 1, only single value of essentially mono impulse
                conv_engine_->impdata_create(c, c, 1, &gain, 0, 1);
            }

            // prepare engine, pass input buffers
            conv_engine_->prepare(buffer_size_*2, inpbuff_);

            inpoffs_ = 0;

            state_ = CONVOLVER_IDLE;
            enabled_ = false;
            idle_ = true;
            enable_fade_samples_ = PLG_CLOCK_BUFFER_SIZE;
            fade_gain_.set(gain_);

        }

        ~convolver_cfilterfunc_t()
        {
            delete conv_engine_;
        }


        void set_sample_rate(float sample_rate, unsigned buffer_size)
        {
            sample_rate_=sample_rate;
            buffer_size_=buffer_size;

            switch((unsigned)sample_rate_)
            {
            case 44100:
            case 48000:
                gain_=1;
                break;
            case 88200:
            case 96000:
                // must halve output presumably because buffer size remains constant,
                // twice as many ffts are performed at higher rates which perhaps is
                // not normalized by the zita convolver
                gain_=0.5;
                break;
            default:
                gain_=1;
            }
        }

        void set_impl_sample_rate(float sample_rate,unsigned buffer_size)
        {
            // set the sample rate for the current impulse
            if(sample_rate!=sample_rate_ || buffer_size!=buffer_size_)
            {
#if CONVOLVER_DEBUG>0
                pic::logmsg() << "responding to sample rate change";
#endif // CONVOLVER_DEBUG>0
                set_sample_rate(sample_rate,buffer_size);
                // if an impulse response is loaded, then update to new sample rate
                if(imp_resp_.isvalid())
                    set_impulse_response(imp_resp_);
            }
        }

        void set_impulse_response(plg_convolver::samplearray2ref_t &imp_resp)
        {
#if CONVOLVER_DEBUG>0
            pic::logmsg() << "setting impulse  enabled=" << enabled_ << " idle=" << idle_;
#endif // CONVOLVER_DEBUG>0
            updating_ = true;

            if(enabled_ && !idle_)
            {
#if CONVOLVER_DEBUG>1
                pic::logmsg() << "setting impulse enabled"; fflush(stdout);
#endif // CONVOLVER_DEBUG>1

                state_ = CONVOLVER_FADEDOWN;
                fade_gain_.set(0.0f);
                fade_count_=0;
#if CONVOLVER_DEBUG>1
                pic::logmsg() << "convolver set impulse: start fadedown, waiting for lock"; fflush(stdout);
#endif // CONVOLVER_DEBUG>0
                // wait until processing thread ready for update
                update_.untimeddown();
#if CONVOLVER_DEBUG>1
                pic::logmsg() << "convolver set impulse: unlocked state=" << state_ ; fflush(stdout);
#endif // CONVOLVER_DEBUG>1
            }
#if CONVOLVER_DEBUG>0
            pic::logmsg() << "updating"; fflush(stdout);
#endif // CONVOLVER_DEBUG>0

            // set the operating mode mono/stereo (not the channels in the impulse)
            mono_ = new_mono_;

            imp_resp_ = imp_resp;
            unsigned chans = imp_resp_->chans;

            if(conv_engine_)
            {
                conv_engine_->cleanup();

                // flag to see if a copy of the impulse is made
                bool new_data = false;
                float *imp_resp_data = 0;
                unsigned long imp_resp_size = 0;

                // perform sample rate conversion of impulse to current sample rate
                // and stereo to mono conversion
                if(imp_resp_->rate != sample_rate_ || (chans==2 && mono_))
                {
                    // need copy of data if resampling or converting stereo to mono

                    // find peak of original impulse
                    float peak = 0;
                    for(unsigned i=0; i<imp_resp_->size; i++)
                        if(fabs(imp_resp_->data[i]) > peak)
                            peak = fabs(imp_resp_->data[i]);

                    SRC_DATA src_data;

                    src_data.data_in = imp_resp_->data;
                    src_data.input_frames = (long)(imp_resp_->size/(unsigned long)chans);
                    src_data.src_ratio = (double)(sample_rate_/imp_resp_->rate);
                    src_data.output_frames = (long)(ceil(src_data.src_ratio*(double)src_data.input_frames));

                    new_data = true;
                    imp_resp_data = (float *)malloc(sizeof(float)*src_data.output_frames*(long)chans);
                    if(!imp_resp_data)
                        fprintf(stderr, "warning: Out of memory in resampling impulse response.");

                    src_data.data_out = imp_resp_data;

                    // do the conversion
                    int result = src_simple(&src_data, SRC_SINC_BEST_QUALITY, chans);

                    imp_resp_size = (unsigned long)(src_data.output_frames_gen*chans);

                    // find peak of resampled impulse
                    peak = 0;
                    for(unsigned i=0; i<imp_resp_size; i++)
                        if(fabs(imp_resp_data[i]) > peak)
                            peak = fabs(imp_resp_data[i]);

                    if(result)
                    {
                        fprintf(stderr, "warning: Convolver impulse response resampling failed.");
                        imp_resp_size = 0;
                    }
                    else
                    {
#if CONVOLVER_DEBUG>0
                        pic::logmsg() << "resampled impulse response  isize=" << src_data.input_frames << " osize=" << imp_resp_size << " result=" << result;
#endif // CONVOLVER_DEBUG>0
                    }

                    if(chans==2 && mono_)
                    {
#if CONVOLVER_DEBUG>0
                        pic::logmsg() << "converting from stereo to mono";
#endif // CONVOLVER_DEBUG>0

                        // convert from stereo to mono, average the channels in place
                        for(unsigned i=0; i<imp_resp_size/2; i++)
                        {
                            imp_resp_data[i] = (imp_resp_data[i*2] + imp_resp_data[(i*2)+1])/2.0;
                        }
                        // now have a mono impulse
                        chans=1;
                        imp_resp_size = imp_resp_size/2;
                    }

                }
                else
                {
                    // use original impulse data
                    imp_resp_data = imp_resp_->data;
                    imp_resp_size = imp_resp_->size;
                }

                // configure the convolver engine

                unsigned max_pars = (unsigned)(imp_resp_size/buffer_size_)+1;
                // linger tail length in buffers is length of impulse response in buffers
                linger_num_ = max_pars;

                conv_engine_->configure( 0,                      // offset
                                         max_pars,               // max num pars in impulse response
                                         buffer_size_,  // par size
                                         buffer_size_,  // out step
                                         FFTW_ESTIMATE,          // FFTW planning
                                         0 );                    // vector optimization enable


                if(chans == 1)
                {
#if CONVOLVER_DEBUG>0
                    pic::logmsg() << "mono impulse response";
#endif // CONVOLVER_DEBUG>0
                    conv_engine_->impdata_create(0, 0, 1, imp_resp_data, 0, imp_resp_size);
                    // if stereo processing and mono impulse, then copy to both channels
                    if(!mono_)
                        conv_engine_->impdata_create(1, 1, 1, imp_resp_data, 0, imp_resp_size);
                }
                if(chans == 2)
                {
#if CONVOLVER_DEBUG>0
                    pic::logmsg() << "stereo impulse response";
#endif // CONVOLVER_DEBUG>0
                    // set the impulse response data for each channel
                    conv_engine_->impdata_create(0, 0, 2, imp_resp_data, 0, (imp_resp_size/2));
                    conv_engine_->impdata_create(1, 1, 2, (imp_resp_data+1), 0, (imp_resp_size/2));
                }

                conv_engine_->prepare(buffer_size_*2, inpbuff_);

                inpoffs_ = 0;

                if(new_data)
                {
                    free(imp_resp_data);
                }

            }

            if(enabled_)
            {
                if(!idle_)
                {
                    state_ = CONVOLVER_FADEUP;
                    fade_gain_.set(gain_);
                    fade_count_=0;
#if CONVOLVER_DEBUG>1
                    pic::logmsg() << "start fadeup";
#endif // CONVOLVER_DEBUG>1
                }
                else
                {
                    // process is idle, so make enable change happen immediately
                    state_ = CONVOLVER_RUNNING;
                    fade_gain_.set_immediately(gain_);
                }
            }

            updating_ = false;
        }

        void set_mono_processing(bool mono)
        {
            // update mono flag when faded down and reinitialize impulse response
            new_mono_ = mono;
            if(imp_resp_.ptr()!=0)
                set_impulse_response(imp_resp_);
        }

        void set_enable(bool enable)
        {
            if(enable!=enabled_)
            {
                if(enable)
                {
                    if(!idle_)
                    {
#if CONVOLVER_DEBUG>1
                        pic::logmsg() << "enable fade";
#endif // CONVOLVER_DEBUG>1
                        fade_gain_.set_interp_samples((float)enable_fade_samples_);
                        fade_gain_.set(gain_);
                        state_ = CONVOLVER_ENABLE_FADEUP;
                        fade_count_=0;
                    }
                    else
                    {
                        // process is idle, so make enable change happen immediately
                        state_ = CONVOLVER_RUNNING;
                        fade_gain_.set_immediately(gain_);
                    }
                    enabled_ = true;
                }
                else
                {
                    if(!idle_)
                    {
#if CONVOLVER_DEBUG>1
                        pic::logmsg() << "disable fade";
#endif // CONVOLVER_DEBUG>1
                        fade_gain_.set_interp_samples((float)enable_fade_samples_);
                        fade_gain_.set(0.0f);
                        state_ = CONVOLVER_ENABLE_FADEDOWN;
                        fade_count_=0;
                    }
                    else
                    {
                        // process is idle, so make enable change happen immediately
                        state_ = CONVOLVER_IDLE;
                        fade_gain_.set_immediately(0.0f);
                        enabled_ = false;
                    }
                }
            }
        }

        void set_enable_time(float time)
        {
            enable_fade_samples_ = (unsigned)((float)sample_rate_*(time/1000.0f));
        }

        unsigned long long cfilterfunc_thru()
        {
            return 0;
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
#if CONVOLVER_DEBUG>1
            pic::logmsg() << "convolver start";
#endif // CONVOLVER_DEBUG>1

            piw::data_nb_t d;
            lingering_ = false;

            // reset audio input queues
            for(unsigned i=0; i<2; i++)
            {
                env->cfilterenv_reset(i+1, id.time());
            }

            // get the latest mix to make sure the first one is set
            if(env->cfilterenv_latest(3,d,id.time()))
            {
                wet_dry_mix_.set(d.as_renorm(0,1,0));
            }

            return true;
        }

        void input(piw::cfilterenv_t *env, unsigned long long t)
        {
            piw::data_nb_t d;

            // audio input
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

        }

        bool cfilterfunc_process(piw::cfilterenv_t *e, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
        {
            //pic::printmsg() << "process"; fflush(stdout);

            idle_ = false;

            input(e, t);

            // create input and output buffers
            float *buffer_out[2],*scalar[2];
            const float *buffer_in[2];
            piw::data_nb_t audio_out[2];

            for(unsigned i=0; i<2; i++)
            {
                // get audio input pointers
                if(last_audio_[i].get().is_array())
                {
                    buffer_in[i] = last_audio_[i].get().as_array();
                }
                else
                {
                    buffer_in[i] = silence_;
                }

                // get audio output pointers
                audio_out[i] = piw::makenorm_nb(t, buffer_size_,&buffer_out[i], &scalar[i]);

            }

            if(state_!=CONVOLVER_IDLE)
            {
                if(state_==CONVOLVER_FADEUP || state_==CONVOLVER_FADEDOWN)
                {
                    fade_samples_ = (unsigned)((float)sample_rate_*CONVOLVER_FADE_TIME);
                    fade_gain_.set_interp_samples((float)fade_samples_);
                }

                if(state_==CONVOLVER_ENABLE_FADEUP || state_==CONVOLVER_ENABLE_FADEDOWN)
                {
                    fade_gain_.set_interp_samples((float)enable_fade_samples_);
                }


                //float ch_wet[2];

                // ------- process audio -------

                // number of channels to process
                unsigned channels = mono_?1:2;

                for(unsigned c=0; c<channels; c++)
                {
                    //ch_wet[c] = 0;

                    //float dry = buffer_in[c][i];

                    // copy input buffer to engine
                    memcpy(inpbuff_[c]+inpoffs_, buffer_in[c], buffer_size_*sizeof(float));

                    // have to clear output buffer before processing
                    memset(outbuff_[c], 0, buffer_size_ * sizeof (float));

                } // channel

                // flip-flop input buffers
                inpoffs_ += buffer_size_;
                if (inpoffs_ == 2*buffer_size_)
                    inpoffs_ = 0;

                // process convolution on stereo channels
                // read out calls process and puts results into output buffers
                conv_engine_->readout(outbuff_, false);

                // ------- write to output buffers -------
                for(unsigned c=0; c<channels; c++)
                {
                    // copy output buffer from engine
                    memcpy(buffer_out[c], outbuff_[c], buffer_size_*sizeof(float));

                    for(unsigned i=0; i<buffer_size_; i++)
                    {
                        // (1-wet_dry_mix_)*dry + wet_dry_mix_*wet;
                        buffer_out[c][i] = buffer_in[c][i] + wet_dry_mix_.get() * (fade_gain_.get() * buffer_out[c][i] - buffer_in[c][i]);
                    }

                } // channel

                // copy left to right for mono processing
                if(mono_)
                    memcpy(buffer_out[1], buffer_out[0], buffer_size_*sizeof(float));

                if(state_==CONVOLVER_FADEUP)
                {
#if CONVOLVER_DEBUG>1
                    pic::logmsg() << "fading up " << fade_count_ << " " << fade_samples_; fflush(stdout);
#endif // CONVOLVER_DEBUG>1
                    fade_count_ += buffer_size_;

                    // faded to one
                    if(fade_count_ >= fade_samples_)
                    {
                        state_=CONVOLVER_RUNNING;
                        fade_count_ = 0;
                    }
                }

                if(state_==CONVOLVER_FADEDOWN)
                {
#if CONVOLVER_DEBUG>1
                    pic::logmsg() << "fading down " << fade_count_ << " " << fade_samples_; fflush(stdout);
#endif // CONVOLVER_DEBUG>1
                    fade_count_ += buffer_size_;

                    // faded to zero
                    if(fade_count_ >= fade_samples_)
                    {
                        // signal update to happen in control thread
#if CONVOLVER_DEBUG>1
                        pic::logmsg() << "unlocking..."; fflush(stdout);
#endif // CONVOLVER_DEBUG>1
                        update_.up();
                        state_=CONVOLVER_IDLE;
                        fade_count_ = 0;
                    }
                }




                if(state_==CONVOLVER_ENABLE_FADEUP)
                {
#if CONVOLVER_DEBUG>1
                    pic::logmsg() << "enable fading up " << fade_count_ << " " << enable_fade_samples_; fflush(stdout);
#endif // CONVOLVER_DEBUG>1
                    fade_count_ += buffer_size_;

                    if(fade_count_ >= enable_fade_samples_)
                    {
                        state_=CONVOLVER_RUNNING;
                        fade_count_ = 0;
                    }
                }

                if(state_==CONVOLVER_ENABLE_FADEDOWN)
                {
#if CONVOLVER_DEBUG>1
                    pic::logmsg() << "enable fading down " << fade_count_ << " " << enable_fade_samples_; fflush(stdout);
#endif // CONVOLVER_DEBUG>1
                    fade_count_ += buffer_size_;

                    if(fade_count_ >= enable_fade_samples_)
                    {
                        state_=CONVOLVER_IDLE;
                        enabled_ = false;
                        fade_count_ = 0;
                    }
                }


            }
            else
            {
                // bypass effect during impulse update
                for(unsigned c=0; c<2; c++)
                {
                    // copy input to output applying mix level to the input
                    for(unsigned i=0; i<buffer_size_; i++)
                    {
                        buffer_out[c][i] = (1 - wet_dry_mix_.get() ) * buffer_in[c][i];
                    }
                }
            }


            for(unsigned i=0; i<2; i++)
            {
                // set scalar to last value
                *scalar[i]=buffer_out[i][buffer_size_-1];
                // output processed audio
                e->cfilterenv_output(i+1, audio_out[i]);
                // clear the last_audio_ inputs to prevent the input buffer cycling
                // when the input event stop
                last_audio_[i].clear_nb();
            }


            if(lingering_)
            {
                if(linger_count_==0)
                {
#if CONVOLVER_DEBUG>1
                    pic::logmsg() << "convolver: going idle";
#endif // CONVOLVER_DEBUG>1
                    idle_ = true;
                    lingering_ = false;

                    return false;
                }
                else
                {
                    if(state_==CONVOLVER_RUNNING || (state_==CONVOLVER_IDLE && !updating_))
                        linger_count_--;
                }
            }

            return true;
        }

        bool cfilterfunc_end(piw::cfilterenv_t *, unsigned long long time)
        {
#if CONVOLVER_DEBUG>1
            pic::logmsg() << "cfilter_linger";
#endif // CONVOLVER_DEBUG>1

            linger_count_ = linger_num_;
            lingering_ = true;

            return true;
        }

//        void cfilterfunc_changed(piw::cfilterenv_t *, void *p_)
//        {
//
//        }


        // the convolution engine
        Convlevel *conv_engine_;

        // input audio buffers
        piw::dataholder_nb_t last_audio_[2];

        // engine buffers
        float *inpbuff_[2];
        float *outbuff_[2];

        // engine input buffer offset
        unsigned int inpoffs_;

        // silence buffer when no input
        float silence_[PLG_CLOCK_BUFFER_SIZE];

        // global parameters
        interp_param_t wet_dry_mix_;
        interp_param_t fade_gain_;

        // global variables
        bool     enabled_;
        // mono processing mode
        bool     mono_;
        bool     new_mono_;
        float    sample_rate_;
        unsigned buffer_size_;

        // the loaded impulse response
        plg_convolver::samplearray2ref_t imp_resp_;
        // the sample rate converted impulse response
        plg_convolver::samplearray2ref_t imp_resp_src_;

        pic::semaphore_t update_;
        unsigned state_;
        unsigned fade_count_;
        unsigned fade_samples_;
        unsigned enable_fade_samples_;

        unsigned linger_count_;
        unsigned linger_num_;
        bool     idle_;
        bool     lingering_;
        bool     updating_;

        // output gain, adjusted for sample rate
        float    gain_;
    };





    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // convolver cfilter class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct convolver_cfilter_t: piw::cfilterctl_t, piw::cfilter_t, pic::tracked_t
    {
        convolver_cfilter_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : cfilter_t(this,o,d), convolver_func_(), clockdomain_(d)
        {
            d->add_listener(pic::notify_t::method(this,&convolver_cfilter_t::clock_changed));

            // get the current sample rate on initialization
            float sample_rate = (float)clockdomain_->get_sample_rate();
            unsigned buffer_size = clockdomain_->get_buffer_size();
            convolver_func_.set_sample_rate(sample_rate,buffer_size);

        }

        void clock_changed()
        {
            float sample_rate = (float)clockdomain_->get_sample_rate();
            unsigned buffer_size = clockdomain_->get_buffer_size();
            convolver_func_.set_impl_sample_rate(sample_rate,buffer_size);
        }

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &path)
        {
            return &convolver_func_;
        }

        void cfilterctl_delete(piw::cfilterfunc_t *f) { }

        convolver_cfilterfunc_t convolver_func_;
        piw::clockdomain_ctl_t *clockdomain_;
    };


} // namespace



namespace plg_convolver
{
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // canonicalise_samples: unpack samples according to format from a string to floats
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    samplearray2ref_t canonicalise_samples(const std::string &data,float rate,unsigned chans,unsigned bitsPerSamp)
    {
        samplearray2ref_t ret;

        float f = 0;
        unsigned char b0 = 0;
        unsigned char b1 = 0;
        unsigned char b2 = 0;
        unsigned char b3 = 0;

        long n = data.size();
        switch(bitsPerSamp)
        {
        case 8:
            // 8-bit unsigned int samples
            ret = pic::ref(new samplearray2_t(n));
            for(long i=0; i<n; i++)
            {
                b0 = (unsigned char)data[i];
                f = (((float)b0)-128.0f)/128.0f;
                ret->data[i] = f;
            }
            break;
        case 16:
            // 16-bit int samples
            ret = pic::ref(new samplearray2_t(n/2));
            for(long i=0; i<n; i+=2)
            {
                b0 = (unsigned char)data[i];
                b1 = (unsigned char)data[i+1];
                short s = (b1<<8)|(b0);
                f = ((float)s)/32768.f;
                ret->data[i/2] = f;
            }
            break;
        case 24:
            // 24-bit int samples
            ret = pic::ref(new samplearray2_t(n/3));
            for(long i=0; i<n; i+=3)
            {
                b0 = (unsigned char)data[i];
                b1 = (unsigned char)data[i+1];
                b2 = (unsigned char)data[i+2];
                int m = (b2<<24)|(b1<<16)|(b0<<8);
                f = ((float)m)/(8388608.f*256.f);
                ret->data[i/3] = f;
            }
            break;
        case 32:
            // 32-bit float samples
            ret = pic::ref(new samplearray2_t(n/4));
            for(long i=0; i<n; i+=4)
            {
                b0 = (unsigned char)data[i];
                b1 = (unsigned char)data[i+1];
                b2 = (unsigned char)data[i+2];
                b3 = (unsigned char)data[i+3];
                unsigned l = (unsigned)((((unsigned)b3)<<24)|(((unsigned)b2)<<16)|(((unsigned)b1)<<8)|((unsigned)b0));
                float *fp = (float *)&l;
                ret->data[i/4] = *fp;
            }
            break;
        }

        ret->rate=rate;
        ret->chans=chans;

        return ret;
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // convolver implementation class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // declare implementation class within convolver class
    struct convolver_t::impl_t
    {
        // also construct the convolver and tap classes
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : convolver_(o,d) {}

        convolver_cfilter_t convolver_;
    };



    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // convolver class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // implement outer class that holds the implementation class
    convolver_t::convolver_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
    convolver_t::~convolver_t() { delete impl_; }
    void convolver_t::set_impulse_response(samplearray2ref_t &imp_resp) { impl_->convolver_.convolver_func_.set_impulse_response(imp_resp); }
    void convolver_t::set_mono_processing(bool mono) { impl_->convolver_.convolver_func_.set_mono_processing(mono); }
    void convolver_t::set_enable(bool enable) { impl_->convolver_.convolver_func_.set_enable(enable); }
    void convolver_t::set_enable_time(float time) { impl_->convolver_.convolver_func_.set_enable_time(time); }
    piw::cookie_t convolver_t::cookie() { return impl_->convolver_.cookie(); }

} // namespace plg_convolver



