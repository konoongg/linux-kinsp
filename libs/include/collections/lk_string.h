#ifndef LK_STRING_H
#define LK_STRING_H

#include <stddef.h>

#include "collections/lk_memblock.h"

typedef struct lk_string {
    lk_memblock mem;
} lk_string;

#define lk_str_empty ((lk_string) { lk_memblock_empty() })

extern lk_string lk_str_create(const char *src);
extern lk_string lk_str_create_bulk(const char *src, size_t len);
extern int lk_str_cmp(const lk_string lhs, const lk_string rhs);
extern lk_string lk_str_copy(const lk_string str);
extern lk_string lk_str_add_back(lk_string str, const char *src);
extern lk_string lk_str_concat(const lk_string lhs, const lk_string rhs);
extern void lk_str_free(lk_string str);

#define lk_str_len(str) ((str).mem.size)

static inline int
lk_str_is_empty(const lk_string str)
{
    return lk_memblock_is_empty(str.mem);
}

#define lk_str_raw(str) ((str).mem.data)

extern ptrdiff_t lk_str_find_first_sym(const lk_string str, char c);
extern ptrdiff_t lk_str_find_last_sym(const lk_string str, char c);

static inline lk_string
lk_str_wrap(char **src, size_t size)
{
    lk_string str = { lk_memblock_ref(src, size) };
    return str;
}

#endif
