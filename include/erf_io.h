#ifndef CBNB_CNBN_IO_H
#define CBNB_CNBN_IO_H

#include <stdarg.h>

#include "erf_types.h"

#define CBNB_LOGGER_COLOR_RED "\x1b[31m"
#define CBNB_LOGGER_COLOR_GREEN "\x1b[32m"
#define CBNB_LOGGER_COLOR_YELLOW "\x1b[33m"
#define CBNB_LOGGER_COLOR_BLUE "\x1b[34m"
#define COLOR_END "\x1b[0m"

typedef enum
{
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERROR = 3,
  FATAL = 4,
  NONE = 5
} CBNB_LOG_LEVEL;

void erf_log (CBNB_LOG_LEVEL level, ConstStr format, ...);

#ifdef ERF_IMPLEMENTATION
void
erf_log (CBNB_LOG_LEVEL level, ConstStr format, ...)
{
  switch (level)
  {
  case DEBUG:
    printf (CBNB_LOGGER_COLOR_BLUE "[DEBUG] " COLOR_END);
    break;
  case INFO:
    printf (CBNB_LOGGER_COLOR_BLUE "[INFO] " COLOR_END);
    break;
  case WARN:
    printf (CBNB_LOGGER_COLOR_YELLOW "[WARN] " COLOR_END);
    break;
  case ERROR:
    printf (CBNB_LOGGER_COLOR_RED "[ERROR] " COLOR_END);
    break;
  case FATAL:
    printf (CBNB_LOGGER_COLOR_RED "[FATAL] ");
    break;
  default:
    printf ("[UNKNOWN] ");
    break;
  }

  va_list args;
  va_start (args, format);
  vprintf (format, args);
  va_end (args);

  printf ("\n");
}
#endif
// ======================================================================================
#endif // erf_io