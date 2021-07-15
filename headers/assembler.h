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
#ifndef JCUNIT_ASSEMBLER_H
#define JCUNIT_ASSEMBLER_H 1

#include "list.h"
#include "ast.h"

#define TEST_CASE_FLAG_HAS_GIVEN    (1)
#define TEST_CASE_FLAG_HAS_WHEN     (2)
#define TEST_CASE_FLAG_HAS_THEN     (4)
#define TEST_CASE_FLAG_INCOMPLETE   (8)

#define WHEN_RUN_REQUIREMENT_NAME       "whenRun"
#define GIVEN_REQUIREMENT_NAME          "given"
#define EXPECT_OUTPUT_REQUIREMENT_NAME  "expectOutput"

#define TEST_CASE_KIND_PROGRAM_RUNNER (1)

#define TEST_CASE_COMPLETE_MASK (TEST_CASE_FLAG_HAS_GIVEN | TEST_CASE_FLAG_HAS_WHEN | TEST_CASE_FLAG_HAS_THEN)

#define TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_NONE      (0)
#define TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT    (1)
#define TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR    (2)

#define TEST_CASE_PROGRAM_RUNNER_STREAM_STDOUT_NAME "stdout"
#define TEST_CASE_PROGRAM_RUNNER_STREAM_STDERR_NAME "stderr"

struct test {
    struct list cases;
    struct string * name;
    unsigned int case_count;
};

struct abstract_test_case {
    struct list list_entry;
    struct string * name;
    struct test * test;
    unsigned int kind;
    unsigned int flags;
};

struct program_runner_test_case {
    struct abstract_test_case base;
    struct string * given_file_content;
    struct string * program_path;
    struct string * expected_output;
    unsigned int stream_code;
};

struct test * assemble_test(const char * filename, struct list * ast_test_cases);

typedef struct abstract_test_case * test_case_assembler_func(struct ast_test_case * ast_test_case);

#endif /* JCUNIT_ASSEMBLER_H */
