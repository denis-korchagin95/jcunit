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


static struct test_suite * make_test_suite(struct string * name);
static struct program_runner_test * make_program_runner_test(void);

static struct abstract_test * assemble_ast_test(struct ast_test * ast_test);
static test_assembler_func * resolve_test_assembler(struct ast_test * ast_test);
static int resolve_stream_code_by_name(struct string * name);
static struct abstract_test * program_runner_test_assembler(struct ast_test * ast_test);


struct test_suite * assemble_test_suite(const char * filename, struct slist * ast_tests)
{
    const char * test_name = basename(filename);
    if (test_name == NULL) {
        test_name = "<unnamed>";
    }

    struct abstract_test * test;
    struct ast_test * ast_test;

    struct test_suite * test_suite = make_test_suite(make_string(test_name, strlen(test_name)));

    slist_foreach(iterator, ast_tests, {
        ast_test = list_get_owner(iterator, struct ast_test, list_entry);
        test = assemble_ast_test(ast_test);
        test->test_suite = test_suite;
        list_append(&test_suite->tests, &test->list_entry);
        ++test_suite->test_count;
    });

    return test_suite;
}

struct abstract_test * assemble_ast_test(struct ast_test * ast_test)
{
    assert(ast_test != NULL);

    test_assembler_func * assembler = resolve_test_assembler(ast_test);
    if (assembler == NULL) {
        fprintf(stderr, "Cannot find any assembler to assemble the test \"%s\"!", ast_test->name->value);
        exit(1);
    }
    struct abstract_test * test = assembler(ast_test);
    test->name = ast_test->name;

    if ((test->flags & TEST_COMPLETE_MASK) != TEST_COMPLETE_MASK) {
        test->flags |= TEST_FLAG_INCOMPLETE;
    }

    return test;
}

struct test_suite * make_test_suite(struct string * name)
{
    struct test_suite * test_suite = alloc_test_suite();
    memset((void *)test_suite, 0, sizeof(struct test_suite));
    list_init(&test_suite->tests);
    test_suite->name = name;
    return test_suite;
}

test_assembler_func * resolve_test_assembler(struct ast_test * ast_test)
{
    assert(ast_test != NULL);

    struct ast_requirement * requirement = find_ast_requirement_by_name(ast_test, WHEN_RUN_REQUIREMENT_NAME);
    if (requirement != NULL) {
        return program_runner_test_assembler;
    }
    return NULL;
}

struct abstract_test * program_runner_test_assembler(struct ast_test * ast_test)
{
    struct ast_requirement * ast_requirement;
    struct program_runner_test * test = make_program_runner_test();
    ast_requirement = find_ast_requirement_by_name(ast_test, GIVEN_REQUIREMENT_NAME);
    if (ast_requirement != NULL) {
        if (!string_equals_with_cstring(ast_requirement->argument, "file")) {
            fprintf(
                stderr,
                "The unknown \"%s\" given type!\n",
                ast_requirement->argument->value
            );
            exit(1);
        }
        test->given_file_content = ast_requirement->content;
        test->base.flags |= TEST_FLAG_HAS_GIVEN;
    }
    ast_requirement = find_ast_requirement_by_name(ast_test, WHEN_RUN_REQUIREMENT_NAME);
    if (ast_requirement != NULL) {
        if (ast_requirement->content != NULL) {
            fprintf(stderr, "The 'whenRun' requirement must not contain any content!\n");
            exit(1);
        }
        test->program_path = ast_requirement->argument;
        test->base.flags |= TEST_FLAG_HAS_WHEN;
    }
    ast_requirement = find_ast_requirement_by_name(ast_test, EXPECT_OUTPUT_REQUIREMENT_NAME);
    if (ast_requirement != NULL) {
        int stream_code = resolve_stream_code_by_name(ast_requirement->argument);
        if (stream_code == TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_NONE) {
            fprintf(
                stderr,
                "The unknown stream name \"%s\" for 'expectOutput' requirement!\n",
                ast_requirement->argument->value
            );
            exit(1);
        }
        test->stream_code = stream_code;
        test->expected_output = ast_requirement->content;
        test->base.flags |= TEST_FLAG_HAS_THEN;
    }
    return (struct abstract_test *)test;
}

struct program_runner_test * make_program_runner_test(void)
{
    struct program_runner_test * test = alloc_program_runner_test();
    memset((void *)test, 0, sizeof(struct program_runner_test));
    test->base.kind = TEST_KIND_PROGRAM_RUNNER;
    return test;
}

int resolve_stream_code_by_name(struct string * name)
{
    int stream_code = TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_NONE;
    if (string_equals_with_cstring(name, TEST_PROGRAM_RUNNER_STREAM_STDOUT_NAME)) {
        stream_code = TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT;
    } else if (string_equals_with_cstring(name, TEST_PROGRAM_RUNNER_STREAM_STDERR_NAME)) {
        stream_code = TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR;
    }
    return stream_code;
}
