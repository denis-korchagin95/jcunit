#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "headers/fs.h"
#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"
#include "headers/errors.h"
#include "headers/util.h"

#define GET_ENTRY_PATH_ADD_PATH_SEPARATOR (1)

static char * path_buffer[PATH_MAX] = {0};

static struct stat st_buf;
static char * get_entry_path(const char * path, uint32_t path_len, struct dirent * dirent, unsigned int flags);

bool fs_is_dir(const char * pathname)
{
    return stat(pathname, &st_buf) == 0 && (st_buf.st_mode & S_IFDIR) > 0;
}

bool fs_is_file_exists(const char * pathname)
{
    return stat(pathname, &st_buf) == 0;
}

bool fs_is_file_executable(const char * pathname)
{
    return stat(pathname, &st_buf) == 0 && (st_buf.st_mode & S_IEXEC) > 0;
}

bool fs_check_extension(const char * path, const char * extension)
{
    char * point = strrchr(path, '.');
    if (point == NULL) {
        return extension == NULL;
    }
    if (extension == NULL) {
        return false;
    }
    return strcmp(extension, point) == 0;
}

void fs_read_dir(const char * path, fs_read_dir_func * read_dir_func, void * context)
{
    struct path_list base_path;
    base_path.path = path;

    struct list queue;
    list_init(&queue);

    list_append(&queue, &base_path.list_entry);

    struct path_list * current_path;
    bool request_to_quit = false;

    for (;;) {
        if (request_to_quit || list_is_empty(&queue)) {
            break;
        }
        struct list *shifted = list_shift(&queue);
        assert(shifted != NULL);
        current_path = list_get_owner(shifted, struct path_list, list_entry);

        DIR * dir = opendir(current_path->path);
        if (dir == NULL) {
            jcunit_fatal_error("Can't open directory \"%s\"!", current_path->path);
        }

        uint32_t current_path_len = strlen(current_path->path);

        for (;;) {
            if (request_to_quit) {
                break;
            }
            struct dirent *dirent = readdir(dir);
            if (dirent == NULL)
                break;
            if (dirent->d_type == DT_DIR) {
                if (
                    strncmp(".", dirent->d_name, dirent->d_namlen) == 0
                    || strncmp("..", dirent->d_name, dirent->d_namlen) == 0
                ) {
                    continue;
                }
                struct path_list * new_directory = alloc_path_list();
                list_init(&new_directory->list_entry);
                new_directory->path = get_entry_path(current_path->path, current_path_len, dirent, GET_ENTRY_PATH_ADD_PATH_SEPARATOR);
                list_append(&queue, &new_directory->list_entry);
                continue;
            }
            if (dirent->d_type != DT_REG) {
                continue;
            }
            char * file_entry_path = get_entry_path(current_path->path, current_path_len, dirent, GET_ENTRY_PATH_ADD_PATH_SEPARATOR);
            unsigned int fs_flags = 0;
            request_to_quit = !read_dir_func(file_entry_path, context, &fs_flags);
            if ((fs_flags & FS_FLAG_USE_PATH) == 0) {
                free_bytes((void *) file_entry_path);
                file_entry_path = NULL;
            }
        }

        if (current_path != &base_path) {
            release_path_list(current_path);
            current_path = NULL;
        }
        (void) closedir(dir);
    }
}

char * get_entry_path(const char * path, uint32_t path_len, struct dirent * dirent, unsigned int flags)
{
    uint32_t entry_len = path_len + dirent->d_namlen + ((flags & GET_ENTRY_PATH_ADD_PATH_SEPARATOR) > 0 ? 1 : 0);
    char * entry_path = alloc_bytes(entry_len + 1);
    memcpy((void *) entry_path, (const void *) path, path_len);
    if ((flags & GET_ENTRY_PATH_ADD_PATH_SEPARATOR) > 0) {
        entry_path[path_len] = PATH_SEPARATOR;
        memcpy((void *) (entry_path + path_len + 1), (const void *) dirent->d_name, dirent->d_namlen);
    }
    else {
        memcpy((void *) (entry_path + path_len), (const void *) dirent->d_name, dirent->d_namlen);
    }
    entry_path[entry_len] = '\0';
    return entry_path;
}

const char * fs_resolve_path(const char * path)
{
    const char * resolved_path = realpath(path, (char *) path_buffer);
    if (resolved_path == NULL) {
        return NULL;
    }
    return duplicate_cstring(resolved_path);
}
