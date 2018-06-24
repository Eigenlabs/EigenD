
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

#include <piw/piw_sample.h>
#include <piw/piw_tsd.h>
#include <picross/pic_error.h>
#include <picross/pic_safeq.h>
#include <picross/pic_power.h>
#include <picross/pic_resources.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFERSIZE_CHARS  PIC_ALLOC_SLABSIZE
#define BUFFERSIZE_SAMPLES (BUFFERSIZE_CHARS/2)
#define BLOCKSIZE_SAMPLES 2048
#define BUFFER_BLOCKS     14
#define BUFFER_HEADROOM   (BUFFERSIZE_CHARS-BLOCKSIZE_SAMPLES*2*BUFFER_BLOCKS)

namespace
{
    struct sample_buffer_t: virtual pic::atomic_counted_t, virtual pic::lckobject_t
    {
        sample_buffer_t(unsigned block, unsigned size,unsigned long long t);
        ~sample_buffer_t();

        unsigned long long qtime() { return time_; }

        unsigned size_;
        short *data_;
        bool valid_;
        unsigned block_;
        unsigned long long time_;
        void (*dealloc_)(void *,void *);
        void *deallocarg_;
    };

    typedef pic::ref_t<sample_buffer_t> sample_bufref_t;
};

struct piw::samplearray_t::impl_t: virtual pic::lckobject_t, pic::safe_worker_t
{
    impl_t(const char *filename, unsigned p, unsigned l);
    bool ping();
    sample_bufref_t allocate_buffer(unsigned o,unsigned long long t) const;
    sample_bufref_t get_block(unsigned o) const;
    void __get_block(unsigned o, short *dst) const;
    static void __reader(void *i_, void *dst_, void *, void *);
    sample_bufref_t queue_read(unsigned o);
    ~impl_t();

    std::string name;
    FILE *fd;
    unsigned size;
    unsigned offset;
    unsigned long long max_total,max_sched,max_read,count,ocount;
};

struct piw::sample_t::impl_t: public pic::lckobject_t
{
    impl_t(const samplearrayref_t &d,unsigned s, unsigned e, unsigned ls, unsigned le, float sr, float rf, float att);

    samplearrayref_t data_;
    unsigned start_, end_;
    unsigned loopstart_, loopend_;
    float samplerate_;
    float rootfreq_;
    float attenuation_;
    sample_bufref_t start_buffer_;
};

struct piw::samplereader_t::rimpl_t: virtual public pic::lckobject_t
{
    rimpl_t(const sampleref_t &sample);
    void queue(sample_bufref_t &next, unsigned nblk);
    const short *bufptr0(unsigned blk, const sample_bufref_t &current, sample_bufref_t &next, sample_bufref_t &next2, sample_bufref_t &next3);
    const short *bufptr(unsigned offset);

    unsigned loop_block1_,loop_block2_,end_block_;
    sampleref_t sample_;
    samplearray_t::impl_t *array_;
    sample_bufref_t buffer1_,buffer2_,buffer3_,buffer4_;
    sample_bufref_t start_buffer_,loop_buffer1_,loop_buffer2_;
    bool looped_;
    unsigned last_block_;
    bool error_;
};

piw::samplearray_t::samplearray_t(const char *filename, unsigned p, unsigned l): impl_(new impl_t(filename,p,l))
{
}

piw::samplearray_t::~samplearray_t()
{
    delete impl_;
}

piw::samplereader_t::samplereader_t(const sampleref_t &sample): impl_(new piw::samplereader_t::rimpl_t(sample))
{
}

piw::samplereader_t::~samplereader_t()
{
    delete impl_;
}

const short *piw::samplereader_t::bufptr(unsigned offset)
{
    return impl_->bufptr(offset);
}

piw::sample_t::sample_t(const samplearrayref_t &d,unsigned s, unsigned e, unsigned ls, unsigned le, float sr, float rf, float att): impl_(new impl_t(d,s,e,ls,le,sr,rf,att))
{
}

piw::sample_t::~sample_t()
{
    delete impl_;
}

void piw::xvoice_t::envelope(float v,float f,float *dep,float *ap,float *hp,float *dcp,float *sp,float *rp)
{
  //  std::list<piw::zoneref_t>::const_iterator z = zones.begin();
    pic::lcklist_t<zoneref_t>::nbtype::const_iterator z = zones.begin();

    *dep=(*z)->de; *ap=(*z)->a; *hp=(*z)->h; *dcp=(*z)->dc; *sp=(*z)->s; *rp=(*z)->r;
}

piw::xpreset_t::xpreset_t(): minv(1.0), minf(1000000.0), maxv(0.0), maxf(0.0)
{
}

void piw::xpreset_t::add_zone(const zoneref_t &z)
{
    zonekey_t k(std::make_pair(z->vmax,z->fmax));
    zones.insert(std::make_pair(k,z));

    if(z->vmin<minv) minv=z->vmin;
    if(z->fmin<minf) minf=z->fmin;
    if(z->vmax>maxv) maxv=z->vmax;
    if(z->fmax>maxf) maxf=z->fmax;
}

piw::voiceref_t piw::xpreset_t::find_zone(float v, float f) const
{
    piw::voiceref_t voice;

    v = std::min(v,1.0f);
    zonemap_t::const_iterator zi,ze;

    if(v<minv) v=minv;
    if(f<minf) f=minf;
    if(v>maxv) v=maxv;
    if(f>maxf) f=maxf;

    zonekey_t k(std::make_pair(v,f));
    zi = zones.lower_bound(k);
    ze = zones.end();

    for(; zi!=ze; ++zi)
    {
        zoneref_t z = zi->second;
        bool vapplies =  (z->vmin <= v  && v <= z->vmax);
        bool fapplies =  (z->fmin <= f  && f <= z->fmax);

        if(vapplies && fapplies)
        {
            if(!voice.isvalid()) voice = piw::create_voice();
            voice->add_zone(zi->second);
        }

        if(!vapplies && !fapplies)
        {
            break;
        }
    }

    return voice;
}

sample_buffer_t::sample_buffer_t(unsigned block, unsigned size,unsigned long long t): size_(size), valid_(false), block_(block), time_(t)
{
    data_=(short *)piw::tsd_alloc(PIC_ALLOC_NB,PIC_ALLOC_SLABSIZE,&dealloc_,&deallocarg_);
}

sample_buffer_t::~sample_buffer_t()
{
    dealloc_(data_,deallocarg_);
}

piw::samplearrayref_t piw::sample_t::data() { return impl_->data_; }
unsigned piw::sample_t::start() const { return impl_->start_; }
unsigned piw::sample_t::end() const { return impl_->end_; }
unsigned piw::sample_t::loopstart() const { return impl_->loopstart_; }
unsigned piw::sample_t::loopend() const { return impl_->loopend_; }
float piw::sample_t::samplerate() const { return impl_->samplerate_; }
float piw::sample_t::rootfreq() const { return impl_->rootfreq_; }
float piw::sample_t::attenuation() const { return impl_->attenuation_; }

piw::samplearray_t::impl_t::impl_t(const char *filename, unsigned p, unsigned l): pic::safe_worker_t(1000,PIC_THREAD_PRIORITY_HIGH), name(filename), size(l/2), offset(p)
{
    fd = (FILE*) pic::fopen(filename,"rb");

    if(fd==nullptr)
    {
        pic::msg() << "Can't open " << filename << pic::hurl;
    }

    max_total=0; max_sched=0; max_read=0; count=0; ocount=0;
    run();
}

bool piw::samplearray_t::impl_t::ping()
{
    pic::disk_active();

    if(count!=ocount)
    {
        ocount=count;
        pic::logmsg() << "buffer stats: max delay=" << max_total << " sched=" << max_sched << " read=" << max_read << " count=" << count;
    }

    return true;
}

sample_bufref_t piw::samplearray_t::impl_t::allocate_buffer(unsigned o,unsigned long long t) const
{
    return pic::ref(new sample_buffer_t(o,BUFFERSIZE_CHARS,t));
}

// read c floats from disk
sample_bufref_t piw::samplearray_t::impl_t::get_block(unsigned o) const
{
    sample_bufref_t b = allocate_buffer(o,0);
    __get_block(o,b->data_);
    b->valid_=true;
    return b;
}

void piw::samplearray_t::impl_t::__get_block(unsigned o, short *dst) const
{
    if(fseek(fd,offset+o*BLOCKSIZE_SAMPLES*2,SEEK_SET)<0)
    {
        pic::logmsg() << "seek error " << name << " " << errno;
        return;
    }

    int sz = BUFFERSIZE_SAMPLES*2;
    int nr;

    if((nr=fread(dst,1,sz,fd))!=sz)
    {
        if(ferror(fd))
        {
            pic::logmsg() << "read error " << name << " " << errno;
            return;
        }
        else
        {
            pic::logmsg() << "read error " << name << " eof";
        }
    }

#ifdef PI_BIGENDIAN
    short *flt = (short *)dst;
    unsigned char *raw = (unsigned char *)dst;
    nr = nr/2;

    for(int i=nr-1; i>=0; --i)
    {
        flt[i] = (raw[i*2+1]<<8)|(raw[i*2]);
    }
#endif
}

void piw::samplearray_t::impl_t::__reader(void *i_, void *dst_, void *, void *)
{
    unsigned long long t2=piw::tsd_time();
    sample_bufref_t buf = sample_bufref_t::from_given((sample_buffer_t *)dst_);
    impl_t *impl = (impl_t *)i_;

    if(buf->count()==0)
    {
        //pic::logmsg() << "read aborted " << buf->block_;
        return;
    }

    impl->__get_block(buf->block_,buf->data_);
    unsigned long long t3=piw::tsd_time();
    unsigned long long tb = buf->qtime();
    buf->valid_=true;
    //pic::logmsg() << "read completed " << buf->block_;
    impl->count++;
    if(t2-tb>impl->max_sched) impl->max_sched=t2-tb;
    if(t3-t2>impl->max_read) impl->max_read=t3-t2;
    if(t3-tb>impl->max_total) impl->max_total=t3-tb;
}

sample_bufref_t piw::samplearray_t::impl_t::queue_read(unsigned o)
{
    //pic::logmsg() << "queue read " << o;
    sample_bufref_t b = allocate_buffer(o,piw::tsd_time());
    add(__reader,this, b.give(), 0, 0);
    return b;
}

piw::samplearray_t::impl_t::~impl_t()
{
    pic::logmsg() << "unloading sampler";
    quit();
    fclose(fd);
}

piw::sample_t::impl_t::impl_t(const samplearrayref_t &d,unsigned s, unsigned e, unsigned ls, unsigned le, float sr, float rf, float att) : data_(d), start_(s), end_(e), loopstart_(ls), loopend_(le), samplerate_(sr), rootfreq_(rf), attenuation_(att)
{
    start_buffer_ = data_->impl()->get_block(start_/BLOCKSIZE_SAMPLES);
}

piw::samplereader_t::rimpl_t::rimpl_t(const sampleref_t &sample): sample_(sample), error_(true)
{
    array_ = sample_->data()->impl();
    start_buffer_ = sample_->impl()->start_buffer_;
    looped_ = (sample_->loopend()!=0);
    end_block_ = sample_->end()/BLOCKSIZE_SAMPLES;

    if(looped_)
    {
        loop_block1_ = sample_->loopstart()/BLOCKSIZE_SAMPLES;
        loop_block1_ = (loop_block1_-start_buffer_->block_)/BUFFER_BLOCKS;
        loop_block1_ = loop_block1_*BUFFER_BLOCKS+start_buffer_->block_;
        loop_block2_ = loop_block1_+BUFFER_BLOCKS;
        //pic::logmsg() << "start " << start_buffer_->block_ << " loop1 " << loop_block1_ << " loop2 " << loop_block2_ << " end " << end_block_;
    }
    else
    {
        loop_block1_=loop_block2_=0;
    }
}

void piw::samplereader_t::rimpl_t::queue(sample_bufref_t &next, unsigned nblk)
{
    if(nblk>end_block_)
    {
        //pic::logmsg() << "block " << nblk << " beyond end " << end_block_;
        return;
    }

    if(looped_)
    {
        if(nblk==loop_block1_)
        {
            //pic::logmsg() << "block " << nblk << " is loop1 " << loop_buffer1_.isvalid();
            if(loop_buffer1_.isvalid())
            {
                next = loop_buffer1_;
            }
            else
            {
                next=array_->queue_read(nblk);
                loop_buffer1_=next;
            }
            return;
        }

        if(nblk==loop_block2_)
        {
            //pic::logmsg() << "block " << nblk << " is loop2 " << loop_buffer2_.isvalid();
            if(loop_buffer2_.isvalid())
            {
                next = loop_buffer2_;
            }
            else
            {
                next=array_->queue_read(nblk);
                loop_buffer2_=next;
            }
            return;
        }
    }

    if(!next.isvalid() || next->block_!=nblk)
    {
        next=array_->queue_read(nblk);
    }
    else
    {
        //pic::logmsg() << "block " << nblk << " already queued ";
    }
}

const short *piw::samplereader_t::rimpl_t::bufptr0(unsigned blk, const sample_bufref_t &current, sample_bufref_t &next, sample_bufref_t &next2, sample_bufref_t &next3)
{
    const sample_bufref_t *c = &current;

    if(blk >= start_buffer_->block_ && blk<start_buffer_->block_+BUFFER_BLOCKS)
    {
        c = &start_buffer_;
    }
    else if(looped_)
    {
        if(blk>=loop_block1_ && blk<loop_block1_+BUFFER_BLOCKS)
        {
            c=&loop_buffer1_;
        }
        else if(blk>=loop_block2_ && blk<loop_block2_+BUFFER_BLOCKS)
        {
            c=&loop_buffer2_;
        }
    }

    if(!(*c).isvalid())
    {
        //pic::logmsg() << "not 1 queueing blocks " << blk;
        return 0;
    }

    unsigned bblk = (*c)->block_;

    if(blk<bblk || blk>=bblk+BUFFER_BLOCKS)
    {
        //pic::logmsg() << "not 2 queueing blocks " << blk << ' ' << bblk+BUFFER_BLOCKS << ' ' << bblk+BUFFER_BLOCKS+BUFFER_BLOCKS;
        return 0;
    }

    if((*c)->valid_)
    {
        /*
        if(blk != last_block_)
        {
            last_block_=blk;
            pic::logmsg() << "block: " << blk << " data: " << current->data_ << " buffer " << current.ptr();
        }
        */

        //pic::logmsg() << " queueing blocks " << bblk+BUFFER_BLOCKS << ' ' << bblk+BUFFER_BLOCKS+BUFFER_BLOCKS;
        queue(next,bblk+BUFFER_BLOCKS);
        queue(next2,bblk+BUFFER_BLOCKS+BUFFER_BLOCKS);
        queue(next3,bblk+BUFFER_BLOCKS+BUFFER_BLOCKS+BUFFER_BLOCKS);

        return (*c)->data_+BLOCKSIZE_SAMPLES*(blk-bblk);
    }

    return 0;
}

const short *piw::samplereader_t::rimpl_t::bufptr(unsigned offset)
{
    unsigned blk = offset/BLOCKSIZE_SAMPLES;
    unsigned ind = offset%BLOCKSIZE_SAMPLES;
    const short *ptr;

    if((ptr=bufptr0(blk,buffer1_,buffer2_,buffer3_,buffer4_))!=0)
    {
        if(!error_) pic::logmsg() << "request fetched " << blk;
        error_=true;
        return ptr+ind;
    }

    if((ptr=bufptr0(blk,buffer2_,buffer3_,buffer4_,buffer1_))!=0)
    {
        if(!error_) pic::logmsg() << "request fetched " << blk;
        error_=true;
        return ptr+ind;
    }

    if((ptr=bufptr0(blk,buffer3_,buffer4_,buffer1_,buffer2_))!=0)
    {
        if(!error_) pic::logmsg() << "request fetched " << blk;
        error_=true;
        return ptr+ind;
    }

    if((ptr=bufptr0(blk,buffer4_,buffer1_,buffer2_,buffer3_))!=0)
    {
        if(!error_) pic::logmsg() << "request fetched " << blk;
        error_=true;
        return ptr+ind;
    }

    if(error_)
    {
        error_=false;
        pic::logmsg() << "request not fetched " << blk;
    }

    return start_buffer_->data_;
}
