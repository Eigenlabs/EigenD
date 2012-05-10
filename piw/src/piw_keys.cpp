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

#include <piw/piw_keys.h>

piw::coordinate_t::coordinate_t() : x_(piw::MAX_KEY+1), y_(piw::MAX_KEY+1), endrel_x_(false), endrel_y_(false)
{
}

piw::coordinate_t::coordinate_t(int x, int y) : x_(x), y_(y), endrel_x_(false), endrel_y_(false)
{
}

piw::coordinate_t::coordinate_t(int x, int y, bool endrel_x, bool endrel_y) : x_(x), y_(y), endrel_x_(endrel_x), endrel_y_(endrel_y)
{
}

piw::coordinate_t::coordinate_t(const coordinate_t &o) : x_(o.x_), y_(o.y_), endrel_x_(o.endrel_x_), endrel_y_(o.endrel_y_)
{
}

piw::coordinate_t::coordinate_t(const piw::data_nb_t &d) : x_(piw::MAX_KEY+1), y_(piw::MAX_KEY+1), endrel_x_(false), endrel_y_(false)
{
    if(d.is_tuple() && 4 == d.as_tuplelen())
    {
        piw::data_nb_t d_x = d.as_tuple_value(0);
        piw::data_nb_t d_y = d.as_tuple_value(1);
        piw::data_nb_t d_endrel_x = d.as_tuple_value(2);
        piw::data_nb_t d_endrel_y = d.as_tuple_value(3);

        if(d_x.is_long() && d_y.is_long() && d_endrel_x.is_bool() && d_endrel_y.is_bool())
        {
            x_ = d_x.as_long();
            y_ = d_y.as_long();
            endrel_x_ = d_endrel_x.as_bool();
            endrel_y_ = d_endrel_y.as_bool();
        }
    }
}

bool piw::coordinate_t::operator==(const coordinate_t &o) const
{
    return x_ == o.x_ && y_ == o.y_ && endrel_x_ == o.endrel_x_ && endrel_y_ == o.endrel_y_;
}

bool piw::coordinate_t::operator!=(const coordinate_t &o) const
{
    return x_ != o.x_ || y_ != o.y_ || endrel_x_ != o.endrel_x_ || endrel_y_ != o.endrel_y_;
}

bool piw::coordinate_t::operator<(const coordinate_t &o) const
{
    if(x_ < o.x_) return true;
    if(x_ > o.x_) return false;
    if(y_ < o.y_) return true;
    if(y_ > o.y_) return false;
    if(endrel_x_ < o.endrel_x_) return true;
    if(endrel_x_ > o.endrel_x_) return false;
    if(endrel_y_ < o.endrel_y_) return true;
    if(endrel_y_ > o.endrel_y_) return false;
    return false;
}

bool piw::coordinate_t::operator>(const coordinate_t &o) const
{
    if(x_ < o.x_) return false;
    if(x_ > o.x_) return true;
    if(y_ < o.y_) return false;
    if(y_ > o.y_) return true;
    if(endrel_x_ < o.endrel_x_) return false;
    if(endrel_x_ > o.endrel_x_) return true;
    if(endrel_y_ < o.endrel_y_) return false;
    if(endrel_y_ > o.endrel_y_) return true;
    return false;
}

piw::data_nb_t piw::coordinate_t::make_data_nb(unsigned long long t) const
{
    piw::data_nb_t result = piw::tuplenull_nb(t);
    result = piw::tupleadd_nb(result, piw::makelong_nb(x_,t));
    result = piw::tupleadd_nb(result, piw::makelong_nb(y_,t));
    result = piw::tupleadd_nb(result, piw::makebool_nb(endrel_x_,t));
    result = piw::tupleadd_nb(result, piw::makebool_nb(endrel_y_,t));
    return result;
}

piw::data_t piw::coordinate_t::make_data(unsigned long long t) const
{
    piw::data_t result = piw::tuplenull(t);
    result = piw::tupleadd(result, piw::makelong(x_,t));
    result = piw::tupleadd(result, piw::makelong(y_,t));
    result = piw::tupleadd(result, piw::makebool(endrel_x_,t));
    result = piw::tupleadd(result, piw::makebool(endrel_y_,t));
    return result;
}

bool piw::coordinate_t::equals(int x, int y, const piw::data_t &geometry) const
{
    return equals(x, y, geometry.make_nb());
}

bool piw::coordinate_t::equals(int x, int y, const piw::data_nb_t &geometry) const
{
    if(!endrel_x_ && !endrel_y_)
    {
        return x==x_ && y==y_;
    }

    if(!geometry.is_tuple() || geometry.as_tuplelen()==0)
    {
        return false;
    }

    int absx = x_;
    int absy = y_;

    int xlen = geometry.as_tuplelen();
    if(endrel_x_)
    {
        absx = xlen-absx+1;
    }

    if(absx!=x) return false;

    if(endrel_y_)
    {
        if(absx<0 || absx>xlen) return false;
        int ylen = geometry.as_tuple_value(absx-1).as_long();
        absy = ylen-absy+1;
    }

    if(absy!=y) return false;

    return true;
}

bool piw::coordinate_t::is_valid() const
{
    return abs(x_) <= piw::MAX_KEY && abs(y_) <= piw::MAX_KEY;
}

std::ostream &operator<<(std::ostream &o, const piw::coordinate_t &c)
{
    o << "(" << c.x_ << "," << c.y_ << "," << c.endrel_x_ << "," << c.endrel_y_ << ")";
    return o;
}

piw::data_nb_t piw::makekey(float column, float row,float course, float key, piw::hardness_t hardness, unsigned long long t)
{
    piw::data_nb_t physical_key = piw::tuplenull_nb(t);
    physical_key = piw::tupleadd_nb(physical_key, piw::makefloat_nb(column,t));
    physical_key = piw::tupleadd_nb(physical_key, piw::makefloat_nb(row,t));

    piw::data_nb_t musical_key = piw::tuplenull_nb(t);
    musical_key = piw::tupleadd_nb(musical_key, piw::makefloat_nb(course,t));
    musical_key = piw::tupleadd_nb(musical_key, piw::makefloat_nb(key,t));

    piw::data_nb_t result = piw::tuplenull_nb(t);
    result = piw::tupleadd_nb(result, physical_key);
    result = piw::tupleadd_nb(result, musical_key);
    result = piw::tupleadd_nb(result, piw::makelong_nb(hardness,t));
    result = piw::tuplenorm_nb(result, float(hardness)/piw::KEY_HARD);

    return result;
}

bool piw::is_key(const piw::data_t &d)
{
    return piw::decode_key(d.make_nb());
}

bool piw::decode_key(const piw::data_nb_t &d, float *column, float *row, float *course, float *key, piw::hardness_t *hardness)
{
    if(!d.is_tuple() || d.as_tuplelen() != 3)
    {
        return false;
    }

    piw::data_nb_t d_pkey = d.as_tuple_value(0);
    piw::data_nb_t d_mkey = d.as_tuple_value(1);
    piw::data_nb_t d_hardness = d.as_tuple_value(2);

    if( !d_pkey.is_tuple() || d_pkey.as_tuplelen() != 2 ||
       !d_mkey.is_tuple() || d_mkey.as_tuplelen() != 2 ||
       !d_hardness.is_long())
    {
        return false;
    }

    piw::data_nb_t d_column = d_pkey.as_tuple_value(0);
    piw::data_nb_t d_row = d_pkey.as_tuple_value(1);
    piw::data_nb_t d_course = d_mkey.as_tuple_value(0);
    piw::data_nb_t d_key = d_mkey.as_tuple_value(1);
    if(!d_column.is_float() || !d_row.is_float() ||
       !d_course.is_float() || !d_key.is_float())
    {
        return false;
    }

    if(column) *column = d_column.as_float();
    if(row) *row = d_row.as_float();
    if(course) *course = d_course.as_float();
    if(key) *key = d_key.as_float();
    if(hardness) *hardness = (piw::hardness_t)(d_hardness.as_long());

    return true;
}

unsigned piw::key_sequential(const piw::data_t &lengths, int x, int y, bool endrel_x, bool endrel_y)
{
    if(lengths.is_null() || !lengths.is_tuple())
        return 0;

    int geolen = lengths.as_tuplelen();
    if(0 == geolen)
        return 0;

    // resolve relative xs
    if(endrel_x)
        x = geolen - x + 1;

    // only calculate the key number when x is positive and exists
    if(x < 1 || x > geolen)
        return 0;

    int xlen = lengths.as_tuple_value(x-1).as_long();

    // resolve relative ys
    if(endrel_y)
        y = xlen - y + 1;

    // only calculate the key number when y exists
    if(xlen < y)
        return 0; 

    // x and y are within existing bounds, iterate
    // through the geometry to calculate the key number
    if(x > 0 && y > 0)
    {
        unsigned keynum = 0;
        for(int i = 1; i < x; ++i)
        {
            keynum += lengths.as_tuple_value(i-1).as_long();
        }
        keynum += y;

        return keynum;
    }

    return 0;
}

void piw::key_coordinates(unsigned seq, const piw::data_nb_t &lengths, int *x, int *y)
{
    *x = 0;
    *y = 0;

    if(seq <= 0 || !lengths.is_tuple())
    {
        return;
    }

    for(unsigned i=0; i<lengths.as_tuplelen(); ++i)
    {
        if(0==*y)
        {
            *y = seq;
        }
        else
        {
            int prev_length = lengths.as_tuple_value(i-1).as_long();
            if(*y<=prev_length)
            {
                break;
            }
            (*y) -= prev_length;
        }

        (*x)++;
    }
}
