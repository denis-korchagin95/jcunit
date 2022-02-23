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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../headers/token.h"
#include "../headers/print.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "No specified args!\n");
        return 1;
    }
    int token_number = -1;
    int i;
    for (i = 1; i < argc; ++i) {
        char * arg = argv[i];
        if (strncmp("--turn-special-mode-after-token=", arg, sizeof("--turn-special-mode-after-token=") - 1) == 0) {
            arg += (sizeof("--turn-special-mode-after-token=") - 1);
            if (!isdigit(*arg)) {
                fprintf(stderr, "Bad token number of option \"--turn-special-mode-after-token\"!\n");
                exit(1);
            }
            token_number = atoi(arg);
        }
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(argv[1]);

    int token_counter = 0;

    struct token * token;
    for(;;) {
        token = get_one_token(context);
        ++token_counter;
        if (token_number != -1 && token_counter > token_number) {
            context->mode = TOKENIZER_MODE_DIRECTIVE_AND_TEXT;
        }
        print_token(token, stdout);
        puts("");
        if (token->kind == TOKEN_KIND_EOF) {
            break;
        }
    }

    destroy_tokenizer_context(context);

    return 0;
}
