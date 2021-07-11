#include <stdio.h>


#include "headers/show_result.h"


static const char * pass_prefix = "\033[42mPASS\033[0m";
static const char * fail_prefix = "\033[41mFAIL\033[0m";


void show_test_result(struct test_runner_context * context, FILE * output)
{
    struct test_case_result * test_case_result;
    const char * prefix;
    fprintf(output, "Test: %s\n", context->test->name->value);
    list_foreach(iterator, &context->results, {
        test_case_result = list_get_owner(iterator, struct test_case_result, list_entry);
        prefix = test_case_result->status == TEST_CASE_RESULT_STATUS_PASS ? pass_prefix : fail_prefix;

        fprintf(output, "\t%s %s\n", prefix, test_case_result->test_case->name->value);

        if (test_case_result->status == TEST_CASE_RESULT_STATUS_FAIL) {
            fprintf(output, "--- Expected\n%s$\n", test_case_result->expected->value);
            fprintf(output, "+++ Actual\n%s$\n", test_case_result->actual->value);
        }
    });
    fprintf(output, "\nPassed: %u, Failed: %u\n", context->passed_count, context->failed_count);
    fflush(output);
}
