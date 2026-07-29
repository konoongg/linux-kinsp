#include "typedef_libc.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include "logger/lk_determ_logger.h"
#include "logger/lk_logger.h"
#include "logger/lk_std_logger.h"

static pthread_mutex_t g_write_mutex = PTHREAD_MUTEX_INITIALIZER;

extern void
lk_log_determ_write(const lk_logger *logger, lk_log_level level,
                    const char *fmt, va_list args)
{
    pthread_mutex_lock(&g_write_mutex);
    lk_log_std_write(logger, level, fmt, args);
    fflush(logger->out);
    pthread_mutex_unlock(&g_write_mutex);
}
