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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "headers/errors.h"

#define ERROR_BUFFER_SIZE (1024)

void jcunit_fatal_error(const char * fmt, ...)
{
    va_list args;
    static char buffer[ERROR_BUFFER_SIZE];

    va_start(args, fmt);
    vsnprintf(buffer, ERROR_BUFFER_SIZE, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", buffer);
    exit(1);
}
