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

#ifndef __MICRO_USB__
#define __MICRO_USB__

#ifdef __cplusplus
extern "C" {
#endif

#define BCTMICRO_USBVENDOR                    0xbeca
#define BCTMICRO_USBPRODUCT                   0x0101
#define BCTMICRO_INTERFACE                    0
#define BCTMICRO_SCANSIZE                     192

#define BCTMICRO_USBENDPOINT_SENSOR_NAME      0x82
#define BCTMICRO_USBENDPOINT_SENSOR_SIZE      (4*BCTMICRO_SCANSIZE)

#define BCTMICRO_CALTABLE_POINTS              30
#define BCTMICRO_CALTABLE_SIZE                (2+BCTMICRO_CALTABLE_POINTS)
#define BCTMICRO_CALTABLE_MIN                 0
#define BCTMICRO_CALTABLE_MAX                 1
#define BCTMICRO_CALTABLE_DATA                2

#define TYPE_VENDOR   0x40

#define BCTMICRO_USBCOMMAND_SETLED   0xb0
#define BCTMICRO_USBCOMMAND_START    0xb1
#define BCTMICRO_USBCOMMAND_SETMLED  0xb2  /* 1 byte (2 bits per mode key) sent in first byte of wValue (SETUPDAT[4]) */
#define BCTMICRO_USBCOMMAND_CALDATA  0xb5
#define BCTMICRO_USBCOMMAND_CALWRITE 0xb6
#define BCTMICRO_USBCOMMAND_CALREAD  0xb7
#define BCTMICRO_USBCOMMAND_CALCLEAR 0xb8
#define BCTMICRO_USBCOMMAND_STOP     0xbb
#define BCTMICRO_USBCOMMAND_DEBUG    0xc0

#ifdef __cplusplus
}
#endif

#endif
