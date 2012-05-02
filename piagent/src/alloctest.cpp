
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

#define THREADS 6
#define ALLOCATIONS 100000000
#define REPORT 100000

class alloc_thread_t: public pic::thread_t
{
    public:

        alloc_thread_t(unsigned index): pic::thread_t((index%2==0)?PIC_THREAD_PRIORITY_NORMAL:PIC_THREAD_PRIORITY_REALTIME), index_(index) {}
        ~alloc_thread_t() {}

        void start() { run(); }

        void thread_main()
        {
            printf("%2u thread main\n", index_); fflush(stdout);
            pic::nballocator_t *a = pic::nballocator_t::tsd_getnballocator();
            printf("%2u thread starting allocations\n", index_); fflush(stdout);

            for(unsigned i=0;i<ALLOCATIONS;i++)
            {

                if(i%REPORT==0) { printf("thread %2u %u allocations            \n",index_,i); fflush(stdout); }

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
    alloc_thread_t *threads[THREADS];

    printf("starting threads\n"); fflush(stdout);

    for(unsigned i=0; i<THREADS; ++i)
    {
        threads[i] = new alloc_thread_t(i);
        threads[i]->start();
    }

    printf("waiting for threads\n"); fflush(stdout);

    for(unsigned i=0; i<THREADS; ++i)
    {
        threads[i]->wait();
        printf("thread %2u finished\n",i); fflush(stdout);
    }

    printf("finished\n");
    return 0;
}
