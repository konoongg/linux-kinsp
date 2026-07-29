#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "allocator/lk_allocator.h"
#include "allocator/lk_libc_alloc.h"

typedef struct allocator_impl {
	lk_allocator_type type;
	lk_malloc_fn malloc_fn;
	lk_realloc_fn realloc_fn;
	lk_free_fn free_fn;
} allocator_impl;

static allocator_impl g_allocator;
static int g_initialized = 0;

extern lk_status
lk_allocator_init(lk_allocator_type type)
{
	assert(!g_initialized);

	switch (type) {

	case LK_DEFAULT_ALLOCATOR:
		g_allocator.malloc_fn = lk_aloc_lc_malloc;
		g_allocator.realloc_fn = lk_aloc_lc_realloc;
		g_allocator.free_fn = lk_aloc_lc_free;
		break;

	default:
		return lk_status_create(LK_INVALID_ARGUMENT);

	}

	g_allocator.type = type;
	g_initialized = 1;
	return lk_status_create(LK_OK);
}

extern void
lk_allocator_deinit(void)
{
	g_allocator.type = LK_NONE_ALLOCATOR;
	g_allocator.malloc_fn = NULL;
	g_allocator.realloc_fn = NULL;
	g_allocator.free_fn = NULL;
	g_initialized = 0;
}

extern void *
lk_malloc(size_t size)
{
	assert(g_initialized);
	return g_allocator.malloc_fn(size);
}

extern void *
lk_realloc(void *ptr, size_t size)
{
	assert(g_initialized);
	return g_allocator.realloc_fn(ptr, size);
}

extern void
lk_free(void *ptr)
{
	assert(g_initialized);
	g_allocator.free_fn(ptr);
}

extern lk_allocator_type
lk_allocator_get_type(void)
{
	assert(g_initialized);
	return g_allocator.type;
}
