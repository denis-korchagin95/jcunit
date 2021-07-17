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
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#include "headers/assembler.h"
#include "headers/util.h"
#include "headers/finder.h"
#include "headers/allocate.h"


static struct test * make_test(struct string * name);
static struct program_runner_test_case * make_program_runner_test_case(void);

static struct abstract_test_case * assemble_ast_test_case(struct ast_test_case * ast_test_case);
static test_case_assembler_func * resolve_test_case_assembler(struct ast_test_case * ast_test_case);
static int resolve_stream_code_by_name(struct string * name);
static struct abstract_test_case * program_runner_test_case_assembler(struct ast_test_case * ast_test_case);


struct test * assemble_test(const char * filename, struct slist * ast_test_cases)
{
    const char * test_name = basename(filename);
    if (test_name == NULL) {
        test_name = "<unnamed>";
    }

    struct abstract_test_case * test_case;
    struct ast_test_case * ast_test_case;

    struct test * test = make_test(make_string(test_name, strlen(test_name)));

    slist_foreach(iterator, ast_test_cases, {
        ast_test_case = list_get_owner(iterator, struct ast_test_case, list_entry);
        test_case = assemble_ast_test_case(ast_test_case);
        test_case->test = test;
        slist_append(test->cases_end, &test_case->list_entry);
        ++test->case_count;
    });

    return test;
}

struct abstract_test_case * assemble_ast_test_case(struct ast_test_case * ast_test_case)
{
    assert(ast_test_case != NULL);

    test_case_assembler_func * assembler = resolve_test_case_assembler(ast_test_case);
    if (assembler == NULL) {
        fprintf(stderr, "Cannot find any assembler to assemble the test case \"%s\"!", ast_test_case->name->value);
        exit(1);
    }
    struct abstract_test_case * test_case = assembler(ast_test_case);
    test_case->name = ast_test_case->name;

    if ((test_case->flags & TEST_CASE_COMPLETE_MASK) != TEST_CASE_COMPLETE_MASK) {
        test_case->flags |= TEST_CASE_FLAG_INCOMPLETE;
    }

    return test_case;
}

struct test * make_test(struct string * name)
{
    struct test * test = alloc_test();
    memset((void *)test, 0, sizeof(struct test));
    slist_init(&test->cases, test->cases_end);
    test->name = name;
    return test;
}

test_case_assembler_func * resolve_test_case_assembler(struct ast_test_case * ast_test_case)
{
    assert(ast_test_case != NULL);

    struct ast_requirement * requirement = find_ast_requirement_by_name(ast_test_case, WHEN_RUN_REQUIREMENT_NAME);
    if (requirement != NULL) {
        return program_runner_test_case_assembler;
    }
    return NULL;
}

struct abstract_test_case * program_runner_test_case_assembler(struct ast_test_case * ast_test_case)
{
    struct ast_requirement * ast_requirement;
    struct program_runner_test_case * test_case = make_program_runner_test_case();
    ast_requirement = find_ast_requirement_by_name(ast_test_case, GIVEN_REQUIREMENT_NAME);
    if (ast_requirement != NULL) {
        if (!string_equals_with_cstring(ast_requirement->argument, "file")) {
            fprintf(
                stderr,
                "The unknown \"%s\" given type!\n",
                ast_requirement->argument->value
            );
            exit(1);
        }
        test_case->given_file_content = ast_requirement->content;
        test_case->base.flags |= TEST_CASE_FLAG_HAS_GIVEN;
    }
    ast_requirement = find_ast_requirement_by_name(ast_test_case, WHEN_RUN_REQUIREMENT_NAME);
    if (ast_requirement != NULL) {
        if (ast_requirement->content != NULL) {
            fprintf(stderr, "The 'whenRun' requirement must not contain any content!\n");
            exit(1);
        }
        test_case->program_path = ast_requirement->argument;
        test_case->base.flags |= TEST_CASE_FLAG_HAS_WHEN;
    }
    ast_requirement = find_ast_requirement_by_name(ast_test_case, EXPECT_OUTPUT_REQUIREMENT_NAME);
    if (ast_requirement != NULL) {
        int stream_code = resolve_stream_code_by_name(ast_requirement->argument);
        if (stream_code == TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_NONE) {
            fprintf(
                stderr,
                "The unknown stream name \"%s\" for 'expectOutput' requirement!\n",
                ast_requirement->argument->value
            );
            exit(1);
        }
        test_case->stream_code = stream_code;
        test_case->expected_output = ast_requirement->content;
        test_case->base.flags |= TEST_CASE_FLAG_HAS_THEN;
    }
    return (struct abstract_test_case *)test_case;
}

struct program_runner_test_case * make_program_runner_test_case(void)
{
    struct program_runner_test_case * test_case = alloc_program_runner_test_case();
    memset((void *)test_case, 0, sizeof(struct program_runner_test_case));
    test_case->base.kind = TEST_CASE_KIND_PROGRAM_RUNNER;
    return test_case;
}

int resolve_stream_code_by_name(struct string * name)
{
    int stream_code = TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_NONE;
    if (string_equals_with_cstring(name, TEST_CASE_PROGRAM_RUNNER_STREAM_STDOUT_NAME)) {
        stream_code = TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT;
    } else if (string_equals_with_cstring(name, TEST_CASE_PROGRAM_RUNNER_STREAM_STDERR_NAME)) {
        stream_code = TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR;
    }
    return stream_code;
}
