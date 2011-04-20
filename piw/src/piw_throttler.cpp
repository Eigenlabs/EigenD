
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
    struct throttling_converter_t: piw::converter_t
    {
        throttling_converter_t(unsigned long long interval): pending_(false), interval_(1000*interval), timer_(0), idle_(IDLE_TICKS)
        {
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q, unsigned long long t)
        {
            iqueue_ = q;
            index_ = 0;
            idle_ = IDLE_TICKS;
            oqueue_ = piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_TINY);
            piw::data_nb_t d;

            if(iqueue_.latest(d,&index_,t?t:piw::tsd_time()))
            {
                index_++;
                do_throt(d);
            }

            return oqueue_;
        }

        int ticked(unsigned long long from, unsigned long long to, unsigned long sr, unsigned bs)
        {
            if(idle_==0)
            {
                idle_=IDLE_TICKS;
                flush();
                return TICK_SUPPRESS;
            }

            --idle_;

            piw::data_nb_t d;
            while(iqueue_.read(d,&index_,to))
            {
                do_throt(d);
                ++index_;
                idle_=IDLE_TICKS;
            }

            return TICK_ENABLE;
        }

        void do_throt(const piw::data_nb_t &d)
        {
            if(d.time()>timer_)
            {
                pending_=false;
                timer_ = d.time()+interval_;
                oqueue_.write_fast(d);
            }
            else
            {
                value_=d;
                pending_=true;
            }
        }

        void flush()
        {
            if(pending_)
            {
                pic::logmsg() << "flushing " << value_;
                oqueue_.write_fast(value_);
                pending_=false;
                timer_=0;
            }
        }

        piw::data_nb_t value_;
        bool pending_;
        unsigned long long interval_;
        unsigned long long timer_;
        piw::dataqueue_t iqueue_,oqueue_;
        unsigned long long index_;
        unsigned idle_;
    };
};

piw::converter_ref_t piw::throttling_converter(unsigned long long interval)
{
    return pic::ref(new throttling_converter_t(interval));
}
