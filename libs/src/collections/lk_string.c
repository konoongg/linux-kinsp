#include <string.h>

#include "allocator/lk_allocator.h"
#include "collections/lk_string.h"

extern lk_string
lk_str_create(const char *src)
{
    if (src == NULL)
        return lk_str_empty;

    return lk_str_create_bulk(src, strlen(src));
}

extern lk_string
lk_str_create_bulk(const char *src, size_t len)
{
    if (src == NULL)
        return lk_str_empty;

    char *data = lk_malloc(len + 1);
    if (data == NULL)
        return lk_str_empty;

    memcpy(data, src, len);
    data[len] = '\0';

    lk_string str = { lk_memblock_ref(&data, len) };
    return str;
}

extern int
lk_str_cmp(const lk_string lhs, const lk_string rhs)
{
    if (lhs.mem.size != rhs.mem.size)
        return (lhs.mem.size < rhs.mem.size) ? -1 : 1;

    if (lhs.mem.size == 0)
        return 0;

    return memcmp(lhs.mem.data, rhs.mem.data, lhs.mem.size);
}

extern lk_string
lk_str_copy(const lk_string str)
{
    return lk_str_create_bulk(str.mem.data, str.mem.size);
}

extern lk_string
lk_str_add_back(lk_string str, const char *src)
{
    if (src == NULL)
        return str;

    size_t add = strlen(src);

    str.mem = lk_memblock_write_pos(str.mem, str.mem.size, src, add);
    if (str.mem.data == NULL)
        return lk_str_empty;

    str.mem.data[str.mem.size] = '\0';

    return str;
}

extern ptrdiff_t
lk_str_find_first_sym(const lk_string str, char c)
{
    if (lk_str_is_empty(str))
        return -1;

    const char *pos = strchr(lk_str_raw(str), c);
    if (pos == NULL)
        return -1;

    return pos - str.mem.data;
}

extern ptrdiff_t
lk_str_find_last_sym(const lk_string str, char c)
{
    if (lk_str_is_empty(str))
        return -1;

    const char *pos = strrchr(lk_str_raw(str), c);
    if (pos == NULL)
        return -1;

    return pos - str.mem.data;
}

extern lk_string
lk_str_concat(const lk_string lhs, const lk_string rhs)
{
    if (lk_str_is_empty(lhs))
        return lk_str_copy(rhs);

    if (lk_str_is_empty(rhs))
        return lk_str_copy(lhs);

    lk_string out = lk_str_create_bulk(lk_str_raw(lhs), lk_str_len(lhs));
    if (lk_str_is_empty(out))
        return lk_str_empty;

    out = lk_str_add_back(out, lk_str_raw(rhs));

    return out;
}

extern void
lk_str_free(lk_string str)
{
    lk_memblock_free(str.mem);
}
