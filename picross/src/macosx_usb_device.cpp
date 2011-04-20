
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

#include <stdlib.h>
#include <memory>

#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_cfrunloop.h>
#include <picross/pic_usb.h>
#include <picross/pic_atomic.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_time.h>

#include "macosx_usb_util.h"

#include <CoreServices/CoreServices.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

#define ISO_IN_URBS        80
#define ISO_IN_URBSIZE     8
#define ISO_IN_FUTURE      50
#define ISO_LOW            40
#define MICROFRAMES_PER_FRAME 8
#define MICROFRAME_INTERVAL (1000/MICROFRAMES_PER_FRAME)

#define ISO_OUT_URBS        80
#define ISO_OUT_FRAMES_PER_URB     4 
#define ISO_OUT_MICROFRAMES_PER_URB     (ISO_OUT_FRAMES_PER_URB*8)
#define ISO_OUT_OFFSET       4 /* ms (frames)*/
#define ISO_OUT_FUTURE       64 /* microframes */
#define ISO_OUT_STRIDE       8

#define OUTPIPE_IDLE 0
#define OUTPIPE_BUSY 1
#define OUTPIPE_START 2
#define OUTPIPE_SHUTDOWN 3

namespace
{
    struct macosx_usbdevice_t
    {
        macosx_usbdevice_t(const char *, unsigned);
        ~macosx_usbdevice_t();

        IOUSBDeviceInterface197 **device;
        IOUSBInterfaceInterface197 **interface;
        bool hispeed;
    };

    struct usburb_t: virtual public pic::lckobject_t
    {
        usburb_t();
        ~usburb_t();

        void *pipe;
        macosx_usbdevice_t *device;
        IOUSBLowLatencyIsocFrame *frame;
        unsigned char *buffer;
        UInt64 fnum;
        unsigned index;
    };

    struct macosx_usbpipe_t: virtual public pic::lckobject_t
    {
        macosx_usbpipe_t(pic::usbdevice_t::impl_t *impl,int piperef,unsigned size);
        virtual ~macosx_usbpipe_t() {}

        void kill();
        void kill_complete();
        void clear_stall();
        void abort_urbs();
        virtual void pipe_died() {}

        pic::usbdevice_t::impl_t *impl_;
        int piperef_;
        unsigned size_;
        UInt64 counter_;

        unsigned queued_;
        pic_atomic_t killed_;
        bool notified_;
        pic::gate_t gate;
    };

    struct macosx_usbpipe_in_t: macosx_usbpipe_t
    {
        macosx_usbpipe_in_t(pic::usbdevice_t::impl_t *impl,pic::usbdevice_t::in_pipe_t *pipe, int ref);
        ~macosx_usbpipe_in_t();

        void start();
        void service();
        void sleep();
        void wakeup();

        virtual void pipe_died() { pipe->pipe_died(); }

        void completed(usburb_t *, IOReturn);
        bool submit(usburb_t *);
        void init_urb(usburb_t *);

        bool poll(unsigned long long t);

        pic::usbdevice_t::in_pipe_t *pipe;

        usburb_t urbs[ISO_IN_URBS];

        unsigned long urb_queued_, urb_reaped_;
        unsigned sleeping;
        bool skipping;
        bool stalled;
        unsigned micro;
        unsigned requests;

        unsigned long long polled_;
        pic::flipflop_t<unsigned long long> urb_reaped_lbound_;
    };

    struct macosx_usbpipe_out_t: macosx_usbpipe_t
    {
        macosx_usbpipe_out_t(pic::usbdevice_t::impl_t *impl,pic::usbdevice_t::out_pipe_t *pipe, int ref);
        ~macosx_usbpipe_out_t();

        void completed(usburb_t *, IOReturn);
        bool submit(usburb_t *);
        void init_urb(usburb_t *);
        void start();
        void sleep();
        void wakeup();
        void wait();

        unsigned char *iso_write_start();
        unsigned char *iso_write_advance(unsigned n);

        pic::usbdevice_t::out_pipe_t *pipe;

        std::auto_ptr<usburb_t> urbs[ISO_OUT_URBS];
        unsigned write_urb_;
        unsigned write_frame_;
        pic_atomic_t shutdown_latch;
    };

    typedef pic::lcklist_t<macosx_usbpipe_in_t *>::lcktype pipe_list_t;
    typedef pic::flipflop_t<pipe_list_t> pipe_flipflop_t;
};

struct pic::usbdevice_t::impl_t: pic::cfrunloop_t, virtual public pic::lckobject_t
{
    impl_t(const char *, unsigned);
    ~impl_t();

    void runloop_init();
    void runloop_term();
    int runloop_cmd(void *c, void *a);
    void runloop_cmd2(void *c, void *a);
    void shutdown();
    void detach();

    void power_sleep();
    void power_wakeup();

    bool poll_pipe(unsigned long long t);

    bool add_inpipe(in_pipe_t *);
    void set_outpipe( out_pipe_t *);
    void clear_pipes();

    void ping(void *a, void *b)
    {
        if(pic_atomiccas(&cmd2_pending,0,1))
            pic::logmsg() << "pinging usb loop";
        cmd2(a,b);
    }

    void abort_urbs(int piperef);

    std::string name_;

    macosx_usbdevice_t device;
    pic::usbdevice_t::power_t *power;

    CFRunLoopSourceRef source;

    IONotificationPortRef power_notify;
    CFRunLoopSourceRef power_source;
    io_object_t power_iterator;
    io_connect_t power_port;

    pipe_flipflop_t inpipes_;
    macosx_usbpipe_out_t *outpipe;
    pic_atomic_t cmd2_pending;
};

macosx_usbpipe_t::macosx_usbpipe_t(pic::usbdevice_t::impl_t *impl,int piperef,unsigned size):
    impl_(impl),
    piperef_(piperef),
    size_(size),
    counter_(0),
    queued_(0),
    killed_(0),
    notified_(false)
{
}

void macosx_usbpipe_t::kill()
{
    if(pic_atomiccas(&killed_,0,1))
    {
        pic::logmsg() << "pipe kill";
        abort_urbs();
    }
}

void macosx_usbpipe_t::kill_complete()
{
    pic::logmsg() << "pipe shutdown";

    if(!notified_)
    {
        notified_=true;
        pipe_died();
    }

    gate.open();
}

void macosx_usbpipe_t::clear_stall()
{
    IOUSBInterfaceInterface197 **intf = impl_->device.interface;
    IOReturn e;

    if((e=(*intf)->ClearPipeStallBothEnds(intf,piperef_)) != kIOReturnSuccess)
    {
        pic::logmsg() << "clear stall failed: " << std::hex << e;
    }
}

void macosx_usbpipe_t::abort_urbs()
{
    impl_->abort_urbs(piperef_);
}

usburb_t::usburb_t(): pipe(0),device(0),frame(0),buffer(0),fnum(0)
{
}

void macosx_usbpipe_in_t::init_urb(usburb_t *u)
{
    IOUSBInterfaceInterface197 **i = impl_->device.interface;
    IOReturn e;

    e=(*i)->LowLatencyCreateBuffer(i,(void **)&u->frame,requests*sizeof(IOUSBLowLatencyIsocFrame),kUSBLowLatencyFrameListBuffer);

    if (e!=kIOReturnSuccess)
    {
        pic::msg() << "LowLatencyCreateBuffer: allocing frame list: " << requests*sizeof(IOUSBLowLatencyIsocFrame) << ' ' << std::hex << e << pic::hurl;
    }

    e=(*i)->LowLatencyCreateBuffer(i,(void **)&u->buffer,requests*size_,kUSBLowLatencyReadBuffer);

    if (e!=kIOReturnSuccess)
    {
        (*i)->LowLatencyDestroyBuffer(i,u->frame);
        pic::msg() << "LowLatencyCreateBuffer: allocing read buffer: " << requests*size_ << ' ' << std::hex << e << pic::hurl;
    }

    u->pipe=this;
    
    u->device=&impl_->device;
}

void macosx_usbpipe_out_t::init_urb(usburb_t *u)
{
    IOUSBInterfaceInterface197 **i = impl_->device.interface;
    IOReturn e;

    e=(*i)->LowLatencyCreateBuffer(i,(void **)&u->frame,ISO_OUT_MICROFRAMES_PER_URB*sizeof(IOUSBLowLatencyIsocFrame),kUSBLowLatencyFrameListBuffer);

    if (e!=kIOReturnSuccess)
    {
        pic::msg() << "LowLatencyCreateBuffer: allocing frame list: " << ISO_OUT_MICROFRAMES_PER_URB*sizeof(IOUSBLowLatencyIsocFrame) << ' ' << std::hex << e << pic::hurl;
    }

    e=(*i)->LowLatencyCreateBuffer(i,(void **)&u->buffer,ISO_OUT_MICROFRAMES_PER_URB*size_,kUSBLowLatencyWriteBuffer);

    if (e!=kIOReturnSuccess)
    {
        (*i)->LowLatencyDestroyBuffer(i,u->frame);
        pic::msg() << "LowLatencyCreateBuffer: allocing write buffer: " << ISO_OUT_MICROFRAMES_PER_URB*size_ << ' ' << std::hex << e << pic::hurl;
    }

    u->pipe=this;
    u->device=&impl_->device;
}

usburb_t::~usburb_t()
{
    if(device)
    {
        IOUSBInterfaceInterface197 **i = device->interface;

        (*i)->LowLatencyDestroyBuffer(i,frame);
        (*i)->LowLatencyDestroyBuffer(i,buffer);
    }
}

static void in_completed__(void *urb_, IOReturn e, void *)
{
    usburb_t *urb = (usburb_t *)urb_;
    macosx_usbpipe_in_t *p = (macosx_usbpipe_in_t *)urb->pipe;
    p->completed(urb,e);
}

static void out_completed__(void *urb_, IOReturn e, void *)
{
    usburb_t *urb = (usburb_t *)urb_;
    macosx_usbpipe_out_t *p = (macosx_usbpipe_out_t *)urb->pipe;
    p->completed(urb,e);
}

bool macosx_usbpipe_in_t::submit(usburb_t *u)
{
    IOUSBInterfaceInterface197 **intf = impl_->device.interface;
    IOReturn e;

    for(unsigned i=0;i<requests;i++)
    {
        u->frame[i].frStatus=kUSBLowLatencyIsochTransferKey;
        u->frame[i].frReqCount=size_;
        u->frame[i].frActCount=0;
    }

restart:

    u->fnum = counter_;

    if((e=(*intf)->LowLatencyReadIsochPipeAsync(intf,piperef_,u->buffer,u->fnum/micro,requests,1,u->frame,in_completed__,u)) != kIOReturnSuccess)
    {
        if(e==kIOReturnIsoTooOld)
        {
            AbsoluteTime time;
            UInt64 f;
            (*intf)->GetBusFrameNumber(intf,&f,&time);
            f+=ISO_IN_FUTURE;
            counter_ = f*micro;
            pic::logmsg() << "fell behind: frame count resynced to " << counter_;
            goto restart;
        }

        pic::logmsg() << "can't submit read request " << std::hex << e;
        return false;
    }

    counter_+=requests;
    return true;
}

void macosx_usbpipe_in_t::service()
{
    if(killed_)
    {
        if(queued_==0)
        {
            pic::logmsg() << "in pipe kill complete";
            kill_complete();
        }

        return;
    }

    if(sleeping==2)
    {
        return;
    }

    bool wakeup = (sleeping==1);
    if(wakeup)
    {
        if(queued_>0)
        {
            return;
        }

        sleeping=0;
        urb_queued_=0;
        urb_reaped_=0;
        pic::logmsg() << "service starting again after sleep";
    }

    unsigned long reaped = urb_reaped_;

    if(wakeup || (stalled && urb_queued_<reaped+ISO_IN_URBS))
    {
        pic::logmsg() << "(re)starting submission";
        counter_=0;
        clear_stall();

        try
        {
            pipe->pipe_started();
        }
        CATCHLOG()

        stalled=false;
    }

    while(urb_queued_<reaped+ISO_IN_URBS)
    {
        usburb_t *urb = &urbs[urb_queued_%ISO_IN_URBS];

        if(!submit(urb))
        {
            kill();
            return;
        }

        urb_queued_++;
        queued_++;
    }

    if(queued_==1)
    {
        pic::logmsg() << "pipe starvation";
        abort_urbs();
        clear_stall();

        try
        {
            pipe->pipe_stopped();
        }
        CATCHLOG()

        stalled=true;
        urb_reaped_lbound_.set(urb_queued_);
    }
}

void pic::usbdevice_t::impl_t::abort_urbs(int pipe)
{
    IOUSBInterfaceInterface197 **intf = device.interface;
    (*intf)->AbortPipe(intf,pipe);
}

void macosx_usbpipe_in_t::sleep()
{
    sleeping=2;
    abort_urbs();
}

void macosx_usbpipe_out_t::sleep()
{
    pic::logmsg() << "out pipe entering sleep";
    abort_urbs();
    pic::logmsg() << "out pipe sleeping";
}

void macosx_usbpipe_in_t::wakeup()
{
    sleeping=1;
    impl_->ping((void *)2,this);
}

void macosx_usbpipe_out_t::wakeup()
{
    pic::logmsg() << "out pipe waking up";
    start();
    pic::logmsg() << "out pipe awake";
}

void macosx_usbpipe_in_t::start()
{
    pic::logmsg() << "in pipe starting...";
    gate.shut();
    urb_queued_=0;
    urb_reaped_=0;
    counter_=0;
    notified_=false;
    pic::logmsg() << "in pipe started.";
}

void macosx_usbpipe_out_t::start()
{
    pic::logmsg() << "out pipe starting...";
    gate.shut();
    counter_=0;
    for(unsigned i=0;i<ISO_OUT_URBS;i++)
    {
        urbs[i] = std::auto_ptr<usburb_t>(new usburb_t());
        usburb_t *u = static_cast< usburb_t *> (urbs[i].get());
        u->index=i;
        init_urb(u);
        submit(u);
    }
    shutdown_latch=OUTPIPE_IDLE;
    pic::logmsg() << "out pipe started.";
}

bool pic::usbdevice_t::poll_pipe(unsigned long long time)
{
    return impl_->poll_pipe(time);
}

bool pic::usbdevice_t::impl_t::poll_pipe(unsigned long long time)
{
    bool skipped = false;
    pipe_flipflop_t::guard_t g(inpipes_);
    pipe_list_t::const_iterator i;

    for(i=g.value().begin(); i!=g.value().end(); i++)
    {
        if((*i)->poll(time))
        {
            skipped=true;
        }
    }

    return skipped;
}

bool macosx_usbpipe_in_t::poll(unsigned long long t)
{
    bool warn = false;
    pic::flipflop_t<unsigned long long>::guard_t reaped_guard(urb_reaped_lbound_);

restart:
    unsigned long queued = urb_queued_;
    unsigned long long reaped_lbound = reaped_guard.value();
    if(urb_reaped_<reaped_lbound)
    {
        urb_reaped_ = reaped_lbound;
    }

    if(killed_ || sleeping==2)
    {
        if(warn) pic::logmsg() << "queued=" << queued_ << " sleeping=" << sleeping << " killed=" << killed_;
        skipping=false;
        return false;
    }

    if(stalled || urb_reaped_>=queued)
    {
        pic::logmsg() << "poll restarting pipe";
        impl_->ping((void *)2,this);
        skipping=false;
        return false;
    }

    usburb_t *urb = &urbs[urb_reaped_%ISO_IN_URBS];

    if(polled_>=urb->fnum+requests)
    {
        pic::logmsg() << "delayed urb";
        urb_reaped_++;
        goto restart;
    }

    if(urb->fnum>polled_)
    {
        pic::logmsg() << "skipped urb";
        polled_ = urb->fnum;

        if(skipping)
        {
            return false;
        }

        skipping=true;
        return true;
    }

    unsigned i = polled_-urb->fnum;
    unsigned char *b = urb->buffer+i*size_;
    unsigned long long f0 = UnsignedWideToUInt64(AbsoluteToNanoseconds(urb->frame[0].frTimeStamp))/1000ULL;

    for(; i<requests; ++i,b+=size_,++polled_)
    {
        IOReturn s = urb->frame[i].frStatus;

        if(s==kUSBLowLatencyIsochTransferKey)
        {
            //if(warn) pic::logmsg() << "polled up to " << polled_;
            skipping=false;
            return false;
        }

        if(s!=kIOReturnSuccess && s!=kIOReturnUnderrun && s!=kIOReturnOverrun)
        {
            if(!skipping)
            {
                pipe->pipe_error(urb->fnum+i,s);
            }

            if(s==kIOReturnNotResponding)
            {
                pic::logmsg() << "fatal error in poll, killing pipe";
                kill();
            }

            polled_ = urb->fnum+requests;
            urb_reaped_++;

            if(skipping)
            {
                return false;
            }

            pic::logmsg() << "skip aborted frames";
            skipping=true;
            return true;
        }

        unsigned long long ft = f0+(i*MICROFRAME_INTERVAL);

        if(t!=0ULL && ft>t)
        {
            if(warn) pic::logmsg() << "queued=" << queued_ << " t=" << t << " ft=" << ft;
            skipping=false;
            return false;
        }

        if(urb->frame[i].frActCount>0)
        {
            pic::flipflop_t<unsigned long long>::unguard_t ug(reaped_guard);
            pipe->call_pipe_data(b,urb->frame[i].frActCount,urb->fnum+i,ft,t);
        }

        if(reaped_guard.value()!=reaped_lbound)
            goto restart;
    }

    urb_reaped_++;
    goto restart;
}

void macosx_usbpipe_in_t::completed(usburb_t *urb, IOReturn result)
{
    --queued_;

    if(!sleeping)
    {
        switch(result)
        {
            case kIOReturnSuccess:
            case kIOReturnUnderrun:
            case kIOReturnOverrun:
            case kIOReturnAborted:
                break;

            case kIOReturnIsoTooOld:
            case kIOUSBNotSent1Err:
            case kIOUSBNotSent2Err:
            case kIOReturnNoBandwidth:
                pic::logmsg() << "urb ignored with error " << std::hex << result;
                break;
            case kIOUSBWrongPIDErr:
                if(!stalled)
                {
                    pic::logmsg() << "pipe stall " << std::hex << result;
                    stalled=true;
                    sleeping=1;
                    abort_urbs();
                    try
                    {
                        pipe->pipe_stopped();
                    }
                    CATCHLOG()
                }
                break;

            default:
                pic::logmsg() << "urb completed with error " << std::hex << result;
                kill();
                break;
        }
    }

    service();
}

macosx_usbpipe_in_t::macosx_usbpipe_in_t(pic::usbdevice_t::impl_t *impl,pic::usbdevice_t::in_pipe_t *p,int r): 
    macosx_usbpipe_t(impl,r,p->in_pipe_size()),
    pipe(p), urb_queued_(0), urb_reaped_(0), sleeping(0), skipping(false), stalled(true), polled_(0), urb_reaped_lbound_(0ULL)
{
    micro = impl_->device.hispeed?MICROFRAMES_PER_FRAME:1;
    requests = micro*ISO_IN_URBSIZE;

    for(unsigned i=0;i<ISO_IN_URBS;i++)
    {
        init_urb(&urbs[i]);
    }
}

macosx_usbpipe_out_t::macosx_usbpipe_out_t(pic::usbdevice_t::impl_t *impl,pic::usbdevice_t::out_pipe_t *p,int r): 
    macosx_usbpipe_t(impl,r,p->out_pipe_size()),
    pipe(p), write_urb_(0), write_frame_(0), shutdown_latch(OUTPIPE_START)
{
    PIC_ASSERT(impl_->device.hispeed);
}

macosx_usbpipe_out_t::~macosx_usbpipe_out_t()
{
    wait();
}

void macosx_usbpipe_out_t::wait()
{
    if(shutdown_latch==OUTPIPE_SHUTDOWN)
        return;

    for(;;)
    {
        if(pic_atomiccas(&shutdown_latch,OUTPIPE_IDLE,OUTPIPE_SHUTDOWN))
            break;

        if(pic_atomiccas(&shutdown_latch,OUTPIPE_START,OUTPIPE_SHUTDOWN))
            break;

        pic_thread_yield();
    }
}

void macosx_usbpipe_out_t::completed(usburb_t *u, IOReturn e)
{
    --queued_;

    if(killed_)
    {
        if(queued_==0)
        {
            pic::logmsg() << "out pipe kill complete";
            wait();
            gate.open();
        }
        return;
    }

    if(e!=kIOReturnSuccess)
    {
        kill();
        return;
    }
    
    if(!submit(u))
    {
        kill();
    }
}

bool macosx_usbpipe_out_t::submit(usburb_t *u)
{
    IOUSBInterfaceInterface197 **intf = impl_->device.interface;
    IOReturn e;

    for(unsigned i=0;i<ISO_OUT_MICROFRAMES_PER_URB;i++)
    {
        u->frame[i].frStatus=kUSBLowLatencyIsochTransferKey;
        u->frame[i].frActCount=0;
        u->frame[i].frTimeStamp.lo=0;
        u->frame[i].frTimeStamp.hi=0;

        if((i%ISO_OUT_STRIDE)==0)
        {
            //pic::printmsg() << "complete clear " << u->index << ":" << i;
            u->frame[i].frReqCount=size_;
            memset(u->buffer+(i/ISO_OUT_STRIDE)*size_,0,size_);
        }
        else
        {
            u->frame[i].frReqCount=0;
        }
    }


restart:

    u->fnum = counter_;

    if((e=(*intf)->LowLatencyWriteIsochPipeAsync(intf,piperef_,u->buffer,u->fnum/8,ISO_OUT_MICROFRAMES_PER_URB,1,u->frame,out_completed__,u)) != kIOReturnSuccess)
    {
        if(e==kIOReturnIsoTooOld)
        {
            AbsoluteTime time;
            UInt64 f;
            (*intf)->GetBusFrameNumber(intf,&f,&time);
            f+=ISO_OUT_FUTURE;
            counter_ = f*8;
            pic::logmsg() << "Fell behind: output frame count resynced to " << counter_;
            goto restart;
        }

        pic::logmsg() << "can't submit write request " << std::hex << e;
        return false;
    }

    counter_+=ISO_OUT_MICROFRAMES_PER_URB;
    ++queued_;
    return true;
}

macosx_usbpipe_in_t::~macosx_usbpipe_in_t()
{
}

macosx_usbdevice_t::macosx_usbdevice_t(const char *name, unsigned iface)
{
    device=open_device(name);

    try
    {
        interface=open_interface(device,iface);
        hispeed=is_hispeed(interface);
        if(hispeed)
        {
            pic::logmsg() << "high speed device detected";
        }
        else
        {
            pic::logmsg() << "full speed device detected";
        }
    }
    catch(...)
    {
        close_device(device);
        throw;
    }

}

macosx_usbdevice_t::~macosx_usbdevice_t()
{
    close_interface(device,interface);
    close_device(device);
}

pic::usbdevice_t::impl_t::impl_t(const char *name, unsigned iface): pic::cfrunloop_t(0), name_(name), device(name,iface), power(0), power_port(MACH_PORT_NULL),outpipe(0),cmd2_pending(0)
{
}

void pic::usbdevice_t::impl_t::set_outpipe( out_pipe_t *p )
{
    int piperef;

    if((piperef=find_iso_pipe(device.interface,p->out_pipe_name()))<0)
    {
        pic::logmsg() << "can't find endpoint " << p->out_pipe_name();
        return;
    }
    
   try 
    {
        outpipe = new macosx_usbpipe_out_t(this,p,piperef);
    }
    catch( std::exception &e )
    {
        pic::logmsg() << " cannot add pipe" << e.what();    
    }
    catch(...)
    {
        pic::logmsg() << "cannot add pipe: unknown error";
    }

}

bool pic::usbdevice_t::impl_t::add_inpipe(in_pipe_t *pipe)
{
    //PIC_ASSERT(!isrunning());
    int piperef;

    if((piperef=find_iso_pipe(device.interface,pipe->in_pipe_name()))<0)
    {
        pic::logmsg() << "can't find endpoint " << pipe->in_pipe_name();
        return false;
    }

    
    macosx_usbpipe_in_t *p = new macosx_usbpipe_in_t(this,pipe,piperef);
    inpipes_.alternate().push_back(p);
    inpipes_.exchange();
    return true;
}

pic::usbdevice_t::impl_t::~impl_t()
{
    shutdown();
    clear_pipes();
}

void pic::usbdevice_t::impl_t::clear_pipes()
{
    while(inpipes_.alternate().size()>0)
    {
        macosx_usbpipe_in_t *p = inpipes_.alternate().front();
        inpipes_.alternate().pop_front();
        inpipes_.exchange();
        delete p;
    }

    if(outpipe)
    {
        delete outpipe;
        outpipe = 0;
    }
}

void pic::usbdevice_t::impl_t::shutdown()
{
    pic::logmsg() << "usb shutdown starting";
    if(outpipe)
    {
        outpipe->kill();

        if(!outpipe->gate.timedpass(1000000ULL))
        {
            pic::logmsg() << "outpipe shutdown timed out";
        }
    }
    {
        pipe_list_t::iterator i;

        for(i=inpipes_.alternate().begin(); i!=inpipes_.alternate().end(); i++)
        {
            (*i)->kill();

            if(!(*i)->gate.timedpass(1000000ULL))
            {
                pic::logmsg() << "inpipe shutdown timed out";
            }
        }
    }

    stop();
    pic::logmsg() << "usb shutdown complete";
}

void pic::usbdevice_t::impl_t::power_sleep()
{
    pic::logmsg() << "usb power sleep";

    {
        pipe_flipflop_t::guard_t g(inpipes_);
        pipe_list_t::const_iterator i;

        for(i=g.value().begin(); i!=g.value().end(); i++)
        {
            (*i)->sleep();
        }
    }

    if(outpipe)
        outpipe->sleep();

    if(power)
    {
        power->on_suspending();
    }
}

void pic::usbdevice_t::impl_t::power_wakeup()
{
    IOReturn e;

    pic::logmsg() << "usb power wakeup";

    if((e=(*device.device)->ResetDevice(device.device)) != kIOReturnSuccess)
    {
        pic::logmsg() << "can't reset device: " << std::hex << e;
    }
    else
        pic::logmsg() << "reset device";

    if(power)
    {
        power->on_waking();
    }

    {
        pipe_flipflop_t::guard_t g(inpipes_);
        pipe_list_t::const_iterator i;

        for(i=g.value().begin(); i!=g.value().end(); i++)
        {
            (*i)->wakeup();
        }

        if(outpipe)
            outpipe->wakeup();
    }
}

static void __power_callback(void *i_, io_service_t s, natural_t mt, void *arg)
{
    pic::usbdevice_t::impl_t *i = (pic::usbdevice_t::impl_t *)i_;

    switch(mt)
    {
        case kIOMessageSystemWillSleep:
            i->power_sleep();
            // fallthrough

        case kIOMessageCanSystemSleep:
            IOAllowPowerChange(i->power_port, (long)arg);
            break;

        case kIOMessageSystemHasPoweredOn:
            i->power_wakeup();
            break;
    }
}

void pic::usbdevice_t::impl_t::runloop_init()
{
    IOReturn e;

    if((e=(*device.interface)->CreateInterfaceAsyncEventSource(device.interface,&source)) != kIOReturnSuccess)
    {
        pic::msg() << "can't create event source for interface: " << e << pic::hurl;
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);

    if(power)
    {
        power_port = IORegisterForSystemPower(this, &power_notify, __power_callback, &power_iterator);
        if(power_port != MACH_PORT_NULL)
        {
            power_source = IONotificationPortGetRunLoopSource(power_notify);
            CFRunLoopAddSource(CFRunLoopGetCurrent(), power_source, kCFRunLoopDefaultMode);
            pic::msg() << "registered for power status" << pic::log;
        }
        else
        {
            pic::msg() << "unable to register for power status" << pic::log;
        }
    }

    try
    {
        {
            pipe_flipflop_t::guard_t g(inpipes_);
            pipe_list_t::const_iterator i;

            for(i=g.value().begin(); i!=g.value().end(); i++)
            {
                (*i)->start();
            }

            if(outpipe)
            {
                outpipe->start();
            }
        }
    }
    catch(...)
    {
        runloop_term();
        throw;
    }
}

void pic::usbdevice_t::impl_t::runloop_term()
{
    if(power_port != MACH_PORT_NULL)
    {
        IODeregisterForSystemPower(&power_iterator);
        IOServiceClose(power_port);
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), power_source, kCFRunLoopDefaultMode);
    }

    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
}

void pic::usbdevice_t::control_in(unsigned char req_type, unsigned char req, unsigned short value, unsigned short index, void *buffer, unsigned buflen, unsigned timeout)
{
    IOUSBDevRequestTO ctrl;
    IOReturn e;

    ctrl.bmRequestType = req_type;
    ctrl.bRequest = req;
    ctrl.wValue = value;
    ctrl.wIndex = index;
    ctrl.wLength = buflen;
    ctrl.pData = buffer;
    ctrl.noDataTimeout = 0;
    ctrl.completionTimeout = timeout;

    if((e=(*impl_->device.device)->DeviceRequestTO(impl_->device.device,&ctrl)))
    {
        pic::msg() << "control_in err=" << std::hex << e << " req " << (int)req_type << ":" << (int)req << pic::hurl;
    }
}

void pic::usbdevice_t::control_out(unsigned char req_type, unsigned char req, unsigned short value, unsigned short index, const void *buffer, unsigned buflen, unsigned timeout)
{
    IOUSBDevRequestTO ctrl;
    IOReturn e;

    ctrl.bmRequestType = req_type;
    ctrl.bRequest = req;
    ctrl.wValue = value;
    ctrl.wIndex = index;
    ctrl.wLength = buflen;
    ctrl.pData = (void *)buffer;
    ctrl.noDataTimeout = 0;
    ctrl.completionTimeout = timeout;

    if((e=(*impl_->device.device)->DeviceRequestTO(impl_->device.device,&ctrl)))
    {
        pic::msg() << "control_out err=" << std::hex << e << " req " << (int)req_type << ":" << (int)req << pic::hurl;
    }
}

void pic::usbdevice_t::control(unsigned char req_type, unsigned char req, unsigned short value, unsigned short index, unsigned timeout)
{
    IOUSBDevRequestTO ctrl;
    IOReturn e;

    ctrl.bmRequestType = req_type;
    ctrl.bRequest = req;
    ctrl.wValue = value;
    ctrl.wIndex = index;
    ctrl.wLength = 0;
    ctrl.pData = 0;
    ctrl.noDataTimeout = 0;
    ctrl.completionTimeout = timeout;

    if((e=(*impl_->device.device)->DeviceRequestTO(impl_->device.device,&ctrl)))
    {
        pic::msg() << "control err=" << std::hex << e << " req " << (int)req_type << ":" << (int)req << pic::hurl;
    }
}

void pic::usbdevice_t::bulk_write(unsigned name, const void *data, unsigned len, unsigned timeout)
{
    int ref;
    IOReturn e;

    if((ref=find_bulk_pipe(impl_->device.interface,name))<0)
    {
        pic::msg() << "can't find bulk endpoint " << name << pic::hurl;
    }

    e = (*impl_->device.interface)->WritePipeTO(impl_->device.interface,ref,(void *)data,len,0,timeout);

    if(e!=0)
    {
        pic::msg() << "write pipe err=" << std::hex << e << pic::hurl;
    }
}

int pic::usbdevice_t::impl_t::runloop_cmd(void *c, void *a)
{
    return 0;
}

void pic::usbdevice_t::impl_t::runloop_cmd2(void *c, void *a)
{
    if(c==(void *)2)
    {
        if(pic_atomiccas(&cmd2_pending,1,0))
        {
            macosx_usbpipe_in_t *pipe = (macosx_usbpipe_in_t *)a;
            pic::logmsg() << "usb pipe wakeup";
            pipe->service();
        }
    }
}

pic::usbdevice_t::usbdevice_t(const char *name, unsigned iface)
{
    impl_ = new pic::usbdevice_t::impl_t(name,iface);
}

const char *pic::usbdevice_t::name()
{
    return impl_->name_.c_str();
}

void pic::usbdevice_t::start_pipes()
{
    impl_->run();
}

void pic::usbdevice_t::stop_pipes()
{
    impl_->shutdown();
}

pic::usbdevice_t::~usbdevice_t()
{
    delete impl_;
}

bool pic::usbdevice_t::add_inpipe(in_pipe_t *pipe)
{
    return impl_->add_inpipe(pipe);
}

void pic::usbdevice_t::set_outpipe( out_pipe_t *p)
{
    impl_->set_outpipe(p);
}

void pic::usbdevice_t::set_power_delegate(power_t *p)
{
    impl_->power = p;
}

void pic::usbdevice_t::impl_t::detach()
{
    pic::logmsg() << "detaching client";
    clear_pipes();
    power = 0;
    pic::logmsg() << "done detaching client";
}

void pic::usbdevice_t::detach()
{
    impl_->detach();
}

unsigned char *macosx_usbpipe_out_t::iso_write_start()
{
    // find current write position in buffer
    unsigned last_status = kUSBLowLatencyIsochTransferKey;
    for(;;)
    {
        unsigned status = urbs[write_urb_]->frame[write_frame_].frStatus;
        if(status!=0 && status!=kUSBLowLatencyIsochTransferKey)
        {
            return 0;
        }

        if(last_status==0 && status==kUSBLowLatencyIsochTransferKey)
            break;

        last_status=status;
        iso_write_advance(ISO_OUT_STRIDE);
    }

    return iso_write_advance(8*ISO_OUT_OFFSET);
}

unsigned char *macosx_usbpipe_out_t::iso_write_advance(unsigned n)
{
    unsigned f=(write_urb_*ISO_OUT_MICROFRAMES_PER_URB)+write_frame_+n;
    write_urb_=(f/ISO_OUT_MICROFRAMES_PER_URB)%ISO_OUT_URBS;
    write_frame_=f%ISO_OUT_MICROFRAMES_PER_URB;
    return urbs[write_urb_]->buffer+(write_frame_/ISO_OUT_STRIDE)*size_;
}

pic::usbdevice_t::iso_out_guard_t::iso_out_guard_t(usbdevice_t *d): impl_(d->impl_), current_(0)
{
    if(!impl_->outpipe)
    {
        return;
    }

    if(impl_->outpipe->killed_)
    {
        return;
    }

    if(!pic_atomiccas(&impl_->outpipe->shutdown_latch,OUTPIPE_IDLE,OUTPIPE_BUSY))
    {
        return;
    }

    current_ = impl_->outpipe->iso_write_start();
}

pic::usbdevice_t::iso_out_guard_t::~iso_out_guard_t()
{
    if(impl_->outpipe)
        pic_atomiccas(&impl_->outpipe->shutdown_latch,OUTPIPE_BUSY,OUTPIPE_IDLE);
}

unsigned char *pic::usbdevice_t::iso_out_guard_t::advance()
{
    PIC_ASSERT(impl_->outpipe);
    current_=impl_->outpipe->iso_write_advance(ISO_OUT_STRIDE);
    return current_;
}

