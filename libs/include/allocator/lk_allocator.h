#ifndef LK_ALLOCATOR_H
#define LK_ALLOCATOR_H

#include "markers.h"
#include "object/lk_status.h"

typedef enum {
	LK_NONE_ALLOCATOR = 0,
	LK_DEFAULT_ALLOCATOR = 1,
} lk_allocator_type;

typedef void *(*lk_malloc_fn) (size_t size);

typedef void *(*lk_realloc_fn) (void *ptr, size_t size);

typedef void (*lk_free_fn) (void *ptr);

SINGLTON extern lk_status lk_allocator_init(lk_allocator_type type);

#define lk_default_allocator() lk_allocator_init(LK_DEFAULT_ALLOCATOR)

SINGLTON extern void lk_allocator_deinit(void);

SINGLTON extern void *lk_malloc(size_t size);

SINGLTON extern void *lk_realloc(void *ptr, size_t size);

SINGLTON extern void lk_free(void *ptr);

SINGLTON extern lk_allocator_type lk_allocator_get_type(void);

#endif
