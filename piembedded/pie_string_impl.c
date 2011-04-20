
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

#include "pie_string.h"
#include "pie_print.h"

#ifdef __cplusplus
extern "C" {
#endif

void pie_writestr_init(pie_strwriter_t *a, char *s, unsigned l)
{
    a->ptr=s;
    a->len=l;
}

void pie_readstr_init(pie_strreader_t *a, const char *s, unsigned l)
{
    a->ptr=s;
    a->len=l;
}

void pie_writestr(void *a, char p)
{
    pie_strwriter_t *s = (pie_strwriter_t *)a;

    if(s->len>1)
    {
        *(s->ptr)++=p;
        *(s->ptr)=0;
        s->len--;
    }
}

int pie_readstr(void *a, int p)
{
    pie_strreader_t *s = (pie_strreader_t *)a;
    char c;

    if((s->len)==0)
    {
        return -1;
    }

    c=*(s->ptr);

    if(!p)
    {
        (s->ptr)++;
        (s->len)--;
    }

    return c;
}

#ifdef __cplusplus
}
#endif

