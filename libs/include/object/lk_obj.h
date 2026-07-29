#ifndef LK_OBJ_H
#define LK_OBJ_H

#include <stddef.h>

typedef struct lk_obj {
    int refcount;
    void (*destructor)(void *self);
} lk_obj;

#define LK_OBJ_INIT(obj, dtor)            \
    do {                                   \
        ((lk_obj *)(obj))->refcount = 1;   \
        ((lk_obj *)(obj))->destructor = (dtor); \
    } while (0)

void lk_obj_incref(void *obj);
void lk_obj_decref(void *obj);

#endif
