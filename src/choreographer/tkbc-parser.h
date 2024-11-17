#ifndef TKBC_PARSER_H
#define TKBC_PARSER_H

#include "../../external/lexer/tkbc-lexer.h"
#include "../global/tkbc-types.h"

const char *token_to_cstr(Token *token);
void tkbc_script_parser(Env *env);
bool tkbc_parse_ki_check(Env *env, Kite_Indexs *kis);
bool tkbc_parse_move(Env *env, Lexer *lexer, Action_Kind kind, Frames *frames,
                     Kite_Indexs *ki, bool brace);
bool tkbc_parse_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                         Frames *frames, Kite_Indexs *ki, bool brace);
bool tkbc_parse_tip_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                             Frames *frames, Kite_Indexs *ki, bool brace);

#endif // TKBC_PARSER_H
