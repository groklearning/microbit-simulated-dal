/**
 * \file
 **/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct lexer {
  const char *upto;   //!< Pointer to where we are up to within the input byte stream.
  const char *end;    //!< Pointer to the end of the input byte stream.
  size_t nremaining;  //<! How many bytes are remaining in the input byte stream.
};


/**
 * Initialises a #lexer instance with its input byte stream.
 *
 * \param[in] lex The lexer instance. Must not be \p NULL.
 * \param[in] input Pointer to the input byte stream. Must not be \p NULL.
 * \param[in] input_nbytes The length of \p input.
 **/
void
lexer_init(struct lexer *lex, const char *input, size_t input_nbytes) __attribute__((nonnull(1, 2)));

/**
 * Consumes one or more LWS characters from the lexer's byte stream.
 * <pre>LWS = [CRLF] 1*( SP | HT )</pre>
 *
 * \param[in] lex The lexer instance. Must not be \p NULL.
 * \return Whether or not one or more LWS characters were successfully consumed.
 **/
bool
lexer_consume_lws(struct lexer *lex) __attribute__((nonnull(1)));

/**
 * Consumes an unsigned integer from the lexer, storing the result in \p number.
 *
 * \param[in] lex The lexer instance. Must not be \p NULL.
 * \param[out] number The unsigned integer read in from the lexer's byte stream. Must not be \p NULL.
 * \return Whether or not an unsigned integer was successfully consumed.
 **/
bool
lexer_consume_uint32(struct lexer *lex, uint32_t *number) __attribute__((nonnull(1, 2)));

/**
 * Consumes zero or more WS characters from the lexer's byte stream.
 * <pre>WS = ( tab | space | CR | LF )*</pre>
 *
 * \param[in] lex The lexer instance. Must not be \p NULL.
 **/
void
lexer_consume_ws(struct lexer *lex) __attribute__((nonnull(1)));

/**
 * \def lexer_consume
 * Unsafely consumes \p nbytes from the lexer \p lex.
 **/
#define lexer_consume(lex, nbytes) { (lex)->upto += (nbytes); (lex)->nremaining -= (nbytes); }

/**
 * \def lexer_memcmp
 * Unsafely performs a \p memcmp from where the lexer is currently upto through the input stream.
 **/
#define lexer_memcmp(lex, str, n) (memcmp((lex)->upto, (str), (n)))

/**
 * \def lexer_nremaining
 * Returns how many bytes are remaining on the lexer's input stream.
 **/
#define lexer_nremaining(lex) ((lex)->nremaining)

/**
 * \def lexer_peek
 * Unsafely peeks at the next byte in the input stream.
 **/
#define lexer_peek(lex) (*(lex)->upto)

/**
 * \def lexer_strncmp
 * Unsafely performs a \p strncmp from where the lexer is currently upto through the input stream.
 **/
#define lexer_strncmp(lex, str, n) (strncmp((lex)->upto, (str), (n)))

/**
 * \def lexer_upto
 * Returns a pointer to where the lexer is currently upto through the input stream.
 **/
#define lexer_upto(lex) ((lex)->upto)
