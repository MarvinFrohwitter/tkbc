// A C-Programming lexer
// Copyright (c) 2023-2026 Marvin Frohwitter. All Rights Reserved.

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef LEXER_H_
#define LEXER_H_
#include <stddef.h>

#ifndef BASICLEXDEF
#define BASICLEXDEF
#endif // BASICLEXDEF

#ifndef LEX_LOGERROR
// #define LEX_LOGERROR
#endif // LEX_LOGERROR

#ifdef LEXER_IMPLEMENTATION

#ifndef LEXDEF
#define LEXDEF static inline
#endif // LEXDEF

#else

#ifndef LEXDEF
#define LEXDEF
#endif // LEXDEF

#endif // LEXER_IMPLEMENTATION

typedef struct {
  unsigned char *elements;
  size_t count;
  size_t capacity;
} Lexer_Scratch_Buffer; // A representation of a file content.

typedef struct Lexer {
  unsigned char *content;
  unsigned long long int content_length;
  unsigned long long int position;
  unsigned long long int next_start_position;

  int isstrlit;
  unsigned long long int line_count;
  unsigned char *line_start;
  unsigned long long int column_count;
  const char *file_name;

  Lexer_Scratch_Buffer buffer;
} Lexer;

/* The variable that defines the escape characters. */
static const char ESCAPE[] = {'\n', '\r', '\t', '\v', '\f', '\\'};

/* The variable PUNCTUATORS is an array of single tokens of type PUNCTUATOR. */
/* It contains the tokens to match on. */
static const char PUNCTUATORS[] = {
    ',',  ']', ')', '=', ';', '{', '}', '&', '*', '+', '-', '~',  '|', '/',
    '\\', '%', '<', '>', '^', '|', '?', ':', '(', '[', '!', '\'', '.'};

#define KINDS_LIST                                                             \
  X_KIND(INVALID, -1)                                                          \
  X_KIND(NULL_TERMINATOR, 0)                                                   \
  X_KIND(ERROR, 1)                                                             \
  X_KIND(NUMBER, 2)                                                            \
  X_KIND(KEYWORD, 3)                                                           \
  X_KIND(STRINGLITERAL, 4)                                                     \
  X_KIND(IDENTIFIER, 5)                                                        \
  X_KIND(PUNCTUATOR, 6)                                                        \
  X_KIND(CONSTANT, 7)                                                          \
  X_KIND(PREPROCESSING, 8)                                                     \
  X_KIND(COMMENT, 9)                                                           \
  X_KIND(EOF_TOKEN, 10)                                                        \
  X_KIND(ASCII_LOW, 11)                                                        \
  X_KIND(ANSI_HIGH, 12)                                                        \
                                                                               \
  X_KIND(ASCII_DOLLAR, '$', /*      36*/)                                      \
  X_KIND(ASCII_AT, '@', /*          64*/)                                      \
  X_KIND(ASCII_BACKTICK, '`', /*    96*/)                                      \
                                                                               \
  X_KIND(PUNCT_SINGLEQUOTE, 39, /* "'"*/)                                      \
  X_KIND(PUNCT_DOUBLEQUOTE, 34, /* '"'*/)                                      \
  X_KIND(PUNCT_BACKSLASH, '\\', /*  92*/)                                      \
                                                                               \
  /* ALL PUNCTUATORS */                                                        \
                                                                               \
  X_KIND(PUNCT_NOT, '!', /*      33*/)                                         \
  X_KIND(PUNCT_HTAG, '#', /*     35*/)                                         \
  X_KIND(PUNCT_MOD, '%', /*      37*/)                                         \
  X_KIND(PUNCT_BAND, '&', /*     38*/)                                         \
  X_KIND(PUNCT_MUL, '*', /*      42*/)                                         \
  X_KIND(PUNCT_ADD, '+', /*      43*/)                                         \
  X_KIND(PUNCT_COMMA, ',', /*     44*/)                                        \
  X_KIND(PUNCT_SUB, '-', /*      45*/)                                         \
  X_KIND(PUNCT_DIV, '/', /*      47*/)                                         \
  X_KIND(PUNCT_LPAREN, '(', /*   50*/)                                         \
  X_KIND(PUNCT_RPAREN, ')', /*   51*/)                                         \
  X_KIND(PUNCT_POINT, '.', /*    56*/)                                         \
  X_KIND(PUNCT_COLON, ':', /*    58*/)                                         \
  X_KIND(PUNCT_SEMICOLON, ';', /* 59*/)                                        \
  X_KIND(PUNCT_LANGLE, '<', /*   60*/)                                         \
  X_KIND(PUNCT_ASSIGN, '=', /*   61*/)                                         \
  X_KIND(PUNCT_RANGLE, '>', /*   62*/)                                         \
  X_KIND(PUNCT_ASK, '?', /*      63*/)                                         \
  X_KIND(PUNCT_LBRACKET, '[', /* 91*/)                                         \
  X_KIND(PUNCT_RBRACKET, ']', /* 93*/)                                         \
  X_KIND(PUNCT_BXOR, '^', /*     94*/)                                         \
  X_KIND(PUNCT_LBRACE, '{', /*   123*/)                                        \
  X_KIND(PUNCT_BOR, '|', /*      124*/)                                        \
  X_KIND(PUNCT_RBRACE, '}', /*   125*/)                                        \
  X_KIND(PUNCT_BNOT, '~', /*     126*/)                                        \
  X_KIND(ASCII_DEL, 127, /*      127*/)                                        \
                                                                               \
  /* LANGECOLON = LBRACKET;    // '<: = ['        */                           \
  /* COLONRANGE = RBRACKET;    // ':> = ]'        */                           \
  /* LANGELMOD = LBRACE;       // '<% = {'        */                           \
  /* KANGELMOD = RBRACE;       // '%> = }'        */                           \
  /* MODCOLON = HTAG;          // '%: = #'        */                           \
  /* MODCOLONMODCOLON = HHTAG; // '%:%: = ##'     */                           \
                                                                               \
  X_KIND(PUNCT_EQUAL, 257, /*      '=='*/)                                     \
  X_KIND(PUNCT_MULEQ, 258, /*      '*='*/)                                     \
  X_KIND(PUNCT_DIVEQ, 259, /*      '/='*/)                                     \
  X_KIND(PUNCT_MODEQ, 260, /*      '%='*/)                                     \
  X_KIND(PUNCT_ADDEQ, 261, /*      '+='*/)                                     \
  X_KIND(PUNCT_SUBEQ, 262, /*      '-='*/)                                     \
  X_KIND(PUNCT_LEFTSHIFTEQ, 263, /*'<<='*/)                                    \
  X_KIND(PUNCT_RIGHTSHIFTEQ, 264, /*'>>='*/)                                   \
  X_KIND(PUNCT_BANDEQ, 265, /*     '&='*/)                                     \
  X_KIND(PUNCT_BOREQ, 266, /*      '|='*/)                                     \
  X_KIND(PUNCT_BXOREQ, 267, /*     '^='*/)                                     \
  X_KIND(PUNCT_NOTEQ, 268, /*      '!='*/)                                     \
  X_KIND(PUNCT_LOREQ, 269, /*      '<='*/)                                     \
  X_KIND(PUNCT_GOREQ, 270, /*      '>='*/)                                     \
  X_KIND(PUNCT_LAND, 271, /*       '&&'*/)                                     \
  X_KIND(PUNCT_LOR, 272, /*        '||'*/)                                     \
  X_KIND(PUNCT_LEFTSHIFT, 273, /*  '<<'*/)                                     \
  X_KIND(PUNCT_RIGHTSHIFT, 274, /* '>>'*/)                                     \
  X_KIND(PUNCT_PDEREF, 275, /*     '->'*/)                                     \
  X_KIND(PUNCT_DECREMENT, 276, /*  '--'*/)                                     \
  X_KIND(PUNCT_INCREMENT, 277, /*  '++'*/)                                     \
  X_KIND(PUNCT_HHTAG, 278, /*      '##'*/)                                     \
  X_KIND(PUNCT_VARIADIC, 279, /*   '...'*/)                                    \
                                                                               \
  /* ALL KEYWORDS */                                                           \
  X_KIND(KEYWORD_AUTO, 280)                                                    \
                                                                               \
  X_KIND(KEYWORD_BREAK, 281)                                                   \
  X_KIND(KEYWORD_CASE, 282)                                                    \
  X_KIND(KEYWORD_CHAR, 283)                                                    \
  X_KIND(KEYWORD_CONST, 284)                                                   \
  X_KIND(KEYWORD_CONTINUE, 285)                                                \
  X_KIND(KEYWORD_DEFAULT, 286)                                                 \
  X_KIND(KEYWORD_DO, 287)                                                      \
  X_KIND(KEYWORD_DOUBLE, 288)                                                  \
  X_KIND(KEYWORD_ELSE, 289)                                                    \
  X_KIND(KEYWORD_ENUM, 290)                                                    \
  X_KIND(KEYWORD_EXTERN, 291)                                                  \
  X_KIND(KEYWORD_FLOAT, 292)                                                   \
  X_KIND(KEYWORD_FOR, 293)                                                     \
  X_KIND(KEYWORD_GOTO, 294)                                                    \
  X_KIND(KEYWORD_IF, 295)                                                      \
  X_KIND(KEYWORD_INT, 296)                                                     \
  X_KIND(KEYWORD_LONG, 297)                                                    \
  X_KIND(KEYWORD_REGISTER, 298)                                                \
  X_KIND(KEYWORD_RETURN, 299)                                                  \
  X_KIND(KEYWORD_SHORT, 300)                                                   \
  X_KIND(KEYWORD_SIGNED, 301)                                                  \
  X_KIND(KEYWORD_SIZEOF, 302)                                                  \
  X_KIND(KEYWORD_STATIC, 303)                                                  \
  X_KIND(KEYWORD_STRUCT, 304)                                                  \
  X_KIND(KEYWORD_SWITCH, 305)                                                  \
  X_KIND(KEYWORD_TYPEDEF, 306)                                                 \
  X_KIND(KEYWORD_UNION, 307)                                                   \
  X_KIND(KEYWORD_UNSIGNED, 308)                                                \
  X_KIND(KEYWORD_VOID, 309)                                                    \
  X_KIND(KEYWORD_VOLATILE, 310)                                                \
  X_KIND(KEYWORD_WHILE, 311)                                                   \
  X_KIND(KEYWORD_SIZE_T, 312)

/* The Token Kind that can be detected by the lexer. */
typedef enum Kind {
#define X_KIND(name, value, ...) name = value,
  KINDS_LIST
#undef X_KIND
} Kind;

typedef struct Token {
  Kind kind;
  const unsigned char *content;
  size_t size;
} Token;

/** @brief The initial capacity of the dynamic arrays. */
#define LEXER_DAP_CAP 64

/**
 * @brief The macro pushes the new array element at the end of the dynamic
 * array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param element The given new element by value of the same type the array
 * holds elements.
 */
#define lexer_dap(dynamic_array, element)                                      \
  do {                                                                         \
    if ((dynamic_array)->capacity <= (dynamic_array)->count) {                 \
      if ((dynamic_array)->capacity == 0)                                      \
        (dynamic_array)->capacity = LEXER_DAP_CAP;                             \
      else                                                                     \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
                                                                               \
      (dynamic_array)->elements = realloc((dynamic_array)->elements,           \
                                          sizeof(*(dynamic_array)->elements) * \
                                              (dynamic_array)->capacity);      \
                                                                               \
      if ((dynamic_array)->elements == NULL) {                                 \
        fprintf(                                                               \
            stderr,                                                            \
            "The allocation for the dynamic array has failed in: %s: %d\n",    \
            __FILE__, __LINE__);                                               \
        abort();                                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    (dynamic_array)->elements[(dynamic_array)->count] = (element);             \
    (dynamic_array)->count = (dynamic_array)->count + 1;                       \
  } while (0)

/**
 * @brief The macro pushes the given amount of new elements to the end of the
 * dynamic array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param new_element The given new elements by pointer of the same type the
 * array holds elements.
 * @param new_elements_count The amount of elements to add to the array.
 */
#define lexer_dapc(dynamic_array, new_elements, new_elements_count)            \
  do {                                                                         \
    if (new_elements != NULL) {                                                \
      if ((dynamic_array)->capacity <                                          \
          (dynamic_array)->count + new_elements_count) {                       \
        if ((dynamic_array)->capacity == 0) {                                  \
          (dynamic_array)->capacity = LEXER_DAP_CAP;                           \
        }                                                                      \
        while ((dynamic_array)->capacity <                                     \
               (dynamic_array)->count + new_elements_count) {                  \
          (dynamic_array)->capacity = (dynamic_array)->capacity * 2;           \
        }                                                                      \
        (dynamic_array)->elements = realloc(                                   \
            (dynamic_array)->elements,                                         \
            sizeof(*(dynamic_array)->elements) * (dynamic_array)->capacity);   \
        if ((dynamic_array)->elements == NULL) {                               \
          fprintf(                                                             \
              stderr,                                                          \
              "The allocation for the dynamic array has failed in: %s: %d\n",  \
              __FILE__, __LINE__);                                             \
          abort();                                                             \
        }                                                                      \
      }                                                                        \
      memcpy((dynamic_array)->elements + (dynamic_array)->count,               \
             (new_elements),                                                   \
             sizeof(*(dynamic_array)->elements) * (new_elements_count));       \
      (dynamic_array)->count = (dynamic_array)->count + (new_elements_count);  \
    }                                                                          \
  } while (0)

/* ========================================================================= */
/* ============================ PUBLIC FUNCTIONS =========================== */
/* ========================================================================= */

BASICLEXDEF Lexer *lexer_new(const char *file_path, char *content, size_t size,
                             size_t position);
BASICLEXDEF void lexer_del(Lexer *lexer);
BASICLEXDEF Token lexer_next(Lexer *lexer);
const char *lexer_token_to_cstr(Lexer *lexer, Token *token);

/* ========================================================================= */
/* ============================ PRIVAT FUNCTIONS =========================== */
/* ========================================================================= */

LEXDEF Token lexer_eof_token(Lexer *lexer);
LEXDEF Token lexer_error(Lexer *lexer);
LEXDEF void lexer_trace_token(Lexer *lexer, Token *token);
LEXDEF Token lexer_invalid_token(Lexer *lexer);
LEXDEF Token lexer_chop_char(Lexer *lexer, size_t count);

LEXDEF int lexer_check_is_number(Lexer *lexer, Token *token);
LEXDEF int lexer_check_is_preproc(Lexer *lexer, Token *token);
LEXDEF int lexer_check_is_str(Lexer *lexer, Token *token);
LEXDEF int lexer_check_is_comment(Lexer *lexer, Token *token, int is_multi);

LEXDEF void lexer_trim_left(Lexer *lexer);
LEXDEF int lexer_next_char_is(Lexer *lexer, char c);
LEXDEF int lexer_char_is(Lexer *lexer, char c);
LEXDEF int lexer_is_escape_seq_or_space(Lexer *lexer);
LEXDEF int lexer_is_punctuator_exclude(Lexer *lexer, char exclude);
LEXDEF int lexer_is_punctuator(Lexer *lexer);
LEXDEF int lexer_check_punctuator_lookahead(Lexer *lexer);
LEXDEF int lexer_check_boundary(Lexer *lexer);
LEXDEF int lexer_check_boundary_next(Lexer *lexer);

LEXDEF int lexer_keyword_set_token(Lexer *lexer, Token *token, size_t length);
LEXDEF int lexer_punctuator_set_token(Lexer *lexer, Token *token,
                                      size_t length);
LEXDEF const char *lexer_kind_to_str(Kind kind);

/* ========================================================================= */
LEXDEF int is_escape_seq(char c);
LEXDEF int is_sybol_alnum_and_(char c);
LEXDEF int is_sybol_alpha_and_(char c);
/* ========================================================================= */

#endif // LEXER_H_

#ifdef LEXER_IMPLEMENTATION

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The function lexer_new() creates the new state of a lexer. */
/* @param content The content that the lexer has to tokenise. */
/* @param size The content length. */
/* @param The position of the content the lexer should begin tokenise. */
/* @return Lexer The function returns a new initialized lexer. */
BASICLEXDEF Lexer *lexer_new(const char *file_path, char *content, size_t size,
                             size_t position) {
  Lexer *lexer = calloc(1, sizeof(Lexer));
  if (lexer == NULL) {
#ifdef LEX_LOGERROR
    fprintf(stderr,
            "ERROR: No more memory for the content to lex can be allocated.\n");
#endif // LEX_LOGERROR
    return NULL;
  }

  lexer->content_length = size;
  lexer->content = (unsigned char *)content;
  lexer->position = position;
  lexer->next_start_position = lexer->position;
  lexer->file_name = file_path;
  lexer->line_count = 1;
  lexer->line_start = lexer->content;
  lexer->column_count = 1;
  lexer->isstrlit = 0;

  return lexer;
}

/* The function lexer_del() frees the allocated lexer. */
/* @param lexer The given Lexer that contains the current state. */
BASICLEXDEF void lexer_del(Lexer *lexer) {
  if (lexer->content) {
    free(lexer->content);
  }
  if (lexer->buffer.elements) {
    free(lexer->buffer.elements);
  }
  free(lexer);
}

/**
 * @brief The function token_to_cstr() returns a pointer in the given buffer
 * that is null terminated.
 *
 * @param lexer The given Lexer that contains the current state.
 * @param token The sized token that should be appended to the buffer.
 * @return The null terminated c-string.
 */
const char *lexer_token_to_cstr(Lexer *lexer, Token *token) {
  lexer->buffer.count = 0;
  if (token->kind == NULL_TERMINATOR) {
    lexer_dapc(&lexer->buffer, "\\0", 2);

  } else {
    lexer_dapc(&lexer->buffer, token->content, token->size);
  }
  lexer_dap(&lexer->buffer, 0);
  return (char *)lexer->buffer.elements;
}

/* The function lexer_chop_char() consumes the amount of chars given by the */
/* count. So it cuts of the content that the lexer tokenise form the left. */
/* @param lexer The given Lexer that contains the current state. */
/* @param count The amount of chars that will be choped from the left of the */
/* content. */
/* @return Token the Error token or the Invalid token. */
LEXDEF Token lexer_chop_char(Lexer *lexer, size_t count) {
  if (!lexer_check_boundary(lexer)) {
    return lexer_invalid_token(lexer);
  }
  for (size_t i = 0; i < count; i++) {
    if (lexer->content[lexer->position] == '\n') {
      lexer->line_count++;
      if (lexer_check_boundary_next(lexer)) {
        lexer->line_start = &lexer->content[lexer->position + 1];
      }
    }
    if (!lexer_check_boundary(lexer)) {
      return lexer_error(lexer);
    }
    lexer->position = lexer->position + 1;
  }

  return lexer_invalid_token(lexer);
}

/* The function lexer_invalid_token() sets the lexer values to the type of an */
/* invalid token. */
/* @param lexer The given Lexer that contains the current state. */
/* @return Token that has values of kind=INVALID, size=1 and content= current */
/* lexer position. */
LEXDEF Token lexer_invalid_token(Lexer *lexer) {
  Token token;
  token.kind = INVALID;
  token.size = 1;
  token.content = &lexer->content[lexer->position];
  return token;
}

/* The function check_boundary() checks for the length of the lexer content, if
 */
/* the current lexer position is in the content length. */
/* @param lexer The given Lexer that contains the current state. */
/* @return boolean True, if the lexer position is in the content length, */
/* otherwise false. */
LEXDEF int lexer_check_boundary(Lexer *lexer) {
  if (lexer->position < lexer->content_length) {
    return 1;
  }
  return 0;
}

/* The function check_boundary_next() checks for the length of the lexer
 * content, if the current lexer position+1 is in the content length. */
/* @param lexer The given Lexer that contains the current state. */
/* @return boolean True, if the lexer position+1 is in the content length, */
/* otherwise false. */
LEXDEF int lexer_check_boundary_next(Lexer *lexer) {
  if (lexer->position + 1 < lexer->content_length) {
    return 1;
  }
  return 0;
}

/* The function lexer_next_char_is() returns, if the next character in the */
/* content is  equal to the given char c. */
/* @param lexer The given Lexer that contains the current state. */
/* @param c The character that will be compared with the lexer position. */
/* @return boolean True, if the given char c is identical to the next char in */
/* the content that lexer tokenise. */
LEXDEF int lexer_next_char_is(Lexer *lexer, char c) {

  if (c == lexer->content[lexer->position + 1]) {

    return 1;
  }
  return 0;
}

/* The function lexer_char_is() returns, if the character in the content is */
/* equal to the given char c. */
/* @param lexer The given Lexer that contains the current state. */
/* @param c The character that will be compared with the lexer position. */
/* @return boolean True, if the given char c is identical to the current char in
 */
/* the content that lexer tokenise. */
LEXDEF int lexer_char_is(Lexer *lexer, char c) {
  if (c == lexer->content[lexer->position]) {
    return 1;
  }
  return 0;
}

/* The function lexer_trim_left() consumes the characters from the left side of
 */
/* the given content. If the content matches isspace(). */
/* White space Characters: Blank space, newline, horizontal tab, carriage */
/* return and form feed. */
/* @param lexer The given Lexer that contains the current state. */
LEXDEF void lexer_trim_left(Lexer *lexer) {
  while (lexer_check_boundary(lexer) &&
         isspace(lexer->content[lexer->position])) {
    lexer_chop_char(lexer, 1);
  }
}

/* The function lexer_is_punctuator() returns, if the lexer has found a */
/* punctuator of the given length at the current lexer content position. */
/* @param lexer The given Lexer that contains the current state. */
/* @param exclude The character that should be excluded from the check. */
/* Pass 0 if the complete default list of punctuators should be searched thru.
 */
/* @return boolean True, if the content at the current position has a punctuator
 */
/* of given length, otherwise false. */
LEXDEF int lexer_is_punctuator_exclude(Lexer *lexer, char exclude) {
  for (size_t i = 0; i < sizeof(PUNCTUATORS) / sizeof(PUNCTUATORS[0]); i++) {
    if (PUNCTUATORS[i] == exclude) {
      continue;
    }
    if (PUNCTUATORS[i] == lexer->content[lexer->position]) {
      return 1;
    }
  }
  return 0;
}

/* The function lexer_is_punctuator() returns, if the lexer has found a */
/* punctuator of the given length at the current lexer content position. */
/* @param lexer The given Lexer that contains the current state. */
/* @return boolean True, if the content at the current position has a punctuator
 */
/* of given length, otherwise false. */
LEXDEF int lexer_is_punctuator(Lexer *lexer) {
  return lexer_is_punctuator_exclude(lexer, 0);
}

/** The function lexer_check_punctuator_lookahead() computes, if the next
 * character can also be part of the punctuator.
 * @param lexer The given Lexer that contains the current state.
 * @return boolean True, if the next character is also part of the punctuator,
 * otherwise false. On error -1.*/
LEXDEF int lexer_check_punctuator_lookahead(Lexer *lexer) {

  if (!lexer_check_boundary_next(lexer)) {
    return -1;
  }
  lexer_chop_char(lexer, 1);
  if (!lexer_check_boundary_next(lexer)) {
    lexer->position -= 1;
    return -1;
  }
  lexer->position -= 1;

  switch (lexer->content[lexer->position]) {
  case '<': {
    if (lexer_next_char_is(lexer, '<') &&
        lexer->content[lexer->position + 2] == '=') {
      lexer_chop_char(lexer, 3);
      return 1;
    }
    if (lexer_next_char_is(lexer, '=') || lexer_next_char_is(lexer, '<')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }

  } break;
  case '>': {
    if (lexer_next_char_is(lexer, '>') &&
        lexer->content[lexer->position + 2] == '=') {
      lexer_chop_char(lexer, 3);
      return 1;
    }
    if (lexer_next_char_is(lexer, '=') || lexer_next_char_is(lexer, '>')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }

  } break;
  case '.': {
    if (lexer_next_char_is(lexer, '.') &&
        lexer->content[lexer->position + 2] == '.') {
      lexer_chop_char(lexer, 3);
      return 1;
    }
  } break;
  case '=': {
    if (lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  case '!': {
    if (lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  case '&': {
    if (lexer_next_char_is(lexer, '&') || lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }

  } break;
  case '|': {
    if (lexer_next_char_is(lexer, '|') || lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }

  } break;
  case '-': {
    if (lexer_next_char_is(lexer, '>') || lexer_next_char_is(lexer, '-') ||
        lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }

  } break;
  case '+': {
    if (lexer_next_char_is(lexer, '+') || lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }

  } break;
  case '*': {
    if (lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  case '/': {
    if (lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  case '%': {
    if (lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  case '^': {
    if (lexer_next_char_is(lexer, '=')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  case '#': {
    if (lexer_next_char_is(lexer, '#')) {
      lexer_chop_char(lexer, 2);
      return 1;
    }
  } break;
  default:
    lexer_chop_char(lexer, 1);
    return 0;
  }
  lexer_chop_char(lexer, 1);
  return 0;
}

/* The function lexer_check_is_number() computes if the given part to 'lex' is
 * a number. It then computes the corresponding token or calls the lexer error
 * token handler method. */
/* @param lexer The given Lexer that contains the current state. */
/* @param token The token that will be modified and contains the final token
 * that can also contain the error token that is passed back up in the call
 * stack. */
/* @return int The error code of the corresponding action result. It returns
 * one if the function computes a number in the token or 0 if the token could
 * potentially be a punctuator. -1 if the token is the error token. */
LEXDEF int lexer_check_is_number(Lexer *lexer, Token *token) {
  int is_floating = 0;
  token->kind = NUMBER;
  size_t pos = lexer->position;

  if (lexer_char_is(lexer, '0') && lexer_check_boundary_next(lexer) &&
      (lexer_next_char_is(lexer, 'b') || lexer_next_char_is(lexer, 'B'))) {
    lexer_chop_char(lexer, 2);

    if (!lexer_check_boundary(lexer) ||
        !(lexer_char_is(lexer, '0') || lexer_char_is(lexer, '1'))) {
      goto errortoken;
    }

    while (lexer_check_boundary(lexer) &&
           (lexer_char_is(lexer, '0') || lexer_char_is(lexer, '1'))) {
      lexer_chop_char(lexer, 1);
    }

    if (!lexer_check_boundary(lexer)) {
      goto compute_return;
    }

  } else if (lexer_char_is(lexer, '0') && lexer_check_boundary_next(lexer) &&
             (lexer_next_char_is(lexer, 'x') ||
              lexer_next_char_is(lexer, 'X'))) {
    lexer_chop_char(lexer, 2);

    if (!lexer_check_boundary(lexer) ||
        !isxdigit(lexer->content[lexer->position])) {
      if (!lexer_char_is(lexer, '.')) {
        goto errortoken;
      }
    }

    int is_exponent = 0;
    while (lexer_check_boundary(lexer)) {
      if (isxdigit(lexer->content[lexer->position])) {
        if (is_exponent && !isdigit(lexer->content[lexer->position])) {
          if (lexer_char_is(lexer, 'f') || lexer_char_is(lexer, 'F')) {
            break;
          }
          goto errortoken;
        }
      } else if (lexer->content[lexer->position] == '.') {
        is_floating += 1;
        if (is_floating > 1 || is_exponent) {
          goto errortoken;
        }
      } else if (lexer->content[lexer->position] == 'p' ||
                 lexer->content[lexer->position] == 'P') {
        is_exponent += 1;
        if (is_exponent != 1) {
          goto errortoken;
        }
        if (!isxdigit(lexer->content[lexer->position - 1])) {
          goto errortoken;
        }
      } else if (lexer->content[lexer->position] == '-' ||
                 lexer->content[lexer->position] == '+') {
        if (!(lexer->content[lexer->position - 1] == 'p' ||
              lexer->content[lexer->position - 1] == 'P')) {
          goto compute_return;
        }
      } else {
        break;
      }

      {
        Token token = lexer_chop_char(lexer, 1);
        if (token.kind == ERROR)
          goto errortoken;
      }
    }

    if (is_exponent) {
      // This is just for the case where the exponent is defined but not
      // a dot before, but a suffix can still happen, so that the
      // postfixcheck will compute the suffix, just set it to floating.
      is_floating += 1;
      goto postfixcheck;
    }

    if (!lexer_check_boundary(lexer)) {
      goto compute_return;
    }

  } else {

    {
      Token token = lexer_chop_char(lexer, 1);
      if (token.kind == ERROR)
        goto errortoken;
    }

    char c = lexer->content[lexer->position];
    if (lexer->content[lexer->position - 1] == '.' &&
        (isalpha(c) || isspace(c) || lexer_char_is(lexer, '_') ||
         lexer_is_punctuator(lexer))) {
      goto punctuator;
    }
    lexer->position = pos;

    while (lexer_check_boundary(lexer) &&
           (isdigit(lexer->content[lexer->position]) ||
            lexer->content[lexer->position] == '.') &&
           !is_escape_seq(lexer->content[lexer->position])) {

      if (lexer_char_is(lexer, '.')) {
        is_floating += 1;
      }

      if (is_floating > 1) {
        goto errortoken;
      }

      {
        Token token = lexer_chop_char(lexer, 1);
        if (token.kind == ERROR)
          goto errortoken;
      }
    }

    if (!lexer_check_boundary(lexer)) {
      goto compute_return;
    }

    if (lexer_char_is(lexer, 'e') || lexer_char_is(lexer, 'E')) {
      {
        Token token = lexer_chop_char(lexer, 1);
        if (token.kind == ERROR)
          goto errortoken;
      }
      if (lexer_char_is(lexer, '-') || lexer_char_is(lexer, '+') ||
          isdigit(lexer->content[lexer->position])) {
        {
          Token token = lexer_chop_char(lexer, 1);
          if (token.kind == ERROR)
            goto errortoken;
        }
      }

      while (lexer_check_boundary(lexer) &&
             isdigit(lexer->content[lexer->position])) {

        lexer_chop_char(lexer, 1);
      }

      if (!lexer_check_boundary(lexer)) {
        goto compute_return;
      }
      goto postfixcheck;
    }
  }

postfixcheck: {
  size_t maxiter = 4;
  if (is_floating) {
    switch (lexer->content[lexer->position]) {
    case 'L':
    case 'l':
    case 'F':
    case 'f': {
      lexer_chop_char(lexer, 1);
      goto compute_return;
    } break;
    case ' ':
      goto compute_return;
      break;
    default:
      if (lexer_check_boundary(lexer) &&
          (lexer_is_punctuator_exclude(lexer, '.') ||
           is_escape_seq(lexer->content[lexer->position]))) {
        goto compute_return;
        break;
      } else if (!lexer_check_boundary(lexer)) {
        break;
      } else {
        goto errortoken;
      }
    }
    maxiter = 0;
  }

  // The 4th iteration handles the next char so, if the next char is not a
  // space the default catches the error.
  for (size_t i = 0; i < maxiter; i++) {
    switch (lexer->content[lexer->position]) {
    case 'L':
    case 'l':
    case 'U':
    case 'u': {
      lexer_chop_char(lexer, 1);
      break;
    }
    case 'F':
    case 'f': {
      lexer_chop_char(lexer, 1);
      i = maxiter;
      break;
    }
    case ' ': {
      i = maxiter;
      break;
    }
    default:
      if (lexer_check_boundary(lexer) &&
          (lexer_is_punctuator_exclude(lexer, '.') ||
           is_escape_seq(lexer->content[lexer->position]))) {
        i = maxiter;
        break;
      } else if (!lexer_check_boundary(lexer)) {
        i = maxiter;
        break;
      } else {
        goto errortoken;
      }
    }
  }
}

compute_return:
  token->size = lexer->position - pos;
  return 1;

punctuator: {
  // Is probably a punctuator.
  lexer->position = pos;
  return 0;
}

errortoken: {
  Token t = lexer_error(lexer);

  token->content = t.content;
  token->size = t.size;
  token->kind = t.kind;
  return -1;
}
}

/* The function lexer_check_is_preproc() computes the token for the
 * preprocessor. */
/* @param lexer The given Lexer that contains the current state. */
/* @param token The token that will be modified and contains the final token
 * that is passed up in the call stack. */
/* @return boolean 1, if the function check is passed, otherwise 0. */
LEXDEF int lexer_check_is_preproc(Lexer *lexer, Token *token) {

  size_t position = lexer->position;
  lexer_chop_char(lexer, 1);
  if (lexer->position >= lexer->content_length) {
    lexer->position = lexer->position - 1;
    return 0;
  }

  while (lexer_check_boundary(lexer) && !lexer_char_is(lexer, '\n')) {
    lexer_chop_char(lexer, 1);
  }

  token->size = lexer->position - position;

  return 1;
}

/* The function lexer_check_is_comment() computes the comment token */
/* @param lexer The given Lexer that contains the current state. */
/* @param token The token that will be modified and contains the final token
 * that is passed up in the call stack. */
/* @return boolean 1, if the function check is passed, otherwise 0. */
LEXDEF int lexer_check_is_comment(Lexer *lexer, Token *token, int is_multi) {

  size_t position = lexer->position;
  {
    Token token = lexer_chop_char(lexer, 2);
    if (token.kind == ERROR) {
      return 0;
    }
  }

  while (lexer_check_boundary(lexer)) {
    if (is_multi) {
      if (lexer_char_is(lexer, '*') && lexer_check_boundary_next(lexer) &&
          lexer_next_char_is(lexer, '/')) {
        lexer_chop_char(lexer, 2);
        break;
      }
    } else {
      if (lexer_char_is(lexer, '\n'))
        break;
    }
    lexer_chop_char(lexer, 1);
  }

  token->size = lexer->position - position;
  return 1;
}

/* The function lexer_check_is_str() computes the possible string token */
/* @param lexer The given Lexer that contains the current state. */
/* @param token The token that will be modified and contains the final token
 * that is passed up in the call stack. */
/* @return boolean 1, if the function check is passed, otherwise 0. */
LEXDEF int lexer_check_is_str(Lexer *lexer, Token *token) {
  if (lexer->position > 0) {
    if ((long long int)lexer->position - 1 < 0) {
      lexer_chop_char(lexer, 1);
      // This ensures that the escape double quotes is in the boundary.
      return 0;
    }
    if (lexer->content[lexer->position - 1] == '\\' ||
        lexer->content[lexer->position - 1] == '\'') {
      lexer_chop_char(lexer, 1);
      token->kind = PUNCT_DOUBLEQUOTE;
      token->size = 1;
      return 1;
    }
  }

  token->kind = STRINGLITERAL;
  lexer->isstrlit = 1;
  size_t position = lexer->position;
  lexer_chop_char(lexer, 1);

  if (!lexer_check_boundary(lexer)) {
    return 0;
  }

  while (lexer_check_boundary(lexer)) {
    if (lexer_char_is(lexer, '"')) {

      if ((long long int)lexer->position - 2 >= 0) {
        if (lexer->content[lexer->position - 1] == '\\' &&
            lexer->content[lexer->position - 2] != '\\') {
          lexer_chop_char(lexer, 1);
          continue;
        }
      }

      lexer_chop_char(lexer, 1);
      break;
    } else if (is_escape_seq(lexer->content[lexer->position])) {
      Token token = lexer_chop_char(lexer, 1);
      if (token.kind == ERROR) {
        return 0;
      }
    } else {
      lexer_chop_char(lexer, 1);
      // This is for EOF errors where the string literal is not closed.
      if (!lexer_check_boundary(lexer)) {
        return 0;
      }
    }
  }
  token->size = lexer->position - position;
  return 1;
}

/* The function ilexer_is_escape_seq_or_space() returns, if the given char is
 * equal to an escape character or a space. */
/* @param lexer The given Lexer that contains the current state. */
/* @return boolean True, if the current lexer position is an escape char or a
 * space, otherwise false. */
LEXDEF int lexer_is_escape_seq_or_space(Lexer *lexer) {
  return is_escape_seq(lexer->content[lexer->position]) ||
         lexer_char_is(lexer, ' ');
}

/* =========================================================================
 */

/* The function is_escape_seq() returns, if the given char is equal to an
 * escape
 */
/* character. */
/* @param c The char to compare. */
/* @return boolean True, if the given character is an escape char, otherwise
 */
/* false. */
LEXDEF int is_escape_seq(char c) {
  size_t length = sizeof(ESCAPE) / sizeof(ESCAPE[0]);
  for (size_t j = 0; j < length; j++) {
    if (c == ESCAPE[j]) {
      return 1;
    }
  }
  return 0;
}

/* The function is_sybol_alnum_and_() computes the, if the characters in the
 */
/* name of an identifier is part of the alnum group and the underscore. */
/* @param c The character to check for. */
/* @return boolean True, if the given character is part of the alnum group and
 */
/* the additional underscore. */
LEXDEF int is_sybol_alnum_and_(char c) { return isalnum(c) || c == '_'; }

/* The function is_sybol_alpha_and_() computes the, if the characters in the
 */
/* name of an identifier is part of the alpha group and the underscore. */
/* @param c The character to check for. */
/* @return boolean True, if the given character is part of the alpha group and
 */
/* the additional underscore. */
LEXDEF int is_sybol_alpha_and_(char c) { return isalpha(c) || c == '_'; }

/* =========================================================================
 */

/* The function lexer_next() computes the token by type, size and contents. */
/* Token Kind that can be detected are KEYWORD, NUMBER, IDENTIFIER, CONSTANT,
 */
/* STRINGLITERAL, PUNCTUATOR, INVALID */
/* @param lexer The given Lexer that contains the current state. */
/* @return Token The next found token in the given content */
BASICLEXDEF Token lexer_next(Lexer *lexer) {
  Token token = {0};
  token.kind = INVALID;
  if (lexer->content_length == 0 || !lexer->content) {
    token = lexer_eof_token(lexer);
    lexer->column_count = 0;
    lexer->line_count = 0;
    return token;
  }

  lexer->isstrlit = 0;
  lexer->next_start_position = lexer->position;
  lexer->column_count =
      &lexer->content[lexer->position] - lexer->line_start + 1;

  lexer_trim_left(lexer);
  if (lexer->position >= lexer->content_length) {
    return lexer_eof_token(lexer);
  }

  token.content = &lexer->content[lexer->position];

  // Check for null terminator
  if (lexer_char_is(lexer, '\0')) {
    lexer_chop_char(lexer, 1);
    token.kind = NULL_TERMINATOR;
    token.size = 1;
    return token;
  }

  // Check for preprocessing
  if (lexer_char_is(lexer, '#')) {
    token.kind = PREPROCESSING;

    if (!lexer_check_is_preproc(lexer, &token)) {
      return lexer_error(lexer);
    };
    return token;
  }

  // Check for Comments
  if (lexer_char_is(lexer, '/') && lexer_check_boundary_next(lexer) &&
      lexer_next_char_is(lexer, '/')) {

    if (!lexer_check_is_comment(lexer, &token, 0)) {
      return lexer_error(lexer);
    };
    token.kind = COMMENT;
    return token;
  }

  // Check for multi line Comments
  if (lexer_char_is(lexer, '/') && lexer_check_boundary_next(lexer) &&
      lexer_next_char_is(lexer, '*')) {

    if (!lexer_check_is_comment(lexer, &token, 1)) {
      return lexer_error(lexer);
    };
    token.kind = COMMENT;
    return token;
  }

  // Check for string literals
  if (lexer_char_is(lexer, '"')) {
    if (!lexer_check_is_str(lexer, &token)) {
      return lexer_error(lexer);
    };
    return token;
  }

  // Check for numbers
  if (isdigit(lexer->content[lexer->position]) || lexer_char_is(lexer, '.')) {
    if (lexer_check_is_number(lexer, &token)) {
      return token;
    }
    token.kind = INVALID;
  }

#ifdef LEX_NUMBER_SIGN
  if ((lexer_char_is(lexer, '-') || lexer_char_is(lexer, '+'))) {
    lexer_chop_char(lexer, 1);

    if (isdigit(lexer->content[lexer->position]) || lexer_char_is(lexer, '.')) {
      if (lexer_check_is_number(lexer, &token)) {
        token.size++;
        return token;
      }
      token.kind = INVALID;
    }
    lexer->position--;
  }
#endif // LEX_NUMBER_SIGN

  // Check for keywords
  if (isalpha(lexer->content[lexer->position])) {

    size_t startpos = token.size = lexer->position;
    lexer_chop_char(lexer, 1);
    while (lexer_check_boundary(lexer) &&
           !lexer_is_escape_seq_or_space(lexer)) {

      lexer_chop_char(lexer, 1);
    }

    token.size = lexer->position - token.size;

    lexer->position = lexer->position - token.size;
    if (lexer_keyword_set_token(lexer, &token, token.size)) {

      lexer_chop_char(lexer, token.size);

      if (token.size < 2) {
        goto single_token;
      }
#ifndef EXLEX_IMPLEMENTATION
      token.kind = KEYWORD;
#endif /* EXLEX_IMPLEMENTATION */

      return token;
    } else {
    single_token:
      token.kind = INVALID;
      lexer->position = startpos;
    }
  }

  // Fall throw KEYWORDS to check for identifiers
  if (is_sybol_alpha_and_(lexer->content[lexer->position])) {
    token.kind = IDENTIFIER;
    token.size = lexer->position;
    lexer_chop_char(lexer, 1);
    while (lexer_check_boundary(lexer) &&
           is_sybol_alnum_and_(lexer->content[lexer->position])) {
      lexer_chop_char(lexer, 1);
    }
    token.size = lexer->position - token.size;
    return token;
  }

  // Check for punctuators
  if (lexer_check_boundary(lexer)) {
    token.size = lexer->position;
    if (lexer_is_punctuator(lexer)) {

      // TODO: Check for
      // '\0':  // @TODO ocatal constants
      // '\x': '\X': // @TODO hex constants
      // '\u': // @TODO unicode constants

#ifdef LEXLOOKAHEAD

      int lookahead = lexer_check_punctuator_lookahead(lexer);
      if (lookahead < 1) {
        token.size = 1;

      } else {
        token.size = lexer->position - token.size;
      }

      if (lookahead == -1) {
        lexer_punctuator_set_token(lexer, &token, token.size);
        lexer_chop_char(lexer, 1);
      } else {
        lexer->position = lexer->position - token.size;
        lexer_punctuator_set_token(lexer, &token, token.size);
        lexer_chop_char(lexer, token.size);
      }
#else
      token.size = 1;
      lexer_punctuator_set_token(lexer, &token, token.size);
      lexer_chop_char(lexer, 1);
#endif /* LEXLOOKAHEAD */
#ifndef EXLEX_IMPLEMENTATION
      token.kind = PUNCTUATOR;
#endif /* EXLEX_IMPLEMENTATION */

      return token;
    }

    // Manual ANSI range check for remaining unchecked values.
    uint32_t temp_val = lexer->content[lexer->position];
    // This is check is made like that to be able to switch to Unicode later and
    // forget this check.
    if (256 > temp_val) {
      token.kind = ANSI_HIGH;
    }

    if (' ' > temp_val) {
      token.kind = ASCII_LOW;
    }

    if (ASCII_DEL == temp_val) {
      token.kind = ASCII_DEL;
    }

    if (ASCII_DOLLAR == temp_val) {
      token.kind = ASCII_DOLLAR;
    }

    if (ASCII_AT == temp_val) {
      token.kind = ASCII_AT;
    }

    if (ASCII_BACKTICK == temp_val) {
      token.kind = ASCII_BACKTICK;
    }
  }

  if (!lexer_check_boundary(lexer)) {
    return lexer_eof_token(lexer);
  } else {
    lexer_chop_char(lexer, 1);
  }
  token.size = 1;
  return token;
}

/* The  function creates the EOF_TOKEN */
/* @return token The token that will be modified and contains the EOF_TOKEN */
LEXDEF Token lexer_eof_token(Lexer *lexer) {
  Token token;
  token.kind = EOF_TOKEN;
  token.size = 0;
  token.content = NULL;
  if (lexer->column_count > 1) {
    lexer->column_count -= 1;
  }
  return token;
}

/* The function lexer_trace_token() try's to detect the first part of the */
/* broken token */
/* @param lexer The Lexer that will be modified. */
/* @param token The token that will be modified and contains the final token*/
LEXDEF void lexer_trace_token(Lexer *lexer, Token *token) {
  if (lexer->position == 0) {
    token->size = 1;
  } else {
    token->size = lexer->position - lexer->next_start_position + 1;
  }

  lexer->position = lexer->next_start_position;
  token->content = &lexer->content[lexer->next_start_position];

  token->kind = ERROR;
}

/* The function lexer_error() handles the cases if a default token should be */
/* returned or an explicit error has to be handelt. As well as print */
/* the corresponding error message to standard error */
/* @param lexer The Lexer that will be modified. */
/* @return Token The ERROR token.  */
LEXDEF Token lexer_error(Lexer *lexer) {

  Token t;
  int checkend = 0;
  char *pstring, *pstring_tail = 0, *final_token;
  size_t current_lexer_posion = lexer->position;

  /* ------------------ IF POSSIBLE DETECT THE TOKEN. ----------------------
   */

  lexer_trace_token(lexer, &t);

  /* Allocate memory for the prefix string of the token. */
  pstring = malloc(sizeof(char) * t.size);
  assert(pstring != NULL && "Buy more RAM !!");
  pstring[0] = '\0';

  /* Add the chars of the first part from the lexer to the pstring string */
  /* manually. */
  for (size_t i = 0; i < t.size; i++) {
    pstring[i] = t.content[i];
  }
  pstring[t.size] = '\0';

  size_t preserve_lexer_posion = lexer->position;
  /* Compute the second part of the token after the fail point. */
  lexer->position = current_lexer_posion;

  if (lexer_check_boundary_next(lexer)) {
    lexer->position = lexer->position + 1;
  } else {
    // That is very unlikely to happen. Only at the end of the content.
    checkend = 1;
    goto handle;
  }

  size_t k = 0;
  while (lexer_check_boundary(lexer)) {
    if (lexer_is_escape_seq_or_space(lexer))
      break;
    lexer_chop_char(lexer, 1);
    k++;
  }

  /* Combine the to parts of the assumed token. */
  /* Allocate memory for the tail string of the token. */
  pstring_tail = malloc(sizeof(char) * k + 1);
  assert(pstring_tail != NULL && "Buy more RAM !!");
  pstring_tail[0] = '\0';

  /* Add the chars of the second part from the lexer to the tail string */
  /* manually. */
  for (size_t j = 0; j < k; j++) {
    pstring_tail[j] = lexer->content[current_lexer_posion + 1 + j];
  }
  pstring_tail[k] = '\0';

handle:
  /* Allocate memory for the final combined string of the token. */
  if (checkend) {
    final_token = calloc(1, (strlen(pstring) + 1 + 1) * sizeof(char));
  } else {
    final_token =
        calloc(1, (strlen(pstring) + strlen(pstring_tail) + 1) * sizeof(char));
  }
  assert(final_token != NULL && "Buy more RAM !!");
  final_token[0] = '\0';

  /* Add the to parts of the pstring and pstring_tail together to the */
  /* final_token string. */
  strcpy(final_token, pstring);

  if (!checkend) {
    strcat(final_token, pstring_tail);
  }

#ifdef LEX_LOGERROR
  /* Print the Error Message. */
  fprintf(stderr, "\n");
  fprintf(stderr, "================= ERROR: Broken Token! =================\n");
  fprintf(stderr, "ERROR: The first chars are:                     [%s]\n",
          pstring);
  fprintf(stderr, "ERROR: The assumed second part of the token is: [%s]\n",
          pstring_tail);
  fprintf(stderr, "ERROR: The assumed final broken token:          [%s]\n",
          final_token);
  fprintf(stderr, "Failed at CHAR: [%c]\n",
          lexer->content[current_lexer_posion]);
  fprintf(stderr, "          POS:  [%zu]\n", current_lexer_posion);
  fprintf(stderr, "          LINE: [%llu]\n", lexer->line_count);
  fprintf(stderr, "--------------------------------------------------------\n");
  fprintf(stderr, "The current file name: %s\n", lexer->file_name);
  fprintf(stderr, "--------------------------------------------------------\n");
  fprintf(stderr, "\n");

#endif // LEX_LOGERROR

  /* Create the final Error Token to return. */
  Token token;
  token.kind = ERROR;
  token.size = strlen(final_token);
  token.content = &lexer->content[preserve_lexer_posion];

  /* Free the allocated memory for the individual parts of the token string.
   */

  free(pstring);
  free(pstring_tail);
  free(final_token);

  return token;
}

/**
 * @brief The function lexer_keyword_set_token() computes the token type for
 * the current lexer position for the keywords.
 *
 * @param lexer The given Lexer that contains the current state.
 * @param token The token the type is going to be set for.
 * @param length The length the expected keyword has.
 * @return boolean Returns true if a keyword was found, otherwise false.
 */
LEXDEF int lexer_keyword_set_token(Lexer *lexer, Token *token, size_t length) {
  char *ptr = (char *)&lexer->content[lexer->position];
  switch (length) {
  case 2: {
    if (strncmp("do", ptr, length) == 0)
      token->kind = KEYWORD_DO;
    else if (strncmp("if", ptr, length) == 0)
      token->kind = KEYWORD_IF;
  } break;

  case 3: {
    if (strncmp("for", ptr, length) == 0)
      token->kind = KEYWORD_FOR;
    else if (strncmp("int", ptr, length) == 0)
      token->kind = KEYWORD_INT;
  } break;

  case 4: {
    if (strncmp("char", ptr, length) == 0)
      token->kind = KEYWORD_CHAR;
    else if (strncmp("void", ptr, length) == 0)
      token->kind = KEYWORD_VOID;
    else if (strncmp("else", ptr, length) == 0)
      token->kind = KEYWORD_ELSE;
    else if (strncmp("long", ptr, length) == 0)
      token->kind = KEYWORD_LONG;
    else if (strncmp("goto", ptr, length) == 0)
      token->kind = KEYWORD_GOTO;
    else if (strncmp("case", ptr, length) == 0)
      token->kind = KEYWORD_CASE;
    else if (strncmp("enum", ptr, length) == 0)
      token->kind = KEYWORD_ENUM;
    else if (strncmp("auto", ptr, length) == 0)
      token->kind = KEYWORD_AUTO;
  } break;

  case 5: {
    if (strncmp("while", ptr, length) == 0)
      token->kind = KEYWORD_WHILE;
    else if (strncmp("float", ptr, length) == 0)
      token->kind = KEYWORD_FLOAT;
    else if (strncmp("const", ptr, length) == 0)
      token->kind = KEYWORD_CONST;
    else if (strncmp("break", ptr, length) == 0)
      token->kind = KEYWORD_BREAK;
    else if (strncmp("short", ptr, length) == 0)
      token->kind = KEYWORD_SHORT;
    else if (strncmp("union", ptr, length) == 0)
      token->kind = KEYWORD_UNION;
  } break;

  case 6: {
    if (strncmp("return", ptr, length) == 0)
      token->kind = KEYWORD_RETURN;
    if (strncmp("size_t", ptr, length) == 0)
      token->kind = KEYWORD_SIZE_T;
    else if (strncmp("struct", ptr, length) == 0)
      token->kind = KEYWORD_STRUCT;
    else if (strncmp("switch", ptr, length) == 0)
      token->kind = KEYWORD_SWITCH;
    else if (strncmp("double", ptr, length) == 0)
      token->kind = KEYWORD_DOUBLE;
    else if (strncmp("sizeof", ptr, length) == 0)
      token->kind = KEYWORD_SIZEOF;
    else if (strncmp("static", ptr, length) == 0)
      token->kind = KEYWORD_STATIC;
    else if (strncmp("signed", ptr, length) == 0)
      token->kind = KEYWORD_SIGNED;
    else if (strncmp("extern", ptr, length) == 0)
      token->kind = KEYWORD_EXTERN;
  } break;

  case 7: {
    if (strncmp("default", ptr, length) == 0)
      token->kind = KEYWORD_DEFAULT;
    else if (strncmp("typedef", ptr, length) == 0)
      token->kind = KEYWORD_TYPEDEF;
  } break;

  case 8: {
    if (strncmp("continue", ptr, length) == 0)
      token->kind = KEYWORD_CONTINUE;
    else if (strncmp("register", ptr, length) == 0)
      token->kind = KEYWORD_REGISTER;
    else if (strncmp("unsigned", ptr, length) == 0)
      token->kind = KEYWORD_UNSIGNED;
    else if (strncmp("volatile", ptr, length) == 0)
      token->kind = KEYWORD_VOLATILE;
  } break;

  default:
    return 0;
  }

  if (token->kind == INVALID) {
    return 0;
  }
  return 1;
}

/**
 * @brief The function lexer_punctuator_set_token() sets the corresponding
 * token type of the current lexer position for the punctuators.
 *
 * @param lexer The given Lexer that contains the current state.
 * @param token The token the type is going to be set for.
 * @param length The length the expected punctuator has.
 * @return boolean Returns true if a punctuator was found, otherwise false.
 */
LEXDEF int lexer_punctuator_set_token(Lexer *lexer, Token *token,
                                      size_t length) {
  char *ptr = (char *)&lexer->content[lexer->position];
  switch (length) {
  case 1: {
    switch (*ptr) {
    case PUNCT_SINGLEQUOTE:
      token->kind = PUNCT_SINGLEQUOTE;
      break;
    case '\\':
      token->kind = PUNCT_BACKSLASH;
      break;
    case '^':
      token->kind = PUNCT_BXOR;
      break;
    case '|':
      token->kind = PUNCT_BOR;
      break;
    case '?':
      token->kind = PUNCT_ASK;
      break;
    case ':':
      token->kind = PUNCT_COLON;
      break;
    case ';':
      token->kind = PUNCT_SEMICOLON;
      break;
    case '[':
      token->kind = PUNCT_LBRACKET;
      break;
    case ']':
      token->kind = PUNCT_RBRACKET;
      break;
    case '(':
      token->kind = PUNCT_LPAREN;
      break;
    case ')':
      token->kind = PUNCT_RPAREN;
      break;
    case '{':
      token->kind = PUNCT_LBRACE;
      break;
    case '}':
      token->kind = PUNCT_RBRACE;
      break;
    case '.':
      token->kind = PUNCT_POINT;
      break;
    case '&':
      token->kind = PUNCT_BAND;
      break;
    case '*':
      token->kind = PUNCT_MUL;
      break;
    case '+':
      token->kind = PUNCT_ADD;
      break;
    case '-':
      token->kind = PUNCT_SUB;
      break;
    case '~':
      token->kind = PUNCT_BNOT;
      break;
    case '!':
      token->kind = PUNCT_NOT;
      break;
    case '/':
      token->kind = PUNCT_DIV;
      break;
    case '%':
      token->kind = PUNCT_MOD;
      break;
    case '<':
      token->kind = PUNCT_LANGLE;
      break;
    case '>':
      token->kind = PUNCT_RANGLE;
      break;
    case ',':
      token->kind = PUNCT_COMMA;
      break;
    case '#':
      token->kind = PUNCT_HTAG;
      break;
    case '=':
      token->kind = PUNCT_ASSIGN;
      break;
    default:
      return 0;
    }
  } break;
  case 2: {
    if (0)
      return 0;
    else if (strncmp("->", ptr, length) == 0)
      token->kind = PUNCT_PDEREF;
    else if (strncmp("++", ptr, length) == 0)
      token->kind = PUNCT_INCREMENT;
    else if (strncmp("--", ptr, length) == 0)
      token->kind = PUNCT_DECREMENT;
    else if (strncmp("<<", ptr, length) == 0)
      token->kind = PUNCT_LEFTSHIFT;
    else if (strncmp(">>", ptr, length) == 0)
      token->kind = PUNCT_RIGHTSHIFT;
    else if (strncmp("<=", ptr, length) == 0)
      token->kind = PUNCT_LOREQ;
    else if (strncmp(">=", ptr, length) == 0)
      token->kind = PUNCT_GOREQ;
    else if (strncmp("==", ptr, length) == 0)
      token->kind = PUNCT_EQUAL;
    else if (strncmp("!=", ptr, length) == 0)
      token->kind = PUNCT_NOTEQ;
    else if (strncmp("&&", ptr, length) == 0)
      token->kind = PUNCT_LAND;
    else if (strncmp("||", ptr, length) == 0)
      token->kind = PUNCT_LOR;
    else if (strncmp("*=", ptr, length) == 0)
      token->kind = PUNCT_MULEQ;
    else if (strncmp("/=", ptr, length) == 0)
      token->kind = PUNCT_DIVEQ;
    else if (strncmp("%=", ptr, length) == 0)
      token->kind = PUNCT_MODEQ;
    else if (strncmp("+=", ptr, length) == 0)
      token->kind = PUNCT_ADDEQ;
    else if (strncmp("-=", ptr, length) == 0)
      token->kind = PUNCT_SUBEQ;
    else if (strncmp("&=", ptr, length) == 0)
      token->kind = PUNCT_BANDEQ;
    else if (strncmp("^=", ptr, length) == 0)
      token->kind = PUNCT_BXOREQ;
    else if (strncmp("|=", ptr, length) == 0)
      token->kind = PUNCT_BOREQ;
    else if (strncmp("##", ptr, length) == 0)
      token->kind = PUNCT_HHTAG;
    else
      return 0;
  } break;
  case 3: {
    if (0)
      return 0;
    else if (strncmp("...", ptr, length) == 0)
      token->kind = PUNCT_VARIADIC;
    else if (strncmp("<<=", ptr, length) == 0)
      token->kind = PUNCT_LEFTSHIFTEQ;
    else if (strncmp(">>=", ptr, length) == 0)
      token->kind = PUNCT_RIGHTSHIFTEQ;
    else {
      return 0;
    }
  } break;
  default:
    return 0;
  }

  if (token->kind == INVALID) {
    return 0;
  }
  return 1;
}

/**
 * @brief The function creates a string representation of the available kinds.
 *
 * @param kind The kinds that the lexer knows about.
 * @return The c_string representation of the given kind.
 */
LEXDEF const char *lexer_kind_to_str(Kind kind) {
  switch (kind) {
#define X_KIND(name, value, ...)                                               \
  case name:                                                                   \
    return #name;
    KINDS_LIST
#undef X
  default:
    assert("UNREACHABLE");
  }

  return NULL;
}

#endif // LEXER_IMPLEMENTATION
