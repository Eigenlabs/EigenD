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

#ifndef __PIA_SRC_SLAB__
#define __PIA_SRC_SLAB__

#include <picross/pic_thread.h>

/*
 * Slow allocator for locked space
 */

#define PIA_SLABSIZE 4096

class pia_slabbase_t
{
    private:
        typedef pic::element_t<0> element_t;

    public:
        pia_slabbase_t(unsigned size): size_(size)
        {
        }

        ~pia_slabbase_t()
        {
        }

        static pia_slabbase_t *ptr2slab(void *ptr)
        {
            unsigned long ptr2 = (unsigned long)ptr;
            ptr2 &= ~(PIA_SLABSIZE-1);
            ptr2 = ptr2 + PIA_SLABSIZE - sizeof(pia_slabbase_t *);
            return *(pia_slabbase_t **)ptr2;
        }

        unsigned size()
        {
            return size_;
        }

        void *slab_alloc()
        {
            element_t *e = freelist_.pop_front();

            if(e)
            {
                return e;
            }

            slab_add();
            e = freelist_.pop_front();
            PIC_ASSERT(e);
            return e;
        }

        void slab_free(void *ptr)
        {
            element_t *e = (element_t *)ptr;
            e->reset();
            freelist_.append(e);
        }

        void slab_add()
        {
            unsigned char *ptr=(unsigned char *)pic_thread_lck_malloc(PIA_SLABSIZE);
            unsigned alloc = PIA_SLABSIZE;
            unsigned char *ptr2;

            PIC_ASSERT(ptr);
            PIC_ASSERT((((unsigned long)ptr)&(PIA_SLABSIZE-1))==0);

            ptr2=ptr+PIA_SLABSIZE-sizeof(pia_slabbase_t *);
            alloc-=sizeof(pia_slabbase_t *);
            *(pia_slabbase_t **)ptr2 = this;

            while(alloc>size_)
            {
                element_t *e = (element_t *)ptr;
                e->reset();
                freelist_.append(e);
                ptr+=size_;
                alloc-=size_;
            }
        }

    private:
        pic::ilist_t<element_t> freelist_;
        unsigned size_;
};

template <class N> class pia_slab_t: public pia_slabbase_t
{
    public:
        pia_slab_t(): pia_slabbase_t(sizeof(N))
        {
        }
};

class pia_slabobject_t
{
    public:
        static void *operator new(size_t size, pia_slabbase_t &slab)
        {
            PIC_ASSERT(size==slab.size());
            return slab.slab_alloc();
        }

        static void operator delete(void *ptr)
        {
            pia_slabbase_t *slab = pia_slabbase_t::ptr2slab(ptr);
            slab->slab_free(ptr);
        }
};

#endif
