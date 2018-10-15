/**
 * \file
 *
 * Defines malloc wrappers #xcalloc and #xmalloc which when not running the test cases, are simply
 * #defined'd to \p calloc and \p malloc respectively. During testing, these functions can be
 * configured with the help of \p xmalloc_reset to return \p NULL (failed to alloc) after a certain
 * number of invocations.
 **/

/**
 * \def xcalloc(count, nbytes)
 * An alias to \p calloc which is customised during unit testing.
 **/

/**
 * \def xmalloc(nbytes)
 * An alias to \p malloc which is customised during unit testing.
 **/

/**
 * \fn void xmalloc_reset(unsigned int fail_after_ncalls);
 * Configures the behaviour of #xcalloc and #xmalloc during unit testing.
 **/

#pragma once

#include <stdlib.h>

#ifdef CUSTOM_XMALLOC
  void *xcalloc(size_t count, size_t nbytes);
  void *xmalloc(size_t nbytes);
  void  xmalloc_reset(unsigned int fail_after_ncalls);
#else
  #define xcalloc(count, nbytes) calloc(count, nbytes)
  #define xmalloc(nbytes) malloc(nbytes)
#endif
