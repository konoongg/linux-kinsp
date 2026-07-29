#include <stdlib.h>

#include "allocator/lk_libc_alloc.h"

extern void *
lk_aloc_lc_malloc(size_t size)
{
	return calloc(1, size);
}

extern void *
lk_aloc_lc_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

extern void
lk_aloc_lc_free(void *ptr)
{
	free(ptr);
}
