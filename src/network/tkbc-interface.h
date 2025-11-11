// name : kind : data
typedef enum {
  MESSAGE_ZERO = 0,
  MESSAGE_HELLO,

  MESSAGE_SINGLE_KITE_ADD,    // When a kite is added because a new client has
                              // connected to the server
  MESSAGE_SINGLE_KITE_UPDATE, // Update from the client to the server -> the
                              // server broadcasts the information except to the
                              // client where it's coming from.

  MESSAGE_CLIENT_DISCONNECT,

  MESSAGE_CLIENTKITES, // From server to client in the beginning to inform the
                       // client about all kites and when a acript is running.
  MESSAGE_KITES,

  MESSAGE_KITES_POSITIONS,
  MESSAGE_KITES_POSITIONS_RESET,

  MESSAGE_SCRIPT,
  MESSAGE_SCRIPT_AMOUNT,
  MESSAGE_SCRIPT_PARSED,
  MESSAGE_SCRIPT_META_DATA,
  MESSAGE_SCRIPT_TOGGLE,
  MESSAGE_SCRIPT_NEXT,
  MESSAGE_SCRIPT_SCRUB,
  MESSAGE_SCRIPT_FINISHED,
  MESSAGE_COUNT,
} Message_Kind; // Messages that are supported in the current PROTOCOL_VERSION.

// MESSAGE_COUNT: The toatal amount of message types.
// MESSAGE_ZERO: A reserved Message that could be used to disable messages.

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
 * MESSAGE_SINGLE_KITE_ADD:
 *
 *****
 * MESSAGE_SINGLE_KITE_ADD:kite_id:(x,y):angle:color:texture_id:is_reversed:is_active:\r\n
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

/////////////////// Duplicate ?
/**
 *
 * MESSAGE_CLIENTKITES: From server to client in the beginning to inform the
 * client about all kites and when a acript is running.
 *
 *****
 * MESSAGE_CLIENTKITES:active_count:[kite_id:(x,y):angle:color:texture_id:is_reversed:is_active:]^*\r\n
 *****
 */

/**
 *
 * MESSAGE_KITES: From server to client
 *
 *****
 * MESSAGE_KITES:active_count:[kite_id:(x,y):angle:color:texture_id:is_reversed:is_active:]^*\r\n
 *****
 */
///////////////////

/**
 *
 * MESSAGE_KITES_POSITIONS: UNUSED
 *
 *****
 * MESSAGE_KITES_POSITIONS:kite_array->count:[kite_id:(x,y):angle:color:texture_id:is_reversed:is_active:]^*\r\n
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
 * MESSAGE_SINGLE_KITE_UPDATE: Duplicate?
 *
 *****
 * MESSAGE_SINGLE_KITE_UPDATE:kite_id:(x,y):angle:color:texture_id:is_reversed:is_active:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT:
 *
 *****
 * MESSAGE_SCRIPT:script_id:script->count:
 * [frames->index:frames->count:
 * [frame->index:frame->finished:frame->kind:
 *   {quit->starttime|wait->starttime|move->x:move->y|rotation->angle|tip_rotation->tip:tip_rotation->angle}:
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
 * MESSAGE_SCRIPT_SCRUB:
 *
 *****
 * MESSAGE_SCRIPT_SCRUB:drag_left:\r\n
 *****
 */

/**
 *
 * MESSAGE_SCRIPT_FINISHED:
 *
 *****
 * MESSAGE_SCRIPT_FINISHED:\r\n
 *****
 */
