
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

#include <picross/pic_atomic.h>
#include <picross/pic_thread.h>
#include <picross/pic_error.h>
#include <piagent/pia_fastalloc.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#define PIA_ALLOC_LIMIT (15*4)
#define PTRADD(p,o) (((unsigned char *)(p))+(o))
#define WATERMARK 2
#define SLB_WATERMARK 10
#define ALIGN(size, boundary) (((size) + ((boundary) - 1)) & ~((boundary) - 1))

/*
 * We keep blocks on various stacks.  When a stack is exhausted, we get a block
 * from the stack with double the block size, and fragment the block.  If there's
 * no such stack, or the largest stack is exhausted, we have to malloc.
 *
 * we maintain a seperate thread which continually tops up the large stacks
 *
 * Blocks are added and removed from the stacks using a CAS operation.  the ABA
 * problem is avoided by reference counting the blocks.  We increment the
 * reference count before popping a block from a stack, so that even if another
 * thread allocates the block, it can't put it back again.
 *
 * Use of DCAS to manipulate stack head and reference count isn't necessary in 
 * this case because once a block is allocated, it remains valid forever.  Even
 * if the block is fragmented, the reference count isn't touched.  Fragmented
 * blocks are not recombined.
 */
 

namespace
{
    struct slbheader_t
    {
        slbheader_t *next;
        pic_atomic_t refc;
    };

    typedef slbheader_t * volatile vslbhdr_t;

    struct blkheader_t
    {
        blkheader_t *next;
        pic_atomic_t refc;
        pic_atomic_t freelist;
    };

    typedef blkheader_t * volatile vblkhdr_t;
};

struct pia::fastalloc_t::nbimpl_t: pic::thread_t
{
    vblkhdr_t queue_[PIA_ALLOC_LIMIT];
    unsigned sizes_[PIA_ALLOC_LIMIT];
    pic_atomic_t count_;
    volatile bool stop_;
    pic_atomic_t free_[4];
    pic::xgate_t gate_;
    pic_atomic_t waste_;
    pic_atomic_t slbfree_;
    vslbhdr_t slbhead_;

    void add_waste(unsigned waste)
    {
        for(;;)
        {
            pic_atomic_t ow = waste_;
            pic_atomic_t nw = ow+waste;

            if(pic_atomiccas(&waste_,ow,nw))
            {
                break;
            }
        }
    }

    void del_waste(unsigned waste)
    {
        for(;;)
        {
            pic_atomic_t ow = waste_;

            if(waste>ow)
            {
                abort();
            }

            pic_atomic_t nw = ow-waste;

            if(pic_atomiccas(&waste_,ow,nw))
            {
                break;
            }
        }
    }

    void thread_main()
    {
#ifdef DEBUG_DATA_ATOMICITY
        std::cout << "Started fast allocation thread with ID " << pic_current_threadid() << std::endl;
#endif
 
        while(!stop_)
        {
            for(unsigned i=0;i<4;i++)
            {
                pic_atomic_t free = free_[i];

                if(!pic_atomiccas(&free_[i],free,free))
                {
                    continue;
                }

                while(free < WATERMARK)
                {
                    //printf("allocating block for queue %u\n",sizes_[PIA_ALLOC_LIMIT-4+i]);
                    release(newblock(PIA_ALLOC_LIMIT-4+i));
                    free++;
                }
            }

            {
                pic_atomic_t free = slbfree_;

                if(pic_atomiccas(&slbfree_,free,free))
                {
                    while(free < SLB_WATERMARK)
                    {
                        //printf("allocating block for queue %u\n",sizes_[PIA_ALLOC_LIMIT-4+i]);
                        slb_release(newslb());
                        free++;
                    }
                }
            }

            gate_.pass_and_shut_timed(5000000);
        }
    }

	virtual ~nbimpl_t()
	{
        stop_=true;
        gate_.open();
        wait();
	}

    void incref()
    {
        pic_atomicinc(&count_);
    }

    void decref()
    {
        if(pic_atomicdec(&count_)==0)
        {
            printf("deleting allocator %p\n",this);
            delete this;
        }
    }

    void slb_release(slbheader_t *blk)
    {
        if(pic_atomicdec(&blk->refc)>0)
        {
            return;
        }

        while(1)
        {
            slbheader_t *head = slbhead_;
            blk->next = head;
            if(pic_atomicptrcas((void *)&(slbhead_), head, blk))
            {
                pic_atomicinc(&slbfree_);
                return;
            }
        }
    }

    void slb_reserve(slbheader_t *blk)
    {
        pic_atomicinc(&blk->refc);
    }

    void release(blkheader_t *blk)
    {
        if(pic_atomicdec(&blk->refc)>0)
        {
            return;
        }

        unsigned fl = blk->freelist;

        if(fl == PIA_ALLOC_LIMIT)
        {
            return;
        }

        if(!pic_atomiccas(&(blk->freelist),fl,PIA_ALLOC_LIMIT))
        {
            return;
        }

        while(1)
        {
            blkheader_t *head = queue_[fl];
            blk->next = head;
            if(pic_atomicptrcas((void *)&(queue_[fl]), head, blk))
            {
                if(fl+4>=PIA_ALLOC_LIMIT)
                {
                    pic_atomicinc(&free_[fl+4-PIA_ALLOC_LIMIT]);
                }

                return;
            }
        }
    }

    void reserve(blkheader_t *blk)
    {
        pic_atomicinc(&blk->refc);
    }

    slbheader_t *slb_alloc()
    {
        while(1)
        {
            slbheader_t *head = slbhead_;

            if(!head)
            {
                return newslb();
            }

            slb_reserve(head);

            if(pic_atomicptrcas((void *)&(slbhead_), head, head->next))
            {
                pic_atomicdec(&slbfree_);
                gate_.open();
                return head;
            }

            slb_release(head);
        }
    }

    blkheader_t *alloc(unsigned fl)
    {
        while(1)
        {
            blkheader_t *head = queue_[fl];

            if(!head)
            {
                return 0;
            }

            reserve(head);

            if(pic_atomicptrcas((void *)&(queue_[fl]), head, head->next))
            {
                PIC_ASSERT(pic_atomiccas(&(head->freelist),PIA_ALLOC_LIMIT,fl));

                if(fl+4>=PIA_ALLOC_LIMIT)
                {
                    pic_atomicdec(&free_[fl+4-PIA_ALLOC_LIMIT]);
                    gate_.open();
                }

                return head;
            }

            release(head);
        }
    }

	blkheader_t *get0(unsigned i)
	{
		blkheader_t *blk;

        if((blk=alloc(i))!=0)
        {
            return blk;
        }

        if(i+4>=PIA_ALLOC_LIMIT)
        {
            blk = newblock(i);
            return blk;
        }

		blk=get0(i+4);
        blkheader_t *blk2 = (blkheader_t *)PTRADD(blk,sizes_[i]);

        blk2->refc=1;
        blk2->freelist=i;
        blk->freelist=i;

        release(blk2);
        return blk;
	}

    void *allocator_smalloc()
    {
        slbheader_t *ptr;
        ptr = slb_alloc();

        if(!ptr)
        {
            return 0;
        }

        incref();
        return (void *)(ptr+1);
    }

    static void allocator_sfree(void *mem, void *da)
    {
		slbheader_t *ptr = ((slbheader_t *)mem)-1;
        nbimpl_t *a = (nbimpl_t *)da;
		a->slb_release(ptr);
        a->decref();
    }

	void *allocator_malloc(size_t size)
	{
		int i;
		blkheader_t *ptr;

		for(i=0;i<PIA_ALLOC_LIMIT;i++)
		{
			if(size+sizeof(blkheader_t) <= sizes_[i])
			{
				ptr = get0(i);
                incref();
                return (void *)(ptr+1);
			}
		}

        PIC_THROW("size too large for allocation");
		return NULL;
	}

	static void allocator_free(void *mem, void *da)
	{
		blkheader_t *ptr = ((blkheader_t *)mem)-1;
        nbimpl_t *a = (nbimpl_t *)da;
		a->release(ptr);
        a->decref();
	}

    slbheader_t *newslb()
    {
        slbheader_t *p;
        unsigned s = PIC_ALLOC_SLABSIZE+sizeof(slbheader_t);

        if(!(p = (slbheader_t *)pic_thread_lck_malloc(s)))
        {
            abort();
        }

        memset(p,0xaa,s);
        p->refc=1;
        return p;
    }

    blkheader_t *newblock(unsigned fl)
    {
        blkheader_t *p;
        unsigned s = sizes_[fl];

        if(!(p = (blkheader_t *)pic_thread_lck_malloc(s)))
        {
            abort();
        }

        memset(p,0xaa,s);
        p->refc=1;
        p->freelist=fl;

        return p;
    }

	nbimpl_t(): count_(1), stop_(false), waste_(0), slbfree_(0), slbhead_(0)
	{
		unsigned i,j;

		for(i=0;i<4;i++)
        {
			queue_[i]=0;
#ifdef ALIGN_16
			sizes_[i]=64+16*i; 
#else
            sizes_[i]=32+8*i;
#endif
            free_[i]=0;
        }

		for(i=4;i<PIA_ALLOC_LIMIT;i++)
		{
			queue_[i]=0;
			sizes_[i]=sizes_[i-4]*2;
		}

        for(j=0;j<4;j++)
        {
            for(i=0;i<WATERMARK;i++)
            {
                release(newblock(PIA_ALLOC_LIMIT-4+j));
            }
        }

        for(i=0;i<SLB_WATERMARK;i++)
        {
            slb_release(newslb());
        }

        run();
	}

};

pia::fastalloc_t::fastalloc_t()
{
	nbimpl_ = new nbimpl_t;
}

pia::fastalloc_t::~fastalloc_t()
{
	nbimpl_->decref();
}

static void demallocator(void *ptr, void *)
{
    free(ptr);
}

void *pia::fastalloc_t::allocator_xmalloc(unsigned nb,size_t size, deallocator_t *d, void **da)
{
    if(nb==PIC_ALLOC_SLB)
    {
        PIC_ASSERT(size<=PIC_ALLOC_SLABSIZE);
        *d = nbimpl_t::allocator_sfree;
        *da = nbimpl_;
        void *ptr = nbimpl_->allocator_smalloc();
        return ptr;
    }

    if(nb==PIC_ALLOC_NORMAL)
    {
        *d = demallocator;
        void *ptr = malloc(size);
        PIC_ASSERT(ptr);
        return ptr;
    }


    *d = nbimpl_t::allocator_free;
    *da = nbimpl_;

    void *ptr = nbimpl_->allocator_malloc(size);
	return ptr;
}

pia::mallocator_t::mallocator_t()
{
}

pia::mallocator_t::~mallocator_t()
{
}

void *pia::mallocator_t::allocator_xmalloc(unsigned nb,size_t size, deallocator_t *d, void **da)
{
    *d = demallocator;
    void *ptr = malloc(size);
    PIC_ASSERT(ptr);
    return ptr;
}
