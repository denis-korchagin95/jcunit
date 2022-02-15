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
#ifndef JCUNIT_FS_H
#define JCUNIT_FS_H 1

#include <stdbool.h>
#include <stdint.h>

#include "string.h"
#include "list.h"

#define PATH_SEPARATOR '/'

struct path_list
{
    struct list list_entry;
    const char * path;
};

bool fs_is_file_exists(const char * path);
bool fs_is_file_executable(const char * path);
bool fs_is_dir(const char * path);
bool fs_check_extension(const char * path, const char * extension);

typedef bool fs_read_dir_func(const char * path, void * context);

void fs_read_dir(const char * path, fs_read_dir_func * read_dir_func, void * context);

#endif /* JCUNIT_FS_H */
