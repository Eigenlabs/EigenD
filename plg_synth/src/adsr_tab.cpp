
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

#include <math.h>
#include <cstdio>

#define SIZE 1441
#define RANGE 1440.0

int main(void)
{
    printf("float adsr2func_t::__val2amp_table[AMPLITUDE_TABLE_SIZE] = { \n");
    for(unsigned i = 0; i < SIZE; i++) 
    {
        double v = (double)i/RANGE;
        double x = pow(10.0, RANGE*(v-1.0)/200.0);
        printf("%.10f, ", x);
        if((i+1)%8 == 0)
        {
            printf("\n");
        }
    }
    printf("};\n");

    printf("float adsr2func_t::__amp2val_table[AMPLITUDE_TABLE_SIZE] = { \n");
    for(unsigned i = 0; i < SIZE; i++) 
    {
        if(i==0)
        {
            printf("%.10f, ", 0.0);
            continue;
        }

        double a = (double)i/RANGE;
        double x = 1.0+200.0*log10(a)/RANGE;
        printf("%.10f, ", x);
        if((i+1)%8 == 0)
        {
            printf("\n");
        }
    }
    printf("};\n");
    return 0;
}
