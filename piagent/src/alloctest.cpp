
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

#define THREADS 4

class alloc_thread_t: pic::thread_t
{
    public:

        alloc_thread_t(unsigned index) : index_(index) {}
        ~alloc_thread_t() {}

        void start() { run(); }

        void thread_main()
        {
            printf("%2u thread main\n", index_);

            pic::nballocator_t *a = pic::nballocator_t::tsd_getnballocator();

            printf("%2u thread starting allocations\n", index_);
            for(;;)
            {
                pic::nballocator_t::deallocator_t dealloc;
                void *dealloc_arg;

                unsigned s = 1+rand()%4096;
                void *m = a->allocator_xmalloc(PIC_ALLOC_NB,s,&dealloc,&dealloc_arg);

                dealloc(m,dealloc_arg);
            }
        }

        void thread_init() { printf("%2u thread init\n", index_); }
        void thread_term() { printf("%2u thread term\n", index_); }

    private:
        unsigned index_;
};

int main()
{
    pia::fastalloc_t a;
    pic::nballocator_t::tsd_setnballocator(&a);

    printf("starting threads\n");

    for(unsigned i=0; i<THREADS; ++i)
    {
        alloc_thread_t *t = new alloc_thread_t(i);
        t->start();
    }

    for(;;)
    {
        pic_microsleep(1000);
    }

    return 0;
}
