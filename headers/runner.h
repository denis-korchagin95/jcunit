#ifndef JCUNIT_RUNNER_H
#define JCUNIT_RUNNER_H 1

#include "assembler.h"

#define TEST_CASE_RESULT_STATUS_NONE (0)
#define TEST_CASE_RESULT_STATUS_PASS (1)
#define TEST_CASE_RESULT_STATUS_FAIL (2)

struct test_runner_context
{
    struct list results;
    unsigned int passed_count;
    unsigned int failed_count;
    struct test * test;
};

struct test_case_result
{
    struct list list_entry;
    struct test_case * test_case;
    unsigned int status;
};

void test_run(struct test_runner_context * context);

struct test_runner_context * make_test_runner_context(struct test * test);

#endif /* JCUNIT_RUNNER_H */
