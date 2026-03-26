#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

bool tkbc_messages_script_scrub(Lexer *lexer) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  bool drag_left = atoi(lexer_token_to_cstr(lexer, &token));

  // TODO: Ensure a script is loaded. And the kite_ids are correctly mapped.
  {
    if (env->script->count <= 0) {
      return false;
    }

    env->script_finished = true;
    // The block indexes are assumed in order and at the corresponding
    // index.
    // This is needed to avoid a down cast of size_t to long or int that can
    // hold ever value of size_t.
    if (drag_left) {
      if (env->frames->frames_index > 0) {
        env->frames = &env->script->elements[env->frames->frames_index - 1];
      }
    } else {
      env->frames = &env->script->elements[env->frames->frames_index + 1];
    }

    // TODO: map the kite_ids before setting this.
    tkbc_set_kite_positions_from_kite_frames_positions(env);
  }

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  // This parsing function is just used in the server but liked in the client as
  // well so just a simple guard for compilation.
#ifdef TKBC_SERVER
  tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(
      env->script->script_id, env->script->count, env->frames->frames_index);
#endif
  return true;
}
