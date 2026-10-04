#ifndef TKBC_BINARY_PROTOCOL_H
#define TKBC_BINARY_PROTOCOL_H

#include "../../../external/space/space.h"
#include "../tkbc-message.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//
// Wire format:
//   [uint32_t payload_size][uint8_t message_kind][payload_bytes...]
//
// payload_size = sizeof(message_kind) + sizeof(payload)
// All values are native byte order.
//

// ===========================================================================
// Binary Write Helpers
// ===========================================================================

/**
 * @brief Reserves space for the framing header (4 bytes) and writes the
 * message kind. Returns the offset where payload_size was written so it
 * can be patched later with tkbc_message_write_end().
 */
static inline size_t tkbc_message_write_begin(Message *message, Space *space, uint8_t kind) {
    size_t offset = message->count;
    uint32_t payload_size_placeholder = 0;
    space_dapc(space, message, &payload_size_placeholder, sizeof(payload_size_placeholder));
    space_dap(space, message, kind);
    return offset;
}

/**
 * @brief Patches the payload_size at the given offset with the actual size.
 * payload_size = message->count - offset - sizeof(uint32_t)
 */
static inline void tkbc_message_write_end(Message *message, size_t offset) {
    uint32_t payload_size = (uint32_t) (message->count - offset - sizeof(uint32_t));
    memcpy(message->elements + offset, &payload_size, sizeof(payload_size));
}

static inline void tkbc_message_write_u8(Message *message, Space *space, uint8_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_u16(Message *message, Space *space, uint16_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_u32(Message *message, Space *space, uint32_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_u64(Message *message, Space *space, uint64_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_s8(Message *message, Space *space, int8_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_s16(Message *message, Space *space, int16_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_s32(Message *message, Space *space, int32_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_s64(Message *message, Space *space, int64_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_f32(Message *message, Space *space, float_t value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_f64(Message *message, Space *space, double value) {
    space_dapc(space, message, &value, sizeof(value));
}

static inline void tkbc_message_write_bool(Message *message, Space *space, bool value) {
    uint8_t v = value;
    space_dapc(space, message, &v, sizeof(v));
}

static inline void tkbc_message_write_bytes(Message *message, Space *space, const void *data, size_t length) {
    space_dapc(space, message, data, length);
}

/**
 * @brief Writes a length-prefixed string: u32 length + raw bytes (no null
 * terminator).
 */
static inline void tkbc_message_write_string(Message *message, Space *space, const char *str, size_t length) {
    tkbc_message_write_u32(message, space, (uint32_t) length);
    if (length > 0) {
        space_dapc(space, message, str, length);
    }
}

static inline void tkbc_message_write_c_string(Message *message, Space *space, const char *cstr) {
    tkbc_message_write_string(message, space, cstr, strlen(cstr));
}

// ===========================================================================
// Binary Read Helpers
// ===========================================================================

/**
 * @brief Checks that @p count more bytes can be read from @p message.
 *
 * Reading is done with the Message itself: @c elements is the base of the
 * byte array, @c i is the read cursor and @c count is the amount of bytes
 * that are available. Use tkbc_message_reader() to get a Message that is
 * scoped to the payload of a single received message.
 */
static inline bool tkbc_message_read_has_at_least_count_new_data(const Message *message, size_t count) {
    return message->i + count <= message->count;
}

static inline bool tkbc_message_read_u8(Message *message, uint8_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_u16(Message *message, uint16_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_u32(Message *message, uint32_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_u64(Message *message, uint64_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_s8(Message *message, int8_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_s16(Message *message, int16_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_s32(Message *message, int32_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_s64(Message *message, int64_t *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_f32(Message *message, float *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_float(Message *message, float *out) {
    return tkbc_message_read_f32(message, out);
}

static inline bool tkbc_message_read_f64(Message *message, double *out) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, sizeof(*out))) return false;
    memcpy(out, message->elements + message->i, sizeof(*out));
    message->i += sizeof(*out);
    return true;
}

static inline bool tkbc_message_read_double(Message *message, double *out) {
    return tkbc_message_read_f64(message, out);
}

static inline bool tkbc_message_read_bool(Message *message, bool *out) {
    uint8_t v;
    if (!tkbc_message_read_u8(message, &v)) return false;
    *out = !!v;
    return true;
}

static inline bool tkbc_message_read_bytes(Message *message, void *out, size_t length) {
    if (!tkbc_message_read_has_at_least_count_new_data(message, length)) return false;
    memcpy(out, message->elements + message->i, length);
    message->i += length;
    return true;
}

/**
 * @brief Reads a length-prefixed string. Allocates *out from the given
 * space. Caller must free via space_reset_tspace() or similar.
 * *out_lenght is set to the string length (no null terminator included).
 */
static inline bool tkbc_message_read_c_string(Message *message, Space *space, char **out, size_t *out_lenght) {
    uint32_t length;
    if (!tkbc_message_read_u32(message, &length)) return false;
    *out_lenght = length;
    if (length == 0) {
        *out = NULL;
        return true;
    }
    if (!tkbc_message_read_has_at_least_count_new_data(message, length)) return false;
    *out = space_malloc(space, length + 1);
    if (!*out) return false;
    memcpy(*out, message->elements + message->i, length);
    (*out)[length] = '\0';
    message->i += length;
    return true;
}

// ===========================================================================
// Framing Helpers
// ===========================================================================

/**
 * @brief Checks if a complete framed message is available in the receive
 * buffer starting at the given position.
 *
 * @param buffer The receive message buffer.
 * @param pos The current read position in the buffer.
 * @return True if a complete message (header + payload) is available.
 */
static inline bool tkbc_is_message_complete(const Message *buffer, size_t pos) {
    if (pos + sizeof(uint32_t) > buffer->count) {
        return false;
    }
    uint32_t payload_size;
    memcpy(&payload_size, buffer->elements + pos, sizeof(payload_size));
    size_t total = sizeof(uint32_t) + payload_size;
    return pos + total <= buffer->count;
}

/**
 * @brief Returns the total message size (header + payload) for the message
 * starting at pos. Only call when tkbc_is_message_complete() returns true.
 */
static inline size_t tkbc_get_complete_singe_message_size(const Message *buffer, size_t pos) {
    uint32_t payload_size;
    memcpy(&payload_size, buffer->elements + pos, sizeof(payload_size));
    return sizeof(uint32_t) + payload_size;
}

/**
 * @brief Creates a Message that is scoped to the payload of the message
 * starting at @p pos inside @p buffer.
 *
 * The returned Message shares the byte array of @p buffer, but has its own
 * read cursor (@c i) that starts right after the 4-byte header and a @c count
 * that ends at the end of this message, so reads can never run into the next
 * message. Pass a pointer to it to the tkbc_message_read_*() helpers.
 *
 * Only call when tkbc_is_message_complete() returns true.
 */
static inline Message tkbc_message_reader(const Message *buffer, size_t pos) {
    Message reader = *buffer;
    reader.i = pos + sizeof(uint32_t);
    reader.count = pos + tkbc_get_complete_singe_message_size(buffer, pos);
    return reader;
}

/**
 * @brief Removes consumed bytes from the front of a message buffer.
 * Shifts remaining data to the beginning and adjusts count/i.
 */
static inline void tkbc_compact_message(Message *message, size_t consumed) {
    if (consumed == 0) return;
    if (consumed >= message->count) {
        message->count = 0;
        message->i = 0;
        return;
    }
    memmove(message->elements, message->elements + consumed, message->count - consumed);
    message->count -= consumed;
    if (message->i > consumed) {
        message->i -= consumed;
    } else {
        message->i = 0;
    }
}

#endif  // TKBC_BINARY_PROTOCOL_H
