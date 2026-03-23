#ifndef JCUNIT_APPLICATION_H
#define JCUNIT_APPLICATION_H 1

#include <stdio.h>

#include "list.h"

#define RUN_MODE_PASSTHROUGH    (1)
#define RUN_MODE_DETAIL         (2)

#define OPTION_SHOW_VERSION                     (1)
#define OPTION_SHOW_HELP                        (2)
#define OPTION_USE_COLORS                       (4)
#define OPTION_NO_CACHE                         (8)
#define OPTION_CLEAR_CACHE                      (16)
#define OPTION_NO_SUMMARY                       (32)

struct tests_results;

struct application_context
{
    unsigned int run_mode;
    unsigned int options;
};

struct test_suites
{
    struct test_suite ** suites;
    unsigned int suites_count;
};

extern int no_summary;

void init_application_context(struct application_context * application_context);
void fetch_sources(int argc, char * argv[], struct slist * sources);
struct cache_store;

void read_suites(
    struct slist * sources,
    struct test_suites * test_suites,
    struct application_context * application_context,
    struct cache_store ** out_cache_store
);
void save_and_free_cache(struct cache_store * store);
void run_suites(
    struct test_suites * test_suites,
    struct tests_results ** tests_results,
    struct application_context * application_context,
    FILE * output
);

#endif /* JCUNIT_APPLICATION_H */
