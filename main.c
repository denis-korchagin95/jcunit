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

#include "headers/token.h"
#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"
#include "headers/version.h"
#include "headers/options.h"
#include "headers/application.h"

#define PROGRAM_NAME "jcunit"

void show_help(const char * name, FILE * output);

int main(int argc, char * argv[])
{
    struct application_context application_context;
    init_application_context(&application_context);

    parse_options(argc, argv, &application_context);

    if (application_context.options & OPTION_SHOW_HELP) {
        show_help(PROGRAM_NAME, stdout);
        exit(0);
    }
    if (application_context.options & OPTION_SHOW_VERSION) {
        fprintf(stdout, "%s version %s\n", PROGRAM_NAME, JCUNIT_VERSION);
        exit(0);
    }

    struct slist sources;
    slist_init(&sources);

    fetch_sources(argc, argv, &sources);

    /* TODO: FIXME - check on empty sources */

    init_tokenizer();

    struct test_suites test_suites;

    read_suites(&sources, &test_suites);

    slist_protect(&sources);

    /* TODO: FIXME - check on empty test suites */

    struct tests_results * tests_results = NULL;

    run_suites(&test_suites, &tests_results, &application_context, stdout);

    release_test_suites(&test_suites);
    release_tests_results(tests_results);
    tests_results = NULL;

    if (application_context.options & OPTION_SHOW_ALLOCATORS_STATS) {
        fprintf(stdout, "\n\n\n");

        bool show_leak_only = (application_context.options & OPTION_SHOW_ALLOCATORS_STATS_LEAK_ONLY) > 0 ? true : false;

        show_allocators_stats(stdout, show_leak_only);
        show_bytes_allocator_stats(stdout, show_leak_only);
    }

    fflush(stdout);

    return 0;
}

void show_help(const char * name, FILE * output)
{
    fprintf(output, "Usage: %s [options] path [paths...]\n\n\n", name);
    fprintf(output, "OPTIONS\n");
    fprintf(output, "\t--run-mode=(detail|passthrough) default: passthrough\n\t    "
                    "The mode of showing the results of testing.\n\n");
    fprintf(output, "\t--help\n\t    Show this message.\n\n");
    fprintf(output, "\t--version\n\t    Show version of this program.\n\n");
    fprintf(output, "\t--show-allocators-stats\n\t    Show statistics of memory allocators (for developers).\n\n");
    fprintf(output, "\t--show-allocators-stats-leak-only\n\t    "
                    "Show statistics of memory allocators which has a memory leak (for developers).\n\n");
}
