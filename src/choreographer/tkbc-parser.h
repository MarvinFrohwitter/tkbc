#ifndef TKBC_PARSER_H
#define TKBC_PARSER_H

#include "../../external/lexer/tkbc-lexer.h"
#include "../global/tkbc-types.h"

void tkbc_script_parser(Env *env);
bool tkbc_parse_kis_after_generation(Env *env, Lexer *lexer, Kite_Ids *dest_kis,
                                     Kite_Ids orig_kis);
bool tkbc_parse_move(Env *env, Lexer *lexer, Action_Kind kind, Kite_Ids ki,
                     bool brace, Content tmp_buffer);
bool tkbc_parse_rotation(Env *env, Lexer *lexer, Action_Kind kind, Kite_Ids ki,
                         bool brace, Content tmp_buffer);
bool tkbc_parse_tip_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                             Kite_Ids ki, bool brace, Content tmp_buffer);
bool tkbc_parse_float(float *number, Lexer *lexer, Content tmp_buffer);
bool tkbc_parse_size_t(size_t *number, Lexer *lexer, Content tmp_buffer);
bool tkbc_parse_team_figures(Env *env, Kite_Ids kis, Lexer *lexer,
                             const char *function_name, Content tmp_buffer);

#endif // TKBC_PARSER_H
