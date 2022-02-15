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
#include <assert.h>


#include "headers/finder.h"


struct ast_requirement * find_ast_requirement_by_name(
    struct ast_test * ast_test,
    const char * requirement_name
) {
    assert(ast_test != NULL);
    size_t len = strlen(requirement_name);
    struct ast_requirement * requirement;
    slist_foreach(iterator, &ast_test->requirements, {
        requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        if (requirement->name->len == len && strncmp(requirement->name->value, requirement_name, len) == 0) {
            return requirement;
        }
    });
    return NULL;
}
