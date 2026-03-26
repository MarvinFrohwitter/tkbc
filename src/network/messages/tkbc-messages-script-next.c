#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-api.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

bool tkbc_messages_script_next(Lexer *lexer) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  int script_id = atoi(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  if (script_id == 0) {
    tkbc_unload_script(env);
    // This parsing function is just used in the server but liked in the client
    // as well so just a simple guard for compilation.
#ifdef TKBC_SERVER
    tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(0, 0, 0);
#endif
    goto no_script;
  }
  // TODO: Report possible failures of loading back to the client.
  tkbc_load_script_id(env, script_id);
  env->server_script_kite_max_count = 0;

  // TODO: Find a better way to do it reliable.
  // Generate kites if needed, if a script needs more kites than there are
  // currently registered.

  //
  // Activate the kites that belong to the script.
  Kite_Ids ids = {0};
  for (size_t i = 0; i < env->script->count; ++i) {

    // TODO: Just filter for the kites that are in the parsed script_id.

    for (size_t j = 0; j < env->script->elements[i].count; ++j) {
      Kite_Ids *kite_id_array =
          &env->script->elements[i].elements[j].kite_id_array;

      size_t frame_max_kites = kite_id_array->count;
      env->server_script_kite_max_count =
          tkbc_max(frame_max_kites, env->server_script_kite_max_count);

      for (size_t k = 0; k < kite_id_array->count; ++k) {
        Id id = kite_id_array->elements[k];
        if (!tkbc_contains_id(ids, id)) {
          tkbc_dap(&ids, id);
        }
      }
    }
  }

  for (size_t i = 0; i < env->kite_array->count; ++i) {
    env->kite_array->elements[i].is_active = false;
    for (size_t j = 0; j < ids.count; ++j) {
      if (ids.elements[j] == env->kite_array->elements[i].kite_id) {
        env->kite_array->elements[i].is_active = true;
        break;
      }
    }
  }
  free(ids.elements);

  // TODO: Find out if the assert actual triggers in any situation.
  // Otherwise delete the code below if also dies not handle the id
  // remapping.
  assert(env->server_script_kite_max_count <= env->kite_array->count);
  if (env->server_script_kite_max_count > env->kite_array->count) {
    size_t needed_kites =
        env->server_script_kite_max_count - env->kite_array->count;
    Kite_Ids kite_ids = tkbc_kite_array_generate(env, needed_kites);
    free(kite_ids.elements);
  }
no_script:
  return true;
}
