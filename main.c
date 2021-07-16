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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "headers/token.h"
#include "headers/list.h"
#include "headers/parse.h"
#include "headers/assembler.h"
#include "headers/runner.h"
#include "headers/show_result.h"
#include "headers/allocate.h"
#include "headers/version.h"
#include "headers/list-iterator.h"


bool option_show_allocator_stats = false;
bool option_show_version = false;


int main(int argc, char * argv[])
{
    int i;
    char * arg;
    char * filename = NULL;
    for(i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp(arg, "--show-allocator-stats", sizeof("--show-allocator-stats")) == 0) {
            option_show_allocator_stats = true;
            continue;
        }
        if (strncmp(arg, "--version", sizeof("--version")) == 0) {
            option_show_version = true;
            continue;
        }
        if (strncmp("--", arg, 2) == 0) {
            fprintf(stderr, "The unknown option: %s\n", arg);
            exit(1);
        }
        filename = arg;
    }

    if (option_show_version) {
        fprintf(stdout, "jcunit version %s\n", JCUNIT_VERSION);
        exit(0);
    }

    if (filename == NULL) {
        fprintf(stderr, "The filename is missing!\n");
        exit(1);
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(filename);
    struct list * ast_cases = parse_test(context);
    destroy_tokenizer_context(context);

    struct test * test = assemble_test(filename, ast_cases);

    struct test_result * test_result = make_test_result(test);

    struct list_iterator iterator;
    list_iterator_init(&iterator, test->cases.next, &test->cases);

    show_each_test_case_result(stdout, &iterator, test_case_runner_visiter, (void *)test_result);

    if (option_show_allocator_stats) {
        fprintf(stdout, "\n\n\n");

        show_allocator_stats(stdout, SHOW_STAT_ALL_ALLOCATORS);
    }

    fflush(stdout);

    return 0;
}
