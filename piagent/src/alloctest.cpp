
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

#include <picross/pic_thread.h>
#include <picross/pic_time.h>
#include <piagent/pia_fastalloc.h>

#define THREADS 16

void __tester(void *a_)
{
    pia::fastalloc_t *a = (pia::fastalloc_t *)a_;

    for(;;)
    {
        pia::fastalloc_t::deallocator_t d;
        unsigned s=1+random()%4096;
        void *m1=a->allocator_xmalloc(s,&d);
        d(m1);
    }
}

int main()
{
    pia::fastalloc_t a;

    for(unsigned i=0; i<THREADS; ++i)
    {
        pic_thread_t *t = new pic_thread_t;
        pic_thread_create(t,0,__tester,&a);
        pic_thread_run(t);
    }

    for(;;)
    {
        pic_microsleep(1000);
    }

    return 0;
}
