
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
#include <iostream>

#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include "macosx_usb_util.h"

int find_bulk_pipe(IOUSBInterfaceInterface197 **iface, unsigned name)
{
    UInt8 ne;
    int wdir = (name>=0x80)?1:0;
    int wnum = name&0x7f;

    (*iface)->GetNumEndpoints(iface,&ne);

    for(unsigned i=0;i<=ne;i++)
    {
        UInt8 dir, num, type, time; UInt16 size;
        (*iface)->GetPipeProperties(iface,i,&dir,&num,&type,&size,&time);

        if(num==wnum && dir==wdir && type==kUSBBulk)
        {
            return (int)i;
        }
    }

    return -1;
}

int find_iso_pipe(IOUSBInterfaceInterface197 **iface, unsigned name)
{
    UInt8 ne;
    int wdir = (name>=0x80)?1:0;
    int wnum = name&0x7f;

    (*iface)->GetNumEndpoints(iface,&ne);

    for(unsigned i=0;i<=ne;i++)
    {
        UInt8 dir, num, type, time; UInt16 size;
        (*iface)->GetPipeProperties(iface,i,&dir,&num,&type,&size,&time);

        if(num==wnum && dir==wdir && type==kUSBIsoc)
        {
            return (int)i;
        }
    }

    return -1;
}

IOUSBInterfaceInterface197 **open_interface(IOUSBDeviceInterface197 **device, unsigned iface)
{
    IOReturn e;
    UInt8 num_config=0;
    IOUSBConfigurationDescriptorPtr config_desc;
    IOUSBFindInterfaceRequest request;
    io_iterator_t ii;
    IOUSBInterfaceInterface197 **interface;
    io_service_t is;
    SInt32 score;
    IOCFPlugInInterface **plug;
    UInt8 id;

    e = (*device)->USBDeviceOpen(device);
    if(e==kIOReturnExclusiveAccess)
    {
        PIC_THROW("can't open: already opened by someone else");
    }
    PIC_ASSERT(e==kIOReturnSuccess);

    e = (*device)->GetNumberOfConfigurations(device, &num_config);

    if(!num_config)
    {
        (*device)->USBDeviceClose(device);
        PIC_THROW("device has no configurations!");
    }

    if((e = (*device)->GetConfigurationDescriptorPtr(device, 0, &config_desc)))
    {
        (*device)->USBDeviceClose(device);
        PIC_THROW("can't get default config descriptor");
    }

    if((e = (*device)->SetConfiguration(device, config_desc->bConfigurationValue)))
    {
        (*device)->USBDeviceClose(device);
        PIC_THROW("can't set default configuration");
    }

    request.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    request.bAlternateSetting = kIOUSBFindInterfaceDontCare;

    if((e = (*device)->CreateInterfaceIterator(device, &request, &ii)))
    {
        (*device)->USBDeviceClose(device);
        PIC_THROW("can't enumerate interfaces");
    }

    while((is = IOIteratorNext(ii)))
    {
        e=IOCreatePlugInInterfaceForService(is, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score);
        IOObjectRelease(is);
        if(e || !plug) continue;
        (*plug)->QueryInterface(plug,CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID197), (void **)&interface);
        (*plug)->Stop(plug);
        IODestroyPlugInInterface(plug);
        (*interface)->GetInterfaceNumber(interface,&id);

        if(id==iface)
        {
            (*interface)->USBInterfaceOpen(interface);
            IOObjectRelease(ii);
            return interface;
        }

        (*interface)->Release(interface);
    }

    (*device)->USBDeviceClose(device);
    IOObjectRelease(ii);
    PIC_THROW("can't find interface");
}

void close_interface(IOUSBDeviceInterface197 **device, IOUSBInterfaceInterface197 **interface)
{
    (*interface)->USBInterfaceClose(interface);
    (*interface)->Release(interface);
    (*device)->USBDeviceClose(device);
}

void close_device(IOUSBDeviceInterface197 **device)
{
    (*device)->Release(device);
}

struct __finderdata
{
    unsigned long name;
    IOUSBDeviceInterface197 **device;
};

static void __finder(void *impl_, IOUSBDeviceInterface197 **device)
{
    __finderdata *f = (__finderdata *)impl_;
    unsigned long dl;

    (*device)->GetLocationID(device,&dl);

    if(dl==f->name)
    {
        (*device)->AddRef(device);
        f->device=device;
    }
}

IOUSBDeviceInterface197 **open_device(const char *name)
{
    __finderdata f;

    f.name = strtoul(name,0,16);
    f.device=0;

    device_enumerate(0,0,__finder,&f);

    if(!f.device)
    {
        pic::msg() << "can't find device " << name << pic::hurl;
    }

    return f.device;
}

unsigned device_enumerate(unsigned long v, unsigned long p, void (*visitor)(void *, IOUSBDeviceInterface197 **), void *arg)
{
    CFMutableDictionaryRef  d;
    kern_return_t e;
    io_service_t s;
    io_iterator_t i;
    SInt32 score;
    IOCFPlugInInterface **plug;
    IOUSBDeviceInterface197 **device;
    unsigned count=0;
    mach_port_t port;

    if((e=IOMasterPort(MACH_PORT_NULL,&port)) || !port)
    {
        PIC_THROW("can't create mach port");
    }

    if(!(d = IOServiceMatching(kIOUSBDeviceClassName)))
    {
        mach_port_deallocate(mach_task_self(),port);
        PIC_THROW("can't create matching dict");
    }

    if(v)
    {
        CFDictionarySetValue(d, CFSTR(kUSBVendorName), CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &v));
        CFDictionarySetValue(d, CFSTR(kUSBProductName), CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &p));
    }

    if((e=IOServiceGetMatchingServices(port,d,&i)))
    {
        mach_port_deallocate(mach_task_self(),port);
        PIC_THROW("can't find any devices of class");
    }

    while((s=IOIteratorNext(i)))
    {
        e=IOCreatePlugInInterfaceForService(s, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score);
        IOObjectRelease(s);
        if(e || !plug) continue;
        (*plug)->QueryInterface(plug,CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID197), (void **)&device);
        (*plug)->Stop(plug);
        IODestroyPlugInInterface(plug);
        if(!device) continue;
        visitor(arg,device);
        (*device)->Release(device);
        count++;
    }

    IOObjectRelease(i);
    mach_port_deallocate(mach_task_self(),port);

    return count;
}


bool is_hispeed(IOUSBInterfaceInterface197 **i)
{
    UInt32 us;
    if((*i)->GetFrameListTime(i,&us)!=kIOReturnSuccess)
    {
        PIC_THROW("cannot determine if this is a USB 2.0 device");
    }

    return us==kUSBHighSpeedMicrosecondsInFrame;
}
