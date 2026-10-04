#ifndef TKBC_INTERFACE_H
#define TKBC_INTERFACE_H

#include "../tkbc-message.h"

// name -> payload, see the PROTOCOL section below for the wire format.
typedef enum {
    MESSAGE_ZERO = 0,
    MESSAGE_HELLO,
    MESSAGE_HELLO_PASSED,

    MESSAGE_SINGLE_KITE_ADD,     // When a kite is added because a new client has
                                 // connected to the server
    MESSAGE_SINGLE_KITE_UPDATE,  // Update from the client to the server -> the
                                 // server broadcasts the information except to the
                                 // client where it's coming from.

    MESSAGE_CLIENT_DISCONNECT,

    MESSAGE_CLIENTKITES,  // From server to client in the beginning to inform the
                          // client about all kites and when a script is running.
    MESSAGE_KITES_POSITIONS_RESET,

    MESSAGE_SCRIPT,
    MESSAGE_SCRIPT_AMOUNT,
    MESSAGE_SCRIPT_PARSED,
    MESSAGE_SCRIPT_META_DATA,
    MESSAGE_SCRIPT_TOGGLE,
    MESSAGE_SCRIPT_NEXT,
    MESSAGE_SCRIPT_SCRUB,
    MESSAGE_SCRIPT_FINISHED,
    MESSAGE_SCRIPT_DELETE,

    MESSAGE_GET_TEXTURE_ID,
    MESSAGE_SEND_TEXTURE_ID,

    MESSAGE_GET_TEXTURE,
    MESSAGE_SEND_TEXTURE,

    MESSAGE_COUNT,
} Message_Kind;  // Messages that are supported in the current PROTOCOL_VERSION.

// MESSAGE_COUNT: The toatal amount of message types.
// MESSAGE_ZERO: A reserved Message that could be used to disable messages.

////////////////////////////////////////////////////////////////////////////////
// PROTOCOL
//
// Every message is framed as:
//
//   [uint32 payload_size][uint8 message_kind][payload_bytes]
//
// payload_size covers the message_kind byte plus the payload, so the total
// wire size of a message is 4 + payload_size. All values are native byte
// order. Messages can be concatenated freely in a stream: a receiver walks
// the buffer with tkbc_is_message_complete() and tkbc_message_reader() and
// skips over a message with tkbc_get_complete_singe_message_size().
//
// PRIMITIVES
//
//   u8/u16/u32/u64/s8/s16/s32/s64  fixed width integers
//   f32/f64                         IEEE-754 floating point
//   bool                            one byte, 0 == false, anything else true
//   string                          uint32 length + length raw bytes
//                                  (no null terminator)
//   uuid                            the 16 raw bytes of the UUID, no quotes and
//                                  no textual representation on the wire
//   image                           uuid id, s32 width, s32 height,
//                                  s32 format, then width * height * 4 raw
//                                  RGBA bytes (row major, top to bottom)
//
// SHARED BLOCKS
//
// A "kite value" is the state of one kite:
//
//   uint64 kite_id
//   f32    x
//   f32    y
//   f32    angle
//   uint32 color
//   -- the next two parts are mutually exclusive --
//   uuid   texture_id       points at a kite design in the global assets
//   image  inline_image     carries the pixels inline, only used when
//                           texture_id is the nil uuid
//   bool   is_reversed
//   bool   is_active
//   bool   is_script_kite
//
// A "frame" is one entry of the original (non-upscaled) script timeline:
//
//   uint64  index
//   bool    finished
//   uint8   kind             the Action_Kind, selects the action layout
//   <action>                 see below, its layout is derived from kind
//   f32     duration         in seconds
//   uint64  kite_ids_count   always written for every kind, even when 0
//   uint64  kite_ids[kite_ids_count]
//
//   action for ACTION_KITE_MOVE / ACTION_KITE_MOVE_ADD:
//     f32 x, f32 y
//   action for ACTION_KITE_ROTATION / ACTION_KITE_ROTATION_ADD:
//     f32 angle
//   action for ACTION_KITE_TIP_ROTATION / ACTION_KITE_TIP_ROTATION_ADD:
//     s32 tip, f32 angle
//   action for ACTION_KITE_WAIT / ACTION_KITE_QUIT:
//     nothing
//
// PROTOCOL NOTE: Every texture_id and asset id in the protocol is a UUID that
// is serialized as the 16 raw bytes of its binary form.
//
// The nil uuid "00000000-0000-0000-0000-000000000000" marks that the pixel
// data is transferred inline. That is the case for a kite design that was just
// created and that the other sides cannot know yet. The inlined image block
// then carries the uuid the originator assigned to the design, so the receiver
// stores the image under that very uuid instead of registering it a second time
// under a fresh one.
//
// The baked in base assets share one deterministic uuid across all sides, since
// they are built in the exact same order everywhere. Assets created at runtime
// (e.g. a colorizer result) get a random uuid, so designs of different users
// never collide.
////////////////////////////////////////////////////////////////////////////////

/**
 *
 * MESSAGE_HELLO:
 *
 *****
 * MESSAGE_HELLO -> string "Hello client from server!" PROTOCOL_VERSION
 * MESSAGE_HELLO -> string "Hello server from client!" PROTOCOL_VERSION
 *****
 */

/**
 *
 * MESSAGE_HELLO_PASSED:
 *
 *****
 * MESSAGE_HELLO_PASSED -> (empty payload)
 *****
 */

/**
 *
 * MESSAGE_SINGLE_KITE_ADD: Server notifies all clients about a new kite.
 * When texture_id is the nil uuid, inline image data is included.
 *
 *****
 * MESSAGE_SINGLE_KITE_ADD -> <kite value>
 *****
 */

/**
 *
 * MESSAGE_CLIENT_DISCONNECT:
 *
 *****
 * MESSAGE_CLIENT_DISCONNECT -> uint64 kite_id
 *****
 */

/**
 *
 * MESSAGE_CLIENTKITES: From server to client in the beginning to inform the
 * client about all kites and when a script is running. When texture_id is the
 * nil uuid, inline image data is included.
 *
 *****
 * MESSAGE_CLIENTKITES -> uint64 amount -> [ <kite value> ] ^ * amount
 *****
 */

/**
 *
 * MESSAGE_KITES_POSITIONS_RESET:
 *
 *****
 * MESSAGE_KITES_POSITIONS_RESET -> (empty payload)
 *****
 */

/**
 *
 * MESSAGE_SINGLE_KITE_UPDATE: Client sends kite position update to server.
 * Server broadcasts to all other clients. When texture_id is the nil uuid,
 * inline image data is included.
 *
 *****
 * MESSAGE_SINGLE_KITE_UPDATE -> <kite value>
 *****
 */

/**
 *
 * MESSAGE_SCRIPT: always carries the original non-upscaled blocks. Each side
 * upscales and bakes locally, so the receiver can still save the original
 * form and per-tick slices never hit the wire.
 *
 *****
 * MESSAGE_SCRIPT -> uuid script_id -> string name -> uint64 script_count ->
 * [ uint64 frames_index -> uint64 frames_count ->
 *   [ <frame> ] ^ * frames_count ] ^ * script_count
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_AMOUNT:
 *
 *****
 * MESSAGE_SCRIPT_AMOUNT -> uint64 script_count
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_PARSED:
 *
 *****
 * MESSAGE_SCRIPT_PARSED -> (empty payload)
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_META_DATA:
 *
 *****
 * MESSAGE_SCRIPT_META_DATA -> uuid script_id -> uint64 script_count -> uint64 frames_index
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_TOGGLE:
 *
 *****
 * MESSAGE_SCRIPT_TOGGLE -> (empty payload)
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_NEXT:
 *
 *****
 * MESSAGE_SCRIPT_NEXT -> uuid script_id
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_SCRUB: absolute timeline jump for continuous scrubbing.
 *
 * The client maps its mouse X onto the (upscaled, per-tick) timeline and
 * sends the target frames_index. The server jumps via tkbc_scrub_to_index,
 * so every animation tick in between keyframes is reachable like in a video.
 *
 *****
 * MESSAGE_SCRIPT_SCRUB -> uint64 target_frames_index
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_FINISHED: no longer sent on natural end (kept for protocol
 * compatibility). A finished script stays paused in script mode; only an
 * explicit NO SCRIPT (SCRIPT_NEXT with nil id) terminates execution.
 *
 *****
 * MESSAGE_SCRIPT_FINISHED -> (empty payload)
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_DELETE:
 *
 *****
 * MESSAGE_SCRIPT_DELETE -> uuid script_id
 *****
 */

/**
 *
 * MESSAGE_GET_TEXTURE_ID:
 *
 *****
 * MESSAGE_GET_TEXTURE_ID -> uint64 kite_id
 *****
 */

/**
 *
 * MESSAGE_SEND_TEXTURE_ID:
 *
 *****
 * MESSAGE_SEND_TEXTURE_ID -> uint64 kite_id -> uuid texture_id
 *****
 */

/**
 *
 * MESSAGE_GET_TEXTURE:
 * The uuid of the requested texture asset.
 *
 *****
 * MESSAGE_GET_TEXTURE -> uuid texture_id
 *****
 */

/**
 *
 * MESSAGE_SEND_TEXTURE: The id is the uuid the originator assigned to the
 * design, so the receiver stores the image under the very same uuid.
 *
 * The receiver ignores the design if it already knows that uuid.
 *
 * The server also sends this unsolicited for every kite design it knows to a
 * client that just joined, right behind the MESSAGE_HELLO_PASSED and before the
 * scripts and kites that reference those designs. Only the designs that were
 * created at runtime are send, the baked in base designs are already known to
 * every side.
 *
 *****
 * MESSAGE_SEND_TEXTURE -> <image>
 *****
 */

#endif  // TKBC_INTERFACE_H
