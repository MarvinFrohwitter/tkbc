#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

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
  env->server_script_id = atoi(lexer_token_to_cstr(lexer, &token));

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }

  env->server_script_frames_count = atoi(lexer_token_to_cstr(lexer, &token));

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }

  env->server_script_frames_index = atoi(lexer_token_to_cstr(lexer, &token));

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }
  env->script_finished = true;
  return true;
}
