/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
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
#include <memory.h>


#include "headers/string.h"
#include "headers/allocate.h"


struct string * make_string(const char * source, unsigned int len)
{
    char * storage = alloc_bytes(len + 1);
    memcpy((void *)storage, source, len * sizeof(char));
    storage[len] = '\0';

    struct string * string = alloc_string();
    string->len = len;
    string->value = storage;

    return string;
}


bool string_equals_with_cstring(struct string * source, const char * dest)
{
    unsigned int len = strlen(dest);
    return source->len == len && strncmp(source->value, dest, len) == 0;
}
