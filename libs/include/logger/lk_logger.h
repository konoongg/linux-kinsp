#ifndef LK_LOGGER_H
#define LK_LOGGER_H

#include <stdio.h>

#include "markers.h"
#include "object/lk_status.h"

#define LK_LOG_DEFAULT_FILE "exec_trace.log"

typedef enum {
    LK_LOGGER_STANDARD = 0,
    LK_LOGGER_DETERM,
} lk_logger_type;

typedef enum {
    LK_LOG_LEVEL_DEBUG = 0,
    LK_LOG_LEVEL_INFO,
    LK_LOG_LEVEL_WARN,
    LK_LOG_LEVEL_ERROR,
    LK_LOG_LEVEL_FATAL,
} lk_log_level;

typedef struct lk_logger {
    lk_log_level level;
    FILE *out;
} lk_logger;

extern void lk_logger_log(lk_log_level level, const char *fmt, ...);

extern void lk_logger_set_level(lk_log_level level);

extern const char *lk_log_level_str(lk_log_level level);

extern lk_log_level lk_logger_level_from_str(const char *name);

SINGLTON extern lk_status lk_logger_global_init(lk_logger_type type,
                                                const char *path,
                                                lk_log_level level);

SINGLTON extern void lk_logger_global_deinit(void);

#define lk_standard_logger(path, level) \
    lk_logger_global_init(LK_LOGGER_STANDARD, (path), (level))

#define lk_determ_logger(path, level) \
    lk_logger_global_init(LK_LOGGER_DETERM, (path), (level))

#define lk_log_debug(...) \
    lk_logger_log(LK_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define lk_log_info(...) \
    lk_logger_log(LK_LOG_LEVEL_INFO, __VA_ARGS__)
#define lk_log_warn(...) \
    lk_logger_log(LK_LOG_LEVEL_WARN, __VA_ARGS__)
#define lk_log_error(...) \
    lk_logger_log(LK_LOG_LEVEL_ERROR, __VA_ARGS__)
#define lk_log_fatal(...) \
    lk_logger_log(LK_LOG_LEVEL_FATAL, __VA_ARGS__)

#endif
