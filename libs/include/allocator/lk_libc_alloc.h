#ifndef LK_LIBC_ALLOC_H
#define LK_LIBC_ALLOC_H

#include <stddef.h>

extern void *lk_aloc_lc_malloc(size_t size);

extern void *lk_aloc_lc_realloc(void *ptr, size_t size);

extern void lk_aloc_lc_free(void *ptr);

#endif
