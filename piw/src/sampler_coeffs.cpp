
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

#include <cstdio>

int main(void)
{
    printf("typedef struct\n");
    printf("{\n");
    printf("    float a0, a1, a2, a3;\n");
    printf("} interp_coeff_t;\n");
    printf("interp_coeff_t interp_coeff[256] = { \n");
    for(unsigned i = 0; i < 256; i++) {
        double x = (double) i / (double) 256;
        printf("{ %f, ", (float) (x * (-0.5 + x * (1 - 0.5 * x))));
        printf("%f, ", (float) (1.0 + x * x * (1.5 * x - 2.5)));
        printf("%f, ", (float) (x * (0.5 + x * (2.0 - 1.5 * x))));
        printf("%f },\n", (float) (0.5 * x * x * (x - 1.0)));
    }
    printf("};\n");
    return 0;
}
