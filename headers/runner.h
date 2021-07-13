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
    struct string * test_case_name;
    struct string * executable;
    struct string * expected;
    struct string * actual;
    unsigned int status;
    unsigned int error_code;
};

void test_run(struct test_runner_context * context);

struct test_runner_context * make_test_runner_context(struct test * test);

#endif /* JCUNIT_RUNNER_H */
