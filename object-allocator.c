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
#include <assert.h>

#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"
#include "headers/errors.h"

/**
 * It's very simple and static allocator. Maybe this needs to be refactor to dynamic version in the future...
 */

struct allocator_stat {
    const char * name;
    unsigned int * total;
    unsigned int * pool_usage;
    unsigned int * freed;
    unsigned int * allocated;
};

allocator(token, struct token, 512)
allocator(string, struct string, 2048)
allocator(tokenizer_context, struct tokenizer_context, 4)
allocator(ast_test, struct ast_test, 256)
allocator(ast_requirement, struct ast_requirement, 512)
allocator(list, struct list, 512)
allocator(slist, struct slist, 512)
allocator(test_suite, struct test_suite, 32)
allocator(tests_results, struct tests_results, 256)
allocator(abstract_test_result, struct abstract_test_result, 256)
allocator(program_runner_test_result, struct program_runner_test_result, 256)
allocator(program_runner_test, struct program_runner_test, 256)
allocator(source, struct source, 256)
allocator(path_list, struct path_list, 256)
allocator(ast_requirement_argument, struct ast_requirement_argument, 512)

static struct allocator_stat stats[] = {
        { "token", &max_token_pool_size, &token_pool_pos, &token_freed, &token_allocated },
        { "string", &max_string_pool_size, &string_pool_pos, &string_freed, &string_allocated },
        {
            "tokenizer_context",
            &max_tokenizer_context_pool_size,
            &tokenizer_context_pool_pos,
            &tokenizer_context_freed,
            &tokenizer_context_allocated
        },
        {
            "ast_test",
            &max_ast_test_pool_size,
            &ast_test_pool_pos,
            &ast_test_freed,
            &ast_test_allocated
        },
        {
            "ast_requirement",
            &max_ast_requirement_pool_size,
            &ast_requirement_pool_pos,
            &ast_requirement_freed,
            &ast_requirement_allocated
        },
        { "list", &max_list_pool_size, &list_pool_pos, &list_freed, &list_allocated },
        { "slist", &max_slist_pool_size, &slist_pool_pos, &slist_freed, &slist_allocated },
        { "test_suite", &max_test_suite_pool_size, &test_suite_pool_pos, &test_suite_freed, &test_suite_allocated },
        {
            "tests_results",
            &max_tests_results_pool_size,
            &tests_results_pool_pos,
            &tests_results_freed,
            &tests_results_allocated
        },
        {
            "program_runner_test_result",
            &max_program_runner_test_result_pool_size,
            &program_runner_test_result_pool_pos,
            &program_runner_test_result_freed,
            &program_runner_test_result_allocated
        },
        {
            "program_runner_test",
            &max_program_runner_test_pool_size,
            &program_runner_test_pool_pos,
            &program_runner_test_freed,
            &program_runner_test_allocated
        },
        {
            "abstract_test_result",
            &max_abstract_test_result_pool_size,
            &abstract_test_result_pool_pos,
            &abstract_test_result_freed,
            &abstract_test_result_allocated
        },
        {
            "source",
            &max_source_pool_size,
            &source_pool_pos,
            &source_freed,
            &source_allocated
        },
        {
            "path_list",
            &max_path_list_pool_size,
            &path_list_pool_pos,
            &path_list_freed,
            &path_list_allocated
        },
        {
            "ast_requirement_argument",
            &max_ast_requirement_argument_pool_size,
            &ast_requirement_argument_pool_pos,
            &ast_requirement_argument_freed,
            &ast_requirement_argument_allocated,
        }
};

void show_allocators_stats(FILE * output, bool show_leak_only)
{
    unsigned int i, len;
    struct allocator_stat * stat;
    for(i = 0, len = sizeof(stats) / sizeof(struct allocator_stat); i < len; ++i) {
        stat = stats + i;
        if (show_leak_only && !(*stat->allocated > *stat->freed)) {
            continue;
        }
        if (*stat->allocated == *stat->freed && *stat->freed == 0) { /* don't show unused allocators */
            continue;
        }
        fprintf(
            output,
            "Allocator: %s, total: %u, pool_usage: %u, allocated: %u, freed: %u%s%s%s\n",
            stat->name,
            *stat->total,
            *stat->pool_usage,
            *stat->allocated,
            *stat->freed,
            *stat->allocated > *stat->freed ? " [LEAK]" : "",
            *stat->allocated == *stat->freed && *stat->freed != 0 ? " [OK]" : "",
            *stat->allocated < *stat->freed ? " [ALLOCATION MISMATCH]" : ""
        );
    }
    fflush(output);
}

void release_ast_tests(struct slist * ast_tests)
{
    assert(ast_tests != NULL);
    slist_foreach_safe(iterator, ast_tests, {
        release_ast_test(list_get_owner(iterator, struct ast_test, list_entry));
    });
}

void release_ast_test(struct ast_test * ast_test)
{
    assert(ast_test != NULL);
    release_ast_arguments(&ast_test->arguments);
    release_ast_requirements(&ast_test->requirements);
    free_ast_test(ast_test);
}

void release_ast_requirements(struct slist * requirements)
{
    assert(requirements != NULL);
    slist_foreach_safe(iterator, requirements, {
        release_ast_requirement(list_get_owner(iterator, struct ast_requirement, list_entry));
    });
}

void release_ast_arguments(struct slist * arguments)
{
    assert(arguments != NULL);
    slist_foreach_safe(iterator, arguments, {
        release_ast_argument(list_get_owner(iterator, struct ast_requirement_argument, list_entry));
    });
}

void release_ast_requirement(struct ast_requirement * requirement)
{
    assert(requirement != NULL);
    release_ast_arguments(&requirement->arguments);
    if (requirement->name != NULL) {
        release_string(requirement->name);
        requirement->name = NULL;
    }
    if (requirement->content != NULL) {
        release_string(requirement->content);
        requirement->content = NULL;
    }
    free_ast_requirement(requirement);
}

void release_ast_argument(struct ast_requirement_argument * argument)
{
    assert(argument != NULL);
    if (argument->name != NULL) {
        release_string(argument->name);
        argument->name = NULL;
    }
    if (argument->value != NULL) {
        release_string(argument->value);
        argument->value = NULL;
    }
    free_ast_requirement_argument(argument);
}

void release_path_list(struct path_list * item)
{
    assert(item != NULL);
    if (item->path != NULL) {
        free_bytes((void *)item->path);
        item->path = NULL;
    }
    free_path_list(item);
}

void release_string(struct string * string)
{
    assert(string != NULL);
    if ((string->flags & STRING_FLAG_DONT_RELEASE) > 0) {
        string->flags &= ~STRING_FLAG_DONT_RELEASE;
        return;
    }
    if (string->value != NULL) {
        free_bytes((void *)string->value);
        string->value = NULL;
    }
    free_string(string);
}

void release_tests_results(struct tests_results * tests_results)
{
    assert(tests_results != NULL);
    if (tests_results->results != NULL) {
        unsigned int i, len;
        for (i = 0, len = tests_results->results_count; i < len; ++i) {
            release_test_result(tests_results->results[i]);
            tests_results->results[i] = NULL;
        }
        free_bytes((void *)tests_results->results);
        tests_results->results = NULL;
    }
    free_tests_results(tests_results);
}

void release_test_result(struct abstract_test_result * test_result)
{
    assert(test_result != NULL);
    if (test_result->kind != TEST_RESULT_KIND_PROGRAM_RUNNER) {
        jcunit_fatal_error("Unknown test result kind!\n");
    }
    struct program_runner_test_result * program_runner_test_result = (struct program_runner_test_result *) test_result;
    if (program_runner_test_result->base.test != NULL) {
        program_runner_test_result->base.test = NULL;
    }
    if (program_runner_test_result->base.name != NULL) {
        release_string(program_runner_test_result->base.name);
        program_runner_test_result->base.name = NULL;
    }
    if (program_runner_test_result->base.expected != NULL) {
        release_string(program_runner_test_result->base.expected);
        program_runner_test_result->base.expected = NULL;
    }
    if (program_runner_test_result->base.actual != NULL) {
        release_string(program_runner_test_result->base.actual);
        program_runner_test_result->base.actual = NULL;
    }
    if (program_runner_test_result->given_filename != NULL) {
        release_string(program_runner_test_result->given_filename);
        program_runner_test_result->given_filename = NULL;
    }
    if (program_runner_test_result->executable != NULL) {
        release_string(program_runner_test_result->executable);
        program_runner_test_result->executable = NULL;
    }
    free_program_runner_test_result(program_runner_test_result);
}

void release_test(struct abstract_test * test)
{
    assert(test != NULL);
    if (test->kind != TEST_KIND_PROGRAM_RUNNER) {
        jcunit_fatal_error("Unknown test kind!\n");
    }
    struct program_runner_test * program_runner_test = (struct program_runner_test *) test;
    if (program_runner_test->given_filename != NULL) {
        release_string(program_runner_test->given_filename);
        program_runner_test->given_filename = NULL;
    }
    if (program_runner_test->given_file_content != NULL) {
        release_string(program_runner_test->given_file_content);
        program_runner_test->given_file_content = NULL;
    }
    if (program_runner_test->program_args != NULL) {
        release_string(program_runner_test->program_args);
        program_runner_test->program_args = NULL;
    }
    if (program_runner_test->expected_output != NULL) {
        release_string(program_runner_test->expected_output);
        program_runner_test->expected_output = NULL;
    }
    if (program_runner_test->program_path != NULL) {
        release_string(program_runner_test->program_path);
        program_runner_test->program_path = NULL;
    }
    if (program_runner_test->base.name != NULL) {
        release_string(program_runner_test->base.name);
        program_runner_test->base.name = NULL;
    }
    test->test_suite = NULL;
    free_program_runner_test(program_runner_test);
}

void release_test_suite(struct test_suite * test_suite)
{
    assert(test_suite != NULL);
    if (test_suite->source != NULL) {
        release_source(test_suite->source);
        test_suite->source = NULL;
    }
    test_suite->name = NULL;
    if (test_suite->tests != NULL) {
        unsigned int i, len;
        for (i = 0, len = test_suite->tests_count; i < len; ++i) {
            release_test(test_suite->tests[i]);
            test_suite->tests[i] = NULL;
        }
        free_bytes((void *)test_suite->tests);
        test_suite->tests = NULL;
    }
    free_test_suite(test_suite);
}

void release_source(struct source * source)
{
    assert(source != NULL);
    if (source->filename != NULL) {
        free_bytes((void *)source->filename);
        source->filename = NULL;
    }
    free_source(source);
}


void release_token(struct token * token)
{
    assert(token != NULL);
    if (token == &newline_token || token == &eof_token) {
        return;
    }
    free_token(token);
}

void release_test_suites(struct test_suites * test_suites)
{
    assert(test_suites != NULL);
    if (test_suites->suites != NULL) {
        unsigned i, len;
        for (i = 0, len = test_suites->suites_count; i < len; ++i) {
            release_test_suite(test_suites->suites[i]);
            test_suites->suites[i] = NULL;
        }
        free_bytes((void *)test_suites->suites);
        test_suites->suites = NULL;
    }
}
