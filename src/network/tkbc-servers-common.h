#ifndef TKBC_SERVERS_COMMON_H
#define TKBC_SERVERS_COMMON_H

//////////////////////////////////////////////////////////////////////////////
#define PROTOCOL_VERSION "0.3.042"
#define SERVER_CONNETCTIONS 64

#define TKBC_LOGGING
#define TKBC_LOGGING_ERROR
#define TKBC_LOGGING_INFO
#define TKBC_LOGGING_WARNING
#define TKBC_LOGGING_MESSAGEHANDLER
//////////////////////////////////////////////////////////////////////////////

#include "../../external/space/space.h"
#include "../choreographer/tkbc-asset-handler.h"
#include "../choreographer/tkbc-ui.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
extern Env *env;
extern Assets assets;

#include <ctype.h>
#include <math.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <winsock2.h>

#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
typedef int SOCKLEN;

#else
#include <netinet/in.h>
#include <netinet/tcp.h>
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef socklen_t SOCKLEN;
#endif  //_WIN32

#include "messages/tkbc-binary-protocol.h"
#include "messages/tkbc-interface.h"

typedef struct {
    ssize_t kite_id;
    Message send_msg_buffer;
    Message recv_msg_buffer;
    Space send_msg_buffer_space;
    Space recv_msg_buffer_space;

    int socket_id;
    SOCKADDR_IN client_address;
    SOCKLEN client_address_length;

    size_t script_amount;
    bool handshake_passed;
} Client;

typedef struct {
    Client *elements;
    size_t count;
    size_t capacity;
} Clients;

/**
 * @brief Checks if a message kind may be sent to a client whose handshake has
 * not passed yet.
 *
 * Only the handshake protocol itself (HELLO in both directions and the
 * server's HELLO_PASSED) is allowed before the client answered the server
 * HELLO with its own HELLO. Everything else has to wait until
 * @c handshake_passed is true.
 *
 * @param kind The message kind byte as stored on the wire.
 * @return True for MESSAGE_HELLO and MESSAGE_HELLO_PASSED, otherwise false.
 */
static inline bool tkbc_server_message_kind_allowed_before_handshake(uint8_t kind) {
    return kind == MESSAGE_HELLO || kind == MESSAGE_HELLO_PASSED;
}

/**
 * @brief Checks if the given client may currently receive the given kind.
 *
 * @param client The client the message should be sent to.
 * @param kind The message kind byte as stored on the wire.
 * @return True if the handshake already passed or the kind is part of the
 * handshake protocol, otherwise false.
 */
static inline bool tkbc_server_client_may_receive_kind(const Client *client, uint8_t kind) {
    return client->handshake_passed || tkbc_server_message_kind_allowed_before_handshake(kind);
}

/**
 * @brief Checks if a batched message contains only handshake protocol frames.
 *
 * Used by the broadcast helpers to silently skip clients whose handshake has
 * not passed yet instead of queuing (and then dropping) per frame.
 *
 * @param message The batched message as passed to tkbc_write_to_*().
 * @return True if every complete frame is HELLO/HELLO_PASSED, false otherwise
 * (including incomplete trailing bytes).
 */
static inline bool tkbc_server_message_batch_allowed_before_handshake(const Message *message) {
    if (message->count == 0 || message->elements == NULL) {
        return true;
    }
    size_t pos = 0;
    while (pos + sizeof(uint32_t) <= message->count) {
        uint32_t payload_size;
        memcpy(&payload_size, message->elements + pos, sizeof(payload_size));
        size_t total = sizeof(uint32_t) + payload_size;
        if (pos + total > message->count) {
            return false;
        }
        uint8_t kind = (uint8_t) message->elements[pos + sizeof(uint32_t)];
        if (!tkbc_server_message_kind_allowed_before_handshake(kind)) {
            return false;
        }
        pos += total;
    }
    return pos == message->count;
}

#define CLIENT_FMT "Index: %zu, Socket: %d, Address: (%s:%hu)"
#define CLIENT_ARG(c)                                                                                                  \
    ((c).kite_id), ((c).socket_id), (inet_ntoa((c).client_address.sin_addr)), (ntohs((c).client_address.sin_port))

/**
 * @brief The function prints the way the program should be called.
 *
 * @param program_name The name of the program that is currently executing.
 */
static inline void tkbc_server_usage(const char *program_name) {
    tkbc_fprintf(stderr, "INFO", "Usage:\n");
    tkbc_fprintf(stderr, "INFO", "      %s <PORT> \n", program_name);
}

/**
 * @brief The function checks if a port is given to that program.
 *
 * @param argc The commandline argument count.
 * @param program_name The name of the program that is currently executing.
 * @return True if there are enough arguments, otherwise false.
 */
static inline bool tkbc_server_commandline_check(int argc, const char *program_name) {
    if (argc > 1) {
        tkbc_fprintf(stderr, "ERROR", "Too may arguments.\n");
        tkbc_server_usage(program_name);
        exit(1);
    }
    if (argc == 0) {
        tkbc_fprintf(stderr, "ERROR", "No arguments were provided.\n");
        tkbc_fprintf(stderr, "INFO", "The default port 8080 is used.\n");
        return false;
    }
    return true;
}

/**
 * @brief The function checks if the given port is valid and if so returns the
 * port as a number. If the given string does not contain a valid port the
 * program will crash.
 *
 * @param port_check The character string that is potently a port.
 * @return The parsed port as a uint16_t.
 */
static inline uint16_t tkbc_port_parsing(const char *port_check) {
    for (size_t i = 0; i < strlen(port_check); ++i) {
        if (!isdigit(port_check[i])) {
            tkbc_fprintf(stderr, "ERROR", "The given port [%s] is not valid.\n", port_check);
            exit(1);
        }
    }
    int port = atoi(port_check);
    if (port >= 65535 || port <= 0) {
        tkbc_fprintf(stderr, "ERROR", "The given port [%s] is not valid.\n", port_check);
        exit(1);
    }

    return (uint16_t) port;
}

/**
 * @brief The function creates a new server socket and sets up the bind and
 * listing.
 *
 * @param addr The address space that the socket should be bound to.
 * @param port The port where the server is listing for connections.
 * @return The newly creates socket id.
 */
static inline int tkbc_server_socket_creation(uint32_t addr, uint16_t port) {
#ifdef _WIN32
    // MAKEWORD(2, 2) is a version, and wsaData will be filled with initialized
    // library information.

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        assert(0 && "ERROR: WSAStartup()");
    } else {
        tkbc_fprintf(stderr, "INFO", "Initialization of WSAStartup() succeed.\n");
    }
#endif

    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
#ifdef _WIN32
        tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
        tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
        exit(1);
    }
    int option = 1;
    int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option));
    if (sso == -1) {
#ifdef _WIN32
        tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
        tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
    }

    int nodelay = 1;
    sso = setsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (char *) &nodelay, sizeof(nodelay));
    if (sso == -1) {
#ifdef _WIN32
        tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
        tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = addr;

    int bind_status = bind(socket_id, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (bind_status == -1) {
#ifdef _WIN32
        tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
        WSACleanup();
#else
        tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
        exit(1);
    }

    int listen_status = listen(socket_id, SERVER_CONNETCTIONS);
    if (listen_status == -1) {
#ifdef _WIN32
        tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
        WSACleanup();
#else
        tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
        exit(1);
    }
    tkbc_fprintf(stderr, "INFO", "%s: %hu\n", "Listening to port", port);

    return socket_id;
}

/**
 * @brief The function appends the given uuid to a message as its 16 raw bytes.
 *
 * @param message The Message struct that should contain the serialized uuid.
 * @param space The space that is used for the message buffer.
 * @param uuid The uuid that should be appended.
 */
static inline void tkbc_message_write_uuid(Message *message, Space *space, UUID uuid) {
    tkbc_message_write_bytes(message, space, uuid.bytes, sizeof(uuid.bytes));
}

/**
 * @brief The function reads a uuid out of a message reader. The uuid is
 * expected as the 16 raw bytes of its binary form.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message and therefore holds its own read cursor.
 * @param uuid The uuid object where the parsed uuid should be stored.
 * @return True if the uuid could be read successfully, otherwise false.
 */
static inline bool tkbc_message_read_uuid(Message *reader, UUID *uuid) {
    return tkbc_message_read_bytes(reader, uuid->bytes, sizeof(uuid->bytes));
}

/**
 * @brief The function appends the pixel data of the given image to a message.
 *
 * @param space The space that is used for the message buffer.
 * @param message The Message struct that should contain the serialized image
 * data.
 * @param image The image whose pixel data should be appended.
 * @param id The uuid that is serialized in front of the image data. This is the
 * uuid of the originator so that the receiver stores the image under the very
 * same uuid and does not register it a second time.
 */
static inline void tkbc_message_append_image_data(Space *space, Message *message, Image image, UUID id) {
    tkbc_message_write_uuid(message, space, id);
    tkbc_message_write_s32(message, space, image.width);
    tkbc_message_write_s32(message, space, image.height);
    tkbc_message_write_s32(message, space, image.format);
    size_t size = image.width * image.height * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *) image.data;
    tkbc_message_write_bytes(message, space, pixels, size);
}

/**
 * @brief The function constructs a message part that contains the information
 * from the given kite_state.
 *
 * @param kite_state The kite state where the information is extracted from.
 * @param message The Message struct that should contain the serialized data.
 * @param space The space that is used for the message buffer.
 */
static inline void tkbc_message_append_kite(Kite_State *kite_state, Message *message, Space *space) {
    size_t kite_id = kite_state->kite_id;
    float x = kite_state->kite->center.x;
    float y = kite_state->kite->center.y;
    float angle = fmodf(kite_state->kite->angle, 360);

    uint32_t color = tkbc_color_to_uint32_t(kite_state->kite->body_color);
    UUID texture_id = kite_state->kite->texture_id;

    bool is_reversed = kite_state->is_kite_reversed;
    bool is_active = kite_state->is_active;
    bool is_script_kite = kite_state->is_script_kite;

    tkbc_message_write_u64(message, space, kite_id);
    tkbc_message_write_f32(message, space, x);
    tkbc_message_write_f32(message, space, y);
    tkbc_message_write_f32(message, space, angle);
    tkbc_message_write_u32(message, space, color);

    // The nil uuid marks that the pixel data is transferred inline. That is the
    // case for a freshly created kite design (is_texture_new) that the other
    // sides cannot know yet.
    if (tkbc_uuid_is_nil(texture_id) || kite_state->kite->is_texture_new) {
        assert(!tkbc_uuid_is_nil(texture_id) && "The texture of a new kite design must be known locally.");
        Asset *asset = tkbc_find_asset_from_id(texture_id);
        assert(asset != NULL && "The texture asset of a new kite design must exist locally.");
        assert(asset->type == ASSETS_KITE_DESIGN);

        tkbc_message_write_uuid(message, space, tkbc_uuid_nil());
        Kite_Image *kite_image = &asset->as.kite_image;
        tkbc_message_append_image_data(space, message, kite_image->normal, asset->id);
        kite_state->kite->is_texture_new = false;
    } else {
        tkbc_message_write_uuid(message, space, texture_id);
    }

    tkbc_message_write_bool(message, space, is_reversed);
    tkbc_message_write_bool(message, space, is_active);
    tkbc_message_write_bool(message, space, is_script_kite);
}

/**
 * @brief The function constructs the message part of a kite.
 *
 * @param client_id The id of the kite which data should be appended to the
 * message.
 * @param message The Message struct that should contain the serialized data.
 * @return True if the given kite id was found and the data is appended,
 * otherwise false.
 */
static inline bool tkbc_message_append_clientkite(size_t client_id, Message *message, Space *space) {
    for (size_t i = 0; i < env->kite_array.count; ++i) {
        if (client_id == env->kite_array.elements[i].kite_id) {
            Kite_State *kite_state = &env->kite_array.elements[i];
            tkbc_message_append_kite(kite_state, message, space);
            return true;
        }
    }
    return false;
}

#endif  // TKBC_SERVERS_COMMON_H
