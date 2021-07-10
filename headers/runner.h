#ifndef JCUNIT_RUNNER_H
#define JCUNIT_RUNNER_H 1

#include "assembler.h"

#define TEST_RESULT_STATUS_NONE (0)
#define TEST_RESULT_STATUS_PASS (1)
#define TEST_RESULT_STATUS_FAIL (2)

struct test_runner_context
{
    struct list results;
};

struct test_case_result
{
    struct list list_entry;
    unsigned int status;
    struct test_case * test_case;
};

void test_run(struct test_runner_context * context, struct test * test);

#endif /* JCUNIT_RUNNER_H */
