#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a SCRIPT_AMOUNT message.
 *
 * @param client The client where the amount of scripts should be assigned.
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @return True if the amount was parsed successfully, otherwise false.
 */
bool tkbc_messages_script_amount(Client *client, Message *reader) {
    if (!tkbc_message_read_u64(reader, &client->script_amount)) {
        return false;
    }
    return true;
}
