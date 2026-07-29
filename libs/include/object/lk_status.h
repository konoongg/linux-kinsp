#ifndef LK_STATUS_H
#define LK_STATUS_H

#include <stdint.h>
#include <stddef.h>

#include "collections/lk_string.h"
#include "lk_obj.h"

typedef enum {
    LK_OK = 0,
    LK_INVALID_ARGUMENT,
    LK_READ_FAIL,
    LK_WRITE_FAIL,
    LK_OPEN_FAIL,
    LK_MEM_ALLOC_FAIL,
    LK_UNDEFINED_ERROR,
    LK_CLOSE,
} lk_error_code;

typedef struct lk_status_obj lk_status_obj;

typedef union lk_status {
    uintptr_t raw;
    lk_status_obj *obj;
} lk_status;

typedef struct lk_status_obj {
    lk_obj obj;
    lk_error_code code;
    lk_string message;
    lk_status cause;
} lk_status_obj;

static inline int lk_status_is_ptr(lk_status s)
{
    return (s.raw & 1u) == 0;
}

static inline lk_status lk_status_create(lk_error_code code)
{
    lk_status s;
    s.raw = ((uintptr_t)code << 1) | 1u;
    return s;
}

#define LK_STATUS_OK()               lk_status_create(LK_OK)
#define LK_STATUS_INVALID_ARGUMENT() lk_status_create(LK_INVALID_ARGUMENT)
#define LK_STATUS_READ_FAIL()        lk_status_create(LK_READ_FAIL)
#define LK_STATUS_WRITE_FAIL()       lk_status_create(LK_WRITE_FAIL)
#define LK_STATUS_OPEN_FAIL()        lk_status_create(LK_OPEN_FAIL)
#define LK_STATUS_MEM_ALLOC_FAIL()   lk_status_create(LK_MEM_ALLOC_FAIL)
#define LK_STATUS_UNDEFINED_ERROR()  lk_status_create(LK_UNDEFINED_ERROR)
#define LK_STATUS_CLOSE()            lk_status_create(LK_CLOSE)

lk_status lk_status_create_msg(lk_error_code code, const char *message);

lk_status lk_status_create_wrp(lk_error_code code, lk_status cause);

lk_status lk_status_create_wrp_msg(lk_error_code code, lk_status cause,
                                   const char *message);

void lk_status_decref(lk_status s);

int lk_status_is_ok(lk_status s);

lk_error_code lk_status_code(lk_status s);

/*
 * Returns a non-owning reference to the status message as a readable
 * string, valid while the status is alive. It is NUL-terminated, so
 * lk_str_raw may be passed to printf("%s"). Do not release it.
 */
lk_string lk_status_msg(lk_status s);

#endif
