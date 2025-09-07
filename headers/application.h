#ifndef JCUNIT_APPLICATION_H
#define JCUNIT_APPLICATION_H 1

#include <stdio.h>

#include "list.h"

#define RUN_MODE_PASSTHROUGH    (1)
#define RUN_MODE_DETAIL         (2)

#define OPTION_SHOW_ALLOCATORS_STATS            (1)
#define OPTION_SHOW_ALLOCATORS_STATS_LEAK_ONLY  (2)
#define OPTION_SHOW_VERSION                     (4)
#define OPTION_SHOW_HELP                        (8)

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

void init_application_context(struct application_context * application_context);
void fetch_sources(int argc, char * argv[], struct slist * sources);
void read_suites(
    struct slist * sources,
    struct test_suites * test_suites
);
void run_suites(
    struct test_suites * test_suites,
    struct tests_results ** tests_results,
    struct application_context * application_context,
    FILE * output
);

#endif /* JCUNIT_APPLICATION_H */
