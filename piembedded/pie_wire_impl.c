
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

#include <picross/pic_config.h>
#include "pie_wire.h"
#include <pibelcanto/state.h>
#include "pie_message.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <math.h>

#ifdef PI_BIGENDIAN

static void __switch2(unsigned char *to, const unsigned char *from)
{
    to[0]=from[1];
    to[1]=from[0];
}

static void __switch4(unsigned char *to, const unsigned char *from)
{
    to[0]=from[3];
    to[1]=from[2];
    to[2]=from[1];
    to[3]=from[0];
}

static void __switch8(unsigned char *to, const unsigned char *from)
{
    to[0]=from[7];
    to[1]=from[6];
    to[2]=from[5];
    to[3]=from[4];
    to[4]=from[3];
    to[5]=from[2];
    to[6]=from[1];
    to[7]=from[0];
}

#else

static void __switch2(unsigned char *to, const unsigned char *from)
{
    memcpy(to,from,2);
}

static void __switch4(unsigned char *to, const unsigned char *from)
{
    memcpy(to,from,4);
}

static void __switch8(unsigned char *to, const unsigned char *from)
{
    memcpy(to,from,8);
}

#endif

int pie_setu16(unsigned char *b, unsigned l, uint16_t v)
{
    if(l<2)
    {
        return -1;
    }

    __switch2(b,(const unsigned char *)&v);

    return 2;
}

int pie_getu16(const unsigned char *b, unsigned l, uint16_t *v)
{
    if(l<2)
    {
        return -1;
    }

    __switch2((unsigned char *)v,b);

    return 2;
}

int pie_set32(unsigned char *b, unsigned l, int32_t v)
{
    if(l<4)
    {
        return -1;
    }

    __switch4(b,(const unsigned char *)&v);

    return 4;
}

int pie_get32(const unsigned char *b, unsigned l, int32_t *v)
{
    if(l<4)
    {
        return -1;
    }

    __switch4((unsigned char *)v,b);

    return 4;
}

int pie_setu32(unsigned char *b, unsigned l, uint32_t v)
{
    if(l<4)
    {
        return -1;
    }

    __switch4(b,(const unsigned char *)&v);

    return 4;
}

int pie_getu32(const unsigned char *b, unsigned l, uint32_t *v)
{
    if(l<4)
    {
        return -1;
    }

    __switch4((unsigned char *)v,b);

    return 4;
}

int pie_setu64(unsigned char *b, unsigned l, uint64_t v)
{
    if(l<8)
    {
        return -1;
    }

    __switch8(b,(const unsigned char *)&v);

    return 8;
}

int pie_getu64(const unsigned char *b, unsigned l, uint64_t *v)
{
    if(l<8)
    {
        return -1;
    }

    __switch8((unsigned char *)v,b);

    return 8;
}

int pie_setf32(unsigned char *b, unsigned l, float v)
{
    if(l<4)
    {
        return -1;
    }

    __switch4(b,(const unsigned char *)&v);

    return 4;
}

int pie_getf32(const unsigned char *b, unsigned l, float *v)
{
    if(l<4)
    {
        return -1;
    }

    __switch4((unsigned char *)v,b);

    return 4;
}

int pie_setf64(unsigned char *b, unsigned l, double v)
{
    if(l<8)
    {
        return -1;
    }

    __switch8(b,(const unsigned char *)&v);

    return 8;
}

int pie_getf64(const unsigned char *b, unsigned l, double *v)
{
    if(l<8)
    {
        return -1;
    }

    __switch8((unsigned char *)v,b);

    return 8;
}

#ifdef __cplusplus
}
#endif

