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
#ifndef JCUNIT_RUNNER_H
#define JCUNIT_RUNNER_H 1

#include "assembler.h"

#define TEST_CASE_RESULT_STATUS_NONE (0)
#define TEST_CASE_RESULT_STATUS_PASS (1)
#define TEST_CASE_RESULT_STATUS_FAIL (2)

#define MAX_TEST_CASE_GIVEN_FILENAME_LEN (50)

struct test_result
{
    struct list test_case_results;
    struct test * test;
    unsigned int passed_count;
    unsigned int failed_count;
};

struct test_case_result
{
    struct list list_entry;
    struct string * test_case_name;
    struct string * executable;
    struct string * expected;
    struct string * actual;
    unsigned int status;
    unsigned int error_code;
    char given_filename[MAX_TEST_CASE_GIVEN_FILENAME_LEN];
};

struct test_case_result * test_case_run(struct test_case * test_case);

struct test_result * make_test_result(struct test * test);
void test_result_add_test_case_result(struct test_result * test_result, struct test_case_result * test_case_result);

#endif /* JCUNIT_RUNNER_H */
