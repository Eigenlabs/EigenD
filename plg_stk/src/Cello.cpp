/*
 * Cello.cpp
 *
 *  Cello model based on the bow-string model of McIntyre Schumacher Woodhouse and extended models of Serafin.
 *  This implementation uses some of the CCRMA STK framework written by by Perry R. Cook and Gary P. Scavone, 1995 - 2007
 */

/*
 *  TODO:
 *  1. filter velocity                              done
 *  2. apply adsr to force                          done
 *  3. sticky mode on strip controller              done
 *  4. interpolate between slip and stick states?
 *  5. problem with delay lines out of range        done
 *  6. thump when changing bow direction            done
 *  7. too many harmonics in string                 done
 *
 *  Other issues:
 *  Glitch when hand off strip controller from firmware
 */
#include <picross/pic_config.h>
#include <picross/pic_float.h>

#include "Cello.h"
#include "SKINI.msg"
#include <math.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>

#define CELLO_DEBUG 0

// note transition states
#define CELLO_SUSTAIN 1
#define CELLO_HIGHER_FADE 2
#define CELLO_HIGHER_FILL 3
#define CELLO_LOWER_FILL 4
#define CELLO_LOWER_FADE 5

/*
const PIC_FASTCODE StkFloat Cello :: delay_compensation_44_[][2] =
{
        {1046.502319, -3.66},        { 987.766602, -3.95},        { 880.000000, -4.19},        { 783.990845, -3.54},
        { 698.456482, -4.01},        { 659.255127, -4.24},        { 587.329529, -4.54},        { 523.251160, -4.66},
        { 493.883301, -4.18},        { 440.000000, -4.52},        { 391.995422, -4.68},        { 349.228241, -4.82},
        { 329.627563, -4.85},        { 293.664764, -4.62},        { 261.625580, -4.85},        { 246.941650, -4.86},
        { 220.000000, -4.80},        { 195.997711, -4.90},        { 174.614120, -5.28},        { 164.813782, -5.35},
        { 146.832382, -5.32},        { 130.812790, -5.17},        { 123.470825, -5.47},        { 110.000000, -6.07},
        {  97.998856, -6.07},        {  87.307060, -7.43},        {  82.406891, -7.52},        {  73.416191, -6.71},
        {  65.406395, -7.43}
};

const PIC_FASTCODE StkFloat Cello :: delay_compensation_48_[][2] =
{
        {1046.502319, -3.63},        { 987.766602, -3.85},        { 880.000000, -4.13},        { 783.990845, -3.57},
        { 698.456482, -4.05},        { 659.255127, -4.28},        { 587.329529, -4.57},        { 523.251160, -4.67},
        { 493.883301, -4.24},        { 440.000000, -4.58},        { 391.995422, -4.78},        { 349.228241, -4.87},
        { 329.627563, -4.88},        { 293.664764, -4.68},        { 261.625580, -4.90},        { 246.941650, -4.90},
        { 220.000000, -4.87},        { 195.997711, -4.96},        { 174.614120, -5.34},        { 164.813782, -5.43},
        { 146.832382, -5.43},        { 130.812790, -6.21},        { 123.470825, -5.58},        { 110.000000, -6.22},
        {  97.998856, -6.28},        {  87.307060, -7.61},        {  82.406891, -7.70},        {  73.416191, -6.90},
        {  65.406395, -7.72}
};

const PIC_FASTCODE StkFloat Cello :: delay_compensation_96_[][2] =
{
        {1046.502319, -3.62},        { 987.766602, -3.87},        { 880.000000, -2.34},        { 783.990845, -3.47},
        { 698.456482, -4.44},        { 659.255127, -4.79},        { 587.329529, -5.16},        { 523.251160, -4.41},
        { 493.883301, -4.77},        { 440.000000, -5.30},        { 391.995422, -5.61},        { 349.228241, -5.73},
        { 329.627563, -5.66},        { 293.664764, -5.59},        { 261.625580, -5.78},        { 246.941650, -5.76},
        { 220.000000, -5.76},        { 195.997711, -6.10},        { 174.614120, -6.79},        { 164.813782, -6.82},
        { 146.832382, -6.88},        { 130.812790, -6.81},        { 123.470825, -7.40},        { 110.000000, -8.42},
        {  97.998856, -8.49},        {  87.307060, -11.24},       {  82.406891, -11.30},       {  73.416191, -10.05},
        {  65.406395, -11.39}
};
*/

const PIC_FASTCODE StkFloat Cello :: delay_compensation_44_[][2] =
{
        {1046.5023,    -4.09},       { 987.7666,    -4.20},       { 880.0000,    -4.17},       { 783.9908,    -4.24},
        { 698.4565,    -4.08},       { 659.2551,    -4.08},       { 587.3295,    -4.30},       { 523.2512,    -4.24},
        { 493.8833,    -4.23},       { 440.0000,    -4.19},       { 391.9954,    -4.51},       { 349.2282,    -4.50},
        { 329.6276,    -4.50},       { 293.6648,    -4.50},       { 261.6256,    -4.76},       { 246.9417,    -4.66},
        { 220.0000,    -4.69},       { 195.9977,    -4.72},       { 174.6141,    -5.23},       { 164.8138,    -5.34},
        { 146.8324,    -5.27},       { 130.8128,    -5.46},       { 123.4708,    -5.78},       { 110.0000,    -6.46},
        {  97.9989,    -7.10},       {  87.3071,    -6.76},       {  82.4069,    -7.83},       {  73.4162,    -9.72},
        {  65.4064,    -7.25}
};

const PIC_FASTCODE StkFloat Cello :: delay_compensation_48_[][2] =
{
        {1046.5023,    -4.27},       { 987.7666,    -4.25},       { 880.0000,    -4.25},       { 783.9908,    -4.25},
        { 698.4565,    -4.25},       { 659.2551,    -4.35},       { 587.3295,    -4.35},       { 523.2512,    -4.35},
        { 493.8833,    -4.49},       { 440.0000,    -4.21},       { 391.9954,    -4.61},       { 349.2282,    -4.54},
        { 329.6276,    -4.57},       { 293.6648,    -4.65},       { 261.6256,    -4.78},       { 246.9417,    -4.72},
        { 220.0000,    -4.72},       { 195.9977,    -4.87},       { 174.6141,    -5.38},       { 164.8138,    -5.38},
        { 146.8324,    -5.45},       { 130.8128,    -5.70},       { 123.4708,    -6.04},       { 110.0000,    -6.81},
        {  97.9989,    -7.47},       {  87.3071,    -7.07},       {  82.4069,    -8.19},       {  73.4162,    -10.16},
        {  65.4064,    -7.57},
};

const PIC_FASTCODE StkFloat Cello :: delay_compensation_96_[][2] =
{
        {1046.5023,    -4.53},       { 987.7666,    -4.72},       { 880.0000,    -4.19},       { 783.9908,    -4.61},
        { 698.4565,    -4.61},       { 659.2551,    -4.51},       { 587.3295,    -4.51},       { 523.2512,    -4.77},
        { 493.8833,    -4.50},       { 440.0000,    -4.84},       { 391.9954,    -5.03},       { 349.2282,    -5.00},
        { 329.6276,    -5.07},       { 293.6648,    -5.51},       { 261.6256,    -5.40},       { 246.9417,    -5.40},
        { 220.0000,    -5.93},       { 195.9977,    -6.08},       { 174.6141,    -6.70},       { 164.8138,    -6.74},
        { 146.8324,    -6.87},       { 130.8128,    -7.30},       { 123.4708,    -8.39},       { 110.0000,    -9.80},
        {  97.9989,   -11.08},       {  87.3071,   -10.40},       {  82.4069,   -12.40},       {  73.4162,   -16.20},
        {  65.4064,   -11.48}
};




Cello :: Cello(StkFloat lowestFrequency)
{
    StkFloat sample_rate = Stk::sampleRate();
    sample_rate_ = sample_rate;

    pi_ = 3.14159265359;

    bowTable_.setSlope( 3.0 );

    vibrato_.setFrequency( 6.12723 );
    vibratoGain_ = 0.0;

    // default string to bow filter parameters
    low_shelf_freq_ = 400;
    low_shelf_gain_ = 12;
    low_shelf_q_ = 1;
    peak_freq_ = 1000;
    peak_gain_ = -24;
    peak_q_ = 1;
    high_shelf_freq_ = 10000;
    high_shelf_gain_ = -24;
    high_shelf_q_ = 1;
    low_shelf_z1_ = low_shelf_z2_ = peak_z1_ = peak_z2_ = high_shelf_z1_ = high_shelf_z2_ = 0;
    nut_z1_ = nut_z2_ = 0;

    // default string filter parmameters
    string_[0].top_note_ = 92.5;
    string_[0].freq_ = 170;
    string_[0].q_ = 0.8;
    string_[0].Z_ = 0.2;
    string_[1].top_note_ = 138.6;
    string_[1].freq_ = 340;
    string_[1].q_ = 0.8;
    string_[1].Z_ = 0.17;
    string_[2].top_note_ = 207.7;
    string_[2].freq_ = 780;
    string_[2].q_ = 0.8;
    string_[2].Z_ = 0.12;
    string_[3].top_note_ = 311.127;
    string_[3].freq_ = 1300;
    string_[3].q_ = 0.8;
    string_[3].Z_ = 0.07;
    string_[4].top_note_ = 466.164;
    string_[4].freq_ = 1900;
    string_[4].q_ = 0.8;
    string_[4].Z_ = 0.07;
    string_[5].top_note_ = 1047;
    string_[5].freq_ = 3700;
    string_[5].q_ = 0.8;
    string_[5].Z_ = 0.07;

    current_string_ = 0;

    designFilters(Stk::sampleRate());

    // initialize bow parameters
    betaRatio_ = 0.127236;
    lastBowPosition_ = 0;
    bowVelocity_ = 0;
    maxBowVelocity_ = 1;
    bowForce_ = 0;
    minBowForce_ = 0.5;
    helmholtzState_ = cello_stick;
//    bowMode_ = cello_width_hyperbolic;
//    bowMode_ = cello_basic_hyperbolic;
    bowMode_ = cello_friedlander;
//    bowMode_ = cello_memoryless;
    bowOnString_ = false;
    envelope_.setRate(0.001);

    // ignore supply lowest freq, we want 20Hz lowest
    lowestFrequency_ = 20;
    // current delay line to use 0/1
    dl_ = 0;
    initDelayLines(sample_rate_, lowestFrequency_);
    // sustain first note
    state_ = CELLO_SUSTAIN;
    // finger on string
    alpha_[dl_] = 1;

    leftBowDelay_.setMaximumDelay(128);
    rightBowDelay_.setMaximumDelay(128);
    setBowWidth(0.1);

    bowVelocity_y_1_ = 0;
    bowVelocity_g_ = (1.0-exp(-2.0*pi_*(5/sample_rate)));
    bowForce_y_1_ = 0;
    bowForce_g_ = (1.0-exp(-2.0*pi_*(1/sample_rate)));

    aout_x1_ = 0;
    aout_y1_ = 0;

    nutVel_1_ = 0;
    nutVel_2_ = 0;
    bridgeVel_1_ = 0;
    bridgeVel_2_ = 0;

    delay_current_ = sample_rate / 220.0 - 4.0;
    delay_target_ = delay_current_;
    delay_step_ = 1;

    current_latch_ = false;

    delay_previous_ = sample_rate / 65;

    // Necessary to initialize internal variables.
    this->setFrequency( 220.0 );

    Stk::addSampleRateAlert( this );

    // initialize the neck delay line to a steady state loop through the bow string junction
//    neckDelay_->fill(0.5);
    // fill right 0 and left 1

    p0_ = 0.55;   // Z
    p1_ = 0.8;    // mus
    p2_ = 0.2;    // alpha
    p3_ = 0.3;    // beta
    p4_ = 0.1;    // gamma


}

void Cello :: initDelayLines( StkFloat sample_rate, StkFloat lowestFrequency )
{
    length_ = (long) ( sample_rate / lowestFrequency + 1 );
    // TODO: minimize these
    nutFingerD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    fingerBowD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    nutFingerD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    fingerBowD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    bowBridgeD_.setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    bridgeBowD_.setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    bowFingerD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    fingerNutD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    bowFingerD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
    fingerNutD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );

    StkFloat bottomC = sample_rate_ / 65.41;
    StkFloat nutFinger, fingerBow, bowBridge;
    nutFinger = 0;
    fingerBow = bottomC*(1-betaRatio_)*0.5;
    bowBridge = bottomC*(betaRatio_)*0.5;

    setDelays(nutFinger, fingerBow, bowBridge);

}

void Cello :: setDelays( StkFloat nutFinger, StkFloat fingerBow, StkFloat bowBridge )
{
    // ------- right going wave delays
    nutFingerD_[dl_].setDelay( nutFinger );
    fingerBowD_[dl_].setDelay( fingerBow );
    bowBridgeD_.setDelay( bowBridge );

    // ------- left going wave delays
    bridgeBowD_.setDelay( bowBridge );
    bowFingerD_[dl_].setDelay( fingerBow );
    fingerNutD_[dl_].setDelay( nutFinger );

}


Cello :: ~Cello()
{
    Stk::removeSampleRateAlert( this );
}

void Cello :: clear()
{
    nutFingerD_[0].clear();
    fingerBowD_[0].clear();
    nutFingerD_[1].clear();
    fingerBowD_[1].clear();
    bowBridgeD_.clear();
    bridgeBowD_.clear();
    bowFingerD_[0].clear();
    fingerNutD_[0].clear();
    bowFingerD_[1].clear();
    fingerNutD_[1].clear();
}

void Cello :: sampleRateChanged( StkFloat newRate, StkFloat oldRate )
{
    pic::logmsg() << "Cello::sampleRateChanged";

    sample_rate_ = newRate;

    initDelayLines( sample_rate_, lowestFrequency_ );
    designFilters( sample_rate_ );

    bowVelocity_g_ = (1.0-exp(-2.0*pi_*(5/newRate)));
    bowForce_g_ = (1.0-exp(-2.0*pi_*(1/newRate)));

    setBowWidth(bowWidth_);

}

void Cello :: pickString(StkFloat frequency)
{
    if(frequency <= string_[0].top_note_)
        current_string_ = 0;
    else
        if(frequency <= string_[1].top_note_)
            current_string_ = 1;
        else
            if(frequency <= string_[2].top_note_)
                current_string_ = 2;
            else
                if(frequency <= string_[3].top_note_)
                    current_string_ = 3;
                else
                    if(frequency <= string_[4].top_note_)
                        current_string_ = 4;
                    else
                        current_string_ = 5;

}

StkFloat Cello :: delayCompensate(StkFloat frequency, const StkFloat delay[29][2])
{
    if(frequency>=delay[0][0])
       return delay[0][1];
    else
        if(frequency<=delay[28][0])
            return delay[28][1];
        else
            for(unsigned i=1; i<29; i++)
                if(frequency>=delay[i][0])
                {
                    StkFloat a = (frequency - delay[i][0])/(delay[i-1][0]-delay[i][0]);
                    StkFloat c = a*(delay[i-1][1]) + (1-a)*(delay[i][1]);
                    return c;
                }
    return 0;
}

void Cello :: tuneNote(StkFloat frequency)
{
    delay_target_ = baseDelay_ - phaseDelay(frequency, string_[current_string_].coeff_, sample_rate_);

    switch((unsigned)sample_rate_)
    {
    case 44100:
        delay_target_ += delayCompensate(frequency, delay_compensation_44_);
        break;
    case 48000:
        delay_target_ += delayCompensate(frequency, delay_compensation_48_);
        break;
    case 96000:
        delay_target_ += delayCompensate(frequency, delay_compensation_96_);
//        delay_target_ += (testValue_[0]);
        break;
    }

}

void Cello :: setFrequency(StkFloat frequency)
{
    StkFloat freakency = frequency;

    if(frequency < lowestFrequency_)
    {
        freakency = lowestFrequency_;
    }

    // Delay = length - approximate filter delay.
    baseDelay_ = Stk::sampleRate() / freakency;
    //pic::logmsg() << "freq = " << frequency << "  delay = " << baseDelay_ << "  sr = " << Stk::sampleRate();

    // account for delay across the bow
    if(bowMode_==cello_width_hyperbolic)
        baseDelay_ -= (bowWidthSamps_*2);

    if ( baseDelay_ <= 0.0 ) baseDelay_ = 0.3;


    // --------------------- Note transition ---------------------
    if(noteTransition_ && !current_latch_)
    {
        // pick a string to play the note on
        pickString(frequency);
        tuneNote(frequency);

        if(delay_target_>delay_current_)
        {
            // go to lower note
            state_ = CELLO_LOWER_FADE;

            alpha_[dl_] = 1;
            StkFloat nutFinger = (delay_target_-delay_current_)*0.5 - 1;
            if(nutFinger<0)
                nutFinger = 0;
            nutFingerD_[(dl_)].setDelay( nutFinger );
            fingerNutD_[(dl_)].setDelay( nutFinger );

            alpha_[dl_^1] = 1;
            nutFingerD_[(dl_^1)].setDelay( 0 );
            fingerNutD_[(dl_^1)].setDelay( 0 );
            fingerBowD_[(dl_^1)].setDelay((delay_target_-delay_current_*betaRatio_)*0.5);
            bowFingerD_[(dl_^1)].setDelay((delay_target_-delay_current_*betaRatio_)*0.5);

            alphaFadeSamples_ = pitch_samps_;
            alphaStep_ = -1/alphaFadeSamples_;
            delayFillSamples_ = 2*delay_current_;

        }
        else
        {
            // go to higher note
            state_ = CELLO_HIGHER_FILL;

            alpha_[dl_^1] = 0;
            StkFloat nutFinger = (delay_current_-delay_target_)*0.5 - 1;
            if(nutFinger<0)
                nutFinger = 0;
            nutFingerD_[(dl_^1)].setDelay( nutFinger );
            fingerNutD_[(dl_^1)].setDelay( nutFinger );
            fingerBowD_[(dl_^1)].setDelay((delay_target_-delay_current_*betaRatio_)*0.5);
            bowFingerD_[(dl_^1)].setDelay((delay_target_-delay_current_*betaRatio_)*0.5);

            alphaFadeSamples_ = pitch_samps_;
            alphaStep_ = 1/alphaFadeSamples_;
            delayFillSamples_ = 2*delay_current_;
        }

        // latch the freq change to prevent pitch bend from adjusting freq
        current_latch_ = true;
        noteTransition_ = false;

#if CELLO_DEBUG==1
        pic::logmsg() << "trans: curr="<<delay_current_<<" targ="<<delay_target_<<" step="<<delay_step_ << " string="<<current_string_ << " fade samps=" << pitch_samps_;
#endif // CELLO_DEBUG==1
    }
    else
    {

        // --------------------- New note ---------------------
        if(newNote_)
        {
            // new note so change frequency immediately
            state_ = CELLO_SUSTAIN;

            // pick a string to play the note on
            pickString(frequency);
            tuneNote(frequency);

            delay_current_ = delay_target_;
            delay_step_ = 1;
            alpha_[dl_] = 1;

            StkFloat d1, d2, d3;
            d1 = 0;
            d2 = delay_current_*(1-betaRatio_)*0.5;
            d3 = delay_current_*(betaRatio_)*0.5;

            setDelays(d1, d2, d3);

            bowForce_y_1_ = 0.0;
            bowVelocity_y_1_ = 0.0;

            newNote_ = false;

            current_latch_ = false;

            // initialize delay lines with a triangular function peaking at the bowing point
            // this helps ensure Helmholtz oscillation
            StkFloat startupGain = 1;

            for(unsigned long i=0; i<(((unsigned long)d2)+1); i++)
                fingerBowD_[dl_].setContentsAt(i, startupGain*(StkFloat)i/d2);

            for(unsigned long i=0; i<(((unsigned long)d3)+1); i++)
                bowBridgeD_.setContentsAt(i, startupGain*(1.0-(StkFloat)i/d3));

            for(unsigned long i=0; i<(((unsigned long)d3)+1); i++)
                bridgeBowD_.setContentsAt(i, startupGain*((StkFloat)i/d3));

            for(unsigned long i=0; i<(((unsigned long)d2)+1); i++)
                bowFingerD_[dl_].setContentsAt(i, startupGain*(1.0-(StkFloat)i/d2));



#if CELLO_DEBUG==1
            pic::logmsg() << "new: curr="<<delay_current_<<" targ="<<delay_target_<<" step="<<delay_step_;
#endif // CELLO_DEBUG==1
        }
        else
        {
            // --------------------- Pitch bend ---------------------
            // bending note to a new frequency, sweep slowly to smooth frequency changes
            if(!current_latch_)
            {
                tuneNote(frequency);
                delay_step_ = pow(2, (pic_log2(delay_target_/delay_current_)/100));

                //pic::logmsg() << "bend: curr="<<delay_current_<<" targ="<<delay_target_<<" step="<<delay_step_;
            }
        }
    }

    if(frequency > 1108.730469)
        startTarget_ = 0;
    else
    {
        // adjust the gains of the strings to reduce high string loudness
        switch(current_string_)
        {
        case 0:
            startTarget_ = 1;
            break;
        case 1:
            startTarget_ = 0.9;
            break;
        case 2:
            startTarget_ = 0.7;
            break;
        case 3:
        case 4:
        case 5:
            startTarget_ = 0.5;
            break;
        }
    }
    startEnvelope_.setTarget(startTarget_);


    if(pic::isnan(delay_step_))
    {
        pic::logmsg() << "cello delay time change failed step=" << delay_step_ << " target=" << delay_target_ << " current=" << delay_current_;
        delay_step_ = 1;
        delay_target_ = length_/2;
        delay_current_ = length_/2;
    }

}

void Cello :: startBowing(StkFloat amplitude)
{
//    leftBowDelay_.setDelay(bowWidthSamps_);
//    rightBowDelay_.setDelay(bowWidthSamps_);

    // envelope to stop noise when bow removed from string
    if(bowOnString_)
        envelope_.setTarget(1);

    // amplitude is the bow pressure
    switch(bowMode_)
    {
    case cello_basic_hyperbolic:
        bowForce_ = amplitude;
        break;
    case cello_friedlander:
        bowForce_ = amplitude;
        break;
    case cello_width_hyperbolic:
        // half the force of basic hyperbolic
        // as there are two bow points now
        bowForce_ = amplitude*0.5;
        break;
    case cello_memoryless:
    default:
        bowForce_ = amplitude;
        bowTable_.setSlope( 5.0 - bowForce_ );
        break;
    }

}

void Cello :: stopBowing(StkFloat amplitude)
{
    //pic::logmsg() << "stop bowing";
    envelope_.setTarget(0);
}

void Cello :: setBowVelocity(StkFloat bowVelocity, bool bow_on_string)
{
    bowVelocity_ = bowVelocity;
    bowOnString_ = bow_on_string;

//    if(bowOnString_)
//        printf("bow off string\n");

    if (bowVelocity_>1)
        bowVelocity_=1;
    if (bowVelocity_<-1)
        bowVelocity_=-1;

}


void Cello :: noteOff(StkFloat amplitude)
{
    this->stopBowing( amplitude );

#if defined(_STK_DEBUG_)
    errorString_ << "Cello::NoteOff: amplitude = " << amplitude << ".";
    handleError( StkError::DEBUG_WARNING );
#endif
}

void Cello :: noteTransition()
{
#if defined(_STK_DEBUG_)
    pic::logmsg() << "noteTransition(): newNote=false  noteTransition=true";
#endif

    noteTransition_ = true;

}

void Cello :: setVibrato(StkFloat gain)
{
    vibratoGain_ = gain;
}

void Cello :: setPitchSweepTime(StkFloat pitch_time)
{
    // pitch sweep time in milliseconds
    pitch_samps_ = (pitch_time*(Stk::sampleRate()))/1000;
    if(pitch_samps_==0)
        pitch_samps_=1;
}

void Cello :: setBowWidth(StkFloat bow_width)
{
    bowWidth_ = bow_width;
    bowWidthSamps_ = (50*bowWidth_)*(Stk::sampleRate()/96000);

    leftBowDelay_.setDelay(bowWidthSamps_);
    rightBowDelay_.setDelay(bowWidthSamps_);
}

void Cello :: setLowestFrequency(StkFloat lowest_frequency)
{
    if(lowestFrequency_!=lowest_frequency)
    {
        lowestFrequency_ = lowest_frequency;

        StkFloat sampleRate = Stk::sampleRate();
        length_ = (long) ( sampleRate / lowestFrequency_ + 1 );
        // TODO: minimize these
        nutFingerD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        fingerBowD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        nutFingerD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        fingerBowD_[0].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        bowBridgeD_.setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        bridgeBowD_.setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        bowFingerD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        fingerNutD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        bowFingerD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );
        fingerNutD_[1].setMaximumDelay( (unsigned long)(length_*0.5 + 2) );

    }

}

void Cello :: setMode(StkFloat mode)
{
    // set the bow model mode
    if(mode==1.0)
    {
        bowMode_ = cello_friedlander;
    }
    else
        if(mode==2.0)
        {
            bowMode_ = cello_basic_hyperbolic;
        }
        else
        {
            bowMode_ = cello_basic_hyperbolic;
        }
}

void Cello :: setFilterParams(unsigned param, StkFloat value)
{
    switch(param)
    {
    case 1:
        low_shelf_freq_ = value;
        break;
    case 2:
        low_shelf_gain_ = value;
        break;
    case 3:
        low_shelf_q_ = value;
        break;
    case 4:
        peak_freq_ = value;
        break;
    case 5:
        peak_gain_ = value;
        break;
    case 6:
        peak_q_ = value;
        break;
    case 7:
        high_shelf_freq_ = value;
        break;
    case 8:
        high_shelf_gain_ = value;
        break;
    case 9:
        high_shelf_q_ = value;
        break;
    case 10:
//        string_[3].freq_ = value;
//        pic::logmsg() << "------------------------------------------------------- string freq " << value;
        break;
    case 11:
//        string_[3].q_ = value;
//        pic::logmsg() << "------------------------------------------------------- string q " << value;
        break;
    }
}


void Cello :: designFilters(StkFloat sample_rate)
{

    // set the string to body frequency response
    // biquad filter design equations from RBJ's audio cookbook

    // design low shelf filter

    //printf("sample freq = %f\n", sample_rate);

    StkFloat f0 = low_shelf_freq_;       // 400
    StkFloat dBgain = low_shelf_gain_;   // 12
    StkFloat S = low_shelf_q_;           // 1

    StkFloat A  = sqrt( pow(10,(dBgain/20) ));
    StkFloat w0 = 2*pi_*f0/sample_rate;
    StkFloat alpha = sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 );

    StkFloat b0 =    A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha );
    StkFloat b1 =  2*A*( (A-1) - (A+1)*cos(w0)                   );
    StkFloat b2 =    A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha );
    StkFloat a0 =        (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    StkFloat a1 =   -2*( (A-1) + (A+1)*cos(w0)                   );
    StkFloat a2 =        (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha;

    low_shelf_[0] = b0/a0;
    low_shelf_[1] = b1/a0;
    low_shelf_[2] = b2/a0;
    low_shelf_[3] = a1/a0;
    low_shelf_[4] = a2/a0;

    //printf("low shelf freq=%f gain=%f q=%f\n", low_shelf_freq_, low_shelf_gain_, low_shelf_q_);
    //printf("low shelf: 1, %.15f, %.15f, %.15f, %.15f, %.15f\n",b0/a0,b1/a0,b2/a0,a1/a0,a2/a0);

    // design peak filter

    f0 = peak_freq_;         // 1000
    dBgain = peak_gain_;     // -24
    StkFloat Q = peak_q_;    // 1

    A  = sqrt( pow(10,(dBgain/20)) );
    w0 = 2*pi_*f0/sample_rate;
    alpha = sin(w0)/(2*Q);

    b0 =   1 + alpha*A;
    b1 =  -2*cos(w0);
    b2 =   1 - alpha*A;
    a0 =   1 + alpha/A;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha/A;

    peak_[0] = b0/a0;
    peak_[1] = b1/a0;
    peak_[2] = b2/a0;
    peak_[3] = a1/a0;
    peak_[4] = a2/a0;

    //printf("peak freq=%f gain=%f q=%f\n", peak_freq_, peak_gain_, peak_q_);
    //printf("peak: 1, %.15f, %.15f, %.15f, %.15f, %.15f\n",b0/a0,b1/a0,b2/a0,a1/a0,a2/a0);

    f0 = high_shelf_freq_;      // 10000
    dBgain = high_shelf_gain_;  // -24
    Q = high_shelf_q_;          // 1

    A  = sqrt( pow(10,(dBgain/20)) );
    w0 = 2*pi_*f0/sample_rate;
    alpha = sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 );

    b0 =    A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha );
    b1 = -2*A*( (A-1) + (A+1)*cos(w0)                   );
    b2 =    A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha );
    a0 =        (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    a1 =    2*( (A-1) - (A+1)*cos(w0)                   );
    a2 =        (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;

    high_shelf_[0] = b0/a0;
    high_shelf_[1] = b1/a0;
    high_shelf_[2] = b2/a0;
    high_shelf_[3] = a1/a0;
    high_shelf_[4] = a2/a0;

    //printf("peak freq=%f gain=%f q=%f\n", peak_freq_, peak_gain_, peak_q_);
    //printf("high shelf: 1, %.15f, %.15f, %.15f, %.15f, %.15f\n",b0/a0,b1/a0,b2/a0,a1/a0,a2/a0);





    // design the string filters
    for(unsigned i=0; i<6; i++)
    {
        f0 = string_[i].freq_;  // 2000
        Q = string_[i].q_;      // 0.55

        w0 = 2*pi_*f0/sample_rate;
        alpha = sin(w0)/(2*Q);

        b0 =  (1 - cos(w0))/2;
        b1 =   1 - cos(w0);
        b2 =  (1 - cos(w0))/2;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;

        string_[i].coeff_[0] = b0/a0;
        string_[i].coeff_[1] = b1/a0;
        string_[i].coeff_[2] = b2/a0;
        string_[i].coeff_[3] = a1/a0;
        string_[i].coeff_[4] = a2/a0;
    }

    // nut filter
    nut_freq_ = 100;
    nut_q_ = 2;

    f0 = nut_freq_;      // 800
    Q = nut_q_;          // 1

    w0 = 2*pi_*f0/sample_rate;
    alpha = sin(w0)/(2*Q);

    b0 =  (1 - cos(w0))/2;
    b1 =   1 - cos(w0);
    b2 =  (1 - cos(w0))/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

    nut_[0] = b0/a0;
    nut_[1] = b1/a0;
    nut_[2] = b2/a0;
    nut_[3] = a1/a0;
    nut_[4] = a2/a0;



    // design the force solution filter

    fy_1_ = 0;
    f_g_ = (1.0-exp(-2.0*pi_*(500/sample_rate)));


}


/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * phaseDelay() : calculate the phase delay of a biquad filter
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */
StkFloat Cello :: phaseDelay(StkFloat f, StkFloat *coeffs, StkFloat sample_rate)
{
    StkFloat w = 2*pi_*(f/sample_rate);

    StkFloat b0 = coeffs[0];
    StkFloat b1 = coeffs[1];
    StkFloat b2 = coeffs[2];
    StkFloat a1 = coeffs[3];
    StkFloat a2 = coeffs[4];

    StkFloat cw = cos(w);
    StkFloat sw = sin(w);
    StkFloat cw2 = cos(2*w);
    StkFloat sw2 = sin(2*w);

    StkFloat A = b0+b1*cw+b2*cw2;
    StkFloat B = 1+a1*cw+a2*cw2;
    StkFloat C = b1*sw+b2*sw2;
    StkFloat D = a1*sw+a2*sw2;

    StkFloat N = (B*B + D*D);
    StkFloat X = (A*B+C*D)/N;
    StkFloat Y = (A*D-C*B)/N;

    StkFloat theta = atan(Y/X);

    StkFloat pd = 0;
    if(theta > 0)
        pd = (pi_-atan(Y/X))/w;
    else
        pd = -atan(Y/X)/w;

//    pic::logmsg() << "Cello :: phaseDelay w=" << w << " pd=" << pd;

    return pd;
}


/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * biquad() : biquad filter with a gain control
 *
 *                  B0 + B1*z1 + B2*z2
 *  H(z) = gain * --------------------
 *                  A0 + A1*z1 + A2*z2
 *  A0 = 1
 *
 *  Implemented as transposed direct form II for better speed and floating point precision!
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */
void Cello :: biquad(StkFloat input, StkFloat &output, StkFloat gain,
                        StkFloat b0, StkFloat b1, StkFloat b2,
                        StkFloat a1, StkFloat a2, StkFloat &z1, StkFloat &z2)
{
    output = b0*input + z1;
    z1 = b1*input - a1*output + z2;
    z2 = b2*input - a2*output;
    output = output*gain;
}


/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * memorylessBowJunction() : implementation of Julius Smith's bow-string scattering junction based on a memoryless lookup table.
 *
 * Based on the implementation used in the Stanford STK bowed string class.
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */
void Cello :: memorylessBowJunction(StkFloat bridgeVel, StkFloat nutVel, StkFloat bowVel, StkFloat &newBridgeVel, StkFloat &newNutVel)
{
    StkFloat velDiff, newVel;
    // string velocity is sum of the two velocity waves travelling in opposite directions
    StkFloat stringVel = bridgeVel + nutVel;

    // differential velocity between bow and string
    velDiff = bowVel - stringVel;
    // non-Linear bow function
    newVel = velDiff * bowTable_.tick( velDiff );

    newBridgeVel = bridgeVel + newVel;
    newNutVel = nutVel + newVel;

}



/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * hyperbolicBowJunction() : bow-string interaction model based on friction and string velocity with a hyperbolic friction function.
 *                           Based on the McIntyre Schumacher Woodhouse (MSW) algorithm.
 *
 * The velocity of the string is calculated from the graphical solution of the intersecting friction and string load line functions.
 * Bow slip-stick capture and release hysteresis is modelled by the solving the Friedlander ambiguity using the McIntyre-Woodhouse hysteresis rule.
 *
 * StkFloat vh,         string history velocity
 * StkFloat bowVel,     bow velocity
 * StkFloat bowForce,   normal force of the bow
 * StkFloat Z,          impedance of a cello string
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */
StkFloat Cello :: hyperbolicBowJunction(StkFloat vh, StkFloat bowVel, StkFloat bowForce, StkFloat Z)
{

    StkFloat Z2 = Z*2;

    StkFloat v0 = 1;    // constant that determines friction curve steepness
    StkFloat ud = 0.3;   // dynamic coefficient of friction
    StkFloat us = 0.8;    // static coefficient of friction

    // coefficient to normalize the curve, and position f=1 at the bow point
    StkFloat fmax = 1/(ud + (((us-ud)*v0)/(v0)));

    // solve the intersecting friction and string load line functions:
    // +ve velocity friction function: f+ = -(ud + (((us-ud)*v0)/(v0+vp-vb)))*fmax*fb
    // -ve velocity friction function: f- = (ud + (((us-ud)*v0)/(v0-(vn-vb))))*fmax*fb
    // string load line function: f = 2*Z*(v-vh)

    StkFloat vb = bowVel;
    StkFloat fb = bowForce;
    StkFloat f = 0;                     // force solution

    if (helmholtzState_ == cello_stick)
    {
        // bow and string are stuck together, velocity string = bow velocity

        // solution is where the load line crosses vertical discontinuity at the bow velocity
        f = Z2*(vb-vh);

        // according to McIntyre and Woodhouse, the string will be released when the force solution exceeds
        // the bow force and the system has passed through the Friedlander ambiguity and so has changed state...
        if (fabs(f)>fb)
            helmholtzState_=cello_slip;      // exceeded the sticking friction, so now slipping!

    }
    if (helmholtzState_ == cello_slip)
    {
        // bow and string are slipping past each other
        StkFloat c1, c2, c3, K, r;

        // find
        if (vb>vh)
        {
            // solve for -ve velocity
            K = fmax * fb;
            c1 = -1/(Z2*K);
            c2 = (1/K)*(v0-vh+vb) + (ud/Z2);
            c3 = (vh-vb)*ud - us*v0;
        }
        else
        {
            // solve for +ve velocity
            K = -fmax * fb;
            c1 = 1/(Z2*K);
            c2 = (1/K)*(v0+vh-vb) - (ud/Z2);
            c3 = (vb-vh)*ud - us*v0;
        }

        r = c2*c2-4*c1*c3;
        // check the hyperbolic solution is real valued, otherwise the solution has passed through
        // the Friedlander ambiguity and the system has changed state...
        if (r<0)
        {
            // the string has been captured by the bow, now sticking!
            // the slip->stick solution path is different from stick->slip leading to hysteresis
            helmholtzState_ = cello_stick;
            // solution is where load line crosses vertical discontinuity at the bow velocity
            f = Z2*(vb-vh);
        }
        else
        {
            // solution is where the load line crosses the hyperbolic function
            if (vb>vh)
                f = (-c2+sqrt(r))/(2*c1);    // solve intersection on positive curve
            else
                f = (-c2-sqrt(r))/(2*c1);    // solve intersection on negative curve
        }

    }

    // return force solution
    return f;

}

/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * friedlanderBowJunction() : bow-string interaction model based on friction and string velocity developed by Stefania Serafin and Julius Smith.
 *                            Based on the CCRMA SND Ruby version by Bill Schottstaedt, which was adapted from Serafins model.
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */

StkFloat Cello :: friedlanderBowJunction(StkFloat vh, StkFloat bowVel, StkFloat bowForce, StkFloat Z)
{

    StkFloat f=0, v=0, v1=0, v2=0, vtemp=0;
    StkFloat aa2=0, bb2=0, cc2=0, delta2=0;

    StkFloat mus = 0.7;
    StkFloat alpha = 0.2;
    StkFloat beta = 0.5;
    StkFloat gamma = 0.6;
/*
    StkFloat mus = p1_;
    StkFloat alpha = p2_;
    StkFloat beta = p3_;
    StkFloat gamma = p4_;
*/
    StkFloat vb = bowVel;
    StkFloat fb = bowForce;
    StkFloat zslope = 2*Z;
    StkFloat vhtemp=vh, sgn=1;

    if (vb==0 || fb==0)
    {
        // bow off string
        v = vh;
    }
    else
    {
        if (vh == vb)
        {
            v = vb;
            helmholtzState_ = cello_stick;
        }
        else
        {
            // determine which side of the discontinuity the friction solution is on
            if(vh>vb)
            {
                vhtemp = 2*vb-vh;
                sgn = -1;
            }
            else
            {
                vhtemp = vh;
                sgn = 1;
            }

            // calculate slipping friction curve
            aa2 = zslope;
            bb2 = ((-alpha * zslope - beta * fb) - zslope * vb) - zslope * vhtemp;
            cc2 = (((alpha * beta * fb + zslope * vhtemp * vb) + alpha * zslope * vhtemp) + beta * vb * fb) + gamma * fb;
            delta2 = bb2 * bb2 - 4 * aa2 * cc2;

            if (delta2 < 0)
            {
                // bow starting to stick
                v = vb;
                helmholtzState_ = cello_stick;
            }
            else
            {
                if (helmholtzState_ == cello_stick)
                {
                    // bow sticking, but could slip
                    vtemp = vb;

                    // decide if going to slip
                    f = zslope * (vtemp - vhtemp);
                    if (f <= (mus * fb) && f > 0)
                    {
                        // sticking
                        v = vtemp;
                    }
                    else
                    {
                        // bow starting to slip
                        v1 = (-bb2 - sqrt(delta2)) / (2.0 * aa2);
                        v2 = (-bb2 + sqrt(delta2)) / (2.0 * aa2);
                        v = std::min(v1, v2);
                        helmholtzState_ = cello_slip;
                    }
                }
                else
                {
                    // bow slipping
                    v1 = (-bb2 - sqrt(delta2)) / (2.0 * aa2);
                    v2 = (-bb2 + sqrt(delta2)) / (2.0 * aa2);
                    v = std::min(v1, v2);
                }
            }
            if (v > vb)
            {
                // slipping faster than bow, so sticking again
                v = vb;
                helmholtzState_ = cello_stick;
                //pic::logmsg() << "slipping faster than bow";
            }
        }
    }

    // solution force
    return zslope * (v - vhtemp) * sgn;

}

/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * bowJunction() : top-level bow-string scattering junction function
 *                 applies the selected bow-string model to the scattering junction
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if CELLO_DEBUG==1
unsigned c=0;
#endif // CELLO_DEBUG==1

void Cello :: bowJunction(StkFloat leftBowVel, StkFloat rightBowVel, StkFloat releaseEnvelope, StkFloat &newLeftBowVel, StkFloat &newRightBowVel)
{
    StkFloat bowForce = 0;

    StkFloat bowVelocity = bowVelocity_y_1_ + bowVelocity_g_*(bowVelocity_-bowVelocity_y_1_);
    bowVelocity_y_1_ = bowVelocity;

    StkFloat Z = string_[current_string_].Z_; // 0.55
//    StkFloat Z = p0_;
    StkFloat invZ2 = 1/(2*Z);
    StkFloat vh = leftBowVel+rightBowVel;
    StkFloat f = 0;

    StkFloat bowVel = maxBowVelocity_ * bowVelocity;

    // test mode, auto bowing
//    bowVel = 1;
//    bowForce = 0.47;
//    bowOnString_ = true;

    switch(bowMode_)
    {
    case cello_basic_hyperbolic:
    {
        // condition bow force to make the model playable

        StkFloat freq = sample_rate_/delay_current_;

        StkFloat maxForce = 0;
        StkFloat minForce = 0;

        if(freq<=329.627563)
            maxForce = -2.0426e-06*freq*freq - 5.6272e-03*freq + 2.7736;
        else
            if(freq<=440)
                maxForce = -0.0017828*freq + 1.2844241;
            else
                maxForce = 0.5;

        if(freq<=293.664764)
            minForce = 0.25;
        else
            if(freq<=329.627563)
                minForce = 0.0055613*freq - 1.3831586;
            else
                minForce = 0.45;

    //    minForce = 0.3;
    //    maxForce = testValue_[1];
    //    StkFloat minForce = testValue_[0];

        // smooth the bow force
        bowForce = bowForce_y_1_ + bowForce_g_*(bowForce_-bowForce_y_1_);
        bowForce_y_1_ = bowForce;

        bowForce = (bowForce * (maxForce - minForce) + minForce);     // normal

        // ------- Basic MSW algorithm -------
        // solve force at bow-string junction, impedance of a cello string Z = 0.55
        f = hyperbolicBowJunction(vh, bowVel, bowForce, Z);

        // update the string velocities with the force solution
        if(bowOnString_)
        {
            newRightBowVel = rightBowVel + f*invZ2;
            newLeftBowVel = leftBowVel + f*invZ2;
        }
        else
        {
            // release the string
            newRightBowVel = rightBowVel + f*invZ2*releaseEnvelope;
            newLeftBowVel = leftBowVel + f*invZ2*releaseEnvelope;
        }

#if CELLO_DEBUG==1
        c++;
        if(c==7000)
        {
            printf("f=%f b=%f s=%d vb=%f vh=%f fs=%f a=%f t=%f\n", sample_rate_/baseDelay_, bowForce, current_string_, bowVel, vh, f, alpha_[dl_], testValue_[0]);
            //                    pic::printmsg() << " freq=" << sample_rate_/baseDelay_ << " vb=" << bowVel << " fb=" << bowForce <<
            //                        " f=" << f << " hs=" << helmholtzState_ << " bv-vh" << bowVel-vh <<
            //                        " nv=" << nutVel << " nnv=" << newNutVel << " bv=" << bridgeVel << " nbv=" << newBridgeVel << " vh=" << vh;
            pic::logmsg() <<  string_[0].freq_ << " " << string_[0].q_ << " v0=" << v0 << " ud="<< ud << " us=" << us;

            c=0;
        }
#endif // CELLO_DEBUG==1

    }
    break;

    case cello_friedlander:
    {

        // smooth the bow force
        bowForce = bowForce_y_1_ + bowForce_g_*(bowForce_-bowForce_y_1_);
        bowForce_y_1_ = bowForce;


        switch(current_string_)
        {
        case 0:
        case 1:
            bowForce = std::max((bowForce * 4)-0.1, 0.0);
            bowForce = std::min(bowForce, 1.0);
            break;
        case 2:
            bowForce = std::max((bowForce * 4)+0.1, 0.0);
            bowForce = std::min(bowForce, 1.1);
            break;
        case 3:
        case 4:
            bowForce = std::max((bowForce * 4)+0.1, 0.0);
            bowForce = std::min(bowForce, 0.2);
            break;
        case 5:
            bowForce = std::max((bowForce * 4)+0.1, 0.0);
            bowForce = std::min(bowForce, 0.1);
            break;
        }

        // test mode
//        bowForce = 0.4;

        // ------- Enhanced MSW algorithm with improved Friedlander solution -------

        f = friedlanderBowJunction(vh, bowVel, bowForce*bowVel*bowVel, Z);

        // update the string velocities with the force solution
        if(bowOnString_)
        {
            newRightBowVel = rightBowVel + f*invZ2;
            newLeftBowVel = leftBowVel + f*invZ2;
        }
        else
        {
            // release the string
            newRightBowVel = rightBowVel + f*invZ2*releaseEnvelope;
            newLeftBowVel = leftBowVel + f*invZ2*releaseEnvelope;
        }

#if CELLO_DEBUG==1
        c++;
        if(c==7000)
        {
            pic::logmsg() << "freq=" << sample_rate_/baseDelay_ << " vb=" << bowVel << " fb=" << bowForce << " string=" << current_string_;
            //pic::logmsg() <<  " freq=" << string_[0].freq_ << " q=" << string_[0].q_ << " Z=" << p0_ << " mus="<< p1_ << " alpha=" << p2_ << " beta=" << p3_ << " gamma=" << p4_;
            c=0;
        }
#endif // CELLO_DEBUG==1

        break;
    }


    case cello_width_hyperbolic:
    {
        // ------- Enhanced MSW algorithm accounting for bow width -------
        StkFloat Z = string_[current_string_].Z_; // 0.55
        StkFloat invZ2 = 1/(2*Z);

        // solve left side of bow
        bridgeVel_2_ = rightBowDelay_.lastOut();
        StkFloat vhl = bridgeVel_2_+rightBowVel;
        StkFloat fl = hyperbolicBowJunction(vhl, bowVel, bowForce, Z);
        // update the string velocities
        if(bowOnString_)
        {
            nutVel_1_ = rightBowVel + fl*invZ2;
            newLeftBowVel = bridgeVel_2_ + fl*invZ2;
        }
        else
        {
            // release the string
            nutVel_1_ = rightBowVel;
            newLeftBowVel = bridgeVel_2_;
        }

        // solve right side of bow
        nutVel_2_ = leftBowDelay_.lastOut();
        StkFloat vhr = leftBowVel+nutVel_2_;
        StkFloat fr = hyperbolicBowJunction(vhr, bowVel, bowForce, Z);
        // update the string velocities
        if(bowOnString_)
        {
            newRightBowVel = nutVel_2_ + fr*invZ2;
            bridgeVel_1_ = leftBowVel + fr*invZ2;
        }
        else
        {
            // release the string
            newRightBowVel = nutVel_2_;
            bridgeVel_1_ = leftBowVel;
        }

        // do bow propagations
        leftBowDelay_.tick(nutVel_1_);
        rightBowDelay_.tick(bridgeVel_1_);

        //          c++;
        //          if(c==1000)
        //          {
        //              pic::printmsg() << " vbmax=" << maxBowVelocity_ << " vb=" << bowVel << " fb=" << bowForce <<
        //                                   " fl=" << fl << " hs=" << helmholtzState_ << " bv-vhl" << bowVel-vhl <<
        //                                   " nv=" << nutVel << " nnv=" << newNutVel << " bv=" << bridgeVel << " nbv=" << newBridgeVel << " vhl=" << vhl;
        //              c=0;
        //          }

    }
    break;

    case cello_memoryless:
    default:
        // ------- JOS simple memoryless algorithm -------
        memorylessBowJunction(leftBowVel, rightBowVel, bowVel, newLeftBowVel, newRightBowVel);

        //        c++;
        //        if(c==2000)
        //        {
        //            pic::printmsg() << " vb=" << bowVel << " fb=" << bowForce << " nv=" << nutVel << " nnv=" << newNutVel << " bv=" << bridgeVel << " nbv=" << newBridgeVel;
        //            c=0;
        //        }

        break;
    }
    if(pic::isnan(newLeftBowVel))
        newLeftBowVel = 0;

    if(pic::isnan(newRightBowVel))
        newRightBowVel = 0;

}

void Cello :: fingerJunction(StkFloat leftFingerVel, StkFloat rightFingerVel, StkFloat a, StkFloat &newLeftFingerVel, StkFloat &newRightFingerVel)
{
    StkFloat r = -1;

    newRightFingerVel = a*r*leftFingerVel + (1-a)*rightFingerVel;
    StkFloat filtRightFingerVel;
    biquad(rightFingerVel, filtRightFingerVel, -0.959, nut_[0], nut_[1], nut_[2], nut_[3], nut_[4], nut_z1_, nut_z2_);
    newLeftFingerVel = a*r*filtRightFingerVel + (1-a)*leftFingerVel;
//    newLeftFingerVel = a*r*rightFingerVel + (1-a)*leftFingerVel;
}

void Cello :: noteOn(StkFloat frequency, StkFloat amplitude)
{
    //this->startBowing( amplitude );
    //  this->setFrequency( frequency );

    // start of a note, so frequency will be swept to new frequency, not pitch bend
    newNote_ = true;

    bowForce_y_1_ = 0.0;
    bowVelocity_y_1_ = 0.0;

//    pic::logmsg() << "note on";

    startEnvelope_.setRate(0.0001);
    startEnvelope_.setValue(0.0);
    startEnvelope_.setTarget(startTarget_);


#if defined(_STK_DEBUG_)
    errorString_ << "Cello::NoteOn: frequency = " << frequency << ", amplitude = " << amplitude << ".";
    handleError( StkError::DEBUG_WARNING );
#endif
}


/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * computeSample() : compute one sample of the model
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */

StkFloat Cello :: computeSample()
{
    // ------- Condition bow force and velocity -------


    StkFloat startEnvelope = startEnvelope_.tick();
    StkFloat releaseEnvelope = releaseEnvelope_.tick();

    if(bowOnString_)
    {
        releaseEnvelope_.setTarget(1);
        releaseEnvelope_.setRate(0.001);
    }
    else
    {
        releaseEnvelope_.setTarget(0);
        releaseEnvelope_.setRate(0.001);
    }

    // ------- Pitch-bend -------

    if(delay_current_ != delay_target_ && !current_latch_)
    {
        // sweep the delay time logarithmically using a scaling coefficient
        delay_current_ *= delay_step_;

        if(delay_step_>=1)
            if(delay_current_>=delay_target_)
            {
                delay_current_ = delay_target_;
                delay_step_ = 1;
            }

        if(delay_step_<1)
            if(delay_current_<=delay_target_)
            {
                delay_current_ = delay_target_;
                delay_step_ = 1;
            }

        // set the finger position
        setDelays(0, (1-betaRatio_)*delay_current_*0.5, betaRatio_*delay_current_*0.5);


    }


    // ------- Bow-string scattering junction -------

    StkFloat leftBowVel, rightBowVel, newLeftBowVel, newRightBowVel;

    leftBowVel = bridgeBowD_.lastOut();
    rightBowVel = fingerBowD_[dl_].lastOut();

    bowJunction(leftBowVel, rightBowVel, releaseEnvelope, newLeftBowVel, newRightBowVel);



    // ------- Finger-string scattering junction -------

    StkFloat leftFingerVel[2], rightFingerVel[2], newLeftFingerVel[2], newRightFingerVel[2];
    bool noteTransitioning = state_ != CELLO_SUSTAIN;

    if(noteTransitioning)
    {
        leftFingerVel[0] = bowFingerD_[0].lastOut();
        rightFingerVel[0] = nutFingerD_[0].lastOut();
        fingerJunction(leftFingerVel[0], rightFingerVel[0], alpha_[0], newLeftFingerVel[0], newRightFingerVel[0]);

        leftFingerVel[1] = bowFingerD_[1].lastOut();
        rightFingerVel[1] = nutFingerD_[1].lastOut();
        fingerJunction(leftFingerVel[1], rightFingerVel[1], alpha_[1], newLeftFingerVel[1], newRightFingerVel[1]);
    }
    else
    {
        leftFingerVel[dl_] = bowFingerD_[dl_].lastOut();
        rightFingerVel[dl_] = nutFingerD_[dl_].lastOut();
        fingerJunction(leftFingerVel[dl_], rightFingerVel[dl_], alpha_[dl_], newLeftFingerVel[dl_], newRightFingerVel[dl_]);
    }


    // ------- String damping filter -------

    StkFloat bridgeVel, bridgeVelDamped;

    bridgeVel = bowBridgeD_.lastOut();

    string_t *ptr = &string_[current_string_];
    // TODO: fade coeffs here with alpha
    biquad(bridgeVel, bridgeVelDamped, -0.959, ptr->coeff_[0], ptr->coeff_[1], ptr->coeff_[2], ptr->coeff_[3], ptr->coeff_[4], string_z1_, string_z2_);


    // ------- Nut -------

    StkFloat nutVel[2];
    if(noteTransitioning)
    {
        nutVel[0] = -fingerNutD_[0].lastOut();
        nutVel[1] = -fingerNutD_[1].lastOut();
    }
    else
    {
        nutVel[dl_] = -fingerNutD_[dl_].lastOut();
    }

    // ------- String delay lines -------
    if(noteTransitioning)
    {
        nutFingerD_[0].tick(nutVel[0]);
        fingerBowD_[0].tick(newRightFingerVel[0]);
        bowFingerD_[0].tick(newLeftBowVel);
        fingerNutD_[0].tick(newLeftFingerVel[0]);

        nutFingerD_[1].tick(nutVel[1]);
        fingerBowD_[1].tick(newRightFingerVel[1]);
        bowFingerD_[1].tick(newLeftBowVel);
        fingerNutD_[1].tick(newLeftFingerVel[1]);
    }
    else
    {
        nutFingerD_[dl_].tick(nutVel[dl_]);
        fingerBowD_[dl_].tick(newRightFingerVel[dl_]);
        bowFingerD_[dl_].tick(newLeftBowVel);
        fingerNutD_[dl_].tick(newLeftFingerVel[dl_]);
    }

    bowBridgeD_.tick(newRightBowVel);
    bridgeBowD_.tick(bridgeVelDamped);


    // ------- Note transition state management -------

    StkFloat a = alpha_[dl_];
    StkFloat oma = 1-a;
    StkFloat fingerBow = 0;
    StkFloat bowBridge = 0;
    switch(state_)
    {
    case CELLO_HIGHER_FILL:
        delayFillSamples_--;
        if(delayFillSamples_<=0)
        {
#if CELLO_DEBUG==1
            pic::logmsg() << "------- CELLO_HIGHER_FILL DONE -------";
#endif // CELLO_DEBUG==1
            dl_ = dl_^1;            // switch delay line
            state_ = CELLO_HIGHER_FADE;
        }
        break;
    case CELLO_HIGHER_FADE:
        alpha_[dl_] += alphaStep_;
        // interpolate bow position
        fingerBow = 0.5*(oma*(delay_target_-betaRatio_*delay_current_) + a*(delay_target_-betaRatio_*delay_target_));
        bowBridge = 0.5*(oma*betaRatio_*delay_current_ + a*betaRatio_*delay_target_);
        bowBridgeD_.setDelay(bowBridge);
        bridgeBowD_.setDelay(bowBridge);
        fingerBowD_[dl_].setDelay(fingerBow);
        bowFingerD_[dl_].setDelay(fingerBow);
        // interpolate string filter

        alphaFadeSamples_--;
        if(alphaFadeSamples_<=0)
        {
#if CELLO_DEBUG==1
            pic::logmsg() << "------- CELLO_HIGHER_FADE DONE -------";
#endif // CELLO_DEBUG==1
            alpha_[dl_] = 1.0;
            nutFingerD_[dl_].setDelay(0);
            fingerNutD_[dl_].setDelay(0);
            state_ = CELLO_SUSTAIN;
            current_latch_ = false;
            delay_current_=delay_target_;
            setDelays(0,((1-betaRatio_)*delay_current_)*0.5,betaRatio_*delay_current_*0.5);
        }
        break;
    case CELLO_LOWER_FADE:
        alpha_[dl_] += alphaStep_;
        // interpolate bow position
        fingerBow = 0.5*(oma*(delay_current_-betaRatio_*delay_target_) + a*(delay_current_-betaRatio_*delay_current_));
        bowBridge = 0.5*(oma*betaRatio_*delay_target_ + a*betaRatio_*delay_current_);
        bowBridgeD_.setDelay(bowBridge);
        bridgeBowD_.setDelay(bowBridge);
        fingerBowD_[dl_].setDelay(fingerBow);
        bowFingerD_[dl_].setDelay(fingerBow);
        // interpolate string filter

        alphaFadeSamples_--;
        if(alphaFadeSamples_<=0)
        {
#if CELLO_DEBUG==1
            pic::logmsg() << "------- CELLO_LOWER_FADE DONE -------";
#endif // CELLO_DEBUG==1
            alpha_[dl_] = 0;
            state_ = CELLO_LOWER_FILL;
        }
        break;
    case CELLO_LOWER_FILL:
        delayFillSamples_--;
        if(delayFillSamples_<=0)
        {
#if CELLO_DEBUG==1
            pic::logmsg() << "------- CELLO_LOWER_FILL DONE -------";
#endif // CELLO_DEBUG==1
            dl_ = dl_^1;            // switch delay line
            state_ = CELLO_SUSTAIN;
            current_latch_ = false;
            delay_current_=delay_target_;
            setDelays(0,((1-betaRatio_)*delay_current_)*0.5,betaRatio_*delay_current_*0.5);
        }
        break;
    }

    // ------- Output conditioning -------

    StkFloat envelope = envelope_.tick();

    // output from string
    lastOutput_ = bowBridgeD_.lastOut() * envelope * (startEnvelope); // * startEnvelope);
    //    lastOutput_ = fingerNutD_[dl_].lastOut() * envelope * startEnvelope;

    // D.C. blocking filter
    StkFloat aout_x, aout_y;

    aout_x = lastOutput_;
    aout_y = aout_x - aout_x1_ + 0.995 * aout_y1_;
    aout_x1_ = aout_x;
    aout_y1_ = aout_y;

    // shape the string tone
    StkFloat low_shelf = 0, peak = 0, high_shelf = 0;
    biquad(aout_y, low_shelf, 1, low_shelf_[0], low_shelf_[1], low_shelf_[2], low_shelf_[3], low_shelf_[4], low_shelf_z1_, low_shelf_z2_);
    biquad(low_shelf, peak, 1, peak_[0], peak_[1], peak_[2], peak_[3], peak_[4], peak_z1_, peak_z2_);
    biquad(peak, high_shelf, 1, high_shelf_[0], high_shelf_[1], high_shelf_[2], high_shelf_[3], high_shelf_[4], high_shelf_z1_, high_shelf_z2_);

    // reduce level by about 10dB
    StkFloat output;

    output = 0.3*high_shelf;
//    output = 0.3*aout_y;
    // limiting
    if(output>1)
        output=1;
    if(output<-1)
        output=-1;
    return output;
}

void Cello :: controlChange(int number, StkFloat value)
{
    StkFloat norm = value * ONE_OVER_128;
    if ( norm < 0 ) {
        norm = 0.0;
        errorString_ << "Cello::controlChange: control value less than zero ... setting to zero!";
        handleError( StkError::WARNING );
    }
    else if ( norm > 1.0 ) {
        norm = 1.0;
        errorString_ << "Cello::controlChange: control value greater than 128.0 ... setting to 128.0!";
        handleError( StkError::WARNING );
    }

    if (number == __SK_BowPressure_) // 2
    {
        errorString_ << "Cello::controlChange: need to do something with this...";
    }
    else if (number == __SK_BowPosition_) // 4
    {
        betaRatio_ = 0.027236 + (0.2 * norm);
//        bridgeDelay_->setDelay( baseDelay_ * betaRatio_, 0 );
//        neckDelay_->setDelay( baseDelay_ * (1.0 - betaRatio_), 0 );
    }
    else if (number == __SK_ModFrequency_) // 11
        vibrato_.setFrequency( norm * 12.0 );
    else if (number == __SK_ModWheel_) // 1
        vibratoGain_ = ( norm * 0.4 );
    else if (number == __SK_AfterTouch_Cont_) // 128
        errorString_ << "Cello::controlChange: undefined control number (" << number << ")!";
    else {
        errorString_ << "Cello::controlChange: undefined control number (" << number << ")!";
        handleError( StkError::WARNING );
    }

#if defined(_STK_DEBUG_)
    errorString_ << "Cello::controlChange: number = " << number << ", value = " << value << ".";
    handleError( StkError::DEBUG_WARNING );
#endif
}

