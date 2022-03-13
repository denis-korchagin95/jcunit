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
#include <stddef.h>
#include <string.h>


#include "headers/util.h"
#include "headers/bytes-allocator.h"


const char * basename(const char * path)
{
    const char * iterator = path;
    const char * last_slash_pos = NULL;
    while (*iterator != '\0') {
        if (*iterator == '/') {
            last_slash_pos = iterator;
        }
        ++iterator;
    }
    if (last_slash_pos == NULL) {
        if (*path == '\0') {
            return NULL;
        }
        return path;
    }
    return last_slash_pos + 1;
}

const char * duplicate_cstring(const char * src)
{
    unsigned int len = strlen(src);
    char * dest = alloc_bytes(len + 1);
    strcpy(dest, src);
    dest[len] = '\0';
    return dest;
}
