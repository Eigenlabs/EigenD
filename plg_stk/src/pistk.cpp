/*
 * pistk.cpp
 *
 *  Interface between Belcanto system and Synthesis ToolKit based instruments
 */

#include "pistk.h"

#include "Saxofony.h"
#include "Clarinet.h"
#include "Clarinet2.h"
#include "Cello.h"

#include <piw/piw_resource.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_address.h>
#include <cmath>

#include <picross/pic_log.h>


#define OUT_AUDIO 1
#define OUT_MASK 1
#define BOW_TICKS 3

#define IN_FREQ 2
#define IN_PRESSURE 3
#define IN_P1 4
#define IN_P2 5
#define IN_P3 6
#define IN_P4 7
#define IN_BOW 8
//#define IN_MASK SIG8(IN_FREQ,IN_PRESSURE,IN_P1,IN_P2,IN_P3,IN_P4,IN_BOW,9)
#define IN_MASK SIG23(2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24)

// minimum frequency the STK instruments can play
#define MIN_FREQ 100.0f

// rate at which STK breath instrument envelope increases
#define BLOW_RATE 0.005

#define LINGER 25
#define LINGER_TRANSITION 20

namespace
{


#if CELLO_TESTVALUE==1
    unsigned param_num = 0;
    float param_vals[3];
    void set_param_num(unsigned num) { param_num = num; }
    void set_param_val(float val) { param_vals[param_num] = val; }
    void init_params()
    {
/*
        param_vals[0] = 1;     // v0 = 10;    % constant that determines friction curve steepness
        param_vals[1] = 0.3;   // ud = 0.01;   % dynamic coefficient of friction
        param_vals[2] = 0.8;    // us = 0.8;    % static coefficient of friction
*/
        param_vals[0] = 0.55;   // Z
        param_vals[1] = 0.8;    // mus
        param_vals[2] = 0.2;    // alpha
        param_vals[3] = 0.3;    // beta
        param_vals[4] = 0.1;    // gamma

    }
#endif // CELLO_TESTVALUE==1



    template<class I> struct func_t : piw::cfilterfunc_t
    {
        // TODO factor
        func_t() : inst_(MIN_FREQ), count_(0), on_(false), run_state_(stk::stk_stop), bow_on_(0), velocity_factor_(1),
                                        env_(0), end_time_(0), interp_(0), sample_rate_(0), poly_bow_(false)
        {
            last_[0] = 0;
            last_[1] = 0;
            last_[2] = 0;
            last_[3] = 0;
        }

        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs);

        void process(float *out, unsigned samp_from, unsigned samp_to)
        {
            for(; samp_from < samp_to; ++samp_from)
            {
                out[samp_from] = inst_.tick();
            }
        }

        // process and upsample by 2 with linear interpolation
        void process2(float *out, unsigned samp_from, unsigned samp_to)
        {
            StkFloat next = 0;
            for(; samp_from < samp_to; ++samp_from)
            {
                if(interp_==0)
                {
                    out[samp_from] = last_[0];
                    interp_ = 1;
                }
                else
                {
                    next = inst_.tick();
                    out[samp_from] = 0.5*(last_[0]+next);
                    last_[0] = next;
                    interp_ = 0;
                }
            }
        }

        // process and upsample by 2 with Hermite interpolation
        void process3(float *out, unsigned samp_from, unsigned samp_to)
        {
            StkFloat next = 0;
            for(; samp_from < samp_to; ++samp_from)
            {
                if(interp_==0)
                {
                    out[samp_from] = last_[2];
                    interp_ = 1;
                }
                else
                {
                    next = inst_.tick();
                    out[samp_from] = hermite4(0.5, last_[3], last_[2], last_[1], last_[0]);
                    last_[3] = last_[2];
                    last_[2] = last_[1];
                    last_[1] = last_[0];
                    last_[0] = next;
                    interp_ = 0;
                }
            }
        }

        // hermite4: Laurent de Soras Hermite interpolation
        inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
        {
           const float    c     = (x1 - xm1) * 0.5f;
           const float    v     = x0 - x1;
           const float    w     = c + v;
           const float    a     = w + v + (x2 - x0) * 0.5f;
           const float    b_neg = w + a;

           return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
        }

        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long time)
        {
            //pic::logmsg() << "stk end t=" << time;
            end_time_ = time;
            on_=false;
            count_=LINGER;
            inst_.noteOff(0);
            return true;
        }

        // TODO: remove this temporary fix - setting control parameters here
        // until cfilterenv_next can be got working in cfilterfunc_process
        void setp1(const piw::data_nb_t &d)
        {
            current_p1_ = d.as_renorm(20,127,80);
            inst_.controlChange(2,current_p1_);
        }
        void setp2(const piw::data_nb_t &d)
        {
//            current_p2_ = d.as_renorm(1,127,1);
            // TODO: find a better fix to using the ranger with this parameter? put this in after a merge
            current_p2_ = d.as_renorm_float(BCTUNIT_RATIO,0,1,0)*127.0;
//            pic::printmsg() << "u=" << d.as_array_ubound() << " l=" << d.as_array_lbound() << " r=" << d.as_array_rest() << " n=" << d.as_norm() << " p2=" << current_p2_;
            inst_.controlChange(4,std::fabs(current_p2_));
        }
        void setp3(const piw::data_nb_t &d)
        {
            current_p3_ = d.as_renorm(1,127,1);
            inst_.controlChange(11,current_p3_);
        }
        void setp4(const piw::data_nb_t &d)
        {
            current_p4_ = d.as_renorm(1,127,1);
            inst_.controlChange(1,current_p4_);
        }

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
        {
            //ei.dump();
            unsigned long long t=id.time();
            count_=0;
            on_=true;
            piw::data_nb_t d;

            //pic::logmsg() << "stk start";
            inst_.clear();


            if(env->cfilterenv_latest(IN_FREQ,d,t))
            {
                inst_.setFrequency(d.as_renorm_float(BCTUNIT_HZ,0,96000,0));
            }
            else
            {
                //pic::logmsg() << "stk start with no freq";
            }

            if(env->cfilterenv_latest(IN_PRESSURE,d,t))
            {
                inst_.noteOn(0,std::fabs(d.as_norm()));
            }

            if(env->cfilterenv_latest(IN_P1,d,t)) setp1(d);
            if(env->cfilterenv_latest(IN_P2,d,t)) setp2(d);
            if(env->cfilterenv_latest(IN_P3,d,t)) setp3(d);
            if(env->cfilterenv_latest(IN_P4,d,t)) setp4(d);

            return true;
        }

        float get_velocity() { return bow_vel_; }

        void calculate_velocity(const piw::data_nb_t &d)
        {
            unsigned long long t = d.time();
            float p = d.as_renorm_float(BCTUNIT_RATIO,-1,1,0);
            bow_vel_ = (p-bow_pos_last_)/((float)(t-bow_pos_time_))*1e6*velocity_factor_;
            bow_pos_last_ = p;
            //pic::logmsg() << "p=" << p << " v="<<bow_vel_ << " t-lt=" << t-bow_pos_time_ << " bowon=" << bow_on_ << " t=" << t;
            bow_pos_time_ = t;
        }

        void set_velocity_factor(float velocity_factor) { velocity_factor_ = velocity_factor; }

        I inst_;
        unsigned count_;
        bool on_;
        unsigned run_state_;        // TODO factor this out
        float current_p1_, current_p2_, current_p3_, current_p4_;

        // TODO: factor these out
        float bow_pos_last_;
        unsigned long long bow_pos_time_;
        float bow_vel_;
        unsigned bow_on_;
        float velocity_factor_;

        piw::cfilterenv_t *env_;
        unsigned long long end_time_;

        StkFloat last_[4];
        unsigned interp_;

        double f_;

        StkFloat sample_rate_;

        bool poly_bow_;
    };


    template<class I>
    bool func_t<I>::cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
    {
        double sample_rate = (double)(env->cfilterenv_clock()->get_sample_rate());
        Stk::setSampleRate(sample_rate);

        if(!on_)
        {
            if(count_==0)
            {
                pic::logmsg() << "stk end";
                return false;
            }

            count_--;
        }

        float *out,*fs;
        piw::data_nb_t o = piw::makenorm_nb(t,bs,&out,&fs);
        memset(out,0,bs*sizeof(float));

        unsigned samp_from=0;
        unsigned samp_to=0;

        if(on_)
        {
            piw::data_nb_t d;
            unsigned sig;

            while(env->cfilterenv_next(sig,d,t))
            {
                samp_to = sample_offset(bs,d.time(),f,t);
                if(samp_to>samp_from)
                {
                    process(out,samp_from,samp_to);
                    samp_from = samp_to;
                }

                switch(sig)
                {

                    case IN_FREQ:
                        inst_.setFrequency(d.as_renorm_float(BCTUNIT_HZ,0,96000,0));
                        break;

                    case IN_PRESSURE:
                        inst_.noteOn(0, std::fabs(d.as_norm()));
                        break;

                    case IN_P1:
                        setp1(d);
                        inst_.controlChange(2,current_p1_);
                        break;

                    case IN_P2:
                        setp2(d);
                        inst_.controlChange(4,current_p2_);
                        break;

                    case IN_P3:
                        setp3(d);
                        inst_.controlChange(11,current_p3_);
                        break;

                    case IN_P4:
                        setp4(d);
                        inst_.controlChange(1,current_p4_);
                        break;

                }
            }
        }

        process(out,samp_from,bs);

        *fs=out[bs-1];
        env->cfilterenv_output(OUT_AUDIO,o);
        return true;
    }

    /* -------------------------------------------------------------------------------------------------------------------
     * Clarinet
     *
     * -------------------------------------------------------------------------------------------------------------------
     */

    // TODO: remove this test code when Clarinet2 model design is finished!
    template<>
    bool func_t<Clarinet2>::cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
    {
        //ei.dump();
        unsigned long long t=id.time();
        count_=0;
        piw::data_nb_t d;

        double sample_rate = (double)(env->cfilterenv_clock()->get_sample_rate());
        if(sample_rate!=sample_rate_)
        {
            sample_rate_=sample_rate;
            Stk::setSampleRate(sample_rate_);

            switch((unsigned)sample_rate_)
            {
            case 44100:
            case 48000:
                inst_.sampleRateChanged(sample_rate_, 0);
                break;
            case 88200:
            case 96000:
                inst_.sampleRateChanged(sample_rate_*0.5, 0);
                break;
            }
        }
//        pic::logmsg() << "stk start with env=" << env << ", prev env=" << env_ << " id=" << id;

        // TODO: design filters
//        // reflectance cutoff frequency
//        if(env->cfilterenv_latest(9,d,t))
//        {
//            inst_.setFilterParams(1, d.as_renorm(1,96000,1));
//        }
//        // reflectance width
//        if(env->cfilterenv_latest(10,d,t))
//        {
//            inst_.setFilterParams(2, d.as_renorm(0.01,100,1));
//        }



        if(t!=end_time_)
        {
            // note on
            // clear the clarinet breath startup gain (ramp up to remove start transient)
            // and last amplitude (used to detect start of blowing)
            inst_.noteOn(0, 0);
            inst_.clear();

        }
        else
        {
            // note transition
            // tells instrument about transition before any more control data is input
            inst_.noteTransition();

        }

        if(env->cfilterenv_latest(IN_FREQ,d,t))
        {
            inst_.setFrequency(d.as_renorm_float(BCTUNIT_HZ,0,96000,0));
        }
//        else
//        {
//            //pic::logmsg() << "stk start with no freq";
//        }

        if(env->cfilterenv_latest(IN_PRESSURE,d,t))
        {
            inst_.setPressure(std::fabs(d.as_norm()));
//            if(d.as_norm()>=0)
//                inst_.setPressure(d.as_norm());
        }

        if(env->cfilterenv_latest(IN_P1,d,t)) setp1(d);
        if(env->cfilterenv_latest(IN_P2,d,t)) setp2(d);
        if(env->cfilterenv_latest(IN_P3,d,t)) setp3(d);
        if(env->cfilterenv_latest(IN_P4,d,t)) setp4(d);
        if(env->cfilterenv_latest(8,d,t))
            inst_.setPitchSweepTime(d.as_renorm(0,100000,0));
        if(env->cfilterenv_latest(9,d,t))
            inst_.setLowestFrequency(d.as_renorm(0.1,20,20));


#if CLARINET_TESTVALUE==1
        if(env->cfilterenv_latest(9,d,t))
        {
            double value = d.as_renorm(-10,10,0);
            inst_.setTestValue(value);
        }
#endif // CLARINET_TESTVALUE

        // set note on
        run_state_ = stk::stk_run;

        return true;
    }

    template<>
    bool func_t<Clarinet2>::cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
    {
        switch(run_state_)
        {
        case stk::stk_linger:
//            pic::logmsg() << "stk lingering..." << count_ << run_state_;
            // still accept inputs and run processing (for one clock tick)
            // a new note from a note transition can put it into run state again
            if(count_==LINGER-1)
            {
                run_state_=stk::stk_stop;
                inst_.noteOff(0);
            }
            count_--;
            break;
        case stk::stk_stop:
//            pic::logmsg() << "stk stopping";
            // block inputs and run final processing to finish event
            if(count_==0)
            {
//                pic::logmsg() << "stk stopped";
                env_ = 0;
                return false;
            }
            count_--;
            break;
        }

        float *out,*fs;
        piw::data_nb_t o = piw::makenorm_nb(t,bs,&out,&fs);
        memset(out,0,bs*sizeof(float));

        unsigned samp_from=0;
        unsigned samp_to=0;

        if(run_state_ != stk::stk_stop)
        {
            piw::data_nb_t d;
            unsigned sig;

            while(env->cfilterenv_next(sig,d,t))
            {
                samp_to = sample_offset(bs,d.time(),f,t);

                if(samp_to>samp_from)
                {
                    switch((unsigned)sample_rate_)
                    {
                    case 44100:
                    case 48000:
                        process(out,samp_from,samp_to);
                        break;
                    case 88200:
                    case 96000:
                        process3(out,samp_from,samp_to);
                        break;
                    }
                    samp_from = samp_to;
                }

                switch(sig)
                {

                    case IN_FREQ:
                        inst_.setFrequency(d.as_renorm_float(BCTUNIT_HZ,0,96000,0));
                        break;

                    case IN_PRESSURE:
                        inst_.setPressure(std::fabs(d.as_norm()));
//                        if(d.as_norm()>=0)
//                            inst_.setPressure(d.as_norm());
                        break;

                    case IN_P1:
                        setp1(d);
                        //inst_.controlChange(2,current_p1_);
                        break;

                    case IN_P2:
                        setp2(d);
                        //inst_.controlChange(4,std::fabs(current_p2_));
                        break;

                    case IN_P3:
                        setp3(d);
                        //inst_.controlChange(11,current_p3_);
                        break;

                    case IN_P4:
                        setp4(d);
                        //inst_.controlChange(1,current_p4_);
                        break;

                    case 8:
                        inst_.setPitchSweepTime(d.as_renorm(0,100000,0));
                        break;
                }

            }
        }

//        inst_.dumpState();

        switch((unsigned)sample_rate_)
        {
        case 44100:
        case 48000:
            process(out,samp_from,bs);
            break;
        case 88200:
        case 96000:
            process3(out,samp_from,bs);
            break;
        }


        *fs=out[bs-1];
        env->cfilterenv_output(OUT_AUDIO,o);
        return true;
    }

    template<>
    bool func_t<Clarinet2>::cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long time)
    {
//        pic::logmsg() << "stk linger clarinet";
        end_time_ = time;
        // assume that a note transition could occur, although it might switch off
        count_=LINGER;
        run_state_=stk::stk_linger;
        return true;
    }


    /* -------------------------------------------------------------------------------------------------------------------
     * Cello
     *
     * -------------------------------------------------------------------------------------------------------------------
     */

    /* ----------------------------
     * Cello start
     *
     * ----------------------------
     */

    template<>
    bool func_t<Cello>::cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
    {
        double sample_rate = (double)(env->cfilterenv_clock()->get_sample_rate());
        Stk::setSampleRate(sample_rate);

        //ei.dump();
        unsigned long long t=id.time();
        count_=0;
        bow_vel_=0;
        bow_pos_last_=0;
        bow_pos_time_=0;
        bow_on_=0;

        piw::data_nb_t d;

        //pic::logmsg() << "stk start t=" << t << " end time=" << end_time_;
        //inst_.clear();

        // set parameters first as set freq depends on them

        // pitch sweep time
        if(env->cfilterenv_latest(4,d,t))
            inst_.setPitchSweepTime(d.as_renorm(0,100000,0));

        // bow width
        if(env->cfilterenv_latest(5,d,t))
            inst_.setBowWidth(d.as_renorm(0,1,0));

        // lowest frequency
        if(env->cfilterenv_latest(6,d,t))
            inst_.setLowestFrequency(d.as_renorm(0.1,20,20));

        // mode
        if(env->cfilterenv_latest(7,d,t))
            inst_.setMode(d.as_renorm(1,2,1));

        bool filter_change = false;
        // low shelf cutoff frequency
        if(env->cfilterenv_latest(8,d,t))
        {
            inst_.setFilterParams(1, d.as_renorm(0,96000,1)); filter_change = true;
        }
        // low shelf gain
        if(env->cfilterenv_latest(9,d,t))
        {
            inst_.setFilterParams(2, d.as_renorm(-24,24,0)); filter_change = true;
        }
        // low shelf width
        if(env->cfilterenv_latest(10,d,t))
        {
            inst_.setFilterParams(3, d.as_renorm(0.01,100,1)); filter_change = true;
        }
        // medium cutoff frequency
        if(env->cfilterenv_latest(11,d,t))
        {
            inst_.setFilterParams(4, d.as_renorm(0,96000,1)); filter_change = true;
        }
        // medium gain
        if(env->cfilterenv_latest(12,d,t))
        {
            inst_.setFilterParams(5, d.as_renorm(-96,24,0)); filter_change = true;
        }
        // medium width
        if(env->cfilterenv_latest(13,d,t))
        {
            inst_.setFilterParams(6, d.as_renorm(0.01,100,1)); filter_change = true;
        }
        // high cutoff frequency
        if(env->cfilterenv_latest(14,d,t))
        {
            inst_.setFilterParams(7, d.as_renorm(0,96000,1)); filter_change = true;
        }
        // high gain
        if(env->cfilterenv_latest(15,d,t))
        {
            inst_.setFilterParams(8, d.as_renorm(-96,24,0)); filter_change = true;
        }
        // high width
        if(env->cfilterenv_latest(16,d,t))
        {
            inst_.setFilterParams(9, d.as_renorm(0.01,100,1)); filter_change = true;
        }
        // string cutoff frequency
        if(env->cfilterenv_latest(17,d,t))
        {
            inst_.setFilterParams(10, d.as_renorm(0,96000,1)); filter_change = true;
        }
        // string width
        if(env->cfilterenv_latest(18,d,t))
        {
            inst_.setFilterParams(11, d.as_renorm(0.01,100,1)); filter_change = true;
        }

        if(filter_change)
            inst_.designFilters(sample_rate);

        // determine note start type, new note after silence or transition between notes
        if(t!=end_time_)
        {
            // note on
            inst_.noteOn(0, 0);
        }
        else
        {
            // note transition
            // tells instrument about transition before any more control data is input
            //pic::logmsg() << "note transition";
            inst_.noteTransition();
        }

        // get data from bow
        if(!poly_bow_)
        {
            // bow velocity
            if(env->cfilterenv_latest(23,d,t))
            {
                double v = d.as_renorm_float(BCTUNIT_RATIO,-1,1,0);
                //pic::logmsg() << "v = " << v;
                bow_on_=BOW_TICKS;
                inst_.setBowVelocity(v, true);

            }
            else
            {

                // bow position
                env->cfilterenv_reset(22,1);
                while(env->cfilterenv_nextsig(22,d,t))
                {
                    bow_on_=BOW_TICKS;
                    //pic::logmsg() << "initial bow velocity " << d << ' ' << get_velocity();
                    calculate_velocity(d);
                }

                inst_.setBowVelocity(get_velocity(), bow_on_>0);
                //pic::logmsg() << "vel = " << bow_input_->get_velocity() << " bow on = " << bow_input_->get_bow_on();
            }

        }

        // frequency
        if(env->cfilterenv_latest(IN_FREQ,d,t))
        {
            inst_.setFrequency(d.as_denorm());
        }
        else
        {
            //pic::logmsg() << "stk start with no freq";
        }

        // pressure
        if(env->cfilterenv_latest(IN_PRESSURE,d,t))
        {
            //inst_.noteOn(0,std::fabs(d.as_norm()));
            inst_.startBowing(std::fabs(d.as_norm()));
        }
        else
        {
            //pic::logmsg() << "stk start with no pressure";
        }

        // poly bow enable
        if(env->cfilterenv_latest(20,d,t))
        {
            poly_bow_ = d.as_bool();
        }

        if(poly_bow_)
        {
            // poly bow
            if(env->cfilterenv_latest(19,d,t))
            {
                double v = d.as_renorm_float(BCTUNIT_RATIO,-1,1,0);
                //pic::logmsg() << "v = " << v;
                inst_.setBowVelocity(v, true);

            }
        }

        // bow velocity coefficient
        if(env->cfilterenv_latest(21,d,t))
        {
            double value = d.as_renorm(0,100,1);
            set_velocity_factor(value);
        }

//        if(env->cfilterenv_latest(25,d,t))
//            inst_.setFingerPressure(d.as_norm());


#if CELLO_TESTVALUE==1
        if(env->cfilterenv_latest(24,d,t))
        {
            double value = d.as_renorm(-20,0,0);
            inst_.setTestValue(1, value);
        }
//        if(env->cfilterenv_latest(26,d,t))
//        {
//            double value = d.as_renorm(-10,10,0);
//            inst_.setTestValue(2, value);
//        }

        inst_.set_friction_params(param_vals[0], param_vals[1], param_vals[2], param_vals[3], param_vals[4]);
#endif // CELLO_TESTVALUE



        // set note on
        run_state_ = stk::stk_run;
        return true;
    }

    /* ----------------------------
     * Cello process
     *
     * ----------------------------
     */

    template<>
    bool func_t<Cello>::cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
    {
        double sample_rate = (double)(env->cfilterenv_clock()->get_sample_rate());
        Stk::setSampleRate(sample_rate);

        if(run_state_ == stk::stk_stop)
        {
            if(count_==0)
            {
                //pic::logmsg() << "cello stk end";
                return false;
            }
            count_--;
        }
        if(run_state_ == stk::stk_linger)
        {
            count_--;
            run_state_ = stk::stk_stop;
            inst_.noteOff(0);
        }

        float *out,*fs;
        piw::data_nb_t o = piw::makenorm_nb(t,bs,&out,&fs);
        memset(out,0,bs*sizeof(float));

        unsigned samp_from=0;
        unsigned samp_to=0;

        if(run_state_ != stk::stk_stop)
        {
            piw::data_nb_t d;
            unsigned sig;

            // get data from bow
            if(!poly_bow_)
            {
                if(bow_on_>0)
                {   
                    bow_on_--;
                }
                else
                {
                    inst_.setBowVelocity(get_velocity(), false);
                }
                //pic::logmsg() << "vel = " << bow_input_->get_velocity() << " bow on = " << bow_input_->get_bow_on();
            }

            while(env->cfilterenv_next(sig,d,t))
            {
                samp_to = sample_offset(bs,d.time(),f,t);
                if(samp_to>samp_from)
                {
                    process(out,samp_from,samp_to);
                    samp_from = samp_to;
                }
                switch(sig)
                {
                    case 23:
                        if(!poly_bow_)
                        {
                            double v = d.as_renorm_float(BCTUNIT_RATIO,-1,1,0);
                            //pic::logmsg() << "v = " << v;
                            inst_.setBowVelocity(v, true);
                            bow_on_=BOW_TICKS;
                        }
                        break;

                    case 22:
                        if(!poly_bow_)
                        {
                            calculate_velocity(d);
                            bow_on_=BOW_TICKS;
                            //pic::logmsg() << "bow velocity " << d << ' ' << get_velocity();
                            inst_.setBowVelocity(get_velocity(), true);
                        }
                        break;

                    case IN_FREQ:
                        // frequency
                        inst_.setFrequency(d.as_denorm());
                        break;

                    case IN_PRESSURE:
                        // bow pressure
                        inst_.startBowing(std::fabs(d.as_norm()));
                        break;

                    case 4:
                        // pitch time
                        inst_.setPitchSweepTime(d.as_renorm(0,100000,0));
                        break;

                    case 5:
                        // bow width
                        inst_.setBowWidth(d.as_renorm(0,1,0));
                        break;

                    case 19:
                        if(poly_bow_)
                        {
                            // poly bow velocity
                            double v = d.as_renorm_float(BCTUNIT_RATIO,-1,1,0);
                            inst_.setBowVelocity(v, true);
                        }
                        break;

//                    case 22:
//                        inst_.setFingerPressure(d.as_norm());
//                        break;

#if CELLO_TESTVALUE==1
                    case 24:
                        inst_.setTestValue(1, d.as_renorm(-20,0,0));
                        break;
//                    case 24:
//                        inst_.setTestValue(2, d.as_renorm(-10,10,0));
//                        break;
#endif // CELLO_TESTVALUE

                }
            }


        }

        process(out,samp_from,bs);

        //pic::logmsg() << "pistk t=" << t;

        *fs=out[bs-1];
        env->cfilterenv_output(OUT_AUDIO,o);
        return true;
    }

    /* ----------------------------
     * Cello end
     *
     * ----------------------------
     */

    template<>
    bool func_t<Cello>::cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long time)
    {
        //pic::logmsg() << "stk linger cello t=" << time;
        end_time_ = time;
        // assume that a note transition could occur, although it might switch off
        count_=LINGER;
        run_state_=stk::stk_linger;
        return true;
    }




    /* -------------------------------------------------------------------------------------------------------------------
     * Filter Control class
     *
     * -------------------------------------------------------------------------------------------------------------------
     */

    template<class I>
    struct ctl_t : piw::cfilterctl_t
    {
        ctl_t() {}
        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &)
        {
            return new func_t<I>();
        }
        unsigned long long cfilterctl_thru() { return 0; }
        unsigned long long cfilterctl_inputs() { return IN_MASK; }
        unsigned long long cfilterctl_outputs() { return OUT_MASK; }
    };



} // namespace

namespace stk
{
    struct blownstring_t::impl_t: virtual pic::lckobject_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : filter_(&ctl_,o,d)
        {
            Stk::setRawwavePath(piw::get_release_dir("plg_stk"));
        }
        piw::cookie_t cookie() { return filter_.cookie(); }

        ctl_t<Saxofony> ctl_;
        piw::cfilter_t filter_;
    };

    struct panpipe_t::impl_t: virtual pic::lckobject_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : filter_(&ctl_,o,d)
        {
            Stk::setRawwavePath(piw::get_release_dir("plg_stk"));
        }
        piw::cookie_t cookie() { return filter_.cookie(); }

        // use Cook and Scavone's basic Clarinet model as a panpipe
        // because it sounds more like a panpipe than a Clarinet!
        ctl_t<Clarinet> ctl_;
        piw::cfilter_t filter_;
    };

    struct clarinet_t::impl_t: virtual pic::lckobject_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : filter_(&ctl_,o,d,false)
        {
            Stk::setRawwavePath(piw::get_release_dir("plg_stk"));
        }
        piw::cookie_t cookie() { return filter_.cookie(); }

        ctl_t<Clarinet2> ctl_;
        piw::cfilter_t filter_;
    };

    struct cello_t::impl_t: virtual pic::lckobject_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : filter_(&ctl_,o,d)
        {
            Stk::setRawwavePath(piw::get_release_dir("plg_stk"));

#if CELLO_TESTVALUE==1
            init_params();
#endif // CELLO_TESTVALUE==1
        }
        piw::cookie_t cookie() { return filter_.cookie(); }
        piw::cookie_t bow_cookie() { return piw::cookie_t(0); }
        void calculate_velocity(const piw::data_nb_t &d);

#if CELLO_TESTVALUE==1
        void set_param_num(unsigned num) { ::set_param_num(num); }
        void set_param_val(float val) {::set_param_val(val); }
#endif // CELLO_TESTVALUE==1

        ctl_t<Cello> ctl_;
        piw::cfilter_t filter_;
    };

}

stk::blownstring_t::blownstring_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
stk::blownstring_t::~blownstring_t() { delete impl_; }
piw::cookie_t stk::blownstring_t::cookie() { return impl_->cookie(); }

stk::panpipe_t::panpipe_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
stk::panpipe_t::~panpipe_t() { delete impl_; }
piw::cookie_t stk::panpipe_t::cookie() { return impl_->cookie(); }

stk::clarinet_t::clarinet_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
stk::clarinet_t::~clarinet_t() { delete impl_; }
piw::cookie_t stk::clarinet_t::cookie() { return impl_->cookie(); }

stk::cello_t::cello_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d) : impl_(new impl_t(o,d)) {}
stk::cello_t::~cello_t() { delete impl_; }
piw::cookie_t stk::cello_t::cookie() { return impl_->cookie(); }
piw::cookie_t stk::cello_t::bow_cookie() { return impl_->bow_cookie(); }

#if CELLO_TESTVALUE==1
void stk::cello_t::set_param_num(unsigned num) { impl_->set_param_num(num); }
void stk::cello_t::set_param_val(float val) { impl_->set_param_val(val); }
#endif // CELLO_TESTVALUE==1
