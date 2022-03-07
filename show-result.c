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
#include <stdlib.h>
#include <assert.h>


#include "headers/show-result.h"
#include "headers/child-process.h"
#include "headers/test-suite-iterator.h"
#include "headers/util.h"


static const char * stringify_test_status(struct abstract_test_result * test_result, bool use_short_version);
static void print_error(struct abstract_test_result * test_result, FILE * output);
static void print_program_runner_error(struct program_runner_test_result * test_result, FILE * output);

void show_test_result_in_detail_mode(struct abstract_test_result * test_result, FILE * output)
{
    assert(test_result != NULL);

    const char * test_status = stringify_test_status(test_result, false);

    if (test_status == NULL) {
        fprintf(stderr, "Can't resolve the status of test result!\n");
        exit(1);
    }

    fprintf(output, "    %10s %s\n", test_status, test_result->name->value);

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
        fprintf(output, "--- Expected\n%s$\n", test_result->expected == NULL ? "" : test_result->expected->value);
        fprintf(output, "+++ Actual\n%s$\n", test_result->actual->value);
        return;
    }

    fprintf(output, "The unknown test status!\n");
}

void show_test_result_in_passthrough_mode(struct abstract_test_result * test_result, FILE * output)
{
    assert(test_result != NULL);

    const char * test_status = stringify_test_status(test_result, true);

    if (test_status == NULL) {
        fprintf(stderr, "Can't resolve the status of test result!\n");
        exit(1);
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
    }
}

void show_each_test_result_in_detail_mode(
    FILE * output,
    struct test_suite_iterator * iterator,
    test_suite_iterator_visiter_func * visiter_func,
    struct test_suite_result * test_suite_result
) {
    if (test_suite_iterator_finished(iterator)) {
        return;
    }
    fprintf(output, "Test Suite: %s\n", test_suite_result->test_suite->name->value);
    struct abstract_test_result * test_result;
    for(;;) {
        test_result = test_suite_iterator_visit(iterator, visiter_func, test_suite_result);
        if (test_result == NULL) {
            break;
        }
        show_test_result_in_detail_mode(test_result, output);
    }
    fprintf(
        output,
        "\nPassed: %u, Skipped: %u, Errors: %u, Failed: %u, Incomplete: %u\n\n\n",
        test_suite_result->passed_count,
        test_suite_result->skipped_count,
        test_suite_result->error_count,
        test_suite_result->failure_count,
        test_suite_result->incomplete_count
    );
}

void show_each_test_result_in_passthrough_mode(
    FILE * output,
    struct test_suite_iterator * iterator,
    test_suite_iterator_visiter_func * visiter_func,
    struct test_suite_result * test_suite_result
) {
    if (test_suite_iterator_finished(iterator)) {
        return;
    }
    struct abstract_test_result * test_result;
    for(;;) {
        test_result = test_suite_iterator_visit(iterator, visiter_func, test_suite_result);
        if (test_result == NULL) {
            break;
        }
        show_test_result_in_passthrough_mode(test_result, output);
    }
}

struct abstract_test_result * test_runner(
    struct abstract_test * test,
    struct test_suite_result * test_suite_result,
    unsigned int current_index
) {
    struct abstract_test_result * test_result = test_run(test);

    add_test_result_to_test_suite_result(test_suite_result, test_result, current_index);

    return test_result;
}


void show_error_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int error_number)
{
    if (test_result->kind != TEST_RESULT_KIND_PROGRAM_RUNNER) {
        fprintf(stderr, "An unknown test result!\n");
        exit(1);
    }
    struct program_runner_test_result * result = (struct program_runner_test_result *) test_result;
    const char * test_suite_name = basename(result->given_filename->value);
    if (test_suite_name == NULL) {
        test_suite_name = "<unknown test suite name>";
    }
    fprintf(output, "%u) %s : %s\n", error_number, test_suite_name, test_result->name->value);
    print_error(test_result, output);
}

void show_failure_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int failure_number)
{
    if (test_result->kind != TEST_RESULT_KIND_PROGRAM_RUNNER) {
        fprintf(stderr, "An unknown test result!\n");
        exit(1);
    }
    struct program_runner_test_result * result = (struct program_runner_test_result *) test_result;
    const char * test_suite_name = basename(result->given_filename->value);
    if (test_suite_name == NULL) {
        test_suite_name = "<unknown test suite name>";
    }
    fprintf(output, "%u) %s : %s\n", failure_number, test_suite_name, test_result->name->value);
    fprintf(output, "--- Expected\n%s$\n", test_result->expected == NULL ? "" : test_result->expected->value);
    fprintf(output, "+++ Actual\n%s$\n", test_result->actual->value);
}
