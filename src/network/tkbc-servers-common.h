#ifndef TKBC_SERVERS_COMMON_H
#define TKBC_SERVERS_COMMON_H

//////////////////////////////////////////////////////////////////////////////
#define PROTOCOL_VERSION "0.4.009"
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
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
typedef int SOCKLEN;

#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
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
    // sockaddr_storage holds both IPv4 and IPv6 addresses (and any other
    // family getaddrinfo()/accept() might hand us in the future).
    struct sockaddr_storage client_address;
    SOCKLEN client_address_length;

    size_t script_amount;
    bool handshake_passed;

    // The script kites this client currently controls. The server uses this to
    // tell a single controller (the original kite follows the updates) from
    // multiple controllers (the remote edits are shown as ghosts).
    Kite_Ids selected_kite_ids;
} Client;

typedef struct {
    Client *elements;
    size_t count;
    size_t capacity;
} Clients;

/**
 * @brief A dynamic array of listening socket descriptors.
 *
 * With AF_UNSPEC getaddrinfo() can return both an IPv6 and an IPv4 address,
 * so the server may own more than one listening socket.
 */
typedef struct {
    int *elements;
    size_t count;
    size_t capacity;
} Sockets;

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

/**
 * @brief Checks if a message kind may be sent by the client before its
 * handshake has passed.
 *
 * The client side of the handshake is a single MESSAGE_HELLO reply to the
 * server HELLO. Everything else has to wait until MESSAGE_HELLO_PASSED was
 * received and @c handshake_passed is true.
 *
 * @param kind The message kind byte as stored on the wire.
 * @return True for MESSAGE_HELLO, otherwise false.
 */
static inline bool tkbc_client_message_kind_allowed_before_handshake(uint8_t kind) {
    return kind == MESSAGE_HELLO;
}

/**
 * @brief Checks if the client may currently send the given kind.
 *
 * @param client The client holding the handshake state.
 * @param kind The message kind byte as stored on the wire.
 * @return True if the handshake already passed or the kind is the HELLO
 * reply, otherwise false.
 */
static inline bool tkbc_client_may_send_kind(const Client *client, uint8_t kind) {
    return client->handshake_passed || tkbc_client_message_kind_allowed_before_handshake(kind);
}

/**
 * @brief Formats a peer address of any supported family including its port.
 *
 * @param client The client whose stored address should be formatted.
 * @return Pointer to a static buffer holding e.g. "127.0.0.1:8080" or
 * "[::1]:8080". The buffer is overwritten by the next call.
 */
static inline const char *tkbc_client_address_string(const Client *client) {
    static char formatted[NI_MAXHOST + NI_MAXSERV + 4];
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int result = getnameinfo((const struct sockaddr *) &client->client_address, client->client_address_length, host,
                             sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
    if (result != 0) {
        snprintf(formatted, sizeof(formatted), "<unknown address>");
        return formatted;
    }

    if (client->client_address.ss_family == AF_INET6) {
        snprintf(formatted, sizeof(formatted), "[%s]:%s", host, service);
    } else {
        snprintf(formatted, sizeof(formatted), "%s:%s", host, service);
    }
    return formatted;
}

#define CLIENT_FMT "Index: %zu, Socket: %d, Address: %s"
#define CLIENT_ARG(c) ((c).kite_id), ((c).socket_id), tkbc_client_address_string(&(c))

/**
 * @brief Logs the last socket error in a cross platform way.
 *
 * @param function The name of the call that failed.
 */
static inline void tkbc_socket_error(const char *function) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%s: %ld\n", function, WSAGetLastError());
#else
    tkbc_fprintf(stderr, "ERROR", "%s: %s\n", function, strerror(errno));
#endif
}

/**
 * @brief Closes a socket in a cross platform way.
 *
 * @param socket_id The socket to close.
 */
static inline void tkbc_close_socket(int socket_id) {
#ifdef _WIN32
    closesocket(socket_id);
#else
    close(socket_id);
#endif
}

/**
 * @brief Switches a socket to non-blocking mode.
 *
 * @param socket_id The socket to change.
 * @return True on success, otherwise false.
 */
static inline bool tkbc_set_socket_non_blocking(int socket_id) {
#ifdef _WIN32
    u_long mode = 1;  // 1 to enable non-blocking socket
    if (ioctlsocket(socket_id, FIONBIO, &mode) != 0) {
        tkbc_fprintf(stderr, "ERROR", "ioctlsocket(): %d\n", WSAGetLastError());
        return false;
    }
#else
    int flags = fcntl(socket_id, F_GETFL, 0);
    if (flags == -1) {
        tkbc_fprintf(stderr, "ERROR", "Could not get socket flags: %s\n", strerror(errno));
        return false;
    }
    if (fcntl(socket_id, F_SETFL, flags | O_NONBLOCK) == -1) {
        tkbc_fprintf(stderr, "ERROR", "Could not set the non-blocking flag: %s\n", strerror(errno));
        return false;
    }
#endif
    return true;
}

/**
 * @brief Removes the brackets around an IPv6 address literal.
 *
 * This makes inputs like "[::1]" usable wherever getaddrinfo() expects a
 * plain numeric host. Non-bracketed hosts are returned unchanged.
 *
 * @param host The host string, may be NULL.
 * @param buffer Storage used when the brackets have to be stripped.
 * @param buffer_size Size of @c buffer.
 * @return The normalized host string or NULL if @p host was NULL.
 */
static inline const char *tkbc_host_without_brackets(const char *host, char *buffer, size_t buffer_size) {
    if (host == NULL || buffer_size == 0) {
        return host;
    }
    size_t length = strlen(host);
    if (length < 2 || host[0] != '[' || host[length - 1] != ']') {
        return host;
    }
    length -= 2;
    if (length >= buffer_size) {
        length = buffer_size - 1;
    }
    memcpy(buffer, host + 1, length);
    buffer[length] = '\0';
    return buffer;
}

/**
 * @brief Checks whether the given string consists of digits only.
 *
 * @param string The string to check.
 * @return True if the string is a non-empty port candidate.
 */
static inline bool tkbc_is_port_string(const char *string) {
    if (string == NULL || string[0] == '\0') {
        return false;
    }
    for (size_t i = 0; string[i] != '\0'; ++i) {
        if (!isdigit((unsigned char) string[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @brief The function prints the way the program should be called.
 *
 * @param program_name The name of the program that is currently executing.
 */
static inline void tkbc_server_usage(const char *program_name) {
    tkbc_fprintf(stderr, "INFO", "Usage:\n");
    tkbc_fprintf(stderr, "INFO", "      %s [HOST] [PORT]\n", program_name);
    tkbc_fprintf(stderr, "INFO", " HOST  IPv4 address, IPv6 address or hostname. Omit to listen on all interfaces.\n");
    tkbc_fprintf(stderr, "INFO", " PORT  Port to listen on (default 8080).\n");
}

/**
 * @brief The function checks the amount of commandline arguments.
 *
 * @param argc The commandline argument count.
 * @param program_name The name of the program that is currently executing.
 * @return True if arguments were provided, otherwise false.
 */
static inline bool tkbc_server_commandline_check(int argc, const char *program_name) {
    if (argc > 2) {
        tkbc_fprintf(stderr, "ERROR", "Too may arguments.\n");
        tkbc_server_usage(program_name);
        exit(1);
    }
    if (argc == 0) {
        tkbc_fprintf(stderr, "INFO", "Using all interfaces and the default port 8080.\n");
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
 * @brief Creates the listening sockets for every address returned for the
 * given host and sets up bind, listen and non-blocking mode.
 *
 * With @p host == NULL and AF_UNSPEC this normally creates one IPv6 and one
 * IPv4 socket, so the server accepts both families. A concrete IPv4/IPv6
 * address or hostname restricts the server to that address.
 *
 * @param host The address to bind to, or NULL for all interfaces.
 * @param port The port where the server is listening for connections.
 * @param sockets The dynamic array the created sockets are appended to.
 * @return True if at least one listening socket was created, otherwise false.
 */
static inline bool tkbc_server_socket_creation(const char *host, uint16_t port, Sockets *sockets) {
#ifdef _WIN32
    // MAKEWORD(2, 2) is a version, and wsaData will be filled with initialized
    // library information.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        tkbc_fprintf(stderr, "ERROR", "WSAStartup() failed: %ld\n", WSAGetLastError());
        return false;
    }
    tkbc_fprintf(stderr, "INFO", "Initialization of WSAStartup() succeed.\n");
#endif

    char port_string[16];
    snprintf(port_string, sizeof(port_string), "%u", (unsigned) port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *servinfo = NULL;
    int gai = getaddrinfo(host, port_string, &hints, &servinfo);
    if (gai != 0) {
        tkbc_fprintf(stderr, "ERROR", "getaddrinfo: %s\n", gai_strerror(gai));
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    for (struct addrinfo *rp = servinfo; rp != NULL; rp = rp->ai_next) {
        int socket_id = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_id == -1) {
            tkbc_socket_error("socket()");
            continue;
        }

        int option = 1;
        if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (const char *) &option, sizeof(option)) == -1) {
            tkbc_socket_error("setsockopt(SO_REUSEADDR)");
        }

        int nodelay = 1;
        if (setsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (const char *) &nodelay, sizeof(nodelay)) == -1) {
            tkbc_socket_error("setsockopt(TCP_NODELAY)");
        }

#ifdef IPV6_V6ONLY
        if (rp->ai_family == AF_INET6) {
            // Keep the families independent. A dual-stack IPv6 wildcard
            // socket would also claim the IPv4 port and the IPv4 socket could
            // not bind anymore. Both sockets are polled anyway.
            int v6only = 1;
            if (setsockopt(socket_id, IPPROTO_IPV6, IPV6_V6ONLY, (const char *) &v6only, sizeof(v6only)) == -1) {
                tkbc_socket_error("setsockopt(IPV6_V6ONLY)");
            }
        }
#endif

        if (bind(socket_id, rp->ai_addr, rp->ai_addrlen) == -1) {
            tkbc_socket_error("bind()");
            tkbc_close_socket(socket_id);
            continue;
        }
        if (listen(socket_id, SERVER_CONNETCTIONS) == -1) {
            tkbc_socket_error("listen()");
            tkbc_close_socket(socket_id);
            continue;
        }
        if (!tkbc_set_socket_non_blocking(socket_id)) {
            tkbc_close_socket(socket_id);
            continue;
        }

        char address[NI_MAXHOST + NI_MAXSERV + 4];
        char host_string[NI_MAXHOST];
        char service_string[NI_MAXSERV];
        if (getnameinfo(rp->ai_addr, rp->ai_addrlen, host_string, sizeof(host_string), service_string,
                        sizeof(service_string), NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
            if (rp->ai_family == AF_INET6) {
                snprintf(address, sizeof(address), "[%s]:%s", host_string, service_string);
            } else {
                snprintf(address, sizeof(address), "%s:%s", host_string, service_string);
            }
        } else {
            snprintf(address, sizeof(address), "port %s", port_string);
        }
        tkbc_fprintf(stderr, "INFO", "Listening on %s\n", address);

        tkbc_dap(sockets, socket_id);
    }

    freeaddrinfo(servinfo);

    if (sockets->count == 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not create a listening socket for %s:%s.\n", host ? host : "*",
                     port_string);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }
    return true;
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
 * @brief The function appends an image to a message in compressed PNG form.
 *
 * Raw images (the client colorizer) are exported to PNG here. Images that are
 * already stored as PNG (the server) are forwarded without decoding them.
 *
 * @param space The space that is used for the message buffer.
 * @param message The Message struct that should contain the serialized image
 * data.
 * @param kite_image The image whose data should be appended.
 * @param id The uuid that is serialized in front of the image data. This is the
 * uuid of the originator so that the receiver stores the image under the very
 * same uuid and does not register it a second time.
 */
static inline void tkbc_message_append_image_data(Space *space, Message *message, const Asset *asset) {
    Kite_Image kite_image = asset->as.kite_image;
    Image image = kite_image.normal;

    unsigned char *png_data = NULL;
    int png_size = 0;
    bool borrowed = false;
    if (image.format == TKBC_IMAGE_FORMAT_COMPRESSED_PNG) {
        png_data = (unsigned char *) image.data;
        png_size = kite_image.compressed_size;
        borrowed = true;
    } else {
        png_data = tkbc_export_image_png_fast(image, &png_size);
    }

    tkbc_message_write_uuid(message, space, asset->id);
    tkbc_message_write_bool(message, space, asset->as.is_dual_line);
    tkbc_message_write_s32(message, space, image.width);
    tkbc_message_write_s32(message, space, image.height);
    tkbc_message_write_s32(message, space, TKBC_IMAGE_ENCODING_PNG);
    tkbc_message_write_s32(message, space, png_size);
    tkbc_message_write_bytes(message, space, png_data, (size_t) png_size);

    if (!borrowed && png_data != NULL) {
        MemFree(png_data);
    }
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
        tkbc_message_append_image_data(space, message, asset);
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
