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
#include "headers/runner.h"
#include "headers/show-result.h"
#include "headers/allocate.h"
#include "headers/version.h"
#include "headers/list-iterator.h"
#include "headers/compiler.h"
#include "headers/source.h"


bool option_show_allocator_stats = false;
bool option_show_version = false;


static void parse_args(int argc, char * argv[], struct slist * sources);

int main(int argc, char * argv[])
{
    struct slist sources;
    slist_init(&sources);

    parse_args(argc, argv, &sources);

    if (option_show_version) {
        fprintf(stdout, "jcunit version %s\n", JCUNIT_VERSION);
        exit(0);
    }

    if (list_is_empty(&sources)) {
        fprintf(stderr, "There are no files provided!\n");
        exit(1);
    }

    init_tokenizer();

    struct list_iterator iterator;
    struct source * source;
    struct test_suite_result * test_suite_result;
    slist_foreach(source_iterator, &sources, {
        source = list_get_owner(source_iterator, struct source, list_entry);
        source->parsed_suite = compile_test_suite(source->filename);
        test_suite_result = make_test_suite_result(source->parsed_suite);
        list_iterator_init(&iterator, source->parsed_suite->tests.next, &source->parsed_suite->tests);
        show_each_test_result(stdout, &iterator, test_runner_visiter, (void *)test_suite_result);
    });

    if (option_show_allocator_stats) {
        fprintf(stdout, "\n\n\n");

        show_allocator_stats(stdout, SHOW_STAT_ALL_ALLOCATORS);
    }

    fflush(stdout);

    return 0;
}


void parse_args(int argc, char * argv[], struct slist * suites)
{
    int i;
    char * arg;
    struct source * item;
    struct slist ** end = &suites->next;
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

        item = make_source(arg);
        slist_append(end, &item->list_entry);
    }
}

