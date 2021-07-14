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


#include "headers/show_result.h"
#include "headers/child_process.h"


static const char * pass_prefix = "\033[42mPASS\033[0m";
static const char * fail_prefix = "\033[41mFAIL\033[0m";

static const char * resolve_status_prefix(struct test_case_result * test_case_result);

void show_test_case_result(struct test_case_result * test_case_result, FILE * output)
{
    const char * prefix = resolve_status_prefix(test_case_result);

    if (prefix == NULL) {
        fprintf(stderr, "Can't resolve the status of test case result!\n");
        exit(1);
    }

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
}

static const char * resolve_status_prefix(struct test_case_result * test_case_result)
{
    switch (test_case_result->status) {
        case TEST_CASE_RESULT_STATUS_PASS: return pass_prefix;
        case TEST_CASE_RESULT_STATUS_FAIL: return fail_prefix;
    }
    return NULL;
}
