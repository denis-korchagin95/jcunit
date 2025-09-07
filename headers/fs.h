#ifndef JCUNIT_FS_H
#define JCUNIT_FS_H 1

#include <stdbool.h>
#include <stdint.h>

#include "string.h"
#include "list.h"

#define PATH_SEPARATOR '/'

#define FS_FLAG_USE_PATH (1)

struct path_list
{
    struct list list_entry;
    const char * path;
};

bool fs_is_file_exists(const char * path);
bool fs_is_file_executable(const char * path);
bool fs_is_dir(const char * path);
bool fs_check_extension(const char * path, const char * extension);

typedef bool fs_read_dir_func(const char * path, void * context, unsigned int * flags);

void fs_read_dir(const char * path, fs_read_dir_func * read_dir_func, void * context);
const char * fs_resolve_path(const char * path);

#endif /* JCUNIT_FS_H */
