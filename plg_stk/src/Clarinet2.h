/*
 * Clarinet2.h
 *
 *  Created on: Mar 9, 2009
 *      Author: arran
 *
 *  Clarinet model based on the Csound implementation by Josep M Comajuncosas / dec '98-feb '99
 *  of the JNMR article (Vol.24 pp.148-163) written by Gijs de Bruin & Maarten van Walstijm
 */

#ifndef CLARINET2_H_
#define CLARINET2_H_

#include "Instrmnt.h"
#include "DelayL.h"
#include "DelayA.h"
#include "Envelope.h"
#include "Noise.h"
#include "SineWave.h"
#include <picross/pic_log.h>

#define CLARINET_TESTVALUE 0

class Clarinet2 : public Instrmnt
{
public:
    //! Class constructor, taking the lowest desired playing frequency.
    /*!
    An StkError will be thrown if the rawwave path is incorrectly set.
    */
    Clarinet2(StkFloat lowestFrequency);

    //! Class destructor.
    ~Clarinet2();

    //! Reset and clear all internal state.
    void clear();

    //! Set the note frequency.
    void setFrequency(StkFloat frequency);

    //! Set the breath pressure.
    void setPressure(StkFloat amplitude);

    //! Start a note with the given frequency and amplitude.
    void noteOn(StkFloat frequency, StkFloat amplitude);

    //! Stop a note with the given amplitude (speed of decay).
    void noteOff(StkFloat amplitude);

    //! Transition between notes
    void noteTransition();

    //! Perform the control change specified by \e number and \e value (0.0 - 128.0).
    void controlChange(int number, StkFloat value);

    //! Set the rate at which the pitch is swept between notes
    void setPitchSweepTime(StkFloat pitch_time);

#if CLARINET_TESTVALUE==1
    void setTestValue(StkFloat value)
    {
        testValue_=value;
//        printf("---------------------------- test value = %f\n", testValue_);
    }
#endif // TESTVALUE==1


    void dumpState()
    {
        pic::logmsg() << "breath="<<breathPressure_ << "delay tar=" << delay_target_ << " delay curr=" << delay_current_;
    }

    void sampleRateChanged( StkFloat newRate, StkFloat oldRate );

    void setLowestFrequency(StkFloat lowest_frequency);

protected:

    StkFloat computeSample( void );

    DelayA delayLine_;                      // non-interpolating delay line
    Envelope envelope_;
    Envelope kp0_envelope_;
    Noise noise_;
    SineWave vibrato_;
    long length_;
    StkFloat outputGain_;
    StkFloat noiseGain_;
    StkFloat vibratoGain_;
    StkFloat frequency_current_;
    StkFloat frequency_target_;
    StkFloat frequency_step_;

private:

    void reedOsc(StkFloat dP, StkFloat Vr, StkFloat fr, StkFloat Qr, StkFloat Ur, StkFloat Hr);

    void reedClip(StkFloat g, StkFloat m, StkFloat h);

    void bFlow(StkFloat Z0, StkFloat Ir, StkFloat Er);

    void biquad(StkFloat input, StkFloat &output, StkFloat gain, StkFloat b0, StkFloat b1, StkFloat b2, StkFloat a1, StkFloat a2, StkFloat &z1, StkFloat &z2);

    // physical constants
    StkFloat ipi;
    StkFloat inumb_iter;                    // number of iterations used in the Newton-Rhapson method
    StkFloat ifr;                           // reed resonance frequency
    StkFloat iwr;                           // normalized in radians/cycle
    StkFloat ier;                           //  = B^(-3/2) where B = airflow constant ~ 0.08
    StkFloat ihr;                           // aperture of the reed in equilibrium
    StkFloat iir;                           // inertance of the air above the reed
    StkFloat iq;                            // set q=2 for lip & double reed????
    StkFloat ist;
    StkFloat isr;
    StkFloat iqr;
    StkFloat igr;
    StkFloat iur;
    StkFloat ic;                            // sound velocity
    StkFloat irho;                          // air density
    StkFloat iz0;
    StkFloat iv;                            // iv = -1 (lip) or 1 (cane). Here "cane"

    StkFloat sr;                            // sampling rate
    StkFloat it;                            // sampling period

    // physical variables
    StkFloat kx1;                           // previous reed aperture
    StkFloat kx2;                           // current reed aperture

    StkFloat kuf1;                          // previous normal volume flow
    StkFloat kuf2;                          // current normal volume flow

    StkFloat api2;                          // ingoing pressure wave into the tube

    StkFloat apr2;                          // reflected pressure wave from the tube

    StkFloat apm1;                          // previous mouthpiece pressure
    StkFloat apm2;                          // current mouthpiece pressure

    StkFloat kp0;                           // flow

    unsigned long kdltn1;                   // integer delay length
    StkFloat kdltf1;                        // fractional delay length

    // filter states
    StkFloat zr1_, zr2_;                    // reflectance biquad states
    StkFloat zt1_, zt2_;                    // transmission biquad states

    StkFloat breathPressure_, power_;
    StkFloat breathPressure_y_1_, noise_y_1_, power_y_1_;
    StkFloat breathPressure_g_, noise_g_, power_g_;
    StkFloat breathScaling_;

    StkFloat aprf1_1_;                      // delay interpolation history

    static const StkFloat refl_filt_coeffs_44_[];
    static const StkFloat tran_filt_coeffs_44_[];
    static const StkFloat refl_filt_delay_44_[];

    static const StkFloat refl_filt_coeffs_48_[];
    static const StkFloat tran_filt_coeffs_48_[];
    static const StkFloat refl_filt_delay_48_[];

    static const StkFloat refl_filt_coeffs_88_[];
    static const StkFloat tran_filt_coeffs_88_[];
    static const StkFloat refl_filt_delay_88_[];

    static const StkFloat refl_filt_coeffs_96_[];
    static const StkFloat tran_filt_coeffs_96_[];
    static const StkFloat refl_filt_delay_96_[];

    const StkFloat *rcoeffs_;
    const StkFloat *tcoeffs_;
    const StkFloat *rdelay_;

    StkFloat aout_x1_;
    StkFloat aout_y1_;

    StkFloat fadeGain_;
    StkFloat fadeStep_;
    bool     newNote_;
    StkFloat last_amplitude_;
    bool     fading_;

    StkFloat lowestFrequency_;

    StkFloat delay_target_;                 // target bore delay time from frequency change
    StkFloat delay_current_;                // current bore delay time
    StkFloat delay_step_;                   // rate of change of bore delay
    StkFloat pitch_samps_;                   // normalized rate of change of pitch (bore delay)

    bool     noteTransition_;
    bool     current_latch_;

    bool     soundOn_;

#if CLARINET_TESTVALUE==1
    StkFloat testValue_;
#endif // TESTVALUE==1
};




#endif /* CLARINET2_H_ */








