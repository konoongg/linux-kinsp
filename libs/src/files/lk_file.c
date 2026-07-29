#include "typedef_libc.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "collections/lk_string.h"
#include "files/lk_file.h"

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

extern bool
lk_check_file_exist(lk_string path)
{
    lk_stat st;

    if (lk_str_is_empty(path))
        return false;

    return stat(lk_str_raw(path), &st) == 0;
}

/*
 * Builds "name_old.ext": "_old" is inserted before the extension,
 * e.g. "file.txt" -> "file_old.txt", "file" -> "file_old".
 * The extension is the part after the last dot in the last
 * path component; dotfiles like ".bashrc" have no extension.
 */
static lk_string
make_old_name(lk_string path)
{
    const char *data = lk_str_raw(path);
    size_t len = lk_str_len(path);

    ptrdiff_t base_pos = lk_str_find_last_sym(path, '/');
    size_t base_start = (base_pos >= 0) ? (size_t) base_pos + 1 : 0;

    ptrdiff_t dot = lk_str_find_last_sym(path, '.');
    bool has_ext = (dot >= 0 && (size_t) dot > base_start);

    size_t insert = (has_ext) ? (size_t) dot : len;

    lk_string name = lk_str_create_bulk(data, insert);
    if (lk_str_is_empty(name))
        return lk_str_empty;

    name = lk_str_add_back(name, "_old");
    if (lk_str_is_empty(name))
        return lk_str_empty;

    name = lk_str_add_back(name, data + insert);
    if (lk_str_is_empty(name))
        return lk_str_empty;

    return name;
}

static lk_status
rotate_file(lk_string path)
{
    if (!lk_check_file_exist(path))
        return LK_STATUS_OK();

    ptrdiff_t base_pos = lk_str_find_last_sym(path, '/');
    size_t base_start = (base_pos >= 0) ? (size_t) base_pos + 1 : 0;
    size_t base_len = lk_str_len(path) - base_start;

    if (base_len + 4 > NAME_MAX)
        return LK_STATUS_INVALID_ARGUMENT();

    lk_string old_name = make_old_name(path);
    if (lk_str_is_empty(old_name))
        return lk_status_create_msg(LK_MEM_ALLOC_FAIL,
                                    "failed to allocate old filename");

    lk_status st = rotate_file(old_name);
    if (!lk_status_is_ok(st)) {
        lk_str_free(old_name);
        return st;
    }

    if (rename(lk_str_raw(path), lk_str_raw(old_name)) != 0) {
        lk_str_free(old_name);
        return lk_status_create_msg(LK_OPEN_FAIL,
                                    "failed to rename existing file");
    }

    lk_str_free(old_name);
    return LK_STATUS_OK();
}

extern lk_status
lk_file_create_save(lk_string path, FILE **out)
{
    assert(out != NULL);

    if (lk_str_is_empty(path))
        return lk_status_create_msg(LK_INVALID_ARGUMENT, "path is empty");

    lk_status st = rotate_file(path);
    if (!lk_status_is_ok(st))
        return st;

    *out = fopen(lk_str_raw(path), "a");
    if (*out == NULL)
        return lk_status_create_msg(LK_OPEN_FAIL, lk_str_raw(path));

    return lk_status_create(LK_OK);
}
