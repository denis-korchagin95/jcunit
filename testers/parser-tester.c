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

#include "../headers/token.h"
#include "../headers/print.h"
#include "../headers/parse.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "No specified args!\n");
        return 1;
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(argv[1]);
    struct slist * list = parse_test_suite(context);
    destroy_tokenizer_context(context);

    struct ast_test * test = list_get_owner(list->next, struct ast_test, list_entry);

    print_ast_test(test, stdout);

    return 0;
}
