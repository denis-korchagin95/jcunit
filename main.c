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
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "headers/token.h"
#include "headers/runner.h"
#include "headers/show-result.h"
#include "headers/allocate.h"
#include "headers/version.h"
#include "headers/list-iterator.h"
#include "headers/compiler.h"
#include "headers/source.h"
#include "headers/options.h"
#include "headers/fs.h"

#define RUN_MODE_PASSTHROUGH    (1)
#define RUN_MODE_DETAIL         (2)

static char * path_buffer[PATH_MAX] = {0};

static const char * default_tests_path = "./tests";

struct application_context
{
    struct slist suites;
    struct slist ** end_suites;
    unsigned int run_mode;
};

static void init_application_context(struct application_context * application_context);
static void parse_options(int argc, char * argv[], struct application_context * application_context);
static void fetch_suites(int argc, char * argv[], struct application_context * application_context);
static void read_suites(struct application_context * application_context);
static void run_suites(FILE * output, struct application_context * application_context);
static void fetch_one_suite(const char * suite_path, struct application_context * application_context);
static bool fetch_suites_from_directory_handler(const char * suite_path, void * context);
static void run_suites_in_detail_mode(FILE * output, struct application_context * application_context);
static void run_suites_in_passthrough_mode(FILE * output, struct application_context * application_context);

int main(int argc, char * argv[])
{
    struct application_context application_context;
    init_application_context(&application_context);

    parse_options(argc, argv, &application_context);

    if (option_show_version) {
        fprintf(stdout, "jcunit version %s\n", JCUNIT_VERSION);
        exit(0);
    }


    fetch_suites(argc, argv, &application_context);

    init_tokenizer();

    read_suites(&application_context);
    run_suites(stdout, &application_context);

    if (option_show_allocator_stats) {
        fprintf(stdout, "\n\n\n");

        show_allocators_stats(stdout);
    }

    fflush(stdout);

    return 0;
}

void init_application_context(struct application_context * application_context)
{
    slist_init(&application_context->suites);
    application_context->end_suites = &application_context->suites.next;
    application_context->run_mode = RUN_MODE_PASSTHROUGH;
}

void parse_options(int argc, char * argv[], struct application_context * application_context)
{
    int i;
    char * arg;
    for(i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp("--show-allocators-stats", arg, sizeof("--show-allocators-stats") - 1) == 0) {
            option_show_allocator_stats = true;
            continue;
        }
        if (strncmp("--version", arg, sizeof("--version") - 1) == 0) {
            option_show_version = true;
            continue;
        }
        if (strncmp("--run-mode=", arg, sizeof("--run-mode=") - 1) == 0) {
            const char * run_mode = arg + sizeof("--run-mode=") - 1;
            if (strcmp(run_mode, "detail") == 0) {
                application_context->run_mode = RUN_MODE_DETAIL;
                continue;
            }
            if (strcmp(run_mode, "passthrough") == 0) {
                application_context->run_mode = RUN_MODE_PASSTHROUGH;
                continue;
            }
            fprintf(stderr, "The unknown run mode '%s'!\n", run_mode);
            exit(1);
        }
        if (strncmp("--", arg, 2) == 0) {
            fprintf(stderr, "The unknown option: %s!\n", arg);
            exit(1);
        }
    }
}

void fetch_suites(int argc, char * argv[], struct application_context * application_context)
{
    int i;
    char * arg;
    const char * resolved_path;
    for(i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp("--", arg, 2) == 0) {
            continue;
        }
        resolved_path = realpath(arg, (char *)path_buffer);
        if (resolved_path == NULL) {
            fprintf(stderr, "Can't to resolve the path \"%s\"!", arg);
            exit(1);
        }
        if (fs_is_dir(resolved_path)) {
            fs_read_dir(resolved_path, fetch_suites_from_directory_handler, (void *) application_context);
            continue;
        }
        fetch_one_suite(resolved_path, application_context);
    }
    if (list_is_empty(&application_context->suites)) {
        resolved_path = realpath(default_tests_path, (char *)path_buffer);
        if (resolved_path == NULL) {
            fprintf(stderr, "There are no files provided or default tests path \"%s\" not found!\n", default_tests_path);
            exit(1);
        }
        if (!fs_is_dir(resolved_path)) {
            fprintf(stderr, "Can't found the specified directory \"%s\"!", default_tests_path);
            exit(1);
        }
        fs_read_dir(resolved_path, fetch_suites_from_directory_handler, (void *) application_context);
    }
}

bool fetch_suites_from_directory_handler(const char * suite_path, void * context)
{
    if (!fs_check_extension(suite_path, ".test")) {
        return true;
    }
    fetch_one_suite(suite_path, (struct application_context *)context);
    return true;
}

void fetch_one_suite(const char * suite_path, struct application_context * application_context)
{
    if (!fs_is_file_exists(suite_path)) {
        fprintf(stderr, "The file \"%s\" is not exists!", suite_path);
        exit(1);
    }
    struct source * source = make_source(suite_path);
    slist_append(application_context->end_suites, &source->list_entry);
}

void read_suites(struct application_context * application_context)
{
    struct source * source;
    slist_foreach(iterator, &application_context->suites, {
        source = list_get_owner(iterator, struct source, list_entry);
        source->parsed_suite = compile_test_suite(source->filename);
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
    struct source * source;
    struct test_suite_result * test_suite_result;
    struct list_iterator list_iterator;
    slist_foreach(iterator, &application_context->suites, {
        source = list_get_owner(iterator, struct source, list_entry);
        test_suite_result = make_test_suite_result(source->parsed_suite);
        list_iterator_init(&list_iterator, source->parsed_suite->tests.next, &source->parsed_suite->tests);
        show_each_test_result_in_detail_mode(output, &list_iterator, test_runner, (void *) test_suite_result);
    });
}

void run_suites_in_passthrough_mode(FILE * output, struct application_context * application_context)
{
    struct source * source;
    struct test_suite_result * test_suite_result;
    struct list_iterator list_iterator;
    unsigned int total_passes_count = 0;
    unsigned int total_failed_count = 0;
    unsigned int total_incomplete_count = 0;
    slist_foreach(iterator, &application_context->suites, {
        source = list_get_owner(iterator, struct source, list_entry);
        test_suite_result = make_test_suite_result(source->parsed_suite);
        list_iterator_init(&list_iterator, source->parsed_suite->tests.next, &source->parsed_suite->tests);
        show_each_test_result_in_passthrough_mode(output, &list_iterator, test_runner, (void *) test_suite_result);
        total_passes_count += test_suite_result->passed_count;
        total_failed_count += test_suite_result->failed_count;
        total_incomplete_count += test_suite_result->incomplete_count;
    });
    fprintf(
        output,
        "\n\nPassed: %u, Failed: %u, Incomplete: %u\n\n",
        total_passes_count,
        total_failed_count,
        total_incomplete_count
    );
}
