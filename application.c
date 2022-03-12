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
#include <stdbool.h>
#include <string.h>

#include "headers/application.h"
#include "headers/source.h"
#include "headers/fs.h"
#include "headers/compiler.h"
#include "headers/list.h"
#include "headers/show-result.h"
#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"
#include "headers/test-iterator.h"
#include "headers/errors.h"


#define SOURCES_CACHE_POS 1024
#define TEST_RESULT_PER_LINE (50)

static const char * sources_cache[SOURCES_CACHE_POS];
static unsigned int sources_cache_pos = 0;
static const char * default_tests_path = "./tests";

typedef void test_mode_func(
    struct test_iterator * test_iterator,
    struct tests_results ** tests_results,
    FILE * output
);

static bool fetch_directory_sources(const char * source_path, void * data);
static void fetch_one_source(const char * source_path, struct slist *** end_sources);
static void run_suites_in_detail_mode(
    struct test_iterator * test_iterator,
    struct tests_results ** tests_results,
    FILE * output
);
static void run_suites_in_passthrough_mode(
    struct test_iterator * test_iterator,
    struct tests_results ** tests_results,
    FILE * output
);

void init_application_context(struct application_context * application_context)
{
    application_context->run_mode = RUN_MODE_PASSTHROUGH;
}

void fetch_sources(int argc, char * argv[], struct slist * sources)
{
    const char * resolved_path;
    int i;
    struct slist ** end_sources = &sources->next;
    for(i = 1; i < argc; ++i) {
        const char * arg = argv[i];
        if (strncmp("--", arg, 2) == 0) {
            continue;
        }
        resolved_path = fs_resolve_path(arg);
        if (resolved_path == NULL) {
            jcunit_fatal_error("Can't to resolve the path \"%s\"!", arg);
        }
        if (fs_is_dir(resolved_path)) {
            fs_read_dir(resolved_path, fetch_directory_sources, &end_sources);
            continue;
        }
        fetch_one_source(resolved_path, &end_sources);
    }
    if (list_is_empty(sources)) {
        resolved_path = fs_resolve_path(default_tests_path);
        if (resolved_path == NULL) {
            jcunit_fatal_error("There are no files provided or default tests path \"%s\" not found!", default_tests_path);
        }
        if (!fs_is_dir(resolved_path)) {
            jcunit_fatal_error("Can't found the specified directory \"%s\"!", default_tests_path);
        }
        fs_read_dir(resolved_path, fetch_directory_sources, &end_sources);
    }
}

bool fetch_directory_sources(const char * source_path, void * data)
{
    if (!fs_check_extension(source_path, ".test")) {
        return true;
    }
    fetch_one_source(source_path, (struct slist ***) data);
    return true;
}

void fetch_one_source(const char * source_path, struct slist *** end_sources)
{
    if (!fs_is_file_exists(source_path)) {
        jcunit_fatal_error("The file \"%s\" is not exists!", source_path);
    }
    unsigned int i = 0;
    while (i < sources_cache_pos && strcmp(sources_cache[i], source_path) != 0) {
        ++i;
    }
    if (i == sources_cache_pos) {
        sources_cache[sources_cache_pos++] = source_path;

        struct source * source = make_source(source_path);
        slist_append(*end_sources, &source->list_entry);
    }
}

void read_suites(
    struct slist * sources,
    struct test_suites * test_suites
) {
    test_suites->suites_count = slist_count(sources);
    test_suites->suites = (struct test_suite **) alloc_bytes(
        sizeof(struct test_suite *) * test_suites->suites_count
    );
    unsigned int i = 0;
    slist_foreach_safe(iterator, sources, {
        struct source * source = list_get_owner(iterator, struct source, list_entry);
        test_suites->suites[i++] = compile_test_suite(source);
    });
}

void run_suites(
    struct test_suites * test_suites,
    struct tests_results ** tests_results,
    struct application_context * application_context,
    FILE * output
) {
    test_mode_func * test_mode_runner = NULL;
    switch (application_context->run_mode) {
        case RUN_MODE_DETAIL:
            test_mode_runner = (test_mode_func *) run_suites_in_detail_mode;
            break;
        case RUN_MODE_PASSTHROUGH:
            test_mode_runner = (test_mode_func *) run_suites_in_passthrough_mode;
            break;
    }
    if (test_mode_runner == NULL) {
        jcunit_fatal_error("The unknown running mode '%u'!", application_context->run_mode);
    }
    struct test_iterator test_iterator;
    test_iterator_init_by_suites(
        &test_iterator,
        test_suites->suites,
        test_suites->suites_count
    );
    *tests_results = make_tests_results(test_iterator.tests_count);
    test_mode_runner(&test_iterator, tests_results, output);
}

void run_suites_in_detail_mode(
    struct test_iterator * test_iterator,
    struct tests_results ** tests_results,
    FILE * output
) {
    struct test_suite * previous_test_suite = NULL, * current_test_suite;
    struct abstract_test * next_test;
    for (;;) {
        if (test_iterator_finished(test_iterator)) {
            break;
        }
        current_test_suite = test_iterator_current(test_iterator)->test_suite;
        if (previous_test_suite != current_test_suite) {
            previous_test_suite = current_test_suite;

            fprintf(output, "Test Suite: %s\n", current_test_suite->name);
        }
        struct abstract_test_result * test_result = test_iterator_visit(
            test_iterator,
            test_runner,
            (void *) *tests_results
        );
        if (test_result == NULL) {
            break;
        }
        show_test_result_in_detail_mode(test_result, output);
        next_test = test_iterator_current_safe(test_iterator);
        if (next_test == NULL || next_test->test_suite != current_test_suite)
            fprintf(
                output,
                "\nPassed: %u, Skipped: %u, Errors: %u, Failed: %u, Incomplete: %u\n\n\n",
                (*tests_results)->passed_count,
                (*tests_results)->skipped_count,
                (*tests_results)->error_count,
                (*tests_results)->failure_count,
                (*tests_results)->incomplete_count
            );
    }
}

void run_suites_in_passthrough_mode(
    struct test_iterator * test_iterator,
    struct tests_results ** tests_results,
    FILE * output
) {
    unsigned int test_result_in_line = 0;
    for (;;) {
        if (test_iterator_finished(test_iterator)) {
            break;
        }
        struct abstract_test_result * test_result = test_iterator_visit(
            test_iterator,
            test_runner,
            (void *) *tests_results
        );
        if (test_result == NULL) {
            break;
        }
        show_test_result_in_passthrough_mode(test_result, output);
        ++test_result_in_line;
        if (test_result_in_line == TEST_RESULT_PER_LINE) {
            test_result_in_line = 0;

            fprintf(output, " (%3u%%)\n", test_iterator->cursor * 100 / test_iterator->tests_count);
        }
    }
    {
        unsigned int i, len;
        for (i = 0, len = TEST_RESULT_PER_LINE - test_result_in_line; i < len; ++i) {
            fprintf(output, " ");
        }
        fprintf(output, " (100%%)");
    }
    fprintf(output, "\n\n");
    if ((*tests_results)->error_count > 0) {
        fprintf(output, "There are %u errors:\n\n", (*tests_results)->error_count);
        {
            unsigned int i, error_number = 0;
            for (i = 0; i < (*tests_results)->results_count; ++i) {
                struct abstract_test_result * test_result = (*tests_results)->results[i];
                if (test_result->status == TEST_RESULT_STATUS_ERROR) {
                    ++error_number;

                    show_error_test_result(output, test_result, error_number);
                }
            }
        }
        fprintf(output, "\n");
    }
    if ((*tests_results)->error_count > 0 && (*tests_results)->failure_count > 0) {
        fprintf(output, "---\n\n");
    }
    if ((*tests_results)->failure_count > 0) {
        fprintf(output, "There are %u failures:\n\n", (*tests_results)->failure_count);
        {
            unsigned int i, failure_number = 0;
            for (i = 0; i < (*tests_results)->results_count; ++i) {
                struct abstract_test_result * test_result = (*tests_results)->results[i];
                if (test_result->status == TEST_RESULT_STATUS_FAILURE) {
                    ++failure_number;

                    show_failure_test_result(output, test_result, failure_number);
                }
            }
        }
        fprintf(output, "\n");
    }
    fprintf(
        output,
        "Passed: %u, Skipped: %u, Errors: %u, Failed: %u, Incomplete: %u\n",
        (*tests_results)->passed_count,
        (*tests_results)->skipped_count,
        (*tests_results)->error_count,
        (*tests_results)->failure_count,
        (*tests_results)->incomplete_count
    );
}
