
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

#ifndef __SYNTH_LIMITER__
#define __SYNTH_LIMITER__

#include <picross/pic_nocopy.h>
#include <picross/pic_float.h>

#define ATTACK_COEFF 0.9998
#define RELEASE_COEFF 0.9998
#define AVERAGE_COEFF 0.998
#define THRESHOLD 6.f // dB
#define RATIO 4.f
#define RELEASE 5.f
#define DB2LOG(x) (x*(PIC_LN10/20.f))
#define LOG2DB(x) (x*(20.f/PIC_LN10))
#define ANTI_DENORMAL 1.0e-20

#include <plg_synth/src/synth_exports.h>

namespace synth
{
    struct PISYNTH_DECLSPEC_CLASS onepole_t
    {
        onepole_t(float c): coeff_(c)
        {
        }

        float process(float in, float state)
        {
            return in+coeff_*(state-in);
        }

        float coeff_;
    };

    struct PISYNTH_DECLSPEC_CLASS follower_t
    {
        follower_t() : attack_(ATTACK_COEFF), release_(RELEASE_COEFF), state_(ANTI_DENORMAL)
        {
        }

        float process(float in)
        {
            in += ANTI_DENORMAL;
            state_ = (in>state_) ? attack_.process(in,state_) : release_.process(in,state_);
            return state_-ANTI_DENORMAL;
        }

        onepole_t attack_;
        onepole_t release_;
        float state_;
    };

    struct PISYNTH_DECLSPEC_CLASS averager_t: onepole_t
    {
        averager_t() : onepole_t(AVERAGE_COEFF), state_(ANTI_DENORMAL)
        {
        }

        float process(float in)
        {
            in += ANTI_DENORMAL;
            float squared = in*in;
            state_ = onepole_t::process(squared,state_);
            return sqrtf(state_);
        }

        float state_;
    };

    class PISYNTH_DECLSPEC_CLASS limiter_t : public pic::nocopy_t
    {
        public:
            limiter_t(): release_(false),average_(0),gain_(1)
            {
            }

            float process(float in)
            {
                average_ = averager_.process(in);
                float avdb = LOG2DB(pic::approx::ln(average_));
                float headrm = std::max(0.f,avdb-THRESHOLD);
                if(release_)
                    headrm += RELEASE;
                float gaindb = follower_.process(headrm)*(1.f-RATIO);
                gain_ = std::max(0.f,pic::approx::exp(DB2LOG(gaindb)));
                return gain_*in;
            }

            float average()
            {
                return average_;
            }

            void release(bool b)
            {
                release_=b;
            }

            void dump()
            {
                pic::logmsg() << "rel:" << release_ << " av:" << average_ << " fol:" << follower_.state_ << " gn:" << gain_;
            }

        private:
            follower_t follower_;
            averager_t averager_;
            bool release_;
            float average_;
            float gain_;
    };
}

#endif
