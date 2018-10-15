/**
 * \file
 *
 * This module provides JSON parsing and serialising functionality, with a strong focus on parsing
 * since the JSON that the realtime server sends back can be mostly hard-coded.
 *
 * The JSON format is defined in <a href="https://tools.ietf.org/html/rfc7159">RFC7159</a>. The
 * logic and grammar used for parsing in this module are taken straight from the RFC.
 *
 * \sa <a href="https://tools.ietf.org/html/rfc7159">https://tools.ietf.org/html/rfc7159</a>
 **/
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status.h"


// Forwards declarations.
struct buffer;
struct json_value;


/**
 * An enum used to indicate what type of value a #json_value object houses. JSON only supports six
 * different data types: objects, arrays, strings, numbers, Boolean values, and a null value.
 **/
enum json_value_type {
  JSON_VALUE_TYPE_ARRAY,    //!< A JSON array.
  JSON_VALUE_TYPE_BOOLEAN,  //!< A JSON Boolean value.
  JSON_VALUE_TYPE_NULL,     //!< The JSON `null` value.
  JSON_VALUE_TYPE_NUMBER,   //!< A JSON number (64-bit double).
  JSON_VALUE_TYPE_OBJECT,   //!< A JSON object (key/value pairs).
  JSON_VALUE_TYPE_STRING,   //!< A JSON string, encoded in UTF-8.
};


/**
 * A very simple linked list structure used for representing the elements in JSON objects and JSON
 * arrays.
 **/
struct json_value_list {
  char *key;                     //!< The key for a JSON object pair and \p NULL for a JSON array.
  struct json_value *value;      //!< The value for a JSON object pair and the value for a JSON array.
  struct json_value_list *next;  //!< The next pointer for the linked list.
};


/**
 * A struct to house a single JSON value of a given type.
 **/
struct json_value {
  enum json_value_type type;        //!< The type of this JSON value. Used to indicate which field of \p as houses data.
  union {
    bool boolean;                   //!< A Boolean value used for JSON \p true and \p false values.
    double number;                  //!< A 64-bit double-precision number used for JSON numbers.
    struct json_value_list *pairs;  //!< A linked list of pairs. Used for JSON objects and arrays.
    char *string;                   //!< A UTF-8 encoded string used for JSON strings.
  } as;                             //!< A union for storage of the underlying value of this JSON value.
};


/**
 * \sa json_parse_n
 **/
struct json_value *
json_parse(const char *string) __attribute__((nonnull(1)));

/**
 * Parses a stream of JSON bytes.
 *
 * \param[in] input A byte stream of JSON data. Must not be \p NULL.
 * \param[in] input_nbytes The length of \p input.
 * \return A #json_value instance if the string was successfully parsed, or \p NULL otherwise. It
 *         is the callers responsibility to free the returned object via #json_value_destroy once
 *         they are done with it.
 * \sa json_parse, json_value_destroy
 **/
struct json_value *
json_parse_n(const char *input, size_t input_nbytes) __attribute__((nonnull(1)));

/**
 * Creates a #json_value instance of a given type.
 *
 * \param[in] type The type of JSON value to create.
 * \return A #json_value instance or \p NULL if memory allocation failed. It is the responsibility
 *         of the caller to free the returned object via #json_value_destroy.
 * \sa json_value_destroy
 **/
struct json_value *
json_value_create(enum json_value_type type);

/**
 * Recursively destroys a #json_value instance.
 *
 * \param[in,out] value The (possibly \p NULL) instance to recursively destroy.
 **/
void
json_value_destroy(struct json_value *value);

/**
 * Appends a JSON value to an existing JSON array value.
 *
 * \param[in,out] array The JSON array to append to. Must not be \p NULL.
 * \param[in] value The value to append to the array. Must not be \p NULL.
 * \return Returns #STATUS_OK if the append operation succeeded.
 *         Returns #STATUS_EINVAL if the JSON object is not of type #JSON_VALUE_TYPE_ARRAY.
 *         Returns #STATUS_ENOMEM if memory allocation failed.
 **/
enum status
json_value_append(struct json_value *array, struct json_value *value) __attribute__((nonnull(1, 2)));

/**
 * Gets the value for a given \p key for a JSON object.
 *
 * \param[in] object The JSON object to lookup the key in. Must not be \p NULL.
 * \param[in] key The key to look for. Must not be \p NULL.
 * \return The corresponding value for the key if the key exists, or \p NULL otherwise.
 **/
struct json_value *
json_value_get(const struct json_value *object, const char *key) __attribute__((nonnull(1, 2)));

/**
 * \sa json_value_set_n
 **/
enum status
json_value_set(struct json_value *object, const char *key, struct json_value *value) __attribute__((nonnull(1, 2, 3)));

/**
 * Sets a key/value pair in a JSON object. It is assumed that the key does not already exist in the
 * object. A copy of the input \p key is made, but the \p value is used directly.
 *
 * \param[in,out] object The JSON object to set the key/value pair in. Must not be \p NULL.
 * \param[in] key The key to set. Must not be \p NULL.
 * \param[in] key_nbytes The length of the key.
 * \param[in] value The value to set with the key. Must not be \p NULL.
 * \return Returns #STATUS_OK if the set operation succeeded.
 *         Returns #STATUS_EINVAL if the JSON object is not of type #JSON_VALUE_TYPE_OBJECT.
 *         Returns #STATUS_ENOMEM if memory allocation failed.
 **/
enum status
json_value_set_n(struct json_value *object, const char *key, size_t key_nbytes, struct json_value *value) __attribute__((nonnull(1, 2, 4)));

/**
 * The same as #json_value_set_n, except that the key is used directly instead of a copy being taken.
 * \sa #json_value_set_n
 */
enum status
json_value_set_nocopy(struct json_value *object, char *key, struct json_value *value) __attribute__((nonnull(1, 2, 3)));

/**
 * JSON-escapes the given \p string, writing the result to the \p buffer.
 *
 * \param[out] buffer The buffer to write the result to. Must not be \p NULL.
 * \param[in] string The UTF-8 string to JSON-escape. Must not be \p NULL.
 * \return Returns #STATUS_OK if all writes to the buffer succeeded, otherwise returns the error
 *         returned by the failed buffer write.
 **/
enum status
json_write_escape_string(struct buffer *buffer, const char *string) __attribute__((nonnull(1, 2)));
