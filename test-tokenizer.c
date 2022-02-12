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
#include <stdio.h>

#include "headers/token.h"
#include "headers/print.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "No specified args!\n");
        return 1;
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(argv[1]);

    struct token * token;
    for(;;) {
        token = get_one_token(context);
        print_token(token, stdout);
        puts("");
        if (token->kind == TOKEN_KIND_EOF) {
            break;
        }
    }

    destroy_tokenizer_context(context);

    return 0;
}
