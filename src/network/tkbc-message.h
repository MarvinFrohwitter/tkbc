#ifndef TKBC_MESSAGE_H
#define TKBC_MESSAGE_H

#include <stddef.h>

/**
 * @brief A growable byte buffer that holds one or more messages.
 *
 * The buffer is treated as a flat byte array: @c elements is its base and the
 * bytes at index n is @c elements [n]. The memory is owned by a Space, see
 * tkbc-servers-common.h for the reset/growth helpers.
 *
 * The same type is used for both directions of the network, which is why
 * @c count and @c i have different jobs:
 *
 * Writing (tkbc_message_write_*() in messages/tkbc-binary-protocol.h)
 *   @c count is the write cursor. Every helper appends at @c elements
 *   [@c count] and bumps @c count. @c i is not used for writing, it tracks
 *   how much of the buffer has already been handed to the socket.
 *
 * Reading (tkbc_message_read_*() in messages/tkbc-binary-protocol.h)
 *   @c i is the read cursor, it always advances by the amount that was read.
 *   @c count is the upper bound of what may be read, never a cursor.
 *
 * tkbc_message_reader() returns a Message that shares the byte array but has
 * its own @c i starting after the 4 byte header and a @c count that ends at
 * the end of that message, so reads cannot run into the next message.
 */
typedef struct {
    /** Base of the byte array. */
    char *elements;
    /** Amount of valid bytes in the buffer. Write cursor while serializing. */
    size_t count;
    /** Amount of allocated bytes available at @c elements. */
    size_t capacity;
    /** Read cursor, the index of the next byte to be read. */
    size_t i;
} Message;

#endif  // TKBC_MESSAGE_H