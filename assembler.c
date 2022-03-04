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
static test_requirement_assembler_func * resolve_test_requirement_assembler(struct ast_requirement * requirement);

static void unknown_test_requirement_assembler(struct test_requirement_assembler_context * context);
static void given_test_requirement_assembler(struct test_requirement_assembler_context * context);
static void when_run_requirement_assembler(struct test_requirement_assembler_context * context);
static void expect_output_requirement_assembler(struct test_requirement_assembler_context * context);
static void should_be_skipped_requirement_assembler(struct test_requirement_assembler_context * context);

static void test_arguments_assembler(struct abstract_test * test, struct ast_test * ast_test);

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
        fprintf(stderr, "Cannot find any assembler to assemble the test!\n");
        exit(1);
    }
    struct abstract_test * test = assembler(ast_test);

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

test_requirement_assembler_func * resolve_test_requirement_assembler(struct ast_requirement * requirement)
{
    const char * requirement_name = requirement->name->value;
    if (strcmp(GIVEN_REQUIREMENT_NAME, requirement_name) == 0) {
        return given_test_requirement_assembler;
    }
    if (strcmp(WHEN_RUN_REQUIREMENT_NAME, requirement_name) == 0) {
        return when_run_requirement_assembler;
    }
    if (strcmp(EXPECT_OUTPUT_REQUIREMENT_NAME, requirement_name) == 0) {
        return expect_output_requirement_assembler;
    }
    if (strcmp(SHOULD_BE_SKIPPED_REQUIREMENT_NAME, requirement_name) == 0) {
        return should_be_skipped_requirement_assembler;
    }
    return unknown_test_requirement_assembler;
}

struct abstract_test * program_runner_test_assembler(struct ast_test * ast_test)
{
    struct test_requirement_assembler_context context;
    struct program_runner_test * test = make_program_runner_test();

    test_arguments_assembler((struct abstract_test *) test, ast_test);
    slist_foreach(iterator, &ast_test->requirements, {
        struct ast_requirement * requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        test_requirement_assembler_func * test_requirement_assembler = resolve_test_requirement_assembler(requirement);
        context.test = test;
        context.ast_test = ast_test;
        context.requirement = requirement;
        test_requirement_assembler(&context);
    });
    return (struct abstract_test *) test;
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
    if (string_equals_with_cstring(name, TEST_PROGRAM_RUNNER_STREAM_STDOUT_NAME)) {
        return TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT;
    }
    if (string_equals_with_cstring(name, TEST_PROGRAM_RUNNER_STREAM_STDERR_NAME)) {
        return TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR;
    }
    return TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_UNKNOWN;
}

void unknown_test_requirement_assembler(struct test_requirement_assembler_context * context)
{
    fprintf(
        stderr,
        "An unknown '%s' requirement for the test '%s'!\n",
        context->requirement->name->value,
        context->test->base.name->value
    );
    exit(1);
}

void given_test_requirement_assembler(struct test_requirement_assembler_context * context)
{
    struct string * given_type = NULL, * given_filename = NULL;
    slist_foreach(iterator, &context->requirement->arguments, {
        struct ast_requirement_argument * argument = list_get_owner(
            iterator,
            struct ast_requirement_argument,
            list_entry
        );
        if (argument->name == NULL) {
            given_type = argument->value;
            continue;
        }
        if (strcmp("type", argument->name->value) == 0) {
            if (given_type != NULL) {
                fprintf(
                    stderr,
                    "Cannot be both definition of 'type' named and unnamed or redefinition of 'type' for the 'given' requirement!\n"
                );
                exit(1);
            }
            given_type = argument->value;
            continue;
        }
        if (strcmp("filename", argument->name->value) == 0) {
            if (given_filename != NULL) {
                fprintf(
                    stderr,
                    "Cannot be both redefinition of 'filename' for the 'given' requirement!\n"
                );
                exit(1);
            }
            given_filename = argument->value;
            continue;
        }
        fprintf(stderr, "Unknown named argument '%s' for the 'given' requirement!\n", argument->name->value);
        exit(1);
    });
    if (given_type == NULL) {
        fprintf(stderr, "Missing the 'type' named argument (can be unnamed) for the 'given' requirement!\n");
        exit(1);
    }
    if (given_type->len == 0) {
        fprintf(stderr, "The type cannot be empty for the 'given' requirement!\n");
        exit(1);
    }
    if (!string_equals_with_cstring(given_type, "file")) {
        fprintf(
            stderr,
            "The unknown \"%s\" given type!\n",
            given_type->value
        );
        exit(1);
    }
    if (given_filename != NULL) {
        if (given_filename->len == 0) {
            fprintf(stderr, "The filename cannot be empty for the 'given' requirement!\n");
            exit(1);
        }
        if (strchr(given_filename->value, '/') != NULL) {
            fprintf(stderr, "Cannot be any path separators in 'filename' argument of the 'given' requirement!\n");
            exit(1);
        }
    }
    context->test->given_file_content = context->requirement->content;
    context->test->given_filename = given_filename;
    context->test->base.flags |= TEST_FLAG_HAS_GIVEN;
}

void when_run_requirement_assembler(struct test_requirement_assembler_context * context)
{
    if (context->requirement->content != NULL) {
        fprintf(stderr, "The 'whenRun' requirement must not contain any content!\n");
        exit(1);
    }
    struct string * program = NULL, * args = NULL;
    slist_foreach(argument_iterator, &context->requirement->arguments, {
        struct ast_requirement_argument * argument = list_get_owner(
            argument_iterator,
            struct ast_requirement_argument,
            list_entry
        );
        if (argument->name == NULL) {
            program = argument->value;
            continue;
        }
        if (strcmp("program", argument->name->value) == 0) {
            if (program != NULL) {
                fprintf(
                    stderr,
                    "Cannot be both definition of 'program' named and unnamed for the 'whenRun' requirement!\n"
                );
                exit(1);
            }
            program = argument->value;
            continue;
        }
        if (argument->name != NULL && strcmp("args", argument->name->value) == 0) {
            args = argument->value;
            continue;
        }
        fprintf(stderr, "Unknown named argument '%s' for the 'whenRun' requirement!\n", argument->name->value);
        exit(1);
    });
    if (program == NULL) {
        fprintf(stderr, "Missing the 'program' named argument (can be unnamed) for the 'whenRun' requirement!\n");
        exit(1);
    }
    if (program->len == 0) {
        fprintf(stderr, "The program cannot be empty for the 'whenRun' requirement!\n");
        exit(1);
    }
    if (args != NULL && args->len == 0) {
        fprintf(stderr, "The args cannot be empty for the 'whenRun' requirement!\n");
        exit(1);
    }
    context->test->program_path = program;
    context->test->program_args = args;
    context->test->base.flags |= TEST_FLAG_HAS_WHEN;
}

void expect_output_requirement_assembler(struct test_requirement_assembler_context * context)
{
    unsigned int arguments_count = slist_count(&context->requirement->arguments);
    if (arguments_count == 0) {
        fprintf(stderr, "No arguments are given for 'expectOutput' directive!\n");
        exit(1);
    }
    struct ast_requirement_argument * argument = list_get_owner(
        context->requirement->arguments.next,
        struct ast_requirement_argument,
        list_entry
    );
    if (argument->name != NULL && strcmp("stream", argument->name->value) != 0) {
        fprintf(stderr, "Unexpected named argument '%s' for the 'expectOutput' requirement!\n", argument->name->value);
        exit(1);
    }
    if (argument->value->len == 0) {
        fprintf(stderr, "The stream name cannot be empty for the 'expectOutput' requirement!\n");
        exit(1);
    }
    int stream_code = resolve_stream_code_by_name(argument->value);
    if (stream_code == TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_UNKNOWN) {
        fprintf(
            stderr,
            "The unknown stream name \"%s\" for 'expectOutput' requirement!\n",
            argument->value->value
        );
        exit(1);
    }
    context->test->stream_code = stream_code;
    context->test->expected_output = context->requirement->content;
    context->test->base.flags |= TEST_FLAG_HAS_THEN;
}

void should_be_skipped_requirement_assembler(struct test_requirement_assembler_context * context)
{
    context->test->base.flags |= TEST_FLAG_SKIPPED;
}

void test_arguments_assembler(struct abstract_test * test, struct ast_test * ast_test)
{
    struct string * name = NULL;
    slist_foreach(iterator, &ast_test->arguments, {
        struct ast_requirement_argument * argument = list_get_owner(
            iterator,
            struct ast_requirement_argument,
            list_entry
        );
        if (argument->name == NULL) {
            name = argument->value;
            continue;
        }
        if (strcmp("name", argument->name->value) == 0) {
            if (name != NULL) {
                fprintf(stderr, "Cannot be both definition of 'name' named and unnamed for the 'test' directive!\n");
                exit(1);
            }
            name = argument->value;
            continue;
        }
        fprintf(stderr, "Unknown named argument '%s' for the 'test' directive!\n", argument->name->value);
        exit(1);
    });
    if (name == NULL) {
        fprintf(stderr, "Missing the 'name' named argument (can be unnamed) for the 'test' directive!\n");
        exit(1);
    }
    if (name->len == 0) {
        fprintf(stderr, "The 'name' cannot be empty for the 'test' directive!\n");
        exit(1);
    }
    test->name = name;
}
