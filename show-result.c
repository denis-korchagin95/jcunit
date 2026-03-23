#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>


#include "headers/show-result.h"
#include "headers/child-process.h"
#include "headers/test-iterator.h"
#include "headers/string.h"
#include "headers/errors.h"
#include "headers/diff.h"


static const char * stringify_test_status(struct abstract_test_result * test_result, bool use_short_version);
static void print_error(struct abstract_test_result * test_result, FILE * output);
static void print_program_runner_error(struct program_runner_test_result * test_result, FILE * output);
static void print_stderr_output(struct program_runner_test_result * test_result, FILE * output);
static void print_exit_code(struct program_runner_test_result * test_result, FILE * output);

void show_test_result_in_detail_mode(struct abstract_test_result * test_result, FILE * output)
{
    assert(test_result != NULL);

    const char * test_status = stringify_test_status(test_result, false);

    if (test_status == NULL) {
        jcunit_fatal_error("Can't resolve the status of test result!");
    }

    fprintf(output, "    %10s %s (%.0f ms)\n", test_status, test_result->name->value, test_result->elapsed_ms);

    if (
        test_result->status == TEST_RESULT_STATUS_INCOMPLETE
        || test_result->status == TEST_RESULT_STATUS_PASS
        || test_result->status == TEST_RESULT_STATUS_SKIPPED
    ) {
        return;
    }

    if (test_result->status == TEST_RESULT_STATUS_ERROR) {
        print_error(test_result, output);
        return;
    }

    if (test_result->status == TEST_RESULT_STATUS_FAILURE) {
        struct program_runner_test_result * prt = (struct program_runner_test_result *)test_result;
        print_diff(output, test_result->expected, test_result->actual);
        print_exit_code(prt, output);
        print_stderr_output(prt, output);
        return;
    }

    fprintf(output, "The unknown test status!\n");
}

void show_test_result_in_passthrough_mode(struct abstract_test_result * test_result, FILE * output)
{
    assert(test_result != NULL);

    const char * test_status = stringify_test_status(test_result, true);

    if (test_status == NULL) {
        jcunit_fatal_error("Can't resolve the status of test result!");
    }

    fprintf(output, "%s", test_status);
}

static const char * stringify_test_status(struct abstract_test_result * test_result, bool use_short_version)
{
    switch (test_result->status) {
        case TEST_RESULT_STATUS_PASS: return use_short_version ? "." : "PASS";
        case TEST_RESULT_STATUS_FAILURE: return use_short_version ? "F" : "FAIL";
        case TEST_RESULT_STATUS_INCOMPLETE: return use_short_version ? "I" : "INCOMPLETE";
        case TEST_RESULT_STATUS_ERROR: return use_short_version ? "E" : "ERROR";
        case TEST_RESULT_STATUS_SKIPPED: return use_short_version ? "S" : "SKIPPED";
    }
    return NULL;
}

void print_error(struct abstract_test_result * test_result, FILE * output)
{
    switch (test_result->kind) {
        case TEST_RESULT_KIND_PROGRAM_RUNNER:
            print_program_runner_error((struct program_runner_test_result *)test_result, output);
            break;
    }
}

void print_program_runner_error(struct program_runner_test_result * test_result, FILE * output)
{
    switch (test_result->error_code) {
        case ERROR_CODE_NONE:
            break;
        case ERROR_CODE_FILE_NOT_FOUND:
            fprintf(
                output,
                "The file \"%s\" was not found!\n",
                test_result->executable->value
            );
            break;
        case ERROR_CODE_NOT_EXECUTABLE:
            fprintf(
                output,
                "The file \"%s\" is not executable!\n",
                test_result->executable->value
            );
            break;
        case ERROR_CODE_READ_CHILD_DATA:
            fprintf(output, "Can't read the program data!\n");
            break;
        case ERROR_CODE_OUTPUT_TOO_LONG:
            fprintf(output, "The program output is too long!\n");
            break;
    }
}

struct abstract_test_result * test_runner(
    struct abstract_test * test,
    struct tests_results * tests_results,
    unsigned int current_index
) {
    struct timespec start, end;
    struct abstract_test_result * test_result;

    clock_gettime(CLOCK_MONOTONIC, &start);
    test_result = test_run(test);
    clock_gettime(CLOCK_MONOTONIC, &end);

    test_result->elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0
                            + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    tests_results->total_elapsed_ms += test_result->elapsed_ms;

    add_test_result_to_test_suite_result(tests_results, test_result, current_index);

    return test_result;
}

void show_error_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int error_number)
{
    struct program_runner_test_result * prt;
    if (test_result->kind != TEST_RESULT_KIND_PROGRAM_RUNNER) {
        jcunit_fatal_error("An unknown test result!");
    }
    prt = (struct program_runner_test_result *)test_result;
    fprintf(
        output,
        "%u) %s : %s\n",
        error_number,
        test_result->test->test_suite->name,
        test_result->name->value
    );
    print_error(test_result, output);
    print_stderr_output(prt, output);
}

void show_failure_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int failure_number)
{
    struct program_runner_test_result * prt;
    if (test_result->kind != TEST_RESULT_KIND_PROGRAM_RUNNER) {
        jcunit_fatal_error("An unknown test result!");
    }
    prt = (struct program_runner_test_result *)test_result;
    fprintf(
        output,
        "%u) %s : %s\n",
        failure_number,
        test_result->test->test_suite->name,
        test_result->name->value
    );
    print_diff(output, test_result->expected, test_result->actual);
    print_exit_code(prt, output);
    print_stderr_output(prt, output);
}

void print_exit_code(struct program_runner_test_result * test_result, FILE * output)
{
    if (test_result->exit_code != 0) {
        fprintf(output, "[exit code] %d\n", test_result->exit_code);
    }
}

void print_stderr_output(struct program_runner_test_result * test_result, FILE * output)
{
    const char * start;
    const char * end;
    const char * p;
    if (test_result->stderr_output == NULL || test_result->stderr_output->len == 0) {
        return;
    }
    start = test_result->stderr_output->value;
    end = start + test_result->stderr_output->len;
    for (p = start; p < end; ) {
        const char * nl = p;
        while (nl < end && *nl != '\n') {
            ++nl;
        }
        fprintf(output, "[stderr] %.*s\n", (int)(nl - p), p);
        p = (nl < end) ? nl + 1 : nl;
    }
}
