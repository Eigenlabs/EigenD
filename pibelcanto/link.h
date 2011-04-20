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

#ifndef __PIBELCANTO_LINK__
#define __PIBELCANTO_LINK__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup bct_link link layer values
 * @ingroup bct
 * @{
 */

#define BCTLINK_GROUP_SIZE      42

#define BCTLINK_MAGIC1VAL       0xbe
#define BCTLINK_MAGIC2VAL       0xca

#define BCTLINK_MAGIC1          0
#define BCTLINK_MAGIC2          1
#define BCTLINK_LEN_HI          2
#define BCTLINK_LEN_LO          3
#define BCTLINK_NAMESPACE       4
#define BCTLINK_GROUP_LEN       5
#define BCTLINK_GROUP           6
#define BCTLINK_HEADER          (BCTLINK_GROUP+BCTLINK_GROUP_SIZE)

#define BCTLINK_MAXPACKET       49152 /**< maximum packet length */
#define BCTLINK_MAXPAYLOAD      (BCTLINK_MAXPACKET-BCTLINK_HEADER) /**< absolute maximum payload length */
#define BCTLINK_SMALLPAYLOAD    1200 /**< normal maximum payload length */
#define BCTLINK_SMALLTREE       200 /**< normal maximum payload length */

#define BCTLINK_NAMESPACE_ROUTE 0
#define BCTLINK_NAMESPACE_CLOCK 1
#define BCTLINK_NAMESPACE_INDEX 2
#define BCTLINK_NAMESPACE_FAST  3
#define BCTLINK_NAMESPACE_SLOW  4
#define BCTLINK_NAMESPACE_RPC   5

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
