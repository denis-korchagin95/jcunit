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
#ifndef JCUNIT_RUNNER_H
#define JCUNIT_RUNNER_H 1

#include "compiler.h"

#define TEST_RESULT_STATUS_PASS         (1)
#define TEST_RESULT_STATUS_FAILURE      (2)
#define TEST_RESULT_STATUS_INCOMPLETE   (3)
#define TEST_RESULT_STATUS_ERROR        (4)
#define TEST_RESULT_STATUS_SKIPPED      (5)

#define TEST_RESULT_KIND_PROGRAM_RUNNER    (1)

struct tests_results
{
    struct abstract_test_result ** results;
    unsigned int results_count;
    unsigned int passed_count;
    unsigned int skipped_count;
    unsigned int failure_count;
    unsigned int incomplete_count;
    unsigned int error_count;
};

struct abstract_test_result {
    struct abstract_test * test;
    struct string * name;
    struct string * expected;
    struct string * actual;
    unsigned int status;
    unsigned int kind;
};

struct program_runner_test_result {
    struct abstract_test_result base;  /* this must be a first member */
    struct string * given_filename;
    struct string * executable;
    unsigned int error_code;
};

struct abstract_test_result * test_run(struct abstract_test * test);

struct tests_results * make_tests_results(unsigned int total_tests_count);

void add_test_result_to_test_suite_result(
    struct tests_results * tests_results,
    struct abstract_test_result * test_result,
    unsigned int result_index
);

typedef struct abstract_test_result * test_runner_func(struct abstract_test * test);

#endif /* JCUNIT_RUNNER_H */
