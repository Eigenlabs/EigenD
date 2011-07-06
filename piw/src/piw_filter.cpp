
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

#include <piw/piw_tsd.h>
#include <picross/pic_log.h>
#include <piw/piw_policy.h>

#define IDLE_TICKS 100

namespace
{
    struct filtering_converter_t: piw::converter_t
    {
        filtering_converter_t(const piw::d2b_nb_t filter): init_(false), idle_(IDLE_TICKS), filter_(filter)
        {
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q, unsigned long long t)
        {
            iqueue_ = q;
            index_ = 0;
            init_ = false;
            idle_ = IDLE_TICKS;
            oqueue_ = piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_TINY);
            piw::data_nb_t d;

            if(iqueue_.latest(d,&index_,t))
            {
                index_++;
                do_filter(d);
            }

            return oqueue_;
        }

        int ticked(unsigned long long from,unsigned long long to, unsigned long sr, unsigned bs)
        {
            if(idle_==0)
            {
                idle_=IDLE_TICKS;
                return TICK_SUPPRESS;
            }

            --idle_;

            piw::data_nb_t d;
            while(iqueue_.read(d,&index_,to))
            {
                do_filter(d);
                ++index_;
                idle_=IDLE_TICKS;
            }

            return TICK_ENABLE;
        }

        void do_filter(const piw::data_nb_t &d)
        {
            bool pass = filter_(d);
            if(pass)
            {
                oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,1,d.time()));
            }
        }

        bool init_;
        piw::dataqueue_t iqueue_, oqueue_;
        unsigned long long index_;
        unsigned idle_;
        const piw::d2b_nb_t filter_;
    };
};

piw::converter_ref_t piw::filtering_converter(const piw::d2b_nb_t &f)
{
        return pic::ref(new filtering_converter_t(f));
}
