#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "logging.h"


static FILE *file = NULL;
static bool file_needs_close = false;


void
logging_open(const char *const path) {
  // Close any existing log files.
  logging_close();

  // Don't reopen stdout/err.
  if (strcmp(path, "/dev/stdout") == 0) {
    file_needs_close = false;
    file = stdout;
  }
  else if (strcmp(path, "/dev/stderr") == 0) {
    file_needs_close = false;
    file = stderr;
  }
  else {
    file_needs_close = true;
    file = fopen(path, "w");
    if (file == NULL) {
      perror("Call to `fopen` failed in `logging_open`.");
    }
  }
}


void
logging_close(void) {
  int ret;
  if (file != NULL) {
    ret = fflush(file);
    if (ret != 0) {
      perror("Call to `fflush` failed in `logging_close`.");
    }
    if (file_needs_close) {
      ret = fclose(file);
      if (ret != 0) {
        perror("Call to `fclose` failed in `logging_close`.");
      }
    }
    file = NULL;
  }
}


void
logging_log(const enum logging_level level, const char *const function_name, const char *const file_name, const int line_number, const char *const fmt, ...) {
  va_list args;
  const char *prefix = NULL;
  const time_t now = time(NULL);
  static char buf[128];

  switch (level) {
  case LOGGING_LEVEL_DEBUG:
    prefix = "\033[1;34m[DEBUG]";
    break;
  case LOGGING_LEVEL_INFO:
    prefix = "\033[1;32m[INFO]";
    break;
  case LOGGING_LEVEL_WARNING:
    prefix = "\033[1;33m[WARNING]";
    break;
  case LOGGING_LEVEL_ERROR:
    prefix = "\033[1;31m[ERROR]";
    break;
  }

  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  fprintf(file, "%s[%s][%s:%s:%d]\033[0m ", prefix, buf, function_name, file_name, line_number);
  va_start(args, fmt);
  vfprintf(file, fmt, args);
  va_end(args);
}
