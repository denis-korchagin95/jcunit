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
#include <string.h>

#include "headers/source.h"
#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"

struct source * make_source(const char * filename)
{
    struct source * source = alloc_source();
    unsigned int len = strlen(filename);
    char * _filename = alloc_bytes(len + 1);
    strcpy(_filename, filename);
    _filename[len] = '\0';
    source->filename = _filename;
    slist_init(&source->list_entry);
    return source;
}
