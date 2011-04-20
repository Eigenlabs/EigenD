
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

#include <picross/pic_float.h>
#include <piw/piw_pitchbender.h>
#include <piw/piw_table.h>
#include <math.h>

namespace
{
    struct pitchbender
    {
        piw::data_nb_t operator()(const piw::data_nb_t &f, const piw::data_nb_t &b) const
        {
            unsigned long long t = std::max(f.time(), b.time());
            float freq = bend(f.as_renorm_float(BCTUNIT_HZ,0,96000,0), b.as_norm());
            return piw::makefloat_bounded_nb(f.as_array_ubound(), f.as_array_lbound(), f.as_array_rest(), freq, t);
        }

        bool operator==(const pitchbender& other) const
        {
            return this==&other;
        }

        pitchbender(float range,float offset) : powtable_(1000)
        {
            powtable_.set_bounds(-1.0,1.0);
            range = range/1200.0;
            offset = offset/1200.0;

            float b = -1.0;
            unsigned i = 0;

            for(; i<500; ++i)
            {
                float y = range*b*b;
                powtable_.set_entry(i, powf(2.0, offset-y));
                powtable_.set_entry(999-i, powf(2.0, offset+y));
                b += 0.002;
            }
        }

        float bend(float f, float b) const
        {
            return f*powtable_.interp(b);
        }
        
        pic::ftable_t powtable_;
    };
};

piw::dd2d_nb_t piw::make_pitchbender(float range,float offset)
{
    return piw::dd2d_nb_t::callable(pitchbender(range,offset));
}

struct piw::fast_pitchbender_t::impl_t: piw::dd2d_nb_t::sinktype_t, virtual pic::tracked_t
{
    impl_t() : range_(2.f), offset_(2.f)
    {
    }

    ~impl_t()
    {
        tracked_invalidate();
    }

    piw::data_nb_t invoke(const piw::data_nb_t &p1, const piw::data_nb_t &p2) const
    {
        unsigned long long t = std::max(p1.time(),p2.time());
        float f=p1.as_renorm_float(BCTUNIT_HZ,0,96000,0);
        float b=p2.as_norm();
        float r=(b<0.f) ? offset_-range_*b*b:offset_+range_*b*b;
        float f2=f*pic::approx::pow2(r);
        return piw::makefloat_bounded_units_nb(BCTUNIT_HZ,96000,0,0,f2,t);
    }

    bool iscallable() const
    {
        return true;
    }

    void set_range(const piw::data_nb_t &d)
    {
        range_=d.as_float()/12.f;
        pic::logmsg() << "range -> " << range_;
    }

    void set_offset(const piw::data_nb_t &d)
    {
        offset_=d.as_float()/12.f;
        pic::logmsg() << "offset -> " << offset_;
    }

    float range_;
    float offset_;
};

piw::fast_pitchbender_t::fast_pitchbender_t()
{
    impl_ = new impl_t();
}

piw::fast_pitchbender_t::~fast_pitchbender_t()
{
    delete impl_;
}

piw::change_nb_t piw::fast_pitchbender_t::set_range()
{
    return change_nb_t::method(impl_,&impl_t::set_range);
}

piw::change_nb_t piw::fast_pitchbender_t::set_offset()
{
    return change_nb_t::method(impl_,&impl_t::set_offset);
}

piw::dd2d_nb_t piw::fast_pitchbender_t::bend_functor()
{
    return dd2d_nb_t(pic::ref(impl_));
}

