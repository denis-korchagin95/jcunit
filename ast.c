/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021 Denis Korchagin <denis.korchagin.1995@gmail.com>
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


#include "headers/ast.h"
#include "headers/allocate.h"


struct ast_test_case * make_ast_test_case(void)
{
    struct ast_test_case * ast_test_case = alloc_ast_test_case();
    list_init(&ast_test_case->requirements);
    memset((void *)&ast_test_case->list_entry, 0, sizeof(struct list));
    ast_test_case->name = NULL;
    return ast_test_case;
}

struct ast_requirement * make_ast_requirement(void)
{
    struct ast_requirement * requirement = alloc_ast_requirement();
    memset((void *)&requirement->list_entry, 0, sizeof(struct list));
    requirement->name = NULL;
    requirement->argument = NULL;
    requirement->content = NULL;
    return requirement;
}
