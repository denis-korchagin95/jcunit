#include <stdio.h>


#include "headers/show_result.h"
#include "headers/child_process.h"


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

        fprintf(output, "\t%s %s\n", prefix, test_case_result->test_case_name->value);

        if (test_case_result->error_code != ERROR_CODE_NONE) {
            switch (test_case_result->error_code) {
                case ERROR_CODE_FILE_NOT_FOUND:
                    fprintf(
                        stdout,
                        "\033[0;31mThe file \"%s\" was not found!\033[0m\n",
                        test_case_result->executable->value
                    );
                    break;
                case ERROR_CODE_NOT_EXECUTABLE:
                    fprintf(
                        stdout,
                        "\033[0;31mThe file \"%s\" is not executable!\033[0m\n",
                        test_case_result->executable->value
                    );
                    break;
                case ERROR_CODE_READ_CHILD_DATA:
                    fprintf(stdout, "\033[0;31mCan't read the program data!\033[0m\n");
                    break;
            }
        }

        if (
            test_case_result->error_code == ERROR_CODE_NONE &&
            test_case_result->status == TEST_CASE_RESULT_STATUS_FAIL
        ) {
            fprintf(output, "--- Expected\n%s$\n", test_case_result->expected->value);
            fprintf(output, "+++ Actual\n%s$\n", test_case_result->actual->value);
        }
    });
    fprintf(output, "\nPassed: %u, Failed: %u\n", context->passed_count, context->failed_count);
    fflush(output);
}
