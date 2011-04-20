/***************************************************/
/*! \class ReedTable
    \brief STK reed table class.

    This class implements a simple one breakpoint,
    non-linear reed function, as described by
    Smith (1986).  This function is based on a
    memoryless non-linear spring model of the reed
    (the reed mass is ignored) which saturates when
    the reed collides with the mouthpiece facing.

    See McIntyre, Schumacher, & Woodhouse (1983),
    Smith (1986), Hirschman, Cook, Scavone, and
    others for more information.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2007.
*/
/***************************************************/

#include "ReedTable.h"
#include <math.h>

#define SOFTCLIP 0

const StkFloat ReedTable :: table_[] = {

        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 0.999992,
        0.999167, 0.996992, 0.993467, 0.988592, 0.982367, 0.974792, 0.965867, 0.955592, 0.943967, 0.930992,
        0.916667, 0.900992, 0.883967, 0.865592, 0.845867, 0.824792, 0.802367, 0.778592, 0.753467, 0.726992,
        0.699167, 0.669992, 0.640000, 0.610000, 0.580000, 0.550000, 0.520000, 0.490000, 0.460000, 0.430000,
        0.400000, 0.370000, 0.340000, 0.310000, 0.280000, 0.250000, 0.220000, 0.190000, 0.160000, 0.130000,
        0.100000, 0.070000, 0.040000, 0.010000, -0.020000, -0.050000, -0.080000, -0.110000, -0.140000, -0.170000,
        -0.200000, -0.230000, -0.260000, -0.290000, -0.320000, -0.350000, -0.380000, -0.410000, -0.440000, -0.470000,
        -0.500000, -0.530000, -0.560000, -0.590000, -0.620000, -0.650000, -0.679867, -0.708592, -0.735967, -0.761992,
        -0.786667, -0.809992, -0.831967, -0.852592, -0.871867, -0.889792, -0.906367, -0.921592, -0.935467, -0.947992,
        -0.959167, -0.968992, -0.977467, -0.984592, -0.990367, -0.994792, -0.997867, -0.999592, -1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000, -1.000000
        };


ReedTable :: ReedTable()
{
  offset_ = (StkFloat) 0.6;  // Offset is a bias, related to reed rest position.
  slope_ = (StkFloat) -0.8;  // Slope corresponds loosely to reed stiffness.
}

ReedTable :: ~ReedTable()
{
}

void ReedTable :: setOffset(StkFloat offset)
{
  offset_ = offset;
}

void ReedTable :: setSlope(StkFloat slope)
{
  slope_ = slope;
}

StkFloat ReedTable :: computeSample(StkFloat input)
{

#if SOFTCLIP==1
    if(input<-12)
        lastOutput_ = 1;
    else
        if(input>12-0.1)
            lastOutput_ = -1;
        else
        {

            StkFloat idx = (2*input)*10+120;
            StkFloat int_idx = floor(idx);
            StkFloat frac_idx = idx - int_idx;
            lastOutput_ = (1-frac_idx)*table_[(unsigned)int_idx] + frac_idx*table_[(unsigned)int_idx+1];
        }
#else
  // The input is differential pressure across the reed.
  lastOutput_ = offset_ + (slope_ * input);

  // If output is > 1, the reed has slammed shut and the
  // reflection function value saturates at 1.0.
  if (lastOutput_ > 1.0) lastOutput_ = (StkFloat) 1.0;

  // This is nearly impossible in a physical system, but
  // a reflection function value of -1.0 corresponds to
  // an open end (and no discontinuity in bore profile).
  if (lastOutput_ < -1.0) lastOutput_ = (StkFloat) -1.0;
#endif // SOFTCLIP

  return lastOutput_;


}

