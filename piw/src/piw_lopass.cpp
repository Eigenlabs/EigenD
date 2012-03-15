
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

#include <piw/piw_policy.h>
#include <piw/piw_tsd.h>
#include <picross/pic_stl.h>

#define IDLE_TICKS 100

namespace
{
    struct lopass_converter_t: piw::converter_t
    {
        unsigned long long interval_;
        float coeff_;
        unsigned long long timer_;
        piw::dataholder_nb_t last_;
        piw::dataholder_nb_t current_;
        piw::dataqueue_t iqueue_,oqueue_;
        unsigned long long index_;
        unsigned idle_;
        pic::lcklist_t<piw::data_nb_t>::nbtype buffer_;

        lopass_converter_t(float f, float c): interval_((unsigned long long)(1000000.0/f)), coeff_(c), timer_(0), last_(piw::makenull(0)), current_(piw::makenull(0)), idle_(IDLE_TICKS)
        {
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q, unsigned long long t)
        {
            //pic::logmsg() << "convert start at " << t;
            iqueue_=q;
            timer_=t;
            flush();
            index_=0;
            idle_=IDLE_TICKS;
            oqueue_=piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM);
            buffer_.clear();
            piw::data_nb_t current = current_;
            if(iqueue_.latest(current,&index_,timer_))
            {
                do_filter();
                index_++;
            }
            return oqueue_;
        }

        void flush()
        {
            last_.set_nb(piw::makenull_nb(0));
            current_.set_nb(piw::makenull_nb(0));
            buffer_.clear();
        }

        void do_filter()
        {
            if(current_.get().is_null())
                return;

            if(last_.get().is_null())
            {
                last_.set_nb(current_.get().restamp(timer_));
            }
            else
            {
                float f = coeff_*last_.get().as_denorm_float() + (1.0-coeff_)*current_.get().as_denorm_float();
                last_.set_nb(piw::makefloat_bounded_units_nb(current_.get().units(),current_.get().as_array_ubound(), current_.get().as_array_lbound(), current_.get().as_array_rest(), f, timer_));
            }

            oqueue_.write_fast(last_);
        }

        int ticked(unsigned long long from,unsigned long long now, unsigned long sr, unsigned bs)
        {
            //pic::logmsg() << "lopass tick " << from << "-" << now << " idle " << idle_;
            if(idle_==0)
            {
                //pic::logmsg() << "suppressing";
                idle_=IDLE_TICKS;
                return TICK_SUPPRESS;
            }

            --idle_;

            if(timer_<=from)
            {
                //pic::logmsg() << "catching up";
                timer_=from+1;
                piw::data_nb_t current = current_;
                iqueue_.latest(current,&index_,timer_);
            }

            piw::data_nb_t d;
            while(iqueue_.read(d,&index_,now))
            {
                buffer_.push_back(d);
                index_++;
                idle_=IDLE_TICKS;
            }

            while(timer_<now)
            {
                while(!buffer_.empty() && buffer_.front().time()<timer_)
                {
                    current_.set_nb(buffer_.front());
                    buffer_.pop_front();
                }
                do_filter();
                timer_+=interval_;
            }

            return TICK_ENABLE;
        }
    };
}

piw::converter_ref_t piw::lopass_converter(float f, float c)
{
    return pic::ref(new lopass_converter_t(f,c));
}

