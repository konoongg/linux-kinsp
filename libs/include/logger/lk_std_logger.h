#ifndef LK_STD_LOGGER_H
#define LK_STD_LOGGER_H

#include <stdarg.h>

#include "logger/lk_logger.h"

extern void lk_log_std_write(const lk_logger *logger, lk_log_level level,
                             const char *fmt, va_list args);

#endif
