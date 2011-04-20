
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
#include <errno.h>
#include <memory>

#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>
#include <picross/pic_stack.h>

#include <linux/usbdevice_fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <inttypes.h>
#include <cstring>

#define ISO_IN_URBS    50
#define ISO_IN_PACKETS 8

namespace
{
    struct usbpipe_t;
    struct usburb_t;

    struct linux_usbdevice_t
    {
        linux_usbdevice_t(const char *, unsigned, pic::usbdevice_t *);
        ~linux_usbdevice_t();
        int fd() { return fd_; }
        int fd_;
        pic::usbdevice_t *device_;
        bool stopping_;
    };

    struct usbpipe_t
    {
        usbpipe_t(linux_usbdevice_t *dev,unsigned name,unsigned size);
        virtual ~usbpipe_t();

        void submit(usburb_t *);
        unsigned count() { return count_; }
        unsigned name() { return piperef_; }
        unsigned size() { return size_;} 
        virtual void completed(usburb_t *) = 0;
        virtual void pipe_died() {}
        unsigned get() { return piperef_; }
        linux_usbdevice_t *device_;
        unsigned piperef_;
        unsigned size_;
        int count_;
    };

    struct usburb_t: pic::stacknode_t
    {
        usburb_t(usbpipe_t *pipe, unsigned pktsize);
        virtual ~usburb_t();
        void init(unsigned size);

        void completed() { pipe->completed(this); }
        void submit() { pipe->submit(this); }

        usbpipe_t *pipe;
        struct usbdevfs_urb urb;
        struct usbdevfs_iso_packet_desc iso[ISO_IN_PACKETS];
        unsigned char *buffer;
    };

    struct usbpipe_in_t: usbpipe_t
    {
        usbpipe_in_t(linux_usbdevice_t *dev, pic::usbdevice_t::in_pipe_t *pipe): usbpipe_t(dev,pipe->in_pipe_name(),pipe->in_pipe_size()), pipe_(pipe), frame_(0ULL)
        {
            for(unsigned i=0;i<ISO_IN_URBS;i++)
            {
                urbs_[i] = std::auto_ptr<usburb_t>(new usburb_t(this,size_));
            }
        }

        void pipe_died() { pipe_->pipe_died(); }
        virtual void completed(usburb_t *);
        void start();

        pic::usbdevice_t::in_pipe_t *pipe_;
        unsigned long long frame_;
        std::auto_ptr<usburb_t> urbs_[ISO_IN_URBS];
    };

    struct usbpipe_out_t: usbpipe_t, pic::stack_t
    {
        usbpipe_out_t(linux_usbdevice_t *dev, pic::usbdevice_t::out_pipe_t *pipe): usbpipe_t(dev,pipe->out_pipe_name(),pipe->out_pipe_size())
        {
            for(unsigned i=0; i<ISO_IN_URBS; i++)
            {
                urbs_[i] = std::auto_ptr<usburb_t>(new usburb_t(this,size_));
                free_.push(urbs_[i].get());
            }
        }

        virtual void completed(usburb_t *u)
        {
            printf("urb completed\n");
            free_.push(u);
        }

        pic::stack_t free_;
        std::auto_ptr<usburb_t> urbs_[ISO_IN_URBS];
    };

    typedef pic::lcklist_t<usbpipe_in_t *>::lcktype pipe_list_t;
    typedef pic::flipflop_t<pipe_list_t> pipe_flipflop_t;
};

struct pic::usbdevice_t::impl_t
{
    impl_t(const char *, unsigned, pic::usbdevice_t *);
    ~impl_t();

    bool poll_pipe(unsigned long long);
    bool add_inpipe(in_pipe_t *p);
    void set_outpipe(out_pipe_t *p);
    void start_pipes();

    linux_usbdevice_t device_;
    pipe_flipflop_t inpipes_;
    std::auto_ptr<usbpipe_out_t> pipe_out_;
};

usburb_t::usburb_t(usbpipe_t *p, unsigned s): pipe(p)
{
    buffer = new unsigned char[s*ISO_IN_PACKETS];
    init(s);
}

void usburb_t::init(unsigned s)
{
    bool in = (pipe->name()&0x80)!=0;
    urb.type=USBDEVFS_URB_TYPE_ISO;
    urb.endpoint=pipe->name();
    urb.status=0x00;
    urb.flags=USBDEVFS_URB_ISO_ASAP;
    urb.buffer=buffer;
    urb.buffer_length=s*ISO_IN_PACKETS;
    urb.actual_length=in?0:s*ISO_IN_PACKETS;
    urb.start_frame=0;
    urb.number_of_packets=ISO_IN_PACKETS;
    urb.error_count=0;
    urb.signr=0;
    urb.usercontext=this;
    for(unsigned i=0; i<ISO_IN_PACKETS; ++i)
        iso[i].length=s;
}

usburb_t::~usburb_t()
{
    delete[] buffer;
}

void usbpipe_t::submit(usburb_t *urb)
{
    if(device_)
    {
        if(ioctl(device_->fd(),USBDEVFS_SUBMITURB,&urb->urb) < 0)
        {
            printf("num packets in urb=%d\n",urb->urb.number_of_packets);
            perror("cannot submit frame");
            //pic::msg() << "can't submit frame " << errno << pic::log;
            pipe_died();
            device_=0;
            return;
        }

        count_++;
        //printf("submit count %d\n",count_);
    }
}

void usbpipe_in_t::start()
{
    count_=0;

    for(unsigned i=0;i<ISO_IN_URBS;i++)
    {
        submit(urbs_[i].get());
    }
}

void usbpipe_in_t::completed(usburb_t *urb)
{
    count_--;

    if(device_==0) return;

    for(unsigned i=0; i<ISO_IN_PACKETS; ++i)
    {
        if(urb->iso[i].actual_length)
        {
            pipe_->call_pipe_data(urb->buffer+i*size_,urb->iso[i].actual_length,frame_++,pic_microtime(),pic_microtime());
        }
    }

    if(!device_->stopping_)
    {
        urb->submit();
    }
}

usbpipe_t::usbpipe_t(linux_usbdevice_t *dev, unsigned name, unsigned size): device_(dev)
{
    piperef_=name;
    size_=size;
}

usbpipe_t::~usbpipe_t()
{
}

linux_usbdevice_t::linux_usbdevice_t(const char *name, unsigned iface, pic::usbdevice_t *dev) : device_(dev), stopping_(false)
{
    printf("opening %s (%d)\n",name,iface);
    struct usbdevfs_setinterface setinterface;
    int claiminterface=iface;

    if((fd_=open(name,O_RDWR))<0)
    {
        PIC_THROW("Can't open usb device");
    }

    if(ioctl(fd_, USBDEVFS_CLAIMINTERFACE, (char *)&claiminterface)<0)
    {
        close(fd_);
        PIC_THROW("Can't CLAIMINTERFACE");
    }

    setinterface.interface=0;
    setinterface.altsetting=0;

    if(ioctl(fd_, USBDEVFS_SETINTERFACE, (char *)&setinterface)<0)
    {
        close(fd_);
        PIC_THROW("Can't SETINTERFACE");
    }
}

linux_usbdevice_t::~linux_usbdevice_t()
{
    stopping_ = true;
    close(fd_);
}

pic::usbdevice_t::impl_t::impl_t(const char *name, unsigned iface, pic::usbdevice_t *dev):  device_(name,iface,dev), pipe_out_(0)
{
}

bool pic::usbdevice_t::add_inpipe(in_pipe_t *p)
{
    return impl_->add_inpipe(p);
}

void pic::usbdevice_t::set_outpipe(out_pipe_t *p)
{
    impl_->set_outpipe(p);
}

bool pic::usbdevice_t::impl_t::add_inpipe(in_pipe_t *p)
{
    try
    {
        usbpipe_in_t *pipe  = new usbpipe_in_t(&device_,p);
        inpipes_.alternate().push_back(pipe);
        inpipes_.exchange();
        return true;
    }
    catch(std::exception &e)
    {
        pic::logmsg() << "cannot add pipe: " << e.what();
        return false;
    }
    catch(...)
    {
        pic::logmsg() << "cannot add pipe: unknown error";
        return false;
    }
}

void pic::usbdevice_t::impl_t::set_outpipe(out_pipe_t *p)
{
    try
    {
        pipe_out_  = std::auto_ptr<usbpipe_out_t>(new usbpipe_out_t(&device_,p));
    }
    catch(std::exception &e)
    {
        pic::logmsg() << "cannot add pipe: " << e.what();
    }
    catch(...)
    {
        pic::logmsg() << "cannot add pipe: unknown error";
    }
}

pic::usbdevice_t::impl_t::~impl_t()
{
}

bool pic::usbdevice_t::impl_t::poll_pipe(unsigned long long t)
{
    struct usbdevfs_urb *urb1;
    usburb_t *urb2;

    while(ioctl(device_.fd(), USBDEVFS_REAPURBNDELAY, &urb1) >= 0)
    {
        urb2=(usburb_t *)(urb1->usercontext);
        urb2->completed();
    }

    return false;
}

pic::usbdevice_t::usbdevice_t(const char *name, unsigned iface)
{
    impl_ = new usbdevice_t::impl_t(name,iface,this);
}

void pic::usbdevice_t::start_pipes()
{
    impl_->start_pipes();
}

void pic::usbdevice_t::impl_t::start_pipes()
{
    device_.stopping_ = false;
    pipe_list_t::const_iterator i;

    for(i=inpipes_.alternate().begin(); i!=inpipes_.alternate().end(); i++)
    {
        (*i)->pipe_->pipe_started();
        (*i)->start();
    }
    pic::logmsg() << "pipes started" ; 
}

void pic::usbdevice_t::stop_pipes()
{
    if(!impl_->device_.stopping_)
    {
        impl_->device_.stopping_ = true;
        pic::logmsg() << "pipes stopping" ; 
    }
}

pic::usbdevice_t::~usbdevice_t()
{
    delete impl_;
}

bool pic::usbdevice_t::poll_pipe(unsigned long long t)
{
    return impl_->poll_pipe(t);
}

void pic::usbdevice_t::control_in(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, void *buffer, unsigned len, unsigned timeout)
{
    struct usbdevfs_ctrltransfer ctrl;

    ctrl.bRequestType = type;
    ctrl.bRequest = req;
    ctrl.wValue = val;
    ctrl.wIndex = ind;
    ctrl.wLength = len;
    ctrl.timeout = timeout;
    ctrl.data = buffer;

    if(ioctl(impl_->device_.fd(), USBDEVFS_CONTROL, &ctrl) < 0)
    {
        PIC_THROW("can't do control request");
    }
}

void pic::usbdevice_t::control_out(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, const void *buffer, unsigned len, unsigned timeout)
{
    struct usbdevfs_ctrltransfer ctrl;

    ctrl.bRequestType = type;
    ctrl.bRequest = req;
    ctrl.wValue = val;
    ctrl.wIndex = ind;
    ctrl.wLength = len;
    ctrl.timeout = timeout;
    ctrl.data = (void *)buffer;

    int rc = ioctl(impl_->device_.fd(), USBDEVFS_CONTROL, &ctrl);
    if(rc < 0 || (unsigned)rc != len)
    {
        PIC_THROW("can't do control request");
    }
}

void pic::usbdevice_t::control(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, unsigned timeout)
{
    struct usbdevfs_ctrltransfer ctrl;

    ctrl.bRequestType = type;
    ctrl.bRequest = req;
    ctrl.wValue = val;
    ctrl.wIndex = ind;
    ctrl.wLength = 0;
    ctrl.timeout = timeout;
    ctrl.data = 0;

    if(ioctl(impl_->device_.fd(), USBDEVFS_CONTROL, &ctrl) < 0)
    {
        PIC_THROW("can't do control request");
    }
}

void pic::usbdevice_t::bulk_write(unsigned name, const void *data, unsigned len, unsigned timeout)
{
    struct usbdevfs_bulktransfer bulk;

    bulk.ep = name;
    bulk.len = len;
    bulk.timeout = timeout;
    bulk.data = (void *)data;

    int rc = ioctl(impl_->device_.fd(), USBDEVFS_BULK, &bulk);
    if(rc < 0)
    {
        perror("ioctl");
        PIC_THROW("can't do bulk write");
    }
}

/*
void pic::usbdevice_t::iso_write(const void *data,unsigned len)
{
    PIC_ASSERT(impl_->pipe_out_->get());
    PIC_ASSERT(len<=impl_->pipe_out_->size() );
    usburb_t *u = static_cast<usburb_t *>(impl_->pipe_out_->free_.pop());
    PIC_ASSERT(u);
    memcpy(u->buffer,data,len);
    u->init(len);
    u->submit();
}
*/

pic::usbdevice_t::iso_out_guard_t::iso_out_guard_t(usbdevice_t *d): impl_(d->impl_), current_(0)
{
}

pic::usbdevice_t::iso_out_guard_t::~iso_out_guard_t()
{
}

unsigned char *pic::usbdevice_t::iso_out_guard_t::advance()
{
    return current_;
}

const char *pic::usbdevice_t::name()
{
    return "none";
}

void pic::usbdevice_t::set_power_delegate(power_t *p)
{
}

void pic::usbdevice_t::detach()
{
}
