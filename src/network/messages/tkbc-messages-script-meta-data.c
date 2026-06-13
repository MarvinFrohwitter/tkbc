#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include "../../choreographer/tkbc-script-handler.h"

#include <stdbool.h>
extern Client client;

/**
 * @brief [TODO:description]
 *
 * @param lexer [TODO:parameter]
 * @return [TODO:return]
 */
bool tkbc_messages_script_meta_data(Lexer *lexer) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }

  env->server_script_id = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }

  env->server_script_frames_count =
      strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }

  env->server_script_frames_index =
      strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  if (env->server_script_id == 0) {
    tkbc_unload_script(env);
    for (size_t i = 0; i < env->kite_array->count; ++i) {
      Kite_State *kite_state = &env->kite_array->elements[i];
      if (kite_state->is_script_kite) {
        kite_state->is_active = false;
        kite_state->is_kite_input_handler_active = false;
      } else {
        kite_state->is_active = true;
        kite_state->is_kite_input_handler_active = true;
      }
    }
  }

  return true;
}
