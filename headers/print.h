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
#ifndef JCUNIT_PRINT_H
#define JCUNIT_PRINT_H 1

#include <stdio.h>

#include "token.h"
#include "ast.h"

void print_token(struct token * token, FILE * output);
void print_ast_test(struct ast_test * test, FILE * output);
void print_ast_requirement(struct ast_requirement * requirement, FILE * output);

#endif /* JCUNIT_PRINT_H */
