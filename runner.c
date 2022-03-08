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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


#include "headers/runner.h"
#include "headers/allocate.h"
#include "headers/child-process.h"
#include "headers/fs.h"
#include "headers/errors.h"


#define MAX_PROCESS_OUTPUT_BUFFER_LEN (8192)
#define MAX_TEST_GIVEN_FILENAME_LEN (255)


static void fill_file_from_string(FILE * file, struct string * content);
static struct program_runner_test_result * make_program_runner_test_result(struct string * name);
static void make_given_file(struct string * filename, struct string * content);
static void resolve_given_filename(struct program_runner_test * test, struct program_runner_test_result * test_result);
static void try_to_run_program(
    const char * program,
    const char * given_filename,
    const char * extra_args,
    int run_mode,
    struct process_output * output
);
static bool is_test_passes(struct string * expected, struct process_output * output);
static int resolve_run_mode_by_stream_code(unsigned int stream_code);
static struct abstract_test_result * run_incomplete_test(struct abstract_test * test);
static struct abstract_test_result * run_skipped_test(struct abstract_test * test);

static test_runner_func * resolve_test_runner(struct abstract_test * test);

static struct abstract_test_result * program_runner_test_runner(struct abstract_test * test);

static char jcunit_given_file_template[] = "/tmp/jcunit_gf_XXXXXXXXXXXX";
static char process_output_buffer[MAX_PROCESS_OUTPUT_BUFFER_LEN];
static char given_filename[MAX_TEST_GIVEN_FILENAME_LEN];


struct abstract_test_result * test_run(struct abstract_test * test)
{
    if (test->flags & TEST_FLAG_INCOMPLETE) {
        return run_incomplete_test(test);
    }
    if (test->flags & TEST_FLAG_SKIPPED) {
        return run_skipped_test(test);
    }
    test_runner_func * runner = resolve_test_runner(test);
    if (runner == NULL) {
        jcunit_fatal_error("There is no any runner to run the test \"%s\"!", test->name->value);
    }
    return runner(test);
}

void fill_file_from_string(FILE * file, struct string * content)
{
    fwrite((const void *)content->value, sizeof(char), content->len, file);
}

struct program_runner_test_result * make_program_runner_test_result(struct string * name)
{
    struct program_runner_test_result * test_result = alloc_program_runner_test_result();
    memset((void *)test_result, 0, sizeof(struct program_runner_test_result));
    test_result->base.name = name;
    test_result->base.kind = TEST_RESULT_KIND_PROGRAM_RUNNER;
    return test_result;
}

void make_given_file(struct string * filename, struct string * content)
{
    FILE * file = fopen(filename->value, "w");
    if (file == NULL) {
        jcunit_fatal_error("Can't open the file: %s!\n", strerror(errno));
    }
    if (content != NULL) {
        fill_file_from_string(file, content);
    }
    fclose(file);
}

void resolve_given_filename(struct program_runner_test * test, struct program_runner_test_result * test_result)
{
    if (test->given_filename != NULL) {
        const char * given_filename_prefix = "/tmp/";
        if (test->given_filename->len - sizeof(given_filename_prefix) - 1 >= MAX_TEST_GIVEN_FILENAME_LEN) {
            jcunit_fatal_error("Too long 'filename' of the 'given' requirement!");
        }
        sprintf(given_filename, "%s%s", given_filename_prefix, test->given_filename->value);
    } else {
        strncpy(given_filename, (const char *)jcunit_given_file_template, MAX_TEST_GIVEN_FILENAME_LEN);
        (void)mktemp(given_filename);
    }
    test_result->given_filename = make_string(given_filename, strlen(given_filename));
}

void try_to_run_program(
    const char * program,
    const char * filename,
    const char * extra_args,
    int run_mode,
    struct process_output * output
) {
    static char * args[4] = {0};

    output->buffer = process_output_buffer;
    output->size = MAX_PROCESS_OUTPUT_BUFFER_LEN;
    output->error_code = ERROR_CODE_NONE;
    output->len = 0;

    args[0] = (char *)program;
    args[1] = (char *)filename;

    if (extra_args != NULL) {
        args[2] = (char *)extra_args;
        args[3] = NULL;
    } else {
        args[2] = NULL;
    }

    bool try_to_run = true;

    if (!fs_is_file_exists(program)) {
        output->error_code = ERROR_CODE_FILE_NOT_FOUND;
        try_to_run = false;
    }

    if (try_to_run && !fs_is_file_executable(program)) {
        output->error_code = ERROR_CODE_NOT_EXECUTABLE;
        try_to_run = false;
    }

    if (try_to_run) {
        child_process_run(args[0], args, output, run_mode);
    }
}

int resolve_run_mode_by_stream_code(unsigned int stream_code)
{
    int mode = -1;
    switch (stream_code) {
        case TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT:
            mode = RUN_MODE_CAPTURE_STDOUT;
            break;
        case TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR:
            mode = RUN_MODE_CAPTURE_STDERR;
            break;
    }
    return mode;
}

bool is_test_passes(struct string * expected, struct process_output * output)
{
    return expected != NULL
        && expected->len == output->len
        && strncmp((const char *)expected->value, (const char *)output->buffer, output->len) == 0;
}

struct test_suite_result * make_test_suite_result(struct test_suite * test_suite)
{
    struct test_suite_result * test_suite_result = alloc_test_suite_result();
    memset((void *)test_suite_result, 0, sizeof(struct test_suite_result));
    test_suite_result->test_results_count = test_suite->tests_count;
    test_suite_result->test_results = (struct abstract_test_result **) alloc_bytes(test_suite->tests_count * sizeof(void *));
    test_suite_result->test_suite = test_suite;
    return test_suite_result;
}

void add_test_result_to_test_suite_result(
    struct test_suite_result * test_suite_result,
    struct abstract_test_result * test_result,
    unsigned int test_result_index
) {
    assert(test_suite_result != NULL);
    assert(test_result != NULL);

    test_suite_result->test_results[test_result_index] = test_result;

    switch (test_result->status) {
        case TEST_RESULT_STATUS_PASS:
            ++test_suite_result->passed_count;
            break;
        case TEST_RESULT_STATUS_FAILURE:
            ++test_suite_result->failure_count;
            break;
        case TEST_RESULT_STATUS_INCOMPLETE:
            ++test_suite_result->incomplete_count;
            break;
        case TEST_RESULT_STATUS_ERROR:
            ++test_suite_result->error_count;
            break;
        case TEST_RESULT_STATUS_SKIPPED:
            ++test_suite_result->skipped_count;
            break;
        default:
            jcunit_fatal_error("The unknown status %d of test result!", test_result->status);
    }
}

test_runner_func * resolve_test_runner(struct abstract_test * test)
{
    if (test->kind == TEST_KIND_PROGRAM_RUNNER) {
        return program_runner_test_runner;
    }
    return NULL;
}

struct abstract_test_result * program_runner_test_runner(struct abstract_test * test)
{
    struct program_runner_test * this_test = (struct program_runner_test *)test;

    struct process_output output;

    struct string * executable = this_test->program_path;

    struct program_runner_test_result * test_result = make_program_runner_test_result(
            this_test->base.name
    );
    test_result->executable = executable;

    resolve_given_filename(this_test, test_result);

    make_given_file(test_result->given_filename, this_test->given_file_content);

    int run_mode = resolve_run_mode_by_stream_code(this_test->stream_code);

    if (run_mode == -1) {
        jcunit_fatal_error("The unknown stream code %d!", this_test->stream_code);
    }

    try_to_run_program(
        executable->value,
        test_result->given_filename->value,
        this_test->program_args != NULL ? this_test->program_args->value : NULL,
        run_mode,
        &output
    );

    bool pass = is_test_passes(this_test->expected_output, &output);

    if (!pass) {
        test_result->base.expected = this_test->expected_output;
        test_result->base.actual = make_string(output.buffer, output.len);
    }

    test_result->base.status = pass ? TEST_RESULT_STATUS_PASS : TEST_RESULT_STATUS_FAILURE;
    test_result->error_code = output.error_code;

    if (test_result->error_code != ERROR_CODE_NONE) {
        test_result->base.status = TEST_RESULT_STATUS_ERROR;
    }

    return (struct abstract_test_result *)test_result;
}

struct abstract_test_result * run_incomplete_test(struct abstract_test * test)
{
    struct abstract_test_result * test_result = alloc_abstract_test_result();
    memset((void *)test_result, 0, sizeof(struct abstract_test_result));
    test_result->status = TEST_RESULT_STATUS_INCOMPLETE;
    test_result->name = test->name;
    return test_result;
}

struct abstract_test_result * run_skipped_test(struct abstract_test * test)
{
    struct abstract_test_result * test_result = alloc_abstract_test_result();
    memset((void *)test_result, 0, sizeof(struct abstract_test_result));
    test_result->status = TEST_RESULT_STATUS_SKIPPED;
    test_result->name = test->name;
    return test_result;
}
