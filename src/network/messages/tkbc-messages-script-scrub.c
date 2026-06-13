#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a SCRIPT_SCRUB message by scrubbing through the current script
 * timeline.
 *
 * @param lexer The lexer positioned at the message content.
 * @return True if the scrub was executed successfully, otherwise false.
 */
bool tkbc_messages_script_scrub(Lexer *lexer) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  bool drag_left = !!atoi(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  // No script is available nothing to scrub
  if (env->script == NULL) {
    return true;
  }

  // TODO: Ensure kite_ids are correctly mapped.
  {
    if (env->script->count <= 0) {
      return false;
    }

    // TODO: map the kite_ids before setting this inside the positions are
    // recalculated..
    tkbc_execute_scrub_slide(env, drag_left);
  }

  // This parsing function is just used in the server but liked in the client as
  // well so just a simple guard for compilation.
#ifdef TKBC_SERVER
  tkbc_message_script_meta_data_write_to_all_send_msg_buffers(
      env->script->script_id, env->script->count, env->frames->frames_index);
#endif
  return true;
}
