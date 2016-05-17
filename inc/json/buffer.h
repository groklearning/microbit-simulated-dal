#pragma once

#include <stdlib.h>

#include "status.h"


struct buffer {
  size_t nbytes_allocd;
  size_t nbytes_used;
  char *data;
};

extern const struct buffer BUFFER_INITIALISER;

struct buffer *buffer_create(void);
void           buffer_destroy(struct buffer *buffer);
enum status    buffer_init(struct buffer *buffer);
enum status    buffer_deinit(struct buffer *buffer);
enum status    buffer_append(struct buffer *buffer, const char *str);
enum status    buffer_append_n(struct buffer *buffer, const void *bytes, size_t nbytes);
enum status    buffer_append_printf(struct buffer *buffer, const char *format, ...) __attribute__((format(printf, 2, 3)));
enum status    buffer_clear(struct buffer *buffer);
enum status    buffer_clear_n(struct buffer *buffer, size_t nbytes);
enum status    buffer_reserve(struct buffer *buffer, size_t nbytes);
