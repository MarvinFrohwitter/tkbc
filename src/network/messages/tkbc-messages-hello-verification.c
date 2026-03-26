#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../global/tkbc-types.h"

#include "tkbc-messages.h"

#include <stdbool.h>

extern Kite_Images kite_images;

/**
 * @brief [TODO:description]
 *
 * @param lexer [TODO:parameter]
 * @param greeting [TODO:parameter]
 * @return [TODO:return]
 */
bool tkbc_messages_hello_verification(Lexer *lexer, const char *greeting) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != STRINGLITERAL) {
    return false;
  }

  const char *compare = lexer_token_to_cstr(lexer, &token);
  if (strncmp(compare, greeting, strlen(greeting)) != 0) {
    tkbc_fprintf(stderr, "ERROR", "Hello message failed!\n");
    tkbc_fprintf(stderr, "ERROR", "Wrong protocol version!\n");
    return false;
  }
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }
  return true;
}
