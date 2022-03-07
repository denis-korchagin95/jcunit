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
#include <stdlib.h>

#include "headers/application.h"
#include "headers/source.h"
#include "headers/fs.h"
#include "headers/compiler.h"
#include "headers/list.h"
#include "headers/show-result.h"
#include "headers/allocate.h"
#include "headers/test-suite-iterator.h"


#define SOURCES_CACHE_POS 1024

static const char * sources_cache[SOURCES_CACHE_POS];
static unsigned int sources_cache_pos = 0;
static const char * default_tests_path = "./tests";


static bool fetch_directory_sources(const char * source_path, void * context);
static void fetch_one_source(const char * source_path, struct application_context * application_context);
static void run_suites_in_detail_mode(FILE * output, struct application_context * application_context);
static void run_suites_in_passthrough_mode(FILE * output, struct application_context * application_context);

void init_application_context(struct application_context * application_context)
{
    slist_init(&application_context->sources);
    application_context->end_sources = &application_context->sources.next;
    application_context->parsed_suites = NULL;
    application_context->parsed_suites_count = 0;
    application_context->run_mode = RUN_MODE_PASSTHROUGH;
}

void fetch_sources(int argc, char * argv[], struct application_context * application_context)
{
    const char * resolved_path;
    int i;
    for(i = 1; i < argc; ++i) {
        const char * arg = argv[i];
        if (strncmp("--", arg, 2) == 0) {
            continue;
        }
        resolved_path = fs_resolve_path(arg);
        if (resolved_path == NULL) {
            fprintf(stderr, "Can't to resolve the path \"%s\"!", arg);
            exit(1);
        }
        if (fs_is_dir(resolved_path)) {
            fs_read_dir(resolved_path, fetch_directory_sources, (void *) application_context);
            continue;
        }
        fetch_one_source(resolved_path, (void *) application_context);
    }
    if (list_is_empty(&application_context->sources)) {
        resolved_path = fs_resolve_path(default_tests_path);
        if (resolved_path == NULL) {
            fprintf(stderr, "There are no files provided or default tests path \"%s\" not found!\n", default_tests_path);
            exit(1);
        }
        if (!fs_is_dir(resolved_path)) {
            fprintf(stderr, "Can't found the specified directory \"%s\"!", default_tests_path);
            exit(1);
        }
        fs_read_dir(resolved_path, fetch_directory_sources, (void *) application_context);
    }
}

bool fetch_directory_sources(const char * source_path, void * context)
{
    if (!fs_check_extension(source_path, ".test")) {
        return true;
    }
    fetch_one_source(source_path, (struct application_context *) context);
    return true;
}

void fetch_one_source(const char * source_path, struct application_context * application_context)
{
    if (!fs_is_file_exists(source_path)) {
        fprintf(stderr, "The file \"%s\" is not exists!", source_path);
        exit(1);
    }
    unsigned int i = 0;
    while (i < sources_cache_pos && strcmp(sources_cache[i], source_path) != 0) {
        ++i;
    }
    if (i == sources_cache_pos) {
        sources_cache[sources_cache_pos++] = source_path;

        struct source * source = make_source(source_path);
        slist_append(application_context->end_sources, &source->list_entry);
    }
}

void read_suites(struct application_context * application_context)
{
    application_context->parsed_suites_count = 0;
    slist_foreach(iterator, &application_context->sources, {
        ++application_context->parsed_suites_count;
    });
    application_context->parsed_suites = (struct test_suite **) alloc_bytes(
            sizeof(struct test_suite *) * application_context->parsed_suites_count
    );
    struct source * source;
    unsigned int i = 0;
    slist_foreach(iterator, &application_context->sources, {
        source = list_get_owner(iterator, struct source, list_entry);
        application_context->parsed_suites[i++] = compile_test_suite(source->filename);
    });
}

void run_suites(FILE * output, struct application_context * application_context)
{
    switch (application_context->run_mode) {
        case RUN_MODE_DETAIL:
            run_suites_in_detail_mode(output, application_context);
            break;
        case RUN_MODE_PASSTHROUGH:
            run_suites_in_passthrough_mode(output, application_context);
            break;
        default:
            fprintf(stderr, "The unknown running mode '%u'!\n", application_context->run_mode);
            exit(1);
    }
}

void run_suites_in_detail_mode(FILE * output, struct application_context * application_context)
{
    unsigned int i, len;
    struct test_suite * test_suite;
    struct test_suite_result * test_suite_result;
    struct test_suite_iterator iterator;
    for (i = 0, len = application_context->parsed_suites_count; i < len; ++i) {
        test_suite = application_context->parsed_suites[i];
        test_suite_result = make_test_suite_result(test_suite);
        test_suite_iterator_init(&iterator, test_suite);
        show_each_test_result_in_detail_mode(output, &iterator, test_runner, (void *) test_suite_result);
    }
}

void run_suites_in_passthrough_mode(FILE * output, struct application_context * application_context)
{
    struct test_suite * test_suite;
    struct test_suite_result * test_suite_result;
    struct test_suite_iterator iterator;
    unsigned int total_passes_count = 0;
    unsigned int total_skipped_count = 0;
    unsigned int total_failed_count = 0;
    unsigned int total_incomplete_count = 0;
    unsigned int total_error_count = 0;
    {
        unsigned int i, len;
        for (i = 0, len = application_context->parsed_suites_count; i < len; ++i) {
            test_suite = application_context->parsed_suites[i];
            test_suite_result = make_test_suite_result(test_suite);
            test_suite_iterator_init(&iterator, test_suite);
            show_each_test_result_in_passthrough_mode(output, &iterator, test_runner, (void *) test_suite_result);
            total_passes_count += test_suite_result->passed_count;
            total_skipped_count += test_suite_result->skipped_count;
            total_error_count += test_suite_result->error_count;
            total_failed_count += test_suite_result->failure_count;
            total_incomplete_count += test_suite_result->incomplete_count;
        }
    }
    fprintf(output, "\n\n");
    if (test_suite_result->error_count > 0) {
        fprintf(output, "There are %u errors:\n\n", test_suite_result->error_count);
        {
            int i, len;
            unsigned int error_number = 0;
            for (i = 0, len = test_suite_result->test_results_count; i < len; ++i) {
                struct abstract_test_result * test_result = test_suite_result->test_results[i];
                if (test_result->status == TEST_RESULT_STATUS_ERROR) {
                    ++error_number;

                    show_error_test_result(output, test_result, error_number);
                }
            }
        }
        fprintf(output, "\n");
    }
    if (test_suite_result->error_count > 0  && test_suite_result->failure_count > 0) {
        fprintf(output, "---\n\n");
    }
    if (test_suite_result->failure_count > 0) {
        fprintf(output, "There are %u failures:\n\n", test_suite_result->failure_count);
        {
            int i, len;
            unsigned int failure_number = 0;
            for (i = 0, len = test_suite_result->test_results_count; i < len; ++i) {
                struct abstract_test_result * test_result = test_suite_result->test_results[i];
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
        total_passes_count,
        total_skipped_count,
        total_error_count,
        total_failed_count,
        total_incomplete_count
    );
}
