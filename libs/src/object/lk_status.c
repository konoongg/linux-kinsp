#include <assert.h>
#include <string.h>

#include "allocator/lk_allocator.h"
#include "collections/lk_string.h"
#include "object/lk_status.h"

static lk_string
lk_error_str(lk_error_code code);

static void
lk_status_free(void *self)
{
    lk_status_obj *obj = (lk_status_obj *)self;
    if (obj == NULL)
        return;

    lk_str_free(obj->message);
    lk_status_decref(obj->cause);
    lk_free(obj);
}

static lk_status
status_new(lk_error_code code, lk_string message, lk_status cause)
{
    assert(code != LK_OK);

    lk_status_obj *obj = lk_malloc(sizeof(*obj));
    if (obj == NULL) {
        lk_free(message.mem.data);
        return lk_status_create(LK_MEM_ALLOC_FAIL);
    }

    LK_OBJ_INIT(obj, lk_status_free);
    obj->code = code;
    obj->message = message;
    obj->cause = cause;

    lk_status s;
    s.obj = obj;
    return s;
}

extern lk_status
lk_status_create_msg(lk_error_code code, const char *message)
{
    lk_string msg = (message != NULL) ? lk_str_create(message)
                                      : lk_str_empty;

    return status_new(code, msg, lk_status_create(LK_OK));
}

extern lk_status
lk_status_create_wrp(lk_error_code code, lk_status cause)
{
    return status_new(code, lk_str_copy(lk_status_msg(cause)), cause);
}

extern lk_status
lk_status_create_wrp_msg(lk_error_code code, lk_status cause,
                         const char *message)
{
    if (message == NULL)
        return lk_status_create_wrp(code, cause);

    lk_string msg = lk_str_create(message);
    if (lk_str_is_empty(msg))
        return lk_status_create(LK_MEM_ALLOC_FAIL);

    msg = lk_str_add_back(msg, ":\n\t");
    if (lk_str_is_empty(msg))
        return lk_status_create(LK_MEM_ALLOC_FAIL);

    msg = lk_str_add_back(msg, lk_str_raw(lk_status_msg(cause)));
    if (lk_str_is_empty(msg))
        return lk_status_create(LK_MEM_ALLOC_FAIL);

    return status_new(code, msg, cause);
}

extern void
lk_status_decref(lk_status s)
{
    if (lk_status_is_ptr(s))
        lk_obj_decref(s.obj);
}

extern int
lk_status_is_ok(lk_status s)
{
    return lk_status_code(s) == LK_OK;
}

extern lk_error_code
lk_status_code(lk_status s)
{
    if (lk_status_is_ptr(s))
        return s.obj->code;

    return (lk_error_code)(s.raw >> 1);
}

/*
 * Returns a non-owning string referencing static data. It must not be
 * released; callers that need to keep it deep-copy with lk_str_copy.
 */
static lk_string
lk_error_str(lk_error_code code)
{
    char *str;

    switch (code) {
    case LK_OK:                    str = "ok"; break;
    case LK_INVALID_ARGUMENT:      str = "invalid argument"; break;
    case LK_READ_FAIL:             str = "read failed"; break;
    case LK_WRITE_FAIL:            str = "write failed"; break;
    case LK_OPEN_FAIL:             str = "failed to open file"; break;
    case LK_MEM_ALLOC_FAIL:        str = "memory allocation failed"; break;
    case LK_UNDEFINED_ERROR:       str = "undefined error"; break;
    case LK_CLOSE:                 str = "close failed"; break;
    default:                       str = "unknown error"; break;
    }

    return lk_str_wrap(&str, strlen(str));
}

extern lk_string
lk_status_msg(lk_status s)
{
    if (!lk_status_is_ptr(s))
        return lk_error_str(lk_status_code(s));

    if (!lk_str_is_empty(s.obj->message))
        return s.obj->message;

    return lk_error_str(s.obj->code);
}
