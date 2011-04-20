
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

#include "synth.h"
#include <piw/piw_table.h>
#include <math.h>

#define SIZE 1000
#define SHARPEN_TIME 100000 // us

namespace
{
    struct compressor_t
    {
        compressor_t(float c) : table_(SIZE)
        {
            table_.set_bounds(0,1);

            for(unsigned i=0; i<SIZE; ++i)
            {
                float x = (float)i/(float)SIZE;
                float y = 1.0 - powf(1.0-x, (c+0.1)*10);
                table_.set_entry(i,y);
            }
        }

        piw::data_nb_t operator()(const piw::data_nb_t &in) const
        {
            float val = in.as_renorm(-1,1,0);
            float out = 0;
            piw::data_nb_t out_data;
            if(val<0)
            {
                out = -table_.interp(-val);
                out_data = piw::makefloat_bounded_nb(1, -1, 0, out, in.time());
            }
            else
            {
                out = table_.interp(val);
                out_data = piw::makefloat_bounded_nb(1, -1, 0, out, in.time());
            }
            return out_data;
        }

        bool operator==(const compressor_t &other) const
        {
            return this==&other;
        }

        pic::ftable_t table_;
    };

    struct sharpener_t
    {
        sharpener_t(float s) : sharpen_((unsigned)(1.0+((1.0-s)*99.0))), start_(0), end_(0)
        {
        }

        piw::data_nb_t operator()(const piw::data_nb_t &v, const piw::data_nb_t &p) const
        {
/*
            bool b = (v.as_renorm(0,1,0)>0.0);
            if(b)
            {
                if(!start_)
                {
                    start_ = v.time();
                    end_ = start_+SHARPEN_TIME;
                }
            }
            else
            {
                start_ = 0;
                return p;
            }

            unsigned long long t = p.time();
            if(t<start_ || t > end_)
            {
                return p;
            }

            unsigned long long dt = (t-start_)/100;
            return p.restamp(start_ + dt*sharpen_);
*/

            return p;
        }

        bool operator==(const sharpener_t &other) const
        {
            return this==&other;
        }

        unsigned sharpen_;
        mutable unsigned long long start_,end_;
    };
}

piw::dd2d_nb_t synth::sharpener(float s)
{
    return piw::dd2d_nb_t::callable(sharpener_t(s));
}

piw::d2d_nb_t synth::compressor(float c)
{
    return piw::d2d_nb_t::callable(compressor_t(c));
}

