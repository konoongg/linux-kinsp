#ifndef LK_MEMBLOCK_H
#define LK_MEMBLOCK_H

#include <stddef.h>

typedef struct lk_memblock {
    char *data;
    size_t size;
} lk_memblock;

#define lk_memblock_empty() ((lk_memblock) { NULL, 0 })

static inline int
lk_memblock_is_empty(lk_memblock block)
{
    return block.data == NULL;
}

static inline lk_memblock
lk_memblock_ref(char **src, size_t size)
{
    if (src == NULL || *src == NULL)
        return lk_memblock_empty();

    lk_memblock block = { *src, size };
    *src = NULL;
    return block;
}

extern lk_memblock lk_memblock_copy(lk_memblock block);

/*
 * Appends `len` bytes from `src` to `block`, reallocating its buffer.
 * If `src` is NULL the block is only extended (reallocated) without
 * copying. The buffer always reserves a trailing byte (not written
 * here) so the caller may NUL-terminate the data. On allocation
 * failure the old buffer is freed and an empty block is returned.
 */
extern lk_memblock lk_memblock_add(lk_memblock block, const char *src,
                                   size_t len);

/* Extends `block` by `len` bytes without writing any data. */
#define lk_memblock_extend(block, len) lk_memblock_add((block), NULL, (len))

/*
 * Writes `len` bytes from `src` at `pos`. If the block is too small,
 * it is extended automatically first. Returns the resulting block.
 */
extern lk_memblock lk_memblock_write_pos(lk_memblock block, size_t pos,
                                         const char *src, size_t len);

extern void lk_memblock_free(lk_memblock block);

#endif
