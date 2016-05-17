/**
 * \file
 **/
#pragma once

/**
 * An enum that defines the various levels that logging can occur at.
 **/
enum logging_level {
  LOGGING_LEVEL_DEBUG,    //!< DEBUG logging.
  LOGGING_LEVEL_INFO,     //!< INFO logging.
  LOGGING_LEVEL_WARNING,  //!< WARNING logging.
  LOGGING_LEVEL_ERROR,    //!< ERROR logging.
};


/**
 * Flushes and closes the logging file.
 **/
void
logging_close(void);

/**
 * Logs a message to the logger. This function is not normally called directly, but is instead
 * called via the logging helper macros.
 *
 * \param[in] level The level of the logged  message.
 * \param[in] function_name The name of the function where the logging call was made. Must not be \p NULL.
 * \param[in] file_name The name of the file where the logging call was made. Must not be \p NULL.
 * \param[in] line_number The line number in the source file where the logging call was made.
 * \param[in] fmt The \p printf format string used to write to construct the log message. Must not be \p NULL.
 * \param[in] ... The arguments to the \p printf format string.
 **/
void
logging_log(enum logging_level level, const char *function_name, const char *file_name, int line_number, const char *fmt, ...) __attribute__((format(printf, 5, 6))) __attribute__((nonnull(2, 3, 5)));

/**
 * Opens a file for logging.
 *
 * \param[in] path The path of the file to open for writing. Must not be \p NULL.
 **/
void
logging_open(const char *path) __attribute__((nonnull(1)));

#define DEBUG(fmt, ...)    logging_log(LOGGING_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define DEBUG0(fmt)        logging_log(LOGGING_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, fmt)
#define INFO(fmt, ...)     logging_log(LOGGING_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define INFO0(fmt)         logging_log(LOGGING_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, fmt)
#define WARNING(fmt, ...)  logging_log(LOGGING_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define WARNING0(fmt)      logging_log(LOGGING_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, fmt)
#define ERROR(fmt, ...)    logging_log(LOGGING_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define ERROR0(fmt)        logging_log(LOGGING_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, fmt)
