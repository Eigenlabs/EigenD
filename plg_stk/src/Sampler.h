/***************************************************/
/*! \class Sampler
    \brief STK sampling synthesis abstract base class.

    This instrument provides an ADSR envelope, a one-pole filter, and
    structures for an arbitrary number of attack and loop waves.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2007.
*/
/***************************************************/

#ifndef STK_SAMPLER_H
#define STK_SAMPLER_H

#include "Instrmnt.h"
#include "ADSR.h"
#include "WaveLoop.h"
#include "OnePole.h"

class Sampler : public Instrmnt
{
 public:
  //! Default constructor.
  Sampler();

  //! Class destructor.
  virtual ~Sampler();

  //! Reset and clear all internal state.
  void clear();

  //! Set instrument parameters for a particular frequency.
  virtual void setFrequency(StkFloat frequency) = 0;

  //! Initiate the envelopes with a key-on event and reset the attack waves.
  void keyOn();

  //! Signal a key-off event to the envelopes.
  void keyOff();

  //! Stop a note with the given amplitude (speed of decay).
  virtual void noteOff(StkFloat amplitude);

  //! Perform the control change specified by \e number and \e value (0.0 - 128.0).
  virtual void controlChange(int number, StkFloat value) = 0;

 protected:

  virtual StkFloat computeSample( void ) = 0;

  ADSR adsr_;
  pic::lckvector_t<FileWvIn *>::lcktype attacks_;
  pic::lckvector_t<WaveLoop *>::lcktype loops_;
  OnePole filter_;
  StkFloat baseFrequency_;
  pic::lckvector_t<StkFloat>::lcktype attackRatios_;
  pic::lckvector_t<StkFloat>::lcktype loopRatios_;
  StkFloat attackGain_;
  StkFloat loopGain_;

};

#endif
