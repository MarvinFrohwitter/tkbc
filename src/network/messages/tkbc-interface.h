#ifndef TKBC_INTERFACE_H
#define TKBC_INTERFACE_H
// name : kind : data
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

// PROTOCOL NOTE: Every texture_id and asset id in the protocol is a UUID that
// is serialized as a quoted string literal holding the canonical textual
// representation of the uuid: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
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

/**
 *
 * MESSAGE_HELLO:
 *
 *****
 * MESSAGE_HELLO:quote "Hello client from server!"PROTOCOL_VERSION quote:\r\n
 * MESSAGE_HELLO:quote "Hello server from client!"PROTOCOL_VERSION quote:\r\n
 *****
 */

/**
 *
 * MESSAGE_HELLO_PASSED:
 *
 *****
 * MESSAGE_HELLO_PASSED:\r\n
 *****
 */

/**
 *
 * MESSAGE_SINGLE_KITE_ADD: Server notifies all clients about a new kite.
 * When texture_id is the nil uuid, inline image data is included.
 *
 *****
 * MESSAGE_SINGLE_KITE_ADD:kite_id:(x,y):angle:color:"texture_id_or_nil_uuid":{id:width:height:format:{pixel_data}:}?is_reversed:is_active:\r\n
 *****
 */

/**
 *
 * MESSAGE_CLIENT_DISCONNECT:
 *
 *****
 * MESSAGE_CLIENT_DISCONNECT:kite_id:\r\n
 *****
 */

/**
 *
 * MESSAGE_CLIENTKITES: From server to client in the beginning to inform the
 * client about all kites and when a script is running. When texture_id is the
 * nil uuid, inline image data is included.
 *
 *****
 * MESSAGE_CLIENTKITES:active_count:[kite_id:(x,y):angle:color:"texture_id_or_nil_uuid":{id:width:height:format:{pixel_data}:}?is_reversed:is_active:]^*\r\n
 *****
 */

/**
 *
 * MESSAGE_KITES_POSITIONS_RESET:
 *
 *****
 * MESSAGE_KITES_POSITIONS_RESET:\r\n
 *****
 */

/**
 *
 * MESSAGE_SINGLE_KITE_UPDATE: Client sends kite position update to server.
 * Server broadcasts to all other clients. When texture_id is the nil uuid,
 * inline image data is included.
 *
 *****
 * MESSAGE_SINGLE_KITE_UPDATE:kite_id:(x,y):angle:color:"texture_id_or_nil_uuid":{id:width:height:format:{pixel_data}:}?is_reversed:is_active:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT: always carries the original non-upscaled blocks. Each side
 * upscales and bakes locally, so the receiver can still save the original
 * form and per-tick slices never hit the wire.
 *
 *****
 * MESSAGE_SCRIPT:script_id:name_count:[name:]?script->count:
 * [frames->index:frames->count:
 * [frame->index:frame->finished:frame->kind:
 *   {move->x:move->y|rotation->angle|tip_rotation->tip:tip_rotation->angle}:
 * frame->duraction{:kite_ids->count:({ids,}^*id)}?:
 * ]
 * ]\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_AMOUNT:
 *
 *****
 * MESSAGE_SCRIPT_AMOUNT:script->count:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_PARSED:
 *
 *****
 * MESSAGE_SCRIPT_PARSED:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_META_DATA:
 *
 *****
 * MESSAGE_SCRIPT_META_DATA:script_id:script_count:frames_index:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_TOGGLE:
 *
 *****
 * MESSAGE_SCRIPT_TOGGLE:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_NEXT:
 *
 *****
 * MESSAGE_SCRIPT_NEXT:script_id:\r\n
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
 * MESSAGE_SCRIPT_SCRUB:target_frames_index:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_FINISHED: no longer sent on natural end (kept for protocol
 * compatibility). A finished script stays paused in script mode; only an
 * explicit NO SCRIPT (SCRIPT_NEXT with nil id) terminates execution.
 *
 *****
 * MESSAGE_SCRIPT_FINISHED:\r\n
 *****
 */

/**
 *
 * MESSAGE_GET_TEXTURE_ID:
 *
 *****
 * MESSAGE_GET_TEXTURE_ID:kite_id:\r\n
 *****
 */

/**
 *
 * MESSAGE_SEND_TEXTURE_ID:
 *
 *****
 * MESSAGE_SEND_TEXTURE_ID:kite_id:"texture_id":\r\n
 *****
 */

/**
 *
 * MESSAGE_GET_TEXTURE:
 * The uuid of the requested texture asset.
 *
 *****
 * MESSAGE_GET_TEXTURE:"texture_id":\r\n
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
 * MESSAGE_SEND_TEXTURE:"id":width:height:format:{pixel_data}:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_DELETE:
 *
 *****
 * MESSAGE_SCRIPT_DELETE:script_id:\r\n
 *****
 */

#endif  // TKBC_INTERFACE_H
