#ifndef TKBC_PARSER_H
#define TKBC_PARSER_H

#include "../../external/lexer/tkbc-lexer.h"
#include "../global/tkbc-types.h"

void tkbc_script_parser(Env *env);
bool tkbc_parse_kis_after_generation(Env *env, Lexer *lexer, Token *t,
                                     Kite_Ids *dest_kis, Kite_Ids orig_kis);
bool tkbc_parse_move(Env *env, Lexer *lexer, Action_Kind kind, Frames *frames,
                     Kite_Ids ki, bool brace, Content *tmp_buffer);
bool tkbc_parse_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                         Frames *frames, Kite_Ids ki, bool brace,
                         Content *tmp_buffer);
bool tkbc_parse_tip_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                             Frames *frames, Kite_Ids ki, bool brace,
                             Content *tmp_buffer);

#endif // TKBC_PARSER_H
