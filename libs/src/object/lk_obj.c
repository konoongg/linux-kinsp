#include <stdlib.h>

#include "object/lk_obj.h"

extern void
lk_obj_incref(void *obj)
{
    if (obj == NULL)
        return;

    lk_obj *o = (lk_obj *)obj;
    o->refcount++;
}

extern void
lk_obj_decref(void *obj)
{
    if (obj == NULL)
        return;

    lk_obj *o = (lk_obj *)obj;
    o->refcount--;

    if (o->refcount == 0 && o->destructor != NULL)
        o->destructor(o);
}
