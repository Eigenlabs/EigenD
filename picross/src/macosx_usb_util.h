
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

#ifndef PIU_SRC_MACOSX_ENUM_H
#define PIU_SRC_MACOSX_ENUM_H

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <CoreFoundation/CoreFoundation.h>

unsigned device_enumerate(unsigned long v, unsigned long p, void (*visitor)(void *, IOUSBDeviceInterface197 **), void *arg);
int find_iso_pipe(IOUSBInterfaceInterface197 **iface, unsigned name);
int find_bulk_pipe(IOUSBInterfaceInterface197 **iface, unsigned name);
IOUSBInterfaceInterface197 **open_interface(IOUSBDeviceInterface197 **device, unsigned iface);
void close_interface(IOUSBDeviceInterface197 **device, IOUSBInterfaceInterface197 **interface);
IOUSBDeviceInterface197 **open_device(const char *name);
void close_device(IOUSBDeviceInterface197 **);
bool is_hispeed(IOUSBInterfaceInterface197 **);

#endif
