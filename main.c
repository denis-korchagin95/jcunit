#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "headers/token.h"
#include "headers/version.h"
#include "headers/options.h"
#include "headers/application.h"
#include "headers/runner.h"
#include "headers/allocator.h"
#include "headers/errors.h"
#include "headers/source.h"
#include "headers/test-iterator.h"
#include "headers/util.h"
#include "headers/diff.h"
#include "headers/cache.h"

#define PROGRAM_NAME "jcunit"

void show_help(const char * name, FILE * output);

int main(int argc, char * argv[])
{
    struct application_context application_context;
    memset(&application_context, 0, sizeof(struct application_context));
    init_application_context(&application_context);
    init_tokenizer();

    int exit_code = 0;

    parse_options(argc, argv, &application_context);

    if (application_context.options & OPTION_USE_COLORS) {
        diff_use_colors = 1;
    }

    if (application_context.options & OPTION_SHOW_HELP) {
        show_help(PROGRAM_NAME, stdout);
        exit(exit_code);
    }
    if (application_context.options & OPTION_SHOW_VERSION) {
        fprintf(stdout, "%s version %s\n", PROGRAM_NAME, JCUNIT_VERSION);
        exit(exit_code);
    }
    if (application_context.options & OPTION_NO_CACHE) {
        putenv("JCUNIT_NO_CACHE=1");
    }
    if (application_context.options & OPTION_CLEAR_CACHE) {
        remove(CACHE_FILENAME);
    }

    if (atexit(cleanup) != 0) {
        jcunit_fatal_error("Can't register atexit handler!");
    }

    memory_blob_pool_init_pools();

    struct slist sources;
    slist_init(&sources);

    fetch_sources(argc, argv, &sources);

    if (slist_count(&sources) == 0) {
        fprintf(stdout, "No sources found!\n");
        exit(exit_code);
    }

    struct test_suites test_suites;
    struct cache_store * cache_store_to_save = NULL;

    read_suites(&sources, &test_suites, &application_context, &cache_store_to_save);

    slist_protect(&sources);

    if (get_total_tests_count(test_suites.suites, test_suites.suites_count) == 0) {
        save_and_free_cache(cache_store_to_save);
        fprintf(stdout, "No tests executed!\n");
        exit(exit_code);
    }

    struct tests_results * tests_results = NULL;

    run_suites(&test_suites, &tests_results, &application_context, stdout);

    save_and_free_cache(cache_store_to_save);

    if (tests_results->error_count > 0 || tests_results->failure_count > 0) {
        exit_code = 2;
    }

    fflush(stdout);

    exit(exit_code);
}

void show_help(const char * name, FILE * output)
{
    fprintf(output, "Usage: %s [options] path [paths...]\n\n\n", name);
    fprintf(output, "OPTIONS\n");
    fprintf(output, "\t--run-mode=(detail|passthrough) default: passthrough\n\t    "
                    "The mode of showing the results of testing.\n\n");
    fprintf(output, "\t--help\n\t    Show this message.\n\n");
    fprintf(output, "\t--colors\n\t    Enable colored diff output.\n\n");
    fprintf(output, "\t--version\n\t    Show version of this program.\n\n");
    fprintf(output, "\t--no-cache\n\t    Disable cache for parsing and assembling phases.\n\n");
    fprintf(output, "\t--clear-cache\n\t    Clear the cache file before running.\n\n");
}
