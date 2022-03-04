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
#include "headers/compiler.h"
#include "headers/token.h"
#include "headers/parse.h"
#include "headers/allocate.h"


struct test_suite * compile_test_suite(const char * filename)
{
    struct tokenizer_context * context = make_tokenizer_context(filename);

    struct slist * ast_tests = parse_test_suite(context);

    destroy_tokenizer_context(context);

    struct test_suite * test_suite = assemble_test_suite(filename, ast_tests);

    release_ast_tests(ast_tests);
    free_slist(ast_tests);

    return test_suite;
}
