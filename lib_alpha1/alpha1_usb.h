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

#ifndef __ALPHA1_USB__
#define __ALPHA1_USB__

#include <picross/pic_stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BCTALPHA1_USBVENDOR                    0x049f
#define BCTALPHA1_USBPRODUCT                   0x505a
#define BCTALPHA1_INTERFACE                    0

#define BCTALPHA1_MSGTYPE_NULL          1
#define BCTALPHA1_MSGTYPE_KEYDOWN       3
#define BCTALPHA1_MSGTYPE_RAW           4
#define BCTALPHA1_MSGTYPE_PROCESSED     5

#define BCTALPHA1_HEADER_SIZE           2
#define BCTALPHA1_PAYLOAD_KEYDOWN       9
#define BCTALPHA1_PAYLOAD_PROCESSED     4
#define BCTALPHA1_PAYLOAD_RAW           5
#define BCTALPHA1_MSGSIZE_KEYDOWN       (BCTALPHA1_HEADER_SIZE+BCTALPHA1_PAYLOAD_KEYDOWN)
#define BCTALPHA1_MSGSIZE_PROCESSED     (BCTALPHA1_HEADER_SIZE+BCTALPHA1_PAYLOAD_PROCESSED)
#define BCTALPHA1_MSGSIZE_RAW           (BCTALPHA1_HEADER_SIZE+BCTALPHA1_PAYLOAD_RAW)

struct pik_msg_t
{
    uint8_t  type;
    uint8_t  frame;
    uint16_t timestamp;
    uint16_t payload[0];
};

#define BCTALPHA1_MSG_RAW_KEY           0
#define BCTALPHA1_MSG_RAW_I0            1
#define BCTALPHA1_MSG_RAW_I1            2
#define BCTALPHA1_MSG_RAW_I2            3
#define BCTALPHA1_MSG_RAW_I3            4

#define BCTALPHA1_MSG_PROCESSED_KEY     0
#define BCTALPHA1_MSG_PROCESSED_P       1
#define BCTALPHA1_MSG_PROCESSED_R       2
#define BCTALPHA1_MSG_PROCESSED_Y       3

#define BCTALPHA1_CALTABLE_POINTS              30
#define BCTALPHA1_CALTABLE_SIZE                (2+BCTALPHA1_CALTABLE_POINTS)
#define BCTALPHA1_CALTABLE_MIN                 0
#define BCTALPHA1_CALTABLE_MAX                 1
#define BCTALPHA1_CALTABLE_DATA                2

#define BCTALPHA1_USBENDPOINT_SENSOR_NAME      0x83
#define BCTALPHA1_USBENDPOINT_SENSOR_SIZE      512
#define BCTALPHA1_USBENDPOINT_SENSOR_FREQ      1

#define BCTALPHA1_USBCOMMAND_SETLED_REQTYPE     0x40
#define BCTALPHA1_USBCOMMAND_SETLED_REQ         0xb0

#define BCTALPHA1_USBCOMMAND_START_REQTYPE      0x40
#define BCTALPHA1_USBCOMMAND_START_REQ          0xb1

#define BCTALPHA1_USBCOMMAND_SETRAW_REQTYPE     0x40
#define BCTALPHA1_USBCOMMAND_SETRAW_REQ         0xb3

#define BCTALPHA1_USBCOMMAND_SETCOOKED_REQTYPE  0x40
#define BCTALPHA1_USBCOMMAND_SETCOOKED_REQ      0xb4

#define BCTALPHA1_USBCOMMAND_CALDATA_REQTYPE    0x40
#define BCTALPHA1_USBCOMMAND_CALDATA_REQ        0xb5

#define BCTALPHA1_USBCOMMAND_CALWRITE_REQTYPE   0x40
#define BCTALPHA1_USBCOMMAND_CALWRITE_REQ       0xb6

#define BCTALPHA1_USBCOMMAND_CALCLEAR_REQTYPE   0x40
#define BCTALPHA1_USBCOMMAND_CALCLEAR_REQ       0xb8

#define BCTALPHA1_USBCOMMAND_STOP_REQTYPE       0x40
#define BCTALPHA1_USBCOMMAND_STOP_REQ           0xbb

#define BCTALPHA1_USBCOMMAND_TEMP_REQTYPE       0x40
#define BCTALPHA1_USBCOMMAND_TEMP_REQ           0xc0


#ifdef __cplusplus
}
#endif

#endif
