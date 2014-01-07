/*
 * Clarinet2.cpp
 *
 *  Clarinet model based on the Csound Clarinet implementation by Josep M Comajuncosas / dec '98-feb '99
 *  of the JNMR article (Vol.24 pp.148-163) written by Gijs de Bruin & Maarten van Walstijm.
 *  This implementation uses some of the CCRMA STK framework written by by Perry R. Cook and Gary P. Scavone, 1995 - 2007
 */

#include <picross/pic_config.h>
#include <picross/pic_float.h>

#include "Clarinet2.h"
#include "SKINI.msg"
#include <math.h>
#include <picross/pic_log.h>
#include <picross/pic_float.h>

// no debugging
//#define CLARINET_DEBUG 1
// debugging signals
//#define CLARINET_DEBUG 1
// debugging delay lines
//#define CLARINET_DEBUG 2
#if CLARINET_DEBUG==1
unsigned count = 0;
#endif // CLARINET_DEBUG==1

// TODO:
// 1. 88.2kHz & 96kHz support:
//      1) try all-pass interpolator (linear interpolation creates gain distortion)
//      2) redesign refl filter with lower cutoff (map freq resp to slightly higher Fs than 96kHz)
//      3) change the fading steps/times to be uniform at all frequencies
//      4) calculate fading constants from the sample rate

/* Reflectance Filter Phase Delay Tables
 *
 * Store the phase delay (DC to 2 kHz) computed from
 * the phase response of the 2nd order transmitance filter
 * for different sampling frequencies
 */
// Fs=44.1kHz
const StkFloat Clarinet2 :: refl_filt_coeffs_44_[] =
{-0.06734822596636, 1, 1.60866845795955, 0.79033697044981, -0.00843592161383, -0.75860644369670};
const StkFloat Clarinet2 :: tran_filt_coeffs_44_[] =
{-0.834797947580800, 1, -1.996950273367395, 0.996978312332094, -1.706660482696107, 0.707131817477271};
const StkFloat Clarinet2 :: refl_filt_delay_44_[] =
{
    7.487356, 7.476225, 7.443184, 7.389260,
    7.316061, 7.225639, 7.120338, 7.002632,
    6.874992, 6.739772, 6.599131, 6.454992,
    6.309015, 6.162600, 6.016902, 5.872849,
    5.731172, 5.592429, 5.457036, 5.325287,
    5.197377, 5.073422, 4.953476, 4.837541,
    4.725582, 4.617533, 4.513308, 4.412803,
    4.315905, 4.222492, 4.132439, 4.045618
};

// Fs=48kHz
const StkFloat Clarinet2 :: refl_filt_coeffs_48_[] =
{-0.068578, 0.971461, 1.381875, 0.775549, -0.023429, -0.761997};
const StkFloat Clarinet2 :: tran_filt_coeffs_48_[] =
{-0.855676, 1.006699, -0.048772, -0.954681, 0.207889, -0.682685};
const StkFloat Clarinet2 :: refl_filt_delay_48_[] =
{
    8.149023, 8.136914, 8.100971, 8.042311,
    7.962681, 7.864315, 7.749760, 7.621709,
    7.482847, 7.335735, 7.182723, 7.025902,
    6.867078, 6.707774, 6.549247, 6.392508,
    6.238351, 6.087385, 5.940061, 5.796700,
    5.657515, 5.522632, 5.392109, 5.265950,
    5.144116, 5.026537, 4.913117, 4.803745,
    4.698296, 4.596640, 4.498640, 4.404156
};

// Fs=88.2kHz
const StkFloat Clarinet2 :: refl_filt_coeffs_88_[] =
{-0.068858, 1.056762, -0.334089, 0.880915, -0.256545, -0.633035};
const StkFloat Clarinet2 :: tran_filt_coeffs_88_[] =
{-0.854894, 1.000282, -0.006795, -0.991790, -0.006052, -0.703029};
const StkFloat Clarinet2 :: refl_filt_delay_88_[] =
{
    14.679654, 14.658770, 14.596752, 14.495450,
    14.357766, 14.187421, 13.988681, 13.766078,
    13.524162, 13.267303, 12.999539, 12.724492,
    12.445318, 12.164703, 11.884882, 11.607675,
    11.334527, 11.066563, 10.804632, 10.549349,
    10.301142, 10.060278, 9.826902, 9.601058,
    9.382712, 9.171768, 8.968088, 8.771495,
    8.581791, 8.398761, 8.222177, 8.051807
};

// Fs=96kHz
const StkFloat Clarinet2 :: refl_filt_coeffs_96_[] =
{-0.068918, 1.091212, -0.562248, 0.916703, -0.301445, -0.598922};
const StkFloat Clarinet2 :: tran_filt_coeffs_96_[] =
{-0.854896, 0.994176, -0.007765, -0.984857, -0.042281, -0.691886};
const StkFloat Clarinet2 :: refl_filt_delay_96_[] =
{
    15.927466, 15.904965, 15.838140, 15.728971,
    15.580566, 15.396913, 15.182584, 14.942445,
    14.681383, 14.404098, 14.114938, 13.817807,
    13.516111, 13.212756, 12.910161, 12.610297,
    12.314737, 12.024705, 11.741128, 11.464679,
    11.195829, 10.934877, 10.681984, 10.437207,
    10.200513, 9.971805, 9.750937, 9.537722,
    9.331951, 9.133392, 8.941802, 8.756934
};



Clarinet2 :: Clarinet2(StkFloat lowestFrequency) : lowestFrequency_(lowestFrequency)
{
    pic::logmsg() << "sample rate = " << Stk::sampleRate();

    length_ = (long) (Stk::sampleRate() / lowestFrequency + 1);
    delayLine_.setMaximumDelay( length_ );
    delayLine_.setDelay( length_ / 2.0 );

    vibrato_.setFrequency((StkFloat) 5.735);
    outputGain_ = (StkFloat) 1.0;
    noiseGain_ = (StkFloat) 0.0015;    // 0.2
    vibratoGain_ = (StkFloat) 0.1;

    ipi =  3.14159265359;
    // ixtiny = 0.000000004, small value

    // default clarinet values
    ifr = 2500;             // reed resonance frequency
    iwr = 2*ipi*ifr;        // normalized in radians/cycle
    ier = 44.4;             //  = B^(-3/2) where B = airflow constant ~ 0.08
    ihr = 0.0004;           // aperture of the reed in equilibrium
    iir = 731;              // inertance of the air above the reed
    iq  = 1.5;              // set q=2 for lip & double reed????
    ist = 0.000177;
    isr = 0.000146;
    iqr = 5.0;
    igr = iwr/iqr;
    iur = 0.0231;

    ic  = 343;              // sound velocity
    irho = 1.21;            // air density
    iz0 = (irho*ic)/ist;

    // make iv=1.5 here to add more harmonics to the sound
    iv=1.5;                   // iv = -1 (lip) or 1 (cane).
    // determines how quickly the oscillations build up

    // initialize variables
    sr = Stk::sampleRate(); // default sample rate
    it = 1/sr;

    kx1 = ihr;              // set reed to equilibrium
    kx2 = ihr;

    zr1_ = zr2_ = 0;        // initialize biquads
    zt1_ = zt2_ = 0;

    apm1 = 0;               // initial mouthpiece pressure
    apm2 = 0;
    kuf1 = 0;               // initial normal volume flow
    kuf2 = 0;

    apr2 = 0;               // initial reflected pressure wave from tube
    kp0 = 0;                // initial flow

    breathPressure_y_1_ = 0;
    breathPressure_g_ = (1.0-exp(-2.0*ipi*(100/sr)));
    noise_y_1_ = 0;
    noise_g_ = (1.0-exp(-2.0*ipi*(1000/sr)));
    power_ = 0;
    power_y_1_ = 0;
    power_g_ = (1.0-exp(-2.0*ipi*(500/sr)));

    breathScaling_ = 0;

    aout_x1_ = 0;
    aout_y1_ = 0;

    newNote_ = true;
    fadeGain_ = 0;
    fadeStep_ = 0;
    noteTransition_ = false;
    fading_ = false;

    delayLine_.clear();
    kdltn1 = 0;
    kdltf1 = 0;

    // initial current tube pitch around 440Hz
    delay_current_ = ((sr*(1.0/440.0))/2.0);
    frequency_current_ = 440;
    delay_target_ = delay_current_;
    frequency_target_ = frequency_current_;
    delay_step_ = 1;
    frequency_step_ = 1;
    setPitchSweepTime(10);
    current_latch_ = false;

    soundOn_ = true;

    // TODO: add this in when 96kHz supported
    //Stk::addSampleRateAlert( this );

    // make sure we initialize table pointers
    rcoeffs_ = refl_filt_coeffs_44_;
    tcoeffs_ = tran_filt_coeffs_44_;
    rdelay_ = refl_filt_delay_44_;

    sampleRateChanged(Stk::sampleRate(), 0);


}

Clarinet2 :: ~Clarinet2()
{
}

void Clarinet2 :: sampleRateChanged( StkFloat newRate, StkFloat oldRate )
{
    sr = newRate;
    it = 1/sr;

    // resize delay line
    length_ = (long) (sr / lowestFrequency_ + 1);
    delayLine_.setMaximumDelay( length_ );
    delayLine_.setDelay( length_ / 2.0 );

    // redesign filters
    breathPressure_g_ = (1.0-exp(-2.0*ipi*(100/sr)));
    noise_g_ = (1.0-exp(-2.0*ipi*(1000/sr)));
    power_g_ = (1.0-exp(-2.0*ipi*(500/sr)));


    // switch reflection and transmission biquads coeffs
    // and phase delay compesation
    switch((unsigned)newRate)
    {
    case 44100:
        rcoeffs_ = refl_filt_coeffs_44_;
        tcoeffs_ = tran_filt_coeffs_44_;
        rdelay_ = refl_filt_delay_44_;
        break;
    case 48000:
        rcoeffs_ = refl_filt_coeffs_48_;
        tcoeffs_ = tran_filt_coeffs_48_;
        rdelay_ = refl_filt_delay_48_;
        break;
    case 88200:
        rcoeffs_ = refl_filt_coeffs_88_;
        tcoeffs_ = tran_filt_coeffs_88_;
        rdelay_ = refl_filt_delay_88_;
        break;
    case 96000:
        rcoeffs_ = refl_filt_coeffs_96_;
        tcoeffs_ = tran_filt_coeffs_96_;
        rdelay_ = refl_filt_delay_96_;
        break;
    default:
        rcoeffs_ = refl_filt_coeffs_44_;
        tcoeffs_ = tran_filt_coeffs_44_;
        rdelay_ = refl_filt_delay_44_;
        errorString_ << "Clarinet2::sampleRateChanged: unsupported sample rate for clarinet";
        handleError( StkError::WARNING );
    }


}

void Clarinet2 :: setFrequency(StkFloat frequency)
{
    frequency_target_ = frequency;
    if ( frequency <= 0.0 )
    {
        errorString_ << "Clarinet2::setFrequency: parameter is less than or equal to zero!";
        handleError( StkError::WARNING );
        frequency_target_ = 220.0;
    }

    // limit top frequency playable by model
    if(frequency_target_ > 1567.98)
    {
        frequency_target_ = 1567.98;
        // do not play a new note that is out of range!
        if(newNote_ || noteTransition_)
        {
            // fade to zero and latch gain at zero
            soundOn_ = false;
            fading_ = true;
            fadeStep_ = -0.001;
        }
    }
    else
    {
        // limit bottom frequency playable by model
        if(frequency_target_ < lowestFrequency_)
        {
            frequency_target_ = lowestFrequency_;
            // do not play a new note that is out of range!
            if(newNote_ || noteTransition_)
            {
                // fade to zero and latch gain at zero
                soundOn_ = false;
                fading_ = true;
                fadeStep_ = -0.001;
            }
        }
        else
        {
            soundOn_ = true;
        }
    }

    // ------- calculate delay time -------
    // delay = bore length - approximate filter delay
    StkFloat kbore1;                                    // bore frequency

    kbore1 = frequency_target_;

//    pic::printmsg() << frequency_;

    // kdlt1 = -10/sr+1/kbore1                          // compensate for the Ur term
    StkFloat kdlt1 = 1/kbore1;                          // bore length

    // compute the phase delay
    // caused by the open hole reflectance filter
    // this delay will be substracted from the total waveguide length
    // notice that the table storing the phase delay
    // only works for frequencies up to 2kHz !
    StkFloat kphd1;                                     // phase delay
    // linear interpolate the table
    StkFloat dly_idx = 32*kbore1/(2000-62.5);
    if(dly_idx>=31)
    {
        kphd1 = rdelay_[31];
    }
    else
    {
        StkFloat int_dly_idx = floor(dly_idx);
        StkFloat frac_dly_idx = dly_idx - int_dly_idx;
        kphd1 = (1-frac_dly_idx)*rdelay_[(unsigned)int_dly_idx] + frac_dly_idx*rdelay_[(unsigned)int_dly_idx+1];
    }

    // there is an extra frequency dependent delay, presumably caused by the reed motion (?)
    // its empirically found function is added to the previous delay
//    kphd1 += testValue_;
    switch((unsigned)sr)
    {
    case 44100:
        kphd1 += (0.0015289*frequency_target_ + 1.9243478);
        break;
    case 48000:
        kphd1 += (0.0017265*frequency_target_ + 1.8178261);
        break;
    case 96000:
        kphd1 += (-7.9757e-07*frequency_target_*frequency_target_ + 3.9840e-03*frequency_target_ + 1.7417e+00);
        break;
    }

    delay_target_ = ((sr*kdlt1)/2) - kphd1;

    if (delay_target_ <= 0.0)
    {
        delay_target_ = 0.3;
        pic::logmsg() << "delay target less than zero!";
    }
    else
        if (delay_target_ > length_)
        {
            delay_target_ = length_;
            pic::logmsg() << "delay target greater than max length!";
        }

    if(noteTransition_)
    {
        //        pic::logmsg() << "curr="<<delay_current_<<" targ="<<delay_target_<<" newNote="<<newNote_<<" noteTransition="<<noteTransition_;
        // transitioning to a new note, sweep according to the pitch sweep time
        delay_step_ = pow(2, (pic_log2(delay_target_/delay_current_)/pitch_samps_));
        frequency_step_ = pow(2, (pic_log2(frequency_target_/frequency_current_)/pitch_samps_));
        // latch the freq change to prevent pitch bend from adjusting freq
        current_latch_ = true;
    }
    else
        if(newNote_)
        {
            // new note so change frequency quickly
            delay_step_ = pow(2, (pic_log2(delay_target_/delay_current_)/512));
            frequency_step_ = pow(2, (pic_log2(frequency_target_/frequency_current_)/512));

            // latch the freq change to prevent pitch bend from adjusting freq
            current_latch_ = true;

            newNote_ = false;
        }
        else
        {
            // bending note to a new frequency, sweep to interpolate bending
            if(!current_latch_)
            {
                delay_step_ = pow(2, (pic_log2(delay_target_/delay_current_)/100));
                frequency_step_ = pow(2, (pic_log2(frequency_target_/frequency_current_)/100));
    //            pic::logmsg() << "delay step: " << delay_step_ << " t=" << delay_target_ << " c=" << delay_current_ <<
    //            " l=" << pic_log2(delay_target_/delay_current_);
            }
        }

    // protect against errors with the delay time calculation
    if(pic::isnan(delay_step_))
    {
        pic::logmsg() << "clarinet delay time change failed";
        delay_step_ = 1;
        delay_target_ = length_/2;
        delay_current_ = length_/2;
    }

    if(pic::isnan(frequency_step_))
    {
        pic::logmsg() << "clarinet frequency change failed";
        frequency_step_ = 1;
        frequency_target_ = sr/(length_/2);
        frequency_current_ = sr/(length_/2);
    }


}

void Clarinet2 :: setPressure(StkFloat amplitude)
{

    // detect breath pressure about threshold
    if(amplitude>0.1)
    {
        // ensure fading is active when blowing
        // to make sure new breaths can restart the fade up
        fading_ = true;
    }

    envelope_.setRate(0.1);
    envelope_.setTarget(amplitude);

    last_amplitude_ = amplitude;

}

void Clarinet2 :: clear()
{
    last_amplitude_ = 0;
}

void Clarinet2 :: noteOn(StkFloat frequency, StkFloat amplitude)
{
#if CLARINET_DEBUG==1
    pic::logmsg() << "noteOn(): newNote=true";
#endif // CLARINET_DEBUG==1

    // start of a note, so frequency will be swept to new frequency, not pitch bend
    newNote_ = true;

    fadeStep_ = 0.0005;
    fading_ = true;

#if defined(_STK_DEBUG_)
    errorString_ << "Clarinet2::NoteOn: frequency = " << frequency << ", amplitude = " << amplitude << '.';
    handleError( StkError::DEBUG_WARNING );
#endif
}

void Clarinet2 :: noteOff(StkFloat amplitude)
{
#if CLARINET_DEBUG==1
    pic::logmsg() << "noteOff()";
#endif // CLARINET_DEBUG==1

    envelope_.setRate(0.0005);
    envelope_.setTarget((StkFloat) 0.0);


#if defined(_STK_DEBUG_)
    errorString_ << "Clarinet2::NoteOff: amplitude = " << amplitude << '.';
    handleError( StkError::DEBUG_WARNING );
#endif
}

void Clarinet2 :: noteTransition()
{
#if CLARINET_DEBUG==1
    pic::logmsg() << "noteTransition(): newNote=true";
#endif // CLARINET_DEBUG==1

    // note playing, and transitioning to another note so
    // fade down, then up, over 2000 samples
    // when zero, the new frequency will be applied
    fading_ = true;
    fadeStep_ = -0.001;
    newNote_ = false;
    noteTransition_ = true;

}

void Clarinet2 :: controlChange(int number, StkFloat value)
{
    StkFloat norm = value * ONE_OVER_128;
    if ( norm < 0 ) {
        norm = 0.0;
        errorString_ << "Clarinet2::controlChange: control value less than zero ... setting to zero!";
        handleError( StkError::WARNING );
    }
    else if ( norm > 1.0 ) {
        norm = 1.0;
        errorString_ << "Clarinet2::controlChange: control value greater than 128.0 ... setting to 128.0!";
        handleError( StkError::WARNING );
    }

    if (number == __SK_ReedStiffness_) // 2
    {
        errorString_ << "Clarinet2::controlChange: reed stiffness not implemented";
//        handleError( StkError::WARNING );
    }
    else if (number == __SK_NoiseLevel_) // 4
    {
        noiseGain_ = (norm * (StkFloat) 0.1);
//        pic::printmsg() << noiseGain_;
    }

    else if (number == __SK_ModFrequency_) // 11
        vibrato_.setFrequency((norm * (StkFloat) 12.0));
    else if (number == __SK_ModWheel_) // 1
        vibratoGain_ = (norm * (StkFloat) 0.5);
    else if (number == __SK_AfterTouch_Cont_) // 128
        envelope_.setValue(norm);
    else
    {
        errorString_ << "Clarinet2::controlChange: undefined control number (" << number << ")!";
        handleError( StkError::WARNING );
    }

#if defined(_STK_DEBUG_)
    errorString_ << "Clarinet2::controlChange: number = " << number << ", value = " << value << '.';
    handleError( StkError::DEBUG_WARNING );
#endif
}

void Clarinet2 :: setPitchSweepTime(StkFloat pitch_time)
{
    // pitch sweep time in milliseconds
    pitch_samps_ = (pitch_time*sr)/1000;
    if(pitch_samps_==0)
        pitch_samps_=1;
}

void Clarinet2 :: setLowestFrequency(StkFloat lowest_frequency)
{
    if(lowestFrequency_!=lowest_frequency)
    {
        lowestFrequency_ = lowest_frequency;

        StkFloat sampleRate = Stk::sampleRate();
        length_ = (long) (sampleRate / lowestFrequency_ + 1);
        delayLine_.setMaximumDelay( length_ );
    }
}


/*
 * reedOsc() : the reed oscillator model
 *
 */
void Clarinet2 :: reedOsc(StkFloat dP, StkFloat Vr, StkFloat fr, StkFloat Qr, StkFloat Ur, StkFloat Hr)
{
    // dP = difference in pressure between flow and pressure at mouthpiece

    StkFloat iwr = 2*ipi*fr;        // normalized in radians/cycle
    StkFloat igr = iwr/Qr;

    StkFloat ic1 = 2-iwr*iwr*it*it - igr*it;
    StkFloat ic2 = igr*it - 1;
    StkFloat ic3 = -it*it*Vr/Ur;
    StkFloat ic4 = it*it*iwr*iwr;

    StkFloat kdp1 = dP;

    StkFloat kx0 = kx1;
    // prev reed aperture = curr reed aperture
    kx1 = kx2;
    // new reed aperture
    kx2 = ic1*kx1 + ic2*kx0 + ic3*kdp1 + ic4*Hr;

    //    pic::printmsg() << "ReedOsc: dP="<<dP<<" kx1="<<kx1<<" kx2="<<kx2;
}

/* reedClip(): Clipping Routine for clarinet reed beating
 *
 *  Models the reed resisting closing as it approaches closure,
 *  and prevents complete closure.
 *
 *  The routine uses a value kclipd that sets the distance bewteen
 *  g and kclipx1. So for high values of kclipd, the clipping is
 *  very 'soft'. and for kclipd=0 there is no softening at all,
 *  and the clipping is equivalent to the way we have it now.
 *
 *  The clipping routine has two control parameters:
 *  g = minimum value for x to clip at
 *  m = 'softness' of clipping: m corresponds to
 *  the half-width (kclipd) of the range in which the clipping
 *  does 'circular' waveshaping
 *  m is experessed in fraction(0 to 1) of the equilibrium opening ihr
 *  0 means no softening at all
 *  1 means high softening
 */
void Clarinet2 :: reedClip(StkFloat g, StkFloat m, StkFloat h)
{
    // clip kx2 - the current reed aperture

    // initialisation part of the clipping routine
    StkFloat kclipd = m*h;
    StkFloat kclipk = 1 - (double)sqrt((double)2);
    StkFloat kclipx1 = g - kclipd;
    StkFloat kclipa = kclipx1;
    StkFloat kclipr = (kclipa-g)/kclipk;
    StkFloat kclipb = kclipr + g;
    StkFloat kclipx2 = (kclipx1 + kclipb)/2;

    // the routine itself
    if (kx2 < kclipx1)
    {
        // full clipping
        kx2 = g;
    }
    else
    {
        if (kx2 > kclipx2)
        {
            // no clipping
        }
        else
        {
            // circular (soft) clipping, TODO: optimize
            kx2 = kclipb - (double)sqrt((double)(kclipr*kclipr - (kx2-kclipx1)*(kx2-kclipx1)));
        }
    }

}

/*
 *  bFlow() : Backus equation implementation
 *
 */
void Clarinet2 :: bFlow(StkFloat Z0, StkFloat Ir, StkFloat Er)
{

    //StkFloat ic6 = isr/it;              // it = sampling period
    StkFloat ic7 = -(Z0*it/Ir + 1);
    StkFloat ic8 = -it*Er/Ir;
    StkFloat ic9 = it/Ir;
    //kur2 = ic6*(kx2 - kx1) // must compensate for the phase delay induced by kur2!
    StkFloat kur2 = 0;                  // IN TUNE!!!!!!!

    // input is kx2 - current reed aperture
    StkFloat id1 = ic7;
    StkFloat kd2 = ic8/(kx2 * kx2);
    //    StkFloat kd3 = ic9*(kp0 - 2*kpr2 + Z0*kur2) + kuf1;   // not using k var: kpr2 (= apr2)
    StkFloat kd3 = ic9*(kp0 - 2*apr2 + Z0*kur2) + kuf1;

    // Newton-Rhapson solution

    // Uf(n) is used as initial value.

    StkFloat kd4 = kd2*iq;
    StkFloat kd5 = kd4-kd2;
    StkFloat kxo = kuf1;
    //    pic::printmsg() << kxo;

    StkFloat kbsaxo, ksign, ksign_axo, kbsaxoq, kbsaxoq1;

    // ------- Perform 3 Newton Rhapson iterations -------
    // iteration 1
    kbsaxo = fabs(kxo);
    ksign = kbsaxo;
    ksign_axo = (ksign<0?-1:1);
    // kbsaxoq1 = pow(kbsaxo,(iq-1)) where iq=1.5, -> kbsaxoq1 = sqrt(kbsaxo)
    kbsaxoq1 = sqrt(kbsaxo);
    kbsaxoq  = kbsaxo*kbsaxoq1;

    kxo=(kd5*ksign_axo*kbsaxoq-kd3)/(id1+kd4*kbsaxoq1);

    // iteration 2
    kbsaxo = fabs(kxo);
    ksign = kbsaxo;
    ksign_axo = (ksign<0?-1:1);
    // kbsaxoq1 = pow(kbsaxo,(iq-1)) where iq=1.5, -> kbsaxoq1 = sqrt(kbsaxo)
    kbsaxoq1 = sqrt(kbsaxo);
    kbsaxoq  = kbsaxo*kbsaxoq1;

    kxo=(kd5*ksign_axo*kbsaxoq-kd3)/(id1+kd4*kbsaxoq1);

    // iteration 3
    kbsaxo = fabs(kxo);
    ksign = kbsaxo;
    ksign_axo = (ksign<0?-1:1);
    // kbsaxoq1 = pow(kbsaxo,(iq-1)) where iq=1.5, -> kbsaxoq1 = sqrt(kbsaxo)
    kbsaxoq1 = sqrt(kbsaxo);
    kbsaxoq  = kbsaxo*kbsaxoq1;

    kxo=(kd5*ksign_axo*kbsaxoq-kd3)/(id1+kd4*kbsaxoq1);

    kuf2 = kxo;                     // get value from iteration

    StkFloat kub2 = kuf2 - kur2;    // total volume flow for next pass

    api2 = Z0*kub2 + apr2;          // ingoing pressure wave into the tube
    apm2 = api2 + apr2;             // mouthpiece pressure

    //    pic::printmsg() << "Bflow15: api2="<<api2<<" kxo="<<kxo<<" kx2="<<kx2<<" apr2="<<apr2;
}

/*
 * biquad() : biquad filter with a gain control
 *
 *                  B0 + B1*z1 + B2*z2
 *  H(z) = gain * --------------------
 *                  A0 + A1*z1 + A2*z2
 *  A0 = 1
 *
 *  Implemented as transposed direct form II for better speed and floating point precision!
 *
 */
void Clarinet2 :: biquad(StkFloat input, StkFloat &output, StkFloat gain,
        StkFloat b0, StkFloat b1, StkFloat b2,
        StkFloat a1, StkFloat a2, StkFloat &z1, StkFloat &z2)
{
    output = b0*input + z1;
    z1 = b1*input - a1*output + z2;
    z2 = b2*input - a2*output;
    output = output*gain;
}



/*
 * computeSample() : top level physical modelling function
 *
 *
 */

StkFloat Clarinet2 :: computeSample()
{

    // ------- Breath Model -------

    // calculate the breath pressure (envelope + noise + vibrato)
    breathPressure_ = envelope_.tick();

    // low-pass filter to reduce noise from the breath controller signal
    breathPressure_ = breathPressure_y_1_ + breathPressure_g_*(breathPressure_-breathPressure_y_1_);
    breathPressure_y_1_ = breathPressure_;

    // add automatic vibrato to the breath pressure
    //breathPressure_ += breathPressure_ * vibratoGain_ * vibrato_.tick();

    StkFloat expBreathPressure = pow(10, breathPressure_*2)/100;

    // add turbulence noise to the breath pressure
    StkFloat noise_x = 0, noise_y = 0, noiseRange = 0.5;
    // scale noise threshold with frequency, helps avoid the panpipes!
    if(frequency_current_<110)
        noiseRange = 0.5;
    else
        if(frequency_current_>880)
            noiseRange = 0.07;
        else
            noiseRange = -0.00045455*frequency_current_ + 0.55;

//    StkFloat noiseThreshold = 1-noiseRange;

    // apply noise above threshold = 1-(noise range)
//    if(breathPressure_>noiseThreshold)
//    {
//        breathScaling_ = breathPressure_-noiseThreshold;
//        breathScaling_ = (breathScaling_*breathScaling_)/(noiseRange*noiseRange);
//    }

    // shape noise with 1st-order low-pass filter Fc = 1000Hz
//    noise_x += breathScaling_ * noiseGain_ * noise_.tick();
    noise_x += expBreathPressure * noiseRange * noiseGain_ * noise_.tick();
//    noise_x += breathPressure_*breathPressure_ * noiseRange * noiseGain_ * noise_.tick();
    noise_y = noise_y_1_ + noise_g_*(noise_x-noise_y_1_);
    noise_y_1_ = noise_x;
//    breathPressure_ += noise_y;

    // calculate the maximum breath pressure that can drive the
    // model for a given frequency
    StkFloat breathMax = 850;
//    StkFloat breathMin = 500;
//    if(frequency_>=440)
//        breathMin = 0.33473*frequency_+352.71967+25;

    breathMax = 0.00023031*frequency_current_*frequency_current_-0.39300*frequency_current_+1000.3;

#if CLARINET_DEBUG==2
    if(pic::isnan(breathMax))
    {
        pic::logmsg() << "breathMax nan";
    }
#endif // CLARINET_DEBUG==2

    // map the breath controller input pressure to the model breath pressure
    //kp0 = breathMin + (breathMax - breathMin) * (breathPressure);
    if(breathPressure_>0.5)
        kp0 = breathMax * (((expBreathPressure*0.8) + 0.5) + noise_y);
    else
        if(breathPressure_>0.1)
            kp0 = breathMax * (((expBreathPressure*0.8) +
                    (-2*breathPressure_*breathPressure_ + 2*breathPressure_)) + noise_y);
        else
            kp0 = breathMax * ((expBreathPressure*0.8) +
                    (-2*breathPressure_*breathPressure_ + 2*breathPressure_));


#if CLARINET_DEBUG==1
    count++;
    if(count==32)
    {
        pic::printmsg() << "b=" << breathPressure_ << " k=" << kp0 << " fc_=" << frequency_current_ << " bMin=" << 0 << " bMax=" << breathMax << " noise=" << noiseGain_ << " power=" << power_;
//        pic::printmsg() << "d=" << delay_target_ << "  f=" << frequency_target_ << "  cl=" << current_latch_ << "  ds=" << delay_step_ << "  nt=" << noteTransition_ << "  nn=" << newNote_;
        count=0;
    }
#endif // CLARINET_DEBUG==1


    // ------- Reed Model -------

    // pressure difference across reed
    StkFloat adp1 = kp0 - apm1;

    reedOsc(adp1, iv, ifr, iqr, iur, ihr);
    if(breathPressure_>0.9)
        // make the reed clipping harder as the breath pressure tops out
        reedClip(0.00005, 0.4-((breathPressure_-0.9)*4), ihr);
    else
        reedClip(0.00005, 0.4, ihr);
    bFlow(iz0, iir, ier);


    // ------- Bore Model -------

    // interpolate delay to target value, unless note is transitioning
    if(delay_current_ != delay_target_ && !noteTransition_)
    {
        // sweep the delay time logarithmically using a scaling coefficient
        delay_current_ *= delay_step_;
        frequency_current_ *= frequency_step_;

        if(delay_step_>=1)
            if(delay_current_>=delay_target_)
            {
                delay_current_ = delay_target_;
                frequency_current_ = frequency_target_;
                delay_step_ = 1;
                current_latch_ = false;
            }

        if(delay_step_<1)
            if(delay_current_<=delay_target_)
            {
                delay_current_ = delay_target_;
                frequency_current_ = frequency_target_;
                delay_step_ = 1;
                current_latch_ = false;
            }

        if(delay_current_==delay_target_)
            current_latch_ = false;

        // calculations for fractional delay interpolation filter
        // time = int(time) + frac(time)
        kdltn1 = (unsigned long)floor( delay_current_ );   // find int(time)
        kdltf1 = ( delay_current_ ) - kdltn1;             // find frac(time)

#if CLARINET_DEBUG==2
        if((kdltn1+2)>(unsigned)length_)
        {
            // TODO: this shouldn't happen, why does it?
            pic::logmsg() << " delay too large";
        }
        else
            delayLine_.setDelay(kdltn1+2);
#else
        delayLine_.setDelay(kdltn1+2);
#endif // CLARINET_DEBUG==2

    }

    StkFloat api31, aprf1;

#if CLARINET_DEBUG==2
    StkFloat afdf12 = 0;
    if((kdltn1+2)>(unsigned)length_)
    {
        // TODO: this shouldn't happen, why does it?
        pic::logmsg() << "delay too large";
    }
    else
        // linear interpolate fractional delay, TODO: improve this, use an all-pass interpolator?
        afdf12 = (1-kdltf1)*delayLine_.contentsAt(kdltn1+1) + kdltf1*delayLine_.contentsAt(kdltn1+2);
#else
    StkFloat afdf12 = (1-kdltf1)*delayLine_.contentsAt(kdltn1+1) + kdltf1*delayLine_.contentsAt(kdltn1+2);
#endif // CLARINET_DEBUG==2

    api31 = afdf12;

    // open hole reflectance filter designed by Maarten (coeff. computed with MATLAB)
    biquad(afdf12, aprf1, rcoeffs_[0], rcoeffs_[1], rcoeffs_[2], rcoeffs_[3], rcoeffs_[4], rcoeffs_[5], zr1_, zr2_);

    // write ingoing pressure from reed model to delay line
    delayLine_.tick(api2);


    // signal reaching the bore closed end after filter
    StkFloat apr3 = aprf1;

    // signal leaving the bore open end after sample delay
    StkFloat api3 = api31;

    // update variables for next pass
    kuf1 = kuf2;            // normal volume flow
    apm1 = apm2;            // mouthpiece pressure
    apr2 = apr3;            // reflected pressure wave from the tube


    // ------- Bell Model -------
    StkFloat aout_x, aout_y;

    // transmission high-pass filter design by Maarten
    biquad(api3, aout_x, tcoeffs_[0], tcoeffs_[1], tcoeffs_[2], tcoeffs_[3], tcoeffs_[4], tcoeffs_[5], zt1_, zt2_);
//    aout_x = api3;

    // ------- Output Conditioning -------
    // D.C. blocking filter, remove nasty D.C. offset, due to non-zero reed equilibrium?
    aout_y = aout_x - aout_x1_ + 0.995 * aout_y1_;
    aout_x1_ = aout_x;
    aout_y1_ = aout_y;

    // calculate power in signal to know how to fade
    // if no power, then fade up attacks from 0
    power_ = aout_y*aout_y;
    power_ = power_y_1_ + power_g_*(power_-power_y_1_);
    power_y_1_ = power_;

    if(power_<1e-4)
    {
        // if the power is effectively 0
        // halt fading and fadeGain = 0 ready to fade up when power increases
        // slow fade up
        if(fadeGain_>0)
            fadeGain_ -= 0.01;
        if(fadeGain_<0)
            fadeGain_ = 0;
        fadeStep_ = 0.0005;
        noteTransition_ = false;
    }
    else
    {
        // de-glitch fading
        if(fading_)
        {
            if(!soundOn_)
            {
                // if sound off then hold fadeGain at zero
                fadeStep_ = -0.001;
            }
            fadeGain_+=fadeStep_;
            // if fading down and below zero then fade up
            if(fadeGain_<0)
            {
                fadeGain_ = 0;

                // make step +ve
                fadeStep_ = -fadeStep_;

                // when faded to zero, set frequency for transitional notes
                noteTransition_ = false;

            }
            if(fadeGain_>1)
            {
                // gain greater than 1 so have finished note transition
                fadeGain_=1;
                fading_=false;
            }
        }
    }

    // empirical scaling factor => /70
    return (aout_y/70)*fadeGain_;

}



