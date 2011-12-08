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

unsigned piw::calc_keynum(piw::data_t geo, int row, int col)
{
    if(geo.is_null() || !geo.is_tuple())
        return 0;

    int geolen = geo.as_tuplelen();
    if(0 == geolen)
        return 0;

    if(0 == col)
        return 0;

    // the key was entered in a sequential column-only format
    if(0 == row)
    {
        if (col > 0) return col;
        else return 0;
    }

    // resolve relative rows
    if(row < 0)
        row = geolen + row + 1;

    // only calculate the key number when the row exists
    if(row < 1 || row > geolen)
        return 0;

    int rowlen = geo.as_tuple_value(row-1).as_long();

    // resolve relative columns
    if(col < 0)
        col = rowlen + col + 1;

    // only calculate the key number when the column exists
    if(rowlen < col)
        return 0; 

    // the row and column are within existing bounds, iterate
    // through the geometry to calculate the key number
    if(row > 0 && col > 0)
    {
        unsigned keynum = 0;
        for(int i = 1; i < row; ++i)
        {
            keynum += geo.as_tuple_value(i-1).as_long();
        }
        keynum += col;

        return keynum;
    }

    return 0;
}

PIW_DECLSPEC_FUNC(piw::data_nb_t) piw::key_position(unsigned key, const piw::data_nb_t &lengths, unsigned long long t)
{
    if(key <= 0 || !lengths.is_tuple())
    {
        return piw::makenull_nb(0);
    }

    unsigned row = 0;
    unsigned col = 0;

    for(unsigned i=0; i<lengths.as_tuplelen(); ++i)
    {
        if(0==col)
        {
            col = key;
        }
        else
        {
            unsigned prev_course = lengths.as_tuple_value(i-1).as_long();
            if(col<=prev_course)
            {
                break;
            }
            col -= prev_course;
        }

        row++;
    }

    if(0==row || 0==col)
    {
        return piw::makenull_nb(0);
    }

    piw::data_nb_t d = piw::tuplenull_nb(t);
    d = piw::tupleadd_nb(d, piw::makefloat_nb(row,t));
    d = piw::tupleadd_nb(d, piw::makefloat_nb(col,t));
    return d;
}
