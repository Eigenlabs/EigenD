/***************************************************/
/*! \class Clarinet
    \brief STK clarinet physical model class.

    This class implements a simple clarinet
    physical model, as discussed by Smith (1986),
    McIntyre, Schumacher, Woodhouse (1983), and
    others.

    This is a digital waveguide model, making its
    use possibly subject to patents held by Stanford
    University, Yamaha, and others.

    Control Change Numbers:
       - Reed Stiffness = 2
       - Noise Gain = 4
       - Vibrato Frequency = 11
       - Vibrato Gain = 1
       - Breath Pressure = 128

    by Perry R. Cook and Gary P. Scavone, 1995 - 2007.
*/
/***************************************************/

#include "Clarinet.h"
#include "SKINI.msg"
#include <math.h>
#include <picross/pic_log.h>

Clarinet :: Clarinet(StkFloat lowestFrequency)
{
  length_ = (long) (Stk::sampleRate() / lowestFrequency + 1);
  delayLine_.setMaximumDelay( length_ );
  delayLine_.setDelay( length_ / 2.0 );
  reedTable_.setOffset((StkFloat) 0.7);
  reedTable_.setSlope((StkFloat) -0.3);

  vibrato_.setFrequency((StkFloat) 5.735);
  outputGain_ = (StkFloat) 2.0;
  noiseGain_ = (StkFloat) 0.1;
  vibratoGain_ = (StkFloat) 0.0;

  g_ = (1.0-exp(-2.0*3.141*(10/Stk::sampleRate())));
  y_1_ = 0;

  frequency_ = 220;

  setFilterCoeffs(Stk::sampleRate());

  Stk::addSampleRateAlert( this );
}

Clarinet :: ~Clarinet()
{
}

void Clarinet :: clear()
{
  delayLine_.clear();
  filter_.tick( 0.0 );
}

void Clarinet :: setFilterCoeffs( StkFloat rate )
{
    pic::lckvector_t<StkFloat>::lcktype b;
    pic::lckvector_t<StkFloat>::lcktype a;

    switch((unsigned)rate)
    {
        // Currently the same coefficients are used for
        // 44.1kHz and 48kHz (and 88.2kHz and 96kHz)
        // which will give a slightly brighter tone for 48kHz/96kHz.
        // This is because the STK uses a simple LP FIR to
        // avoid non-linear phase delays in the waveguides,
        // which would give tuning errors without delay compensation.
        // The filters have a cutoff at pi/2. Changing the cutoff
        // with this filter structure would give a non-linear phase delay.
        // For the 88.2kHz and 96kHz case, an equivalent linear phase
        // filter is used with a cutoff at pi/4.
        case 44100:
        case 48000:
            b.assign(2, 0.5);
            a.assign(1, 1.0);
            filter_.setCoefficients( b, a );
            phase_delay_ = 0.5;
            break;
        case 88200:
        case 96000:
            b.assign(4, 0.25);
            a.assign(1, 1.0);
            filter_.setCoefficients( b, a );
            phase_delay_ = 1.5;
            break;
        default:
            b.assign(2, 0.5);
            a.assign(1, 1.0);
            filter_.setCoefficients( b, a );
            phase_delay_ = 0.5;
    }
}

void Clarinet :: sampleRateChanged( StkFloat newRate, StkFloat oldRate )
{
    // redesign breath filter
    g_ = (1.0-exp(-2.0*3.141*(10/newRate)));

    // redesign reflectance filter
    setFilterCoeffs(newRate);

    // set the current note frequency so that currently playing note will
    // keep the correct pitch
    setFrequency(frequency_);

}

void Clarinet :: setFrequency(StkFloat frequency)
{
  StkFloat freakency = frequency;
  if ( frequency <= 0.0 ) {
    errorString_ << "Clarinet::setFrequency: parameter is less than or equal to zero!";
    handleError( StkError::WARNING );
    freakency = 220.0;
  }

  frequency_ = freakency;

  // Delay = length - approximate filter delay.
//  StkFloat delay = (Stk::sampleRate() / freakency) * 0.5 - 1.5;
  StkFloat delay = (Stk::sampleRate() / freakency) * 0.5 - (phase_delay_+1);
  if (delay <= 0.0) delay = 0.3;
  else if (delay > length_) delay = length_;
  delayLine_.setDelay(delay);
}

void Clarinet :: startBlowing(StkFloat amplitude)
{
  envelope_.setRate(0.005);
  envelope_.setTarget(amplitude);
}

void Clarinet :: stopBlowing()
{
  envelope_.setRate(0.005);
  envelope_.setTarget((StkFloat) 0.0);
}

void Clarinet :: noteOn(StkFloat frequency, StkFloat amplitude)
{
//  this->setFrequency(frequency);
  this->startBlowing(amplitude);

#if defined(_STK_DEBUG_)
  errorString_ << "Clarinet::NoteOn: frequency = " << frequency << ", amplitude = " << amplitude << '.';
  handleError( StkError::DEBUG_WARNING );
#endif
}

void Clarinet :: noteOff(StkFloat amplitude)
{
  this->stopBlowing();

#if defined(_STK_DEBUG_)
  errorString_ << "Clarinet::NoteOff: amplitude = " << amplitude << '.';
  handleError( StkError::DEBUG_WARNING );
#endif
}

StkFloat Clarinet :: computeSample()
{
  StkFloat pressureDiff;
  StkFloat breathPressure;

  // Calculate the breath pressure (envelope + noise + vibrato)
  breathPressure = envelope_.tick();

  breathPressure = y_1_ + g_*(breathPressure-y_1_);
  y_1_ = breathPressure;

  breathPressure += breathPressure * noiseGain_ * noise_.tick();
  breathPressure += breathPressure * 0.01 * noise_.tick();
  breathPressure += breathPressure * vibratoGain_ * vibrato_.tick();

  // Perform commuted loss filtering.
  pressureDiff = -0.95 * filter_.tick(delayLine_.lastOut());

  // Calculate pressure difference of reflected and mouthpiece pressures.
  pressureDiff = pressureDiff - breathPressure;

  // Perform non-linear scattering using pressure difference in reed function.
  lastOutput_ = delayLine_.tick(breathPressure + pressureDiff * reedTable_.tick(pressureDiff));

  // Apply output gain.
  lastOutput_ *= outputGain_;

  return lastOutput_;
}

void Clarinet :: controlChange(int number, StkFloat value)
{
  StkFloat norm = value * ONE_OVER_128;
  if ( norm < 0 ) {
    norm = 0.0;
    errorString_ << "Clarinet::controlChange: control value less than zero ... setting to zero!";
    handleError( StkError::WARNING );
  }
  else if ( norm > 1.0 ) {
    norm = 1.0;
    errorString_ << "Clarinet::controlChange: control value greater than 128.0 ... setting to 128.0!";
    handleError( StkError::WARNING );
  }

  if (number == __SK_ReedStiffness_) // 2
    reedTable_.setSlope((StkFloat) -0.44 + ( (StkFloat) 0.26 * norm ));
  else if (number == __SK_NoiseLevel_) // 4
    noiseGain_ = (norm * (StkFloat) 0.4);
  else if (number == __SK_ModFrequency_) // 11
    vibrato_.setFrequency((norm * (StkFloat) 12.0));
  else if (number == __SK_ModWheel_) // 1
    vibratoGain_ = (norm * (StkFloat) 0.5);
  else if (number == __SK_AfterTouch_Cont_) // 128
    envelope_.setValue(norm);
  else {
    errorString_ << "Clarinet::controlChange: undefined control number (" << number << ")!";
    handleError( StkError::WARNING );
  }

#if defined(_STK_DEBUG_)
    errorString_ << "Clarinet::controlChange: number = " << number << ", value = " << value << '.';
    handleError( StkError::DEBUG_WARNING );
#endif
}
