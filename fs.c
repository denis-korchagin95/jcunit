/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021-2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
 *
 * This file is part of JCUnit
 *
 * For the full license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation of version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "headers/fs.h"
#include "headers/allocate.h"

#define GET_ENTRY_PATH_ADD_PATH_SEPARATOR (1)

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
            fprintf(stderr, "Can't open directory \"%s\"!", current_path->path);
            exit(1);
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
            request_to_quit = !read_dir_func(file_entry_path, context);
            /* TODO: free the static memory from bytes allocator */
        }

        if (current_path != &base_path) {
            free_path_list(current_path);
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
