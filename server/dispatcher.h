/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2009-2015 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <spice.h>

typedef struct Dispatcher Dispatcher;

typedef void (*dispatcher_handle_message)(void *opaque,
                                          void *payload);

typedef void (*dispatcher_handle_any_message)(void *opaque,
                                              uint32_t message_type,
                                              void *payload);

typedef void (*dispatcher_handle_async_done)(void *opaque,
                                             uint32_t message_type,
                                             void *payload);


typedef struct DispatcherMessage {
    size_t size;
    int ack;
    dispatcher_handle_message handler;
} DispatcherMessage;

struct Dispatcher {
    SpiceCoreInterface *recv_core;
    int recv_fd;
    int send_fd;
    pthread_t self;
    pthread_mutex_t lock;
    DispatcherMessage *messages;
    int stage;  /* message parser stage - sender has no stages */
    size_t max_message_type;
    void *payload; /* allocated as max of message sizes */
    size_t payload_size; /* used to track realloc calls */
    void *opaque;
    dispatcher_handle_async_done handle_async_done;
    dispatcher_handle_any_message any_handler;
};

/*
 * dispatcher_send_message
 * @message_type: message type
 * @payload:      payload
 */
void dispatcher_send_message(Dispatcher *dispatcher, uint32_t message_type,
                             void *payload);

/*
 * dispatcher_init
 * @max_message_type: number of message types. Allows upfront allocation
 *  of a DispatcherMessage list.
 * up front, and registration in any order wanted.
 */
void dispatcher_init(Dispatcher *dispatcher, size_t max_message_type,
                     void *opaque);

enum {
    DISPATCHER_NONE = 0,
    DISPATCHER_ACK,
    DISPATCHER_ASYNC
};

/*
 * dispatcher_register_handler
 * @dispatcher:     dispatcher
 * @messsage_type:  message type
 * @handler:        message handler
 * @size:           message size. Each type has a fixed associated size.
 * @ack:            One of DISPATCHER_NONE, DISPATCHER_ACK, DISPATCHER_ASYNC.
 *                  DISPATCHER_NONE - only send the message
 *                  DISPATCHER_ACK - send an ack after the message
 *                  DISPATCHER_ASYNC - call send an ack. This is per message type - you can't send the
 *                  same message type with and without. Register two different
 *                  messages if that is what you want.
 */
void dispatcher_register_handler(Dispatcher *dispatcher, uint32_t message_type,
                                 dispatcher_handle_message handler, size_t size,
                                 int ack);

/*
 * dispatcher_register_async_done_callback
 * @dispatcher:     dispatcher
 * @handler:        callback on the receiver side called *after* the
 *                  message callback in case ack == DISPATCHER_ASYNC.
 */
void dispatcher_register_async_done_callback(
                                    Dispatcher *dispatcher,
                                    dispatcher_handle_async_done handler);

/*
 * Hack to allow red_record to see the message being sent so it can record
 * it to file.
 */
void dispatcher_register_universal_handler(Dispatcher *dispatcher,
                                    dispatcher_handle_any_message handler);

/*
 *  dispatcher_handle_recv_read
 *  @dispatcher: Dispatcher instance
 */
void dispatcher_handle_recv_read(Dispatcher *);

/*
 *  dispatcher_get_recv_fd
 *  @return: receive file descriptor of the dispatcher
 */
int dispatcher_get_recv_fd(Dispatcher *);

/*
 * dispatcher_set_opaque
 * @dispatcher: Dispatcher instance
 * @opaque: opaque to use for callbacks
 */
void dispatcher_set_opaque(Dispatcher *dispatcher, void *opaque);

#endif //DISPATCHER_H
