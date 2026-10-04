#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../global/tkbc-types.h"

#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief The function verifies the hello message by comparing the received
 * greeting with the expected protocol greeting.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param greeting The expected greeting string that the received hello message
 * gets compared to.
 * @return Returns true if the greeting matches, otherwise false.
 */
bool tkbc_messages_hello_verification(Message *reader, const char *greeting) {
    bool ok = true;
    char *received = NULL;
    size_t received_len = 0;
    if (!tkbc_message_read_c_string(reader, space_get_tspace(), &received, &received_len)) {
        check_return(false);
    }

    bool result =
        received != NULL && received_len == strlen(greeting) && strncmp(received, greeting, received_len) == 0;
    if (!result) {
        tkbc_fprintf(stderr, "ERROR", "Hello message failed!\n");
        tkbc_fprintf(stderr, "ERROR", "Wrong protocol version!\n");
        check_return(false);
    }

check:
    space_reset_tspace();
    return ok;
}
