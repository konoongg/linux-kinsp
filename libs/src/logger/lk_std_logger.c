#include "typedef_libc.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "logger/lk_logger.h"
#include "logger/lk_std_logger.h"

extern void
lk_log_std_write(const lk_logger *logger, lk_log_level level,
                 const char *fmt, va_list args)
{
    time_t now = time(NULL);
    char timestr[32];
    lk_tm tm;

    localtime_r(&now, &tm);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm);

    fprintf(logger->out, "[%s] [%-5s] ", timestr,
            lk_log_level_str(level));
    vfprintf(logger->out, fmt, args);
    fputc('\n', logger->out);
}
