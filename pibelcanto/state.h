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

#ifndef __PIBELCANTO_STATE__
#define __PIBELCANTO_STATE__

#include <pibelcanto/link.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup bct_mtype request type codes
 * @ingroup bct
 * @{
 */

#define BCTMTYPE_DATA_REQ   0x00
#define BCTMTYPE_DATA_EVT   0x01
#define BCTMTYPE_DATA_SET   0x03
#define BCTMTYPE_TREE_REQ   0x04
#define BCTMTYPE_TREE_EVT   0x05
#define BCTMTYPE_FAST_REQ   0x06
#define BCTMTYPE_FAST_EVT   0x07
#define BCTMTYPE_TREE_SET   0x08
#define BCTMTYPE_IDNT_EVT   0x09
#define BCTMTYPE_IDNT_REQ   0x0a

#define BCTMTYPE_RPC_REQ    0x09
#define BCTMTYPE_RPC_RSP    0x0a
#define BCTMTYPE_RPC_ACK    0x0b
#define BCTMTYPE_RPC_GET    0x0c
#define BCTMTYPE_RPC_DUN    0x0d

/**
 * @}
 */

/**
 * @defgroup bct_vtype value type codes
 * @ingroup bct
 * @{
 */

#define BCTVTYPE_NULL       0x00 /**< Null Data */
#define BCTVTYPE_PATH       0x01 /**< Path; sequence of BctU8 */
#define BCTVTYPE_STRING     0x02 /**< String; Sequence of ascii octets */
#define BCTVTYPE_DOUBLE     0x03 /**< Float Array; precision then floats */
#define BCTVTYPE_FLOAT      0x04 /**< control then mantissa */
#define BCTVTYPE_INT        0x05 /**< 32 bit integer; big endian */
#define BCTVTYPE_BOOL       0x06 /**< boolean; length=0 means false */
#define BCTVTYPE_BLOB       0x07 /**< binary; sequence of BctU8 */
#define BCTVTYPE_DICT       0x08 /**< dictionary; mapping of string to data (wire rep) */

#define BCTVTYPE_RTRANSIENT 0x04 /**< state variable children are not state managed */
#define BCTVTYPE_LIST       0x10 /**< state variable is list */
#define BCTVTYPE_TRANSIENT  0x20 /**< state variable is not state managed */
#define BCTVTYPE_READONLY   0x40 /**< state variable is readonly */
#define BCTVTYPE_FAST       0x80 /**< state variable has fast data */

/**
 * @}
 */

#define BCTLIMIT_PATHMIN    1 /**< smallest path component */
#define BCTLIMIT_PATHMAX    255 /**< largest path component */
#define BCTLIMIT_PATHLEN    64 /**< length of path */
#define BCTLIMIT_DATA       (BCTLINK_MAXPAYLOAD-100) /**< largest data (wire rep) */

#define BCTUNIT_RATIO       0x00 /** ratiometric -1:0:1 */
#define BCTUNIT_GLOBAL      0x10 /** generally matching unit */
#define BCTUNIT_HZ          0x20 /** frequency: hz */
#define BCTUNIT_SECONDS     0x30 /** time: seconds */
#define BCTUNIT_BEATS       0x40 /** time: beats */
#define BCTUNIT_CENTS       0x60 /** notes in cents */
#define BCTUNIT_BPM         0x70 /** tempo in bpm */

#define BCTSTATUS_OFF              0
#define BCTSTATUS_ACTIVE           1
#define BCTSTATUS_INACTIVE         2
#define BCTSTATUS_UNKNOWN          3
#define BCTSTATUS_MIXED            4
#define BCTSTATUS_BLINK            5
#define BCTSTATUS_SELECTOR_ON      6
#define BCTSTATUS_SELECTOR_OFF     7
#define BCTSTATUS_CHOOSE_AVAILABLE 8
#define BCTSTATUS_CHOOSE_USED      9
#define BCTSTATUS_CHOOSE_ACTIVE    10

#define CLR_OFF      0
#define CLR_GREEN    1
#define CLR_RED      2
#define CLR_ORANGE   3
#define CLR_MIXED    4

#ifdef __cplusplus
}
#endif

#endif
