#ifndef LK_FILE_H
#define LK_FILE_H

#include <stdbool.h>
#include <stdio.h>

#include "collections/lk_string.h"
#include "object/lk_status.h"

bool lk_check_file_exist(lk_string path);

/*
 * Opens `path` in append mode, creating it when missing. Any existing
 * file is first rotated recursively into `_old` versions: "file.txt"
 * becomes "file_old.txt", "file_old.txt" becomes "file_old_old.txt",
 * and so on. No rollback is performed if a later step fails.
 * If the resulting rotated name would exceed the filename length
 * limit, an error status is returned. The opened stream is returned
 * through `out`, which must not be NULL.
 */
lk_status lk_file_create_save(lk_string path, FILE **out);

#endif
