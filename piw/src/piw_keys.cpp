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

piw::data_nb_t piw::makekey(unsigned pseq, float column, float row, unsigned mseq, float course, float key, piw::hardness_t hardness, unsigned long long t)
{
    piw::data_nb_t physical_key = piw::tuplenull_nb(t);
    physical_key = piw::tupleadd_nb(physical_key, piw::makefloat_nb(column,t));
    physical_key = piw::tupleadd_nb(physical_key, piw::makefloat_nb(row,t));

    piw::data_nb_t musical_key = piw::tuplenull_nb(t);
    musical_key = piw::tupleadd_nb(musical_key, piw::makefloat_nb(course,t));
    musical_key = piw::tupleadd_nb(musical_key, piw::makefloat_nb(key,t));

    piw::data_nb_t result = piw::tuplenull_nb(t);
    result = piw::tupleadd_nb(result, piw::makelong_nb(pseq,t));
    result = piw::tupleadd_nb(result, physical_key);
    result = piw::tupleadd_nb(result, piw::makelong_nb(mseq,t));
    result = piw::tupleadd_nb(result, musical_key);
    result = piw::tupleadd_nb(result, piw::makelong_nb(hardness,t));
    result = piw::tuplenorm_nb(result, float(hardness)/piw::KEY_HARD);

    return result;
}

bool piw::is_key(const piw::data_t &d)
{
    return piw::decode_key(d.make_nb());
}

bool piw::decode_key(const piw::data_nb_t &d, unsigned *pseq, float *column, float *row, unsigned *mseq, float *course, float *key, piw::hardness_t *hardness)
{
    if(!d.is_tuple() || d.as_tuplelen() != 5)
    {
        return false;
    }

    piw::data_nb_t d_pseq = d.as_tuple_value(0);
    piw::data_nb_t d_pkey = d.as_tuple_value(1);
    piw::data_nb_t d_mseq = d.as_tuple_value(2);
    piw::data_nb_t d_mkey = d.as_tuple_value(3);
    piw::data_nb_t d_hardness = d.as_tuple_value(4);

    if(!d_pseq.is_long() || !d_mseq.is_long() ||
       !d_pkey.is_tuple() || d_pkey.as_tuplelen() != 2 ||
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

    if(pseq) *pseq = d_pseq.as_long();
    if(column) *column = d_column.as_float();
    if(row) *row = d_row.as_float();
    if(mseq) *mseq = d_mseq.as_long();
    if(course) *course = d_course.as_float();
    if(key) *key = d_key.as_float();
    if(hardness) *hardness = (piw::hardness_t)(d_hardness.as_long());

    return true;
}

unsigned piw::key_sequential(const piw::data_t &lengths, int column, int row)
{
    if(lengths.is_null() || !lengths.is_tuple())
        return 0;

    int geolen = lengths.as_tuplelen();
    if(0 == geolen)
        return 0;

    if(0 == row)
        return 0;

    // the key was entered in a sequential row-only format
    if(0 == column)
    {
        if (row > 0) return row;
        else return 0;
    }

    // resolve relative columns
    if(column < 0)
        column = geolen + column + 1;

    // only calculate the key number when the column exists
    if(column < 1 || column > geolen)
        return 0;

    int columnlen = lengths.as_tuple_value(column-1).as_long();

    // resolve relative rows
    if(row < 0)
        row = columnlen + row + 1;

    // only calculate the key number when the row exists
    if(columnlen < row)
        return 0; 

    // the column and row are within existing bounds, iterate
    // through the geometry to calculate the key number
    if(column > 0 && row > 0)
    {
        unsigned keynum = 0;
        for(int i = 1; i < column; ++i)
        {
            keynum += lengths.as_tuple_value(i-1).as_long();
        }
        keynum += row;

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
