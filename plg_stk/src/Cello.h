/*
 * Cello.h
 *
 *  Cello model based on the bow-string model of McIntyre Schumacher Woodhouse and extended models of Serafin.
 *  This implementation uses some of the CCRMA STK framework written by by Perry R. Cook and Gary P. Scavone, 1995 - 2007
 */

#ifndef STK_Cello_H
#define STK_Cello_H

#include "Instrmnt.h"
#include "DelayL.h"
#include "DelayA.h"
#include "BowTable.h"
#include "OnePole.h"
#include "BiQuad.h"
#include "SineWave.h"
#include "ADSR.h"

#include <picross/pic_log.h>

// if 1 have to add functions
//    void set_param_num(unsigned)
//    void set_param_val(float)
// to the stk.pip file in class cello
#define CELLO_TESTVALUE 0

enum helmholtzStates_t
{
    cello_stick = 0,
    cello_slip
};

enum celloBowMode_t
{
    cello_width_hyperbolic = 0,
    cello_basic_hyperbolic,
    cello_friedlander,
    cello_memoryless
};

struct string_t
{
    StkFloat top_note_;
    StkFloat freq_, q_, Z_;
    StkFloat coeff_[5];
};


class Cello : public Instrmnt
{
public:
    //! Class constructor, taking the lowest desired playing frequency.
    Cello(StkFloat lowestFrequency);

    //! Class destructor.
    ~Cello();

    //! Reset and clear all internal state.
    void clear();

    //! Set instrument parameters for a particular frequency.
    void setFrequency(StkFloat frequency);

    //! Set vibrato gain.
    void setVibrato(StkFloat gain);

    //! Apply breath pressure to instrument with given amplitude and rate of increase.
    void startBowing(StkFloat amplitude);

    //! Decrease breath pressure with given rate of decrease.
    void stopBowing(StkFloat amplitude);

    //! Set bow velocity
    void setBowVelocity(StkFloat bowVelocity, bool bow_on_string);

    //! Start a note with the given frequency and amplitude.
    void noteOn(StkFloat frequency, StkFloat amplitude);

    //! Stop a note with the given amplitude (speed of decay).
    void noteOff(StkFloat amplitude);

    //! Perform the control change specified by \e number and \e value (0.0 - 128.0).
    void controlChange(int number, StkFloat value);

    //! Set the time to sweep the pitch between notes
    void setPitchSweepTime(StkFloat pitch_time);

    //! Set the bow width
    void setBowWidth(StkFloat bow_width);

    //! Set the lowest playable frequency
    void setLowestFrequency(StkFloat lowest_frequency);

    //! Set the bow model mode
    void setMode(StkFloat mode);

    //! Set the string to body filter parameters
    void setFilterParams(unsigned param, StkFloat value);

    //! Design the string to body filters
    void designFilters(StkFloat sample_rate);

    //! Calculate the phase delay of a biquad filter
    StkFloat phaseDelay(StkFloat f, StkFloat *coeffs, StkFloat sample_rate);

    //! Transition between notes
    void noteTransition();

    void biquad(StkFloat input, StkFloat &output, StkFloat gain,
                StkFloat b0, StkFloat b1, StkFloat b2,
                StkFloat a1, StkFloat a2, StkFloat &z1, StkFloat &z2);

#if CELLO_TESTVALUE==1
    void setTestValue(unsigned number, StkFloat value)
    {
        testValue_[number-1] = value;
//        printf("test value %d = %f\n", number, value);
        pic::logmsg() << "test value " << number << " = " << value;
        setFrequency(sample_rate_/baseDelay_);
    }

    void set_friction_params(float p0, float p1, float p2, float p3, float p4)
    {
        p0_ = p0;
        p1_ = p1;
        p2_ = p2;
        p3_ = p3;
        p4_ = p4;
    }
#endif // CELLO_TESTVALUE==1

protected:

    void sampleRateChanged( StkFloat newRate, StkFloat oldRate );
    void initDelayLines( StkFloat sample_rate, StkFloat lowestFrequency );
    void setDelays( StkFloat nutFinger, StkFloat fingerBow, StkFloat bowBridge );
    void pickString(StkFloat frequency);
    StkFloat delayCompensate(StkFloat frequency, const StkFloat delay[29][2]);
    void tuneNote(StkFloat frequency);

    void memorylessBowJunction(StkFloat bridgeVel, StkFloat nutVel, StkFloat bowVel, StkFloat &newBridgeVel, StkFloat &newNutVel);
    StkFloat hyperbolicBowJunction(StkFloat vh, StkFloat bowVel, StkFloat bowForce, StkFloat Z);
    StkFloat friedlanderBowJunction(StkFloat vh, StkFloat bowVel, StkFloat bowForce, StkFloat Z);
    void bowJunction(StkFloat leftBowVel, StkFloat rightBowVel, StkFloat releaseEnvelope,
                                StkFloat &newLeftBowVel, StkFloat &newRightBowVel);
    void fingerJunction(StkFloat leftFingerVel, StkFloat rightFingerVel, StkFloat alpha, StkFloat &newLeftFingerVel, StkFloat &newRightFingerVel);

    StkFloat computeSample( void );

    DelayL   bridgeBowD_;       // left 0
    DelayL   bowFingerD_[2];    // left 1
    DelayL   fingerNutD_[2];    // left 2
    DelayL   nutFingerD_[2];    // right 0
    DelayL   fingerBowD_[2];    // right 1
    DelayL   bowBridgeD_;       // right 2

    StkFloat p0_, p1_, p2_, p3_, p4_;

    unsigned dl_;
    unsigned state_;
    StkFloat alpha_[2];
    StkFloat alphaFadeSamples_;
    StkFloat alphaStep_;
    StkFloat delayFillSamples_;

    DelayL   leftBowDelay_;
    DelayL   rightBowDelay_;
    BowTable bowTable_;
    SineWave vibrato_;
    Envelope envelope_;
    Envelope startEnvelope_;
    Envelope releaseEnvelope_;
    StkFloat maxBowVelocity_;
    StkFloat baseDelay_;
    StkFloat vibratoGain_;
    StkFloat betaRatio_;
    StkFloat lastBowPosition_;
    StkFloat bowVelocity_;
    StkFloat bowForce_;
    StkFloat minBowForce_;
    bool bowOnString_;
    helmholtzStates_t helmholtzState_;
    celloBowMode_t bowMode_;
    StkFloat bowVelocity_y_1_;
    StkFloat bowVelocity_g_;
    StkFloat bowForce_y_1_;
    StkFloat bowForce_g_;
    StkFloat nutVel_1_;
    StkFloat nutVel_2_;
    StkFloat bridgeVel_1_;
    StkFloat bridgeVel_2_;
    StkFloat bowWidth_;
    StkFloat bowWidthSamps_;
    StkFloat pi_;
    StkFloat lowestFrequency_;

    bool     current_latch_;                // flag the target delay latched for note transition
    bool     newNote_;                      // flag new note has started
    bool     startBow_;
    bool     noteTransition_;               // flag note transitioning to a new note on same string
    StkFloat delay_target_;                 // target string delay time from frequency change
    StkFloat delay_current_;                // current string delay time
    StkFloat delay_previous_;               // previous string delay time
    StkFloat delay_step_;                   // rate of change of bore delay
    StkFloat pitch_samps_;                  // normalized rate of change of pitch (string delay)
    unsigned long length_;                  // max length of delay lines
    StkFloat sample_rate_;

    StkFloat cutoff_frequency_;

    StkFloat aout_x1_;
    StkFloat aout_y1_;

    StkFloat low_shelf1_freq_;
    StkFloat low_shelf1_gain_;
    StkFloat low_shelf1_q_;

    StkFloat low_shelf_freq_;
    StkFloat low_shelf_gain_;
    StkFloat low_shelf_q_;
    StkFloat peak_freq_;
    StkFloat peak_gain_;
    StkFloat peak_q_;
    StkFloat high_shelf_freq_;
    StkFloat high_shelf_gain_;
    StkFloat high_shelf_q_;
    StkFloat low_shelf_[5];
    StkFloat peak_[5];
    StkFloat high_shelf_[5];
    StkFloat low_shelf_z1_, low_shelf_z2_, peak_z1_, peak_z2_, high_shelf_z1_, high_shelf_z2_;

    StkFloat nut_freq_;
    StkFloat nut_gain_;
    StkFloat nut_q_;
    StkFloat nut_[5];
    StkFloat nut_z1_, nut_z2_;

    string_t string_[6];
    string_t target_string_coeffs_[5];
    unsigned current_string_;
    StkFloat string_z1_, string_z2_;

    StkFloat fy_1_;
    StkFloat f_g_;

    StkFloat startTarget_;                  // target gain for startup, used for disabling output

#if CELLO_TESTVALUE==1
    StkFloat testValue_[2];
#endif // TESTVALUE==1

    static const StkFloat delay_compensation_44_[][2];
    static const StkFloat delay_compensation_48_[][2];
    static const StkFloat delay_compensation_96_[][2];

};

#endif
