#include "typedef_libc.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "allocator/lk_allocator.h"
#include "collections/lk_string.h"
#include "logger/lk_determ_logger.h"
#include "logger/lk_logger.h"
#include "logger/lk_std_logger.h"

typedef void (*lk_logger_write_fn)(const lk_logger *logger,
                                   lk_log_level level, const char *fmt,
                                   va_list args);

typedef struct logger {
    lk_logger base;
    char *path;
    lk_logger_write_fn write;
} logger;

static logger g_logger;

static int g_initialized = 0;

static void
logger_free(logger *logger)
{
    if (logger->base.out != NULL)
        fclose(logger->base.out);

    lk_free(logger->path);
}

extern lk_log_level
lk_logger_level_from_str(const char *name)
{
    if (name == NULL)
        return LK_LOG_LEVEL_DEBUG;

    if (strcasecmp(name, "debug") == 0 || strcasecmp(name, "trace") == 0)
        return LK_LOG_LEVEL_DEBUG;
    if (strcasecmp(name, "info") == 0)
        return LK_LOG_LEVEL_INFO;
    if (strcasecmp(name, "warn") == 0 || strcasecmp(name, "warning") == 0)
        return LK_LOG_LEVEL_WARN;
    if (strcasecmp(name, "error") == 0)
        return LK_LOG_LEVEL_ERROR;
    if (strcasecmp(name, "fatal") == 0)
        return LK_LOG_LEVEL_FATAL;

    return LK_LOG_LEVEL_DEBUG;
}

extern void
lk_logger_set_level(lk_log_level level)
{
    g_logger.base.level = level;
}

extern const char *
lk_log_level_str(lk_log_level level)
{
    switch (level) {
    case LK_LOG_LEVEL_DEBUG: return "DEBUG";
    case LK_LOG_LEVEL_INFO:  return "INFO";
    case LK_LOG_LEVEL_WARN:  return "WARN";
    case LK_LOG_LEVEL_ERROR: return "ERROR";
    case LK_LOG_LEVEL_FATAL: return "FATAL";
    default:                 return "UNKNOWN";
    }
}

extern lk_status
lk_logger_global_init(lk_logger_type type, const char *path,
                      lk_log_level level)
{
    const char *resolved;

    assert(!g_initialized);

    memset(&g_logger, 0, sizeof(g_logger));

    switch (type) {
    case LK_LOGGER_STANDARD:
        g_logger.write = lk_log_std_write;
        break;
    case LK_LOGGER_DETERM:
        g_logger.write = lk_log_determ_write;
        break;
    default:
        return lk_status_create_msg(LK_INVALID_ARGUMENT,
                                    "unknown logger type");
    }

    g_logger.base.level = level;

    resolved = (path != NULL) ? path : LK_LOG_DEFAULT_FILE;

    lk_string str = lk_str_create(resolved);
    if (str.mem.data == NULL)
        return lk_status_create(LK_MEM_ALLOC_FAIL);
    g_logger.path = str.mem.data;

    g_logger.base.out = fopen(resolved, "a");
    if (g_logger.base.out == NULL) {
        logger_free(&g_logger);
        return lk_status_create_msg(LK_OPEN_FAIL, resolved);
    }

    g_initialized = 1;
    return lk_status_create(LK_OK);
}

extern void
lk_logger_global_deinit(void)
{
    if (!g_initialized)
        return;

    logger_free(&g_logger);
    memset(&g_logger, 0, sizeof(g_logger));
    g_initialized = 0;
}

extern void
lk_logger_log(lk_log_level level, const char *fmt, ...)
{
    va_list args;

    assert(g_initialized);
    assert(fmt != NULL);

    if (level < g_logger.base.level)
        return;

    va_start(args, fmt);
    if (g_logger.write != NULL)
        g_logger.write(&g_logger.base, level, fmt, args);
    va_end(args);

    if (level == LK_LOG_LEVEL_FATAL)
        abort();
}
