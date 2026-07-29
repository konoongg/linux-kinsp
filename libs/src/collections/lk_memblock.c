#include <string.h>

#include "allocator/lk_allocator.h"
#include "collections/lk_memblock.h"

extern void
lk_memblock_free(lk_memblock block)
{
    lk_free(block.data);
}

extern lk_memblock
lk_memblock_add(lk_memblock block, const char *src, size_t len)
{
    if (len == 0)
        return block;

    char *data = lk_realloc(block.data, block.size + len + 1);
    if (data == NULL) {
        lk_memblock_free(block);
        return lk_memblock_empty();
    }

    if (src != NULL)
        memcpy(data + block.size, src, len);

    block.data = data;
    block.size += len;
    return block;
}

extern lk_memblock
lk_memblock_write_pos(lk_memblock block, size_t pos, const char *src,
                      size_t len)
{
    size_t needed = pos + len;

    if (needed > block.size)
        block = lk_memblock_extend(block, needed - block.size);

    if (block.data == NULL)
        return lk_memblock_empty();

    if (len > 0)
        memcpy(block.data + pos, src, len);

    return block;
}

extern lk_memblock
lk_memblock_copy(lk_memblock block)
{
    if (block.data == NULL)
        return lk_memblock_empty();

    char *data = lk_malloc(block.size);
    if (data == NULL)
        return lk_memblock_empty();

    memcpy(data, block.data, block.size);

    lk_memblock copy = { data, block.size };
    return copy;
}
