#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "xmalloc.h"

#define GROW_NBYTES ((size_t)512)


const struct buffer BUFFER_INITIALISER = {0, 0, NULL};


static bool
_buffer_maybe_grow(struct buffer *const buffer, const size_t new_nbytes) {
  const size_t nbytes = buffer->nbytes_used + new_nbytes;
  if (nbytes <= buffer->nbytes_allocd) {
    return true;
  }

  size_t nbytes_allocd = buffer->nbytes_allocd;
  while (nbytes_allocd < nbytes) {
    nbytes_allocd += GROW_NBYTES;
  }
  char *tmp = xmalloc(nbytes_allocd);
  if (tmp == NULL) {
    return false;
  }
  memcpy(tmp, buffer->data, buffer->nbytes_used);
  free(buffer->data);
  buffer->data = tmp;
  buffer->nbytes_allocd = nbytes_allocd;
  return true;
}


struct buffer *
buffer_create(void) {
  struct buffer *buffer = xmalloc(sizeof(struct buffer));
  if (buffer == NULL) {
    return NULL;
  }
  buffer_init(buffer);
  return buffer;
}


void
buffer_destroy(struct buffer *const buffer) {
  if (buffer == NULL) {
    return;
  }

  buffer_deinit(buffer);
  free(buffer);
}


enum status
buffer_init(struct buffer *const buffer) {
  if (buffer == NULL) {
    return STATUS_EINVAL;
  }

  memset(buffer, 0, sizeof(struct buffer));
  return STATUS_OK;
}


enum status
buffer_deinit(struct buffer *const buffer) {
  if (buffer == NULL) {
    return STATUS_EINVAL;
  }

  free(buffer->data);
  buffer->nbytes_used = 0;
  buffer->nbytes_allocd = 0;
  buffer->data = NULL;

  return STATUS_OK;
}


static enum status
_buffer_append_n(struct buffer *const buffer, const void *const new_bytes, const size_t new_nbytes) {
  if (!_buffer_maybe_grow(buffer, new_nbytes)) {
    return STATUS_ENOMEM;
  }

  memcpy(buffer->data + buffer->nbytes_used, new_bytes, new_nbytes);
  buffer->nbytes_used += new_nbytes;
  return STATUS_OK;
}


enum status
buffer_append(struct buffer *const buffer, const char *const str) {
  if (buffer == NULL || str == NULL) {
    return STATUS_EINVAL;
  }

  const size_t str_nbytes = strlen(str);
  if (str_nbytes == 0) {
    return STATUS_OK;
  }
  return _buffer_append_n(buffer, str, str_nbytes);
}


enum status
buffer_append_n(struct buffer *const buffer, const void *const new_bytes, const size_t new_nbytes) {
  if (buffer == NULL || new_bytes == NULL) {
    return STATUS_EINVAL;
  }
  else if (new_nbytes == 0) {
    return STATUS_OK;
  }

  return _buffer_append_n(buffer, new_bytes, new_nbytes);
}


enum status
buffer_append_printf(struct buffer *const buffer, const char *const format, ...) {
  if (buffer == NULL || format == NULL) {
    return STATUS_EINVAL;
  }

  static char tmp[32*GROW_NBYTES];
  va_list args;
  va_start(args, format);

  int ret = vsnprintf(tmp, sizeof(tmp), format, args);
  if (ret == -1 || ((size_t)ret) >= sizeof(tmp)) {
    return STATUS_BAD;
  }
  return _buffer_append_n(buffer, tmp, (size_t)ret);
}


enum status
buffer_clear(struct buffer *const buffer) {
  if (buffer == NULL) {
    return STATUS_EINVAL;
  }

  buffer->nbytes_used = 0;
  return STATUS_OK;
}


enum status
buffer_clear_n(struct buffer *const buffer, const size_t nbytes) {
  if (buffer == NULL) {
    return STATUS_EINVAL;
  }

  buffer->nbytes_used -= nbytes;
  if (buffer->nbytes_used != 0) {
    memmove(buffer->data, buffer->data + nbytes, buffer->nbytes_used);
  }
  return STATUS_OK;
}


enum status
buffer_reserve(struct buffer *const buffer, const size_t nbytes) {
  if (buffer == NULL) {
    return STATUS_EINVAL;
  }

  return _buffer_maybe_grow(buffer, nbytes);
}
