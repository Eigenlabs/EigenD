/*
 *  Copyright (C) 2004 Steve Harris
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  $Id$
 */

#ifndef LO_H
#define LO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file lo.h The liblo main headerfile and high-level API functions.
 */

#include "lo_endian.h"
#include "lo_types.h"
#include "lo_osc_types.h"
#include "lo_errors.h"
#include "lo_lowlevel.h"

/**
 * \defgroup liblo High-level OSC API
 *
 * Defines the high-level API functions necessary to implement OSC support.
 * Should be adequate for most applications, but if you require lower level
 * control you can use the functions defined in lo_lowlevel.h
 * @{
 */

/**
 * \brief Declare an OSC destination, given IP address and port number.
 * Same as lo_address_new_with_proto(), but using UDP.
 *
 * \param host An IP address or number, or NULL for the local machine.
 * \param port a decimal port number or service name.
 *
 * The lo_address object may be used as the target of OSC messages.
 *
 * Note: if you wish to receive replies from the target of this address, you
 * must first create a lo_server_thread or lo_server object which will receive
 * the replies. The last lo_server(_thread) object creted will be the receiver.
 */
lo_address lo_address_new(const char *host, const char *port);

/**
 * \brief Declare an OSC destination, given IP address and port number,
 * specifying protocol.
 *
 * \param proto The protocol to use, must be one of LO_UDP, LO_TCP or LO_UNIX.
 * \param host An IP address or number, or NULL for the local machine.
 * \param port a decimal port number or service name.
 *
 * The lo_address object may be used as the target of OSC messages.
 *
 * Note: if you wish to receive replies from the target of this address, you
 * must first create a lo_server_thread or lo_server object which will receive
 * the replies. The last lo_server(_thread) object creted will be the receiver.
 */
lo_address lo_address_new_with_proto(int proto, const char *host, const char *port);

/**
 * \brief Create a lo_address object from an OSC URL.
 *
 * example: \c "osc.udp://localhost:4444/my/path/"
 */
lo_address lo_address_new_from_url(const char *url);

/**
 * \brief Free the memory used by the lo_address object
 */ 
void lo_address_free(lo_address t);

/**
 * \brief Set the Time-to-Live value for a given target address.
 * 
 * This is required for sending multicast UDP messages.  A value of 1
 * (the usual case) keeps the message within the subnet, while 255
 * means a global, unrestricted scope.
 * 
 * \param t An OSC address.
 * \param ttl An integer specifying the scope of a multicast UDP message.
 */ 
void lo_address_set_ttl(lo_address t, int ttl);

/**
 * \brief Get the Time-to-Live value for a given target address.
 * 
 * \param t An OSC address.
 * \return An integer specifying the scope of a multicast UDP message.
 */ 
int lo_address_get_ttl(lo_address t);

/**
 * \brief Send a OSC formatted message to the address specified.
 *
 * \param targ The target OSC address
 * \param path The OSC path the message will be delivered to
 * \param type The types of the data items in the message, types are defined in
 * lo_osc_types.h
 * \param ... The data values to be transmitted. The types of the arguments
 * passed here must agree with the types specified in the type parameter.
 *
 * example:
 * \code
 * lo_send(t, "/foo/bar", "ff", 0.1f, 23.0f);
 * \endcode
 *
 * \return -1 on failure.
 */
int lo_send(lo_address targ, const char *path, const char *type, ...);

/**
 * \brief Send a OSC formatted message to the address specified.
 * Eigenlabs change: liblo does not seem to send to broadcast addresses correctly
 * so this version of send forces the socket option to broadcast.
 * Use only with a broadcast address.
 *
 * \param targ The target OSC address
 * \param path The OSC path the message will be delivered to
 * \param type The types of the data items in the message, types are defined in
 * lo_osc_types.h
 * \param ... The data values to be transmitted. The types of the arguments
 * passed here must agree with the types specified in the type parameter.
 *
 * example:
 * \code
 * lo_send(t, "/foo/bar", "ff", 0.1f, 23.0f);
 * \endcode
 *
 * \return -1 on failure.
 */
int lo_send_broadcast(lo_address targ, const char *path, const char *type, ...);
    
/**
 * \brief Send a OSC formatted message to the address specified, 
 * from the same socket as the specificied server.
 *
 * \param targ The target OSC address
 * \param from The server to send message from   (can be NULL to use new socket)
 * \param ts   The OSC timetag timestamp at which the message will be processed 
 * (can be LO_TT_IMMEDIATE if you don't want to attach a timetag)
 * \param path The OSC path the message will be delivered to
 * \param type The types of the data items in the message, types are defined in
 * lo_osc_types.h
 * \param ... The data values to be transmitted. The types of the arguments
 * passed here must agree with the types specified in the type parameter.
 *
 * example:
 * \code
 * serv = lo_server_new(NULL, err);
 * lo_server_add_method(serv, "/reply", "ss", reply_handler, NULL);
 * lo_send_from(t, serv, LO_TT_IMMEDIATE, "/foo/bar", "ff", 0.1f, 23.0f);
 * \endcode
 *
 * \return on success, the number of bytes sent, or -1 on failure.
 */
int lo_send_from(lo_address targ, lo_server from, lo_timetag ts, 
	       		const char *path, const char *type, ...);

/**
 * \brief Send a OSC formatted message to the address specified, scheduled to
 * be dispatch at some time in the future.
 *
 * \param targ The target OSC address
 * \param ts The OSC timetag timestamp at which the message will be processed
 * \param path The OSC path the message will be delivered to
 * \param type The types of the data items in the message, types are defined in
 * lo_osc_types.h
 * \param ... The data values to be transmitted. The types of the arguments
 * passed here must agree with the types specified in the type parameter.
 *
 * example:
 * \code
 * lo_timetag now;<br>
 * lo_timetag_now(&now);<br>
 * lo_send_timestamped(t, now, "/foo/bar", "ff", 0.1f, 23.0f);
 * \endcode
 *
 * \return on success, the number of bytes sent, or -1 on failure.
 */
int lo_send_timestamped(lo_address targ, lo_timetag ts, const char *path,
	       		const char *type, ...);

/**
 * \brief Return the error number from the last failed lo_send() or
 * lo_address_new() call
 */
int lo_address_errno(lo_address a);

/**
 * \brief Return the error string from the last failed lo_send() or
 * lo_address_new() call
 */
const char *lo_address_errstr(lo_address a);

/**
 * \brief Create a new OSC blob type.
 *
 * \param size The amount of space to allocate in the blob structure.
 * \param data The data that will be used to initialise the blob, should be
 * size bytes long.
 */
lo_blob lo_blob_new(int32_t size, const void *data);

/**
 * \brief Free the memory taken by a blob
 */
void lo_blob_free(lo_blob b);

/**
 * \brief Return the amount of valid data in a lo_blob object.
 *
 * If you want to know the storage size, use lo_arg_size().
 */
uint32_t lo_blob_datasize(lo_blob b);

/**
 * \brief Return a pointer to the start of the blob data to allow contents to
 * be changed.
 */
void *lo_blob_dataptr(lo_blob b);

/** @} */

#include "lo_macros.h"

#ifdef __cplusplus
}
#endif

#endif
