
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

#include <picross/pic_usb.h>
#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_cfrunloop.h>

#include "macosx_usb_util.h"

#include <cstdio>
#include <IOKit/IOMessage.h>

struct pic::usbenumerator_t::impl_t: public pic::cfrunloop_t
{
    impl_t(unsigned short, unsigned short, const pic::f_string_t &, const pic::f_string_t &);
    ~impl_t();

    void runloop_init();
    void runloop_term();
    void runloop_start();

    static void device_attached(void *, io_iterator_t);

    SInt32 vendor_;
    SInt32 product_;
    pic::f_string_t delegate_;
    pic::f_string_t gone_;
    pic::mutex_t lock_;

    IONotificationPortRef notification_port_;
    CFRunLoopSourceRef notification_source_;

    io_iterator_t iterator_;
    io_object_t notification_;
};

pic::usbenumerator_t::impl_t::impl_t(unsigned short v, unsigned short p, const pic::f_string_t &d, const pic::f_string_t &g): pic::cfrunloop_t(0), vendor_(v), product_(p)
{
    delegate_ = d;
    gone_ = g;
}

pic::usbenumerator_t::impl_t::~impl_t()
{
    stop();
}

void pic::usbenumerator_t::impl_t::runloop_init()
{
    mach_port_t master_port;
    CFMutableDictionaryRef matching_dict;
    kern_return_t e;

    if((e=IOMasterPort(MACH_PORT_NULL,&master_port)))
    {
        pic::msg() << "can't create IO master port: " << std::hex << e << pic::hurl;
    }

    matching_dict = IOServiceMatching(kIOUSBDeviceClassName);
    if(!matching_dict)
    {
        mach_port_deallocate(mach_task_self(), master_port);
        pic::msg() << "can't create USB matching dict" << pic::hurl;
        return;
    }

    CFDictionarySetValue(matching_dict, CFSTR(kUSBVendorName), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor_));
    CFDictionarySetValue(matching_dict, CFSTR(kUSBProductName), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &product_));

    notification_port_ = IONotificationPortCreate(master_port);
    notification_source_ = IONotificationPortGetRunLoopSource(notification_port_);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), notification_source_, kCFRunLoopDefaultMode);

    if((e=IOServiceAddMatchingNotification(notification_port_, kIOFirstMatchNotification, matching_dict, device_attached, this, &iterator_)))
    {
        mach_port_deallocate(mach_task_self(), master_port);
        pic::msg() << "can't create IO iterator: " << std::hex << e << pic::hurl;
    }

}

void pic::usbenumerator_t::impl_t::runloop_start()
{
    device_attached(this,iterator_);
}


void pic::usbenumerator_t::impl_t::runloop_term()
{
    IOObjectRelease(iterator_);
}

namespace
{
    struct notify_context_t
    {
        notify_context_t(pic::usbenumerator_t::impl_t *i, const char *n): impl_(i),name_(n)
        {
        }

        static void notify(void *n_, io_service_t s, natural_t t, void *a)
        {
            notify_context_t *n = (notify_context_t *)n_;
            if(t==kIOMessageServiceIsTerminated)
            {
                try
                {
                    pic::mutex_t::guard_t g(n->impl_->lock_);
                    n->impl_->gone_(n->name_.c_str());
                    delete n;
                }
                CATCHLOG()
            }
        }

        pic::usbenumerator_t::impl_t *impl_;
        std::string name_;
        io_object_t obj_;
    };
}

void pic::usbenumerator_t::impl_t::device_attached(void *impl_, io_iterator_t i)
{
    pic::usbenumerator_t::impl_t *impl = (pic::usbenumerator_t::impl_t *)impl_;
    io_service_t s;
    kern_return_t e;
    SInt32 score;
    IOCFPlugInInterface **plug;
    IOUSBDeviceInterface **device;
    char name[10];
    unsigned long location;

    while((s=IOIteratorNext(i)))
    {
        e=IOCreatePlugInInterfaceForService(s, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score);
        if(e || !plug)
        {
            IOObjectRelease(s);
            continue;
        }

        (*plug)->QueryInterface(plug,CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID197), (void **)&device);
        (*plug)->Stop(plug);
        IODestroyPlugInInterface(plug);
        if(!device)
        {
            IOObjectRelease(s);
            continue;
        }
        (*device)->GetLocationID(device,&location);
        sprintf(name,"%08lx",location);

        notify_context_t *n = new notify_context_t(impl,name);
        e=IOServiceAddInterestNotification(impl->notification_port_,s,kIOGeneralInterest,&notify_context_t::notify,n,&(n->obj_));

        if(e)
        {
            pic::logmsg() << "cannot register for notifications: " << e;
            delete n;
        }

        IOObjectRelease(s);

        try
        {
            pic::mutex_t::guard_t g(impl->lock_);
            impl->delegate_(name);
        }
        CATCHLOG()

        (*device)->Release(device);
    }
}

static void _enumerate_visitor_(void *callback_, IOUSBDeviceInterface197 **device)
{
    char buffer[10];
    unsigned long name;

    (*device)->GetLocationID(device,&name);
    sprintf(buffer,"%08lx",name);

    try
    {
        (*(pic::f_string_t *)callback_)(buffer);
    }
    CATCHLOG()
}

unsigned pic::usbenumerator_t::enumerate(unsigned short vend, unsigned short prod, const pic::f_string_t &callback)
{
    return device_enumerate(vend,prod,_enumerate_visitor_, (void *)&callback);
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const pic::f_string_t &d)
{
    impl_ = new impl_t(v,p,d,pic::f_string_t());
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const pic::f_string_t &d, const pic::f_string_t &g)
{
    impl_ = new impl_t(v,p,d,g);
}

pic::usbenumerator_t::~usbenumerator_t()
{
    delete impl_;
}

void pic::usbenumerator_t::start()
{
    impl_->run();
}

void pic::usbenumerator_t::stop()
{
    impl_->stop();
}

int pic::usbenumerator_t::gc_traverse(void *v, void *a) const
{
    return impl_->delegate_.gc_traverse(v,a);
}

int pic::usbenumerator_t::gc_clear()
{
    pic::mutex_t::guard_t g(impl_->lock_);
    return impl_->delegate_.gc_clear();
}
