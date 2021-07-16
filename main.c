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
    struct abstract_test_case_result * test_case_result;
    struct abstract_test_case * test_case;
    bool has_cases = !list_is_empty(&test->cases);

    fprintf(stdout, "Test: %s\n", test->name->value);

    if (has_cases) {
        list_foreach(iterator, &test->cases, {
            test_case = list_get_owner(iterator, struct abstract_test_case, list_entry);

            test_case_result = test_case_run(test_case);

            test_result_add_test_case_result(test_result, test_case_result);

            show_test_case_result(test_case_result, stdout);
        });

        fprintf(
            stdout,
            "\nPassed: %u, Failed: %u, Incomplete: %u\n",
            test_result->passed_count,
            test_result->failed_count,
            test_result->incomplete_count
        );
    } else {
        fprintf(stdout, "\tThere is no any test cases!\n");
    }

    if (option_show_allocator_stats) {
        printf("\n\n\n");

        show_allocator_stats(stdout, SHOW_STAT_ALL_ALLOCATORS);
    }

    fflush(stdout);

    return 0;
}
