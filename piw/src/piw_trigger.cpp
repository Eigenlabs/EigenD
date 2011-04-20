
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
    struct triggering_converter_t: piw::converter_t
    {
        triggering_converter_t(): init_(false), active_(false), idle_(IDLE_TICKS)
        {
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q, unsigned long long t)
        {
            iqueue_ = q;
            index_ = 0;
            init_ = false;
            active_ = false;
            idle_ = IDLE_TICKS;
            oqueue_ = piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_TINY);
            piw::data_nb_t d;

            if(iqueue_.latest(d,&index_,t))
            {
                index_++;
                do_trig(d);
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
                do_trig(d);
                ++index_;
                idle_=IDLE_TICKS;
            }

            return TICK_ENABLE;
        }

        void do_trig(const piw::data_nb_t &d)
        {
            unsigned l = d.as_arraylen();
            const float *f = d.as_array();
            unsigned long long t = d.time();

            if(!init_)
            {
                init_=true;
                active_=false;

                if(l>0 && d.as_norm()!=0.0)
                {
                    active_=true;
                }

                if(active_)
                {
                    oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,1,t));
                }
                else
                {
                    oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,0,t));
                }

                return;
            }

            while(l>0)
            {
                if(active_)
                {
                    while(l>0 && *f!=0)
                    {
                        f++; l--;
                    }
                    if(l>0)
                    {
                        active_=false;
                        f++; l--;
                        oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,0,t));
                    }
                }
                else
                {
                    while(l>0 && *f==0)
                    {
                        f++; l--;
                    }
                    if(l>0)
                    {
                        active_=true;
                        f++; l--;
                        oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,1,t));
                    }
                }
            }
        }

        bool init_;
        bool active_;
        piw::dataqueue_t iqueue_, oqueue_;
        unsigned long long index_;
        unsigned idle_;
    };

    struct impulse_converter_t: piw::converter_t
    {
        impulse_converter_t(): init_(false), active_(false), idle_(IDLE_TICKS)
        {
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q, unsigned long long t)
        {
            iqueue_ = q;
            index_ = 0;
            init_ = false;
            active_ = false;
            idle_ = IDLE_TICKS;
            oqueue_ = piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_TINY);
            piw::data_nb_t d;

            if(iqueue_.latest(d,&index_,t))
            {
                index_++;
                do_imp(d);
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
                do_imp(d);
                ++index_;
                idle_=IDLE_TICKS;
            }

            return TICK_ENABLE;
        }

        void do_imp(const piw::data_nb_t &d)
        {
            unsigned l = d.as_arraylen();
            const float *f = d.as_array();
            unsigned long long t = d.time();

            if(!init_)
            {
                init_=true;
                active_=false;

                if(l>0 && d.as_norm()!=0.0)
                {
                    active_=true;
                }

                if(active_)
                {
                    oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,1,t));
                }

                return;
            }

            while(l>0)
            {
                if(active_)
                {
                    while(l>0 && *f!=0)
                    {
                        f++; l--;
                    }
                    if(l>0)
                    {
                        active_=false;
                        f++; l--;
                    }
                }
                else
                {
                    while(l>0 && *f==0)
                    {
                        f++; l--;
                    }
                    if(l>0)
                    {
                        active_=true;
                        f++; l--;
                        oqueue_.write_fast(piw::makefloat_bounded_nb(1,0,0,1,t));
                    }
                }
            }
        }

        bool init_;
        bool active_;
        piw::dataqueue_t iqueue_, oqueue_;
        unsigned long long index_;
        unsigned idle_;
    };
};

piw::converter_ref_t piw::triggering_converter()
{
    return pic::ref(new triggering_converter_t);
}

piw::converter_ref_t piw::impulse_converter()
{
    return pic::ref(new impulse_converter_t);
}

