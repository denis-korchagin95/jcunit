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
#include <memory.h>
#include <assert.h>

#include "headers/compiler.h"
#include "headers/token.h"
#include "headers/parse.h"
#include "headers/allocate.h"
#include "headers/util.h"
#include "headers/errors.h"


#define WHEN_RUN_REQUIREMENT_NAME           "whenRun"
#define GIVEN_REQUIREMENT_NAME              "given"
#define EXPECT_OUTPUT_REQUIREMENT_NAME      "expectOutput"
#define SHOULD_BE_SKIPPED_REQUIREMENT_NAME  "shouldBeSkipped"

#define TEST_PROGRAM_RUNNER_STREAM_STDOUT_NAME  "stdout"
#define TEST_PROGRAM_RUNNER_STREAM_STDERR_NAME  "stderr"


struct test_requirement_compiler_context
{
    struct ast_requirement * requirement;
    struct program_runner_test * test;
    struct ast_test * ast_test;
};

typedef struct abstract_test * test_compiler_func(struct ast_test * ast_test);
typedef void test_requirement_compiler_func(struct test_requirement_compiler_context * context);

static struct test_suite * do_compile_test_suite(const char * filename, struct slist * ast_tests);
static struct abstract_test * compile_test(struct ast_test * ast_test);
static struct abstract_test * program_runner_test_compiler(struct ast_test * ast_test);
static void test_arguments_compiler(struct abstract_test * test, struct ast_test * ast_test);


static struct ast_requirement * find_ast_requirement_by_name(
    struct ast_test * ast_test,
    const char * requirement_name
);
static test_compiler_func * resolve_test_compiler_func(struct ast_test * ast_test);
static test_requirement_compiler_func * resolve_test_requirement_compiler(struct ast_requirement * requirement);
static int resolve_stream_code_by_name(struct string * name);

static struct test_suite * make_test_suite(struct string * name, unsigned int tests_count);
static struct program_runner_test * make_program_runner_test(void);


static void given_test_requirement_compiler(struct test_requirement_compiler_context * context);
static void when_run_requirement_compiler(struct test_requirement_compiler_context * context);
static void expect_output_requirement_compiler(struct test_requirement_compiler_context * context);
static void should_be_skipped_requirement_compiler(struct test_requirement_compiler_context * context);
static void unknown_test_requirement_compiler(struct test_requirement_compiler_context * context);


struct test_suite * compile_test_suite(const char * filename)
{
    struct tokenizer_context * context = make_tokenizer_context(filename);

    struct slist * ast_tests = parse_test_suite(context);

    destroy_tokenizer_context(context);

    struct test_suite * test_suite = do_compile_test_suite(filename, ast_tests);

    release_ast_tests(ast_tests);
    free_slist(ast_tests);

    return test_suite;
}

struct test_suite * do_compile_test_suite(const char * filename, struct slist * ast_tests)
{
    const char * test_name = basename(filename);
    if (test_name == NULL) {
        test_name = "<unnamed>";
    }

    struct abstract_test * test;
    struct ast_test * ast_test;

    struct test_suite * test_suite = make_test_suite(
        make_string(test_name, strlen(test_name)),
        slist_count(ast_tests)
    );

    unsigned int i = 0;

    slist_foreach(iterator, ast_tests, {
        ast_test = list_get_owner(iterator, struct ast_test, list_entry);
        test = compile_test(ast_test);
        test->test_suite = test_suite;
        test_suite->tests[i++] = test;
    });

    return test_suite;
}

struct test_suite * make_test_suite(struct string * name, unsigned int tests_count)
{
    struct test_suite * test_suite = alloc_test_suite();
    memset((void *)test_suite, 0, sizeof(struct test_suite));
    test_suite->name = name;
    test_suite->tests_count = tests_count;
    test_suite->tests = (struct abstract_test **) alloc_bytes(tests_count * sizeof(void *));
    return test_suite;
}

struct abstract_test * compile_test(struct ast_test * ast_test)
{
    assert(ast_test != NULL);

    test_compiler_func * test_compiler = resolve_test_compiler_func(ast_test);
    if (test_compiler == NULL) {
        jcunit_fatal_error("Cannot find any compiler to compile the test!");
    }
    struct abstract_test * test = test_compiler(ast_test);

    if ((test->flags & TEST_COMPLETE_MASK) != TEST_COMPLETE_MASK) {
        test->flags |= TEST_FLAG_INCOMPLETE;
    }

    return test;
}

test_compiler_func * resolve_test_compiler_func(struct ast_test * ast_test)
{
    assert(ast_test != NULL);

    struct ast_requirement * requirement = find_ast_requirement_by_name(ast_test, WHEN_RUN_REQUIREMENT_NAME);
    if (requirement != NULL) {
        return program_runner_test_compiler;
    }
    return NULL;
}

struct ast_requirement * find_ast_requirement_by_name(
    struct ast_test * ast_test,
    const char * requirement_name
) {
    assert(ast_test != NULL);
    size_t len = strlen(requirement_name);
    struct ast_requirement * requirement;
    slist_foreach(iterator, &ast_test->requirements, {
        requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        if (requirement->name->len == len && strncmp(requirement->name->value, requirement_name, len) == 0) {
            return requirement;
        }
    });
    return NULL;
}

struct abstract_test * program_runner_test_compiler(struct ast_test * ast_test)
{
    struct test_requirement_compiler_context context;
    struct program_runner_test * test = make_program_runner_test();

    test_arguments_compiler((struct abstract_test *) test, ast_test);
    slist_foreach(iterator, &ast_test->requirements, {
        struct ast_requirement * requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        test_requirement_compiler_func * test_requirement_compiler = resolve_test_requirement_compiler(requirement);
        context.test = test;
        context.ast_test = ast_test;
        context.requirement = requirement;
        test_requirement_compiler(&context);
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

test_requirement_compiler_func * resolve_test_requirement_compiler(struct ast_requirement * requirement)
{
    const char * requirement_name = requirement->name->value;
    if (strcmp(GIVEN_REQUIREMENT_NAME, requirement_name) == 0) {
        return given_test_requirement_compiler;
    }
    if (strcmp(WHEN_RUN_REQUIREMENT_NAME, requirement_name) == 0) {
        return when_run_requirement_compiler;
    }
    if (strcmp(EXPECT_OUTPUT_REQUIREMENT_NAME, requirement_name) == 0) {
        return expect_output_requirement_compiler;
    }
    if (strcmp(SHOULD_BE_SKIPPED_REQUIREMENT_NAME, requirement_name) == 0) {
        return should_be_skipped_requirement_compiler;
    }
    return unknown_test_requirement_compiler;
}

void given_test_requirement_compiler(struct test_requirement_compiler_context * context)
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
                jcunit_fatal_error("Cannot be both definition of 'type' named and unnamed or redefinition of 'type' for the 'given' requirement!");
            }
            given_type = argument->value;
            continue;
        }
        if (strcmp("filename", argument->name->value) == 0) {
            if (given_filename != NULL) {
                jcunit_fatal_error("Cannot be both redefinition of 'filename' for the 'given' requirement!");
            }
            given_filename = argument->value;
            continue;
        }
        jcunit_fatal_error("Unknown named argument '%s' for the 'given' requirement!", argument->name->value);
    });
    if (given_type == NULL) {
        jcunit_fatal_error("Missing the 'type' named argument (can be unnamed) for the 'given' requirement!");
    }
    if (given_type->len == 0) {
        jcunit_fatal_error("The type cannot be empty for the 'given' requirement!");
    }
    if (!string_equals_with_cstring(given_type, "file")) {
        jcunit_fatal_error("The unknown \"%s\" given type!", given_type->value);
    }
    if (given_filename != NULL) {
        if (given_filename->len == 0) {
            jcunit_fatal_error("The filename cannot be empty for the 'given' requirement!");
        }
        if (strchr(given_filename->value, '/') != NULL) {
            jcunit_fatal_error("Cannot be any path separators in 'filename' argument of the 'given' requirement!");
        }
    }
    context->test->given_file_content = context->requirement->content;
    context->test->given_filename = given_filename;
    context->test->base.flags |= TEST_FLAG_HAS_GIVEN;
}

void when_run_requirement_compiler(struct test_requirement_compiler_context * context)
{
    if (context->requirement->content != NULL) {
        jcunit_fatal_error("The 'whenRun' requirement must not contain any content!");
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
                jcunit_fatal_error("Cannot be both definition of 'program' named and unnamed for the 'whenRun' requirement!");
            }
            program = argument->value;
            continue;
        }
        if (argument->name != NULL && strcmp("args", argument->name->value) == 0) {
            args = argument->value;
            continue;
        }
        jcunit_fatal_error("Unknown named argument '%s' for the 'whenRun' requirement!", argument->name->value);
    });
    if (program == NULL) {
        jcunit_fatal_error("Missing the 'program' named argument (can be unnamed) for the 'whenRun' requirement!");
    }
    if (program->len == 0) {
        jcunit_fatal_error("The program cannot be empty for the 'whenRun' requirement!");
    }
    if (args != NULL && args->len == 0) {
        jcunit_fatal_error("The args cannot be empty for the 'whenRun' requirement!");
    }
    context->test->program_path = program;
    context->test->program_args = args;
    context->test->base.flags |= TEST_FLAG_HAS_WHEN;
}

void expect_output_requirement_compiler(struct test_requirement_compiler_context * context)
{
    unsigned int arguments_count = slist_count(&context->requirement->arguments);
    if (arguments_count == 0) {
        jcunit_fatal_error("No arguments are given for 'expectOutput' directive!");
    }
    struct ast_requirement_argument * argument = list_get_owner(
            context->requirement->arguments.next,
            struct ast_requirement_argument,
            list_entry
    );
    if (argument->name != NULL && strcmp("stream", argument->name->value) != 0) {
        jcunit_fatal_error("Unexpected named argument '%s' for the 'expectOutput' requirement!", argument->name->value);
    }
    if (argument->value->len == 0) {
        jcunit_fatal_error("The stream name cannot be empty for the 'expectOutput' requirement!");
    }
    int stream_code = resolve_stream_code_by_name(argument->value);
    if (stream_code == TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_UNKNOWN) {
        jcunit_fatal_error(
            "The unknown stream name \"%s\" for 'expectOutput' requirement!",
            argument->value->value
        );
    }
    context->test->stream_code = stream_code;
    context->test->expected_output = context->requirement->content;
    context->test->base.flags |= TEST_FLAG_HAS_THEN;
}

void should_be_skipped_requirement_compiler(struct test_requirement_compiler_context * context)
{
    context->test->base.flags |= TEST_FLAG_SKIPPED;
}

void unknown_test_requirement_compiler(struct test_requirement_compiler_context * context)
{
    jcunit_fatal_error(
        "An unknown '%s' requirement for the test '%s'!",
        context->requirement->name->value,
        context->test->base.name->value
    );
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

void test_arguments_compiler(struct abstract_test * test, struct ast_test * ast_test)
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
                jcunit_fatal_error("Cannot be both definition of 'name' named and unnamed for the 'test' directive!");
            }
            name = argument->value;
            continue;
        }
        jcunit_fatal_error("Unknown named argument '%s' for the 'test' directive!", argument->name->value);
    });
    if (name == NULL) {
        jcunit_fatal_error("Missing the 'name' named argument (can be unnamed) for the 'test' directive!");
    }
    if (name->len == 0) {
        jcunit_fatal_error("The 'name' cannot be empty for the 'test' directive!");
    }
    test->name = name;
}
