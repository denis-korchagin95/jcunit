/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021 Denis Korchagin <denis.korchagin.1995@gmail.com>
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


static const char * pass_prefix = "\033[42mPASS\033[0m      ";
static const char * fail_prefix = "\033[41mFAIL\033[0m      ";
static const char * incomplete_prefix = "\033[43mINCOMPLETE\033[0m";

static const char * resolve_status_prefix(struct abstract_test_case_result * test_case_result);
static bool has_test_case_result_error(struct abstract_test_case_result * test_case_result);
static void print_error(struct abstract_test_case_result * test_case_result, FILE * output);
static void print_program_runner_error(struct program_runner_test_case_result * test_case_result, FILE * output);

void show_test_case_result(struct abstract_test_case_result * test_case_result, FILE * output)
{
    assert(test_case_result != NULL);

    const char * prefix = resolve_status_prefix(test_case_result);

    if (prefix == NULL) {
        fprintf(stderr, "Can't resolve the status of test case result!\n");
        exit(1);
    }

    fprintf(output, "\t%s %s\n", prefix, test_case_result->name->value);

    if (test_case_result->kind == TEST_CASE_RESULT_STATUS_INCOMPLETE) {
        return;
    }

    bool has_error = has_test_case_result_error(test_case_result);

    if (has_error) {
        print_error(test_case_result, output);
    }

    if (!has_error &&test_case_result->status == TEST_CASE_RESULT_STATUS_FAIL) {
        fprintf(output, "--- Expected\n%s$\n", test_case_result->expected->value);
        fprintf(output, "+++ Actual\n%s$\n", test_case_result->actual->value);
    }
}

static const char * resolve_status_prefix(struct abstract_test_case_result * test_case_result)
{
    switch (test_case_result->status) {
        case TEST_CASE_RESULT_STATUS_PASS: return pass_prefix;
        case TEST_CASE_RESULT_STATUS_FAIL: return fail_prefix;
        case TEST_CASE_RESULT_STATUS_INCOMPLETE: return incomplete_prefix;
    }
    return NULL;
}

bool has_test_case_result_error(struct abstract_test_case_result * test_case_result)
{
    if (test_case_result->kind == TEST_CASE_RESULT_KIND_PROGRAM_RUNNER) {
        return ((struct program_runner_test_case_result *)test_case_result)->error_code != ERROR_CODE_NONE;
    }
    return false;
}

void print_error(struct abstract_test_case_result * test_case_result, FILE * output)
{
    switch (test_case_result->kind) {
        case TEST_CASE_RESULT_KIND_PROGRAM_RUNNER:
            print_program_runner_error((struct program_runner_test_case_result *)test_case_result, output);
            break;
    }
}

void print_program_runner_error(struct program_runner_test_case_result * test_case_result, FILE * output)
{
    switch (test_case_result->error_code) {
        case ERROR_CODE_NONE:
            break;
        case ERROR_CODE_FILE_NOT_FOUND:
            fprintf(
                    output,
                    "\033[0;31mThe file \"%s\" was not found!\033[0m\n",
                    test_case_result->executable->value
            );
            break;
        case ERROR_CODE_NOT_EXECUTABLE:
            fprintf(
                    output,
                    "\033[0;31mThe file \"%s\" is not executable!\033[0m\n",
                    test_case_result->executable->value
            );
            break;
        case ERROR_CODE_READ_CHILD_DATA:
            fprintf(output, "\033[0;31mCan't read the program data!\033[0m\n");
            break;
    }
}

void show_each_test_case_result(
    FILE * output,
    struct list_iterator * iterator,
    list_iterator_visiter_func * visiter_func,
    void * context
)
{
    if (list_iterator_finished(iterator)) {
        fprintf(output, "\tThere is no any test cases!\n");
        return;
    }
    struct test_result * test_result = (struct test_result *)context;
    fprintf(output, "Test: %s\n", test_result->test->name->value);
    struct abstract_test_case_result * test_case_result;
    for(;;) {
        test_case_result = list_iterator_visit(iterator, visiter_func, context);
        if (test_case_result == NULL) {
            break;
        }
        show_test_case_result(test_case_result, stdout);
    }
    fprintf(
        output,
        "\nPassed: %u, Failed: %u, Incomplete: %u\n",
        test_result->passed_count,
        test_result->failed_count,
        test_result->incomplete_count
    );
}

void * test_case_runner_visiter(void * object, void * context)
{
    struct abstract_test_case * test_case;
    struct abstract_test_case_result * test_case_result;
    struct test_result * test_result;

    test_result = (struct test_result *)context;
    test_case = list_get_owner((struct list *)object, struct abstract_test_case, list_entry);
    test_case_result = test_case_run(test_case);

    test_result_add_test_case_result(test_result, test_case_result);

    return test_case_result;
}

