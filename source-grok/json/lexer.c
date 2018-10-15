#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


void
lexer_init(struct lexer *const lex, const char *const input, const size_t input_nbytes) {
  lex->upto = input;
  lex->end = input + input_nbytes;
  lex->nremaining = input_nbytes;
}


/*
From the HTTP spec:
  LWS = [CRLF] 1*( SP | HT )
*/
bool
lexer_consume_lws(struct lexer *const lex) {
  bool changed = false;

  // Take a copy of the lexer.
  struct lexer orig;
  memcpy(&orig, lex, sizeof(struct lexer));

  // Try to consume the rule.
  if (lexer_nremaining(lex) >= 2 && lexer_memcmp(lex, "\r\n", 2) == 0) {
    lexer_consume(lex, 2);
    changed = true;
  }
  for (unsigned int i = 0; ; ++i) {
    if (lexer_nremaining(lex) == 0) {
      if (i == 0) {
        goto fail;
      }
      else {
        break;
      }
    }
    else if (lexer_peek(lex) == ' ' || lexer_peek(lex) == '\t') {
      lexer_consume(lex, 1);
      changed = true;
    }
    else {
      if (i == 0) {
        goto fail;
      }
      else {
        break;
      }
    }
  }

  return true;

fail:
  if (changed) {
    memcpy(lex, &orig, sizeof(struct lexer));
  }
  return false;
}


/*
From the HTTP spec:
  Leading zeros MUST be ignored by recipients and MUST NOT be sent.
*/
bool
lexer_consume_uint32(struct lexer *const lex, uint32_t *const number) {
  struct lexer orig;
  bool changed = false;
  uint32_t num = 0;

  // Take a copy of the original state.
  memcpy(&orig, lex, sizeof(struct lexer));

  for (unsigned int i = 0; ; ++i) {
    // Are there any more bytes to read?
    if (lexer_nremaining(lex) == 0) {
      if (i == 0) {
        goto fail;
      }
      else {
        break;
      }
    }

    const char c = lexer_peek(lex);

    // Do we not have a DIGIT?
    if (!isdigit(c)) {
      if (i == 0) {
        goto fail;
      }
      else {
        break;
      }
    }

    // Consume the digit and update our number, allowing for overflow.
    lexer_consume(lex, 1);
    num = (10 * num) + (c - '0');
  }

  // Success.
  *number = num;
  return true;

fail:
  if (changed) {
    memcpy(lex, &orig, sizeof(struct lexer));
  }
  return false;
}


/*
From the JSON spec:
  WS = ( tab | space | CR | LF )*
*/
void
lexer_consume_ws(struct lexer *const lex) {
  while (lexer_nremaining(lex) != 0) {
    const char c = lexer_peek(lex);
    if (c == '\t' || c == ' ' || c == '\n' || c == '\r') {
      lexer_consume(lex, 1);
    }
    else {
      break;
    }
  }
}
