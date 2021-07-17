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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


#include "headers/runner.h"
#include "headers/allocate.h"
#include "headers/child-process.h"
#include "headers/fs.h"


#define MAX_PROCESS_OUTPUT_BUFFER_LEN (8192)
#define MAX_TEST_CASE_GIVEN_FILENAME_LEN (50)


static void fill_file_from_string(FILE * file, struct string * content);
static struct program_runner_test_case_result * make_program_runner_test_case_result(struct string * name);
static void make_given_file(struct program_runner_test_case_result * result, struct string * content);
static void try_to_run_program(
    const char * program,
    const char * given_filename,
    int run_mode,
    struct process_output * output
);
static bool is_test_case_passes(struct string * expected, struct process_output * output);
static int resolve_run_mode_by_stream_code(unsigned int stream_code);
static struct abstract_test_case_result * run_incomplete_test_case(struct abstract_test_case * test_case);

static test_case_runner_func * resolve_test_case_runner(struct abstract_test_case * test_case);

static struct abstract_test_case_result * program_runner_test_case_runner(struct abstract_test_case * test_case);

static char jcunit_given_file_template[] = "/tmp/jcunit_gf_XXXXXXXXXXXX";
static char process_output_buffer[MAX_PROCESS_OUTPUT_BUFFER_LEN];
static char given_filename[MAX_TEST_CASE_GIVEN_FILENAME_LEN];


struct abstract_test_case_result * test_case_run(struct abstract_test_case * test_case)
{
    if (test_case->flags & TEST_CASE_FLAG_INCOMPLETE) {
        return run_incomplete_test_case(test_case);
    }
    test_case_runner_func * runner = resolve_test_case_runner(test_case);
    if (runner == NULL) {
        fprintf(stderr, "There is no any runner to run the test case \"%s\"!", test_case->name->value);
        exit(1);
    }
    return runner(test_case);
}

void fill_file_from_string(FILE * file, struct string * content)
{
    fwrite((const void *)content->value, sizeof(char), content->len, file);
}

struct program_runner_test_case_result * make_program_runner_test_case_result(struct string * name)
{
    struct program_runner_test_case_result * test_case_result = alloc_program_runner_test_case_result();
    memset((void *)&test_case_result->base.list_entry, 0, sizeof(struct list));
    test_case_result->base.name = name;
    test_case_result->base.status = TEST_CASE_RESULT_STATUS_NONE;
    test_case_result->base.expected = NULL;
    test_case_result->base.actual = NULL;
    test_case_result->base.kind = TEST_CASE_RESULT_KIND_PROGRAM_RUNNER;
    test_case_result->executable = NULL;
    test_case_result->error_code = ERROR_CODE_NONE;
    return test_case_result;
}

void make_given_file(struct program_runner_test_case_result * result, struct string * content)
{
    strncpy(given_filename, (const char *)jcunit_given_file_template, MAX_TEST_CASE_GIVEN_FILENAME_LEN);
    (void)mktemp(given_filename);
    FILE * file = fopen(given_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Can't open the file: %s!\n", strerror(errno));
        exit(1);
    }
    if (content != NULL) {
        fill_file_from_string(file, content);
    }
    fclose(file);

    result->given_filename = make_string(given_filename, strlen(given_filename));
}

void try_to_run_program(
    const char * program,
    const char * filename,
    int run_mode,
    struct process_output * output
)
{
    static char * args[3] = {0};

    output->buffer = process_output_buffer;
    output->size = MAX_PROCESS_OUTPUT_BUFFER_LEN;
    output->error_code = ERROR_CODE_NONE;
    output->len = 0;

    args[0] = (char *)program;
    args[1] = (char *)filename;
    args[2] = NULL;

    bool try_to_run = true;

    if (!is_file_exists(program)) {
        output->error_code = ERROR_CODE_FILE_NOT_FOUND;
        try_to_run = false;
    }

    if (try_to_run && !is_file_executable(program)) {
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
        case TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT:
            mode = RUN_MODE_CAPTURE_STDOUT;
            break;
        case TEST_CASE_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR:
            mode = RUN_MODE_CAPTURE_STDERR;
            break;
    }
    return mode;
}

bool is_test_case_passes(struct string * expected, struct process_output * output)
{
    return  expected->len == output->len &&
            strncmp((const char *)expected->value, (const char *)output->buffer, output->len) == 0;
}

struct test_result * make_test_result(struct test * test)
{
    struct test_result * test_result = alloc_test_result();
    list_init(&test_result->test_case_results);
    test_result->test = test;
    test_result->failed_count = 0;
    test_result->passed_count = 0;
    test_result->incomplete_count = 0;
    return test_result;
}

void test_result_add_test_case_result(
    struct test_result * test_result,
    struct abstract_test_case_result * test_case_result
)
{
    assert(test_result != NULL);
    assert(test_case_result != NULL);

    list_append(&test_result->test_case_results, &test_case_result->list_entry);

    switch (test_case_result->status) {
        case TEST_CASE_RESULT_STATUS_PASS:
            ++test_result->passed_count;
            break;
        case TEST_CASE_RESULT_STATUS_FAIL:
            ++test_result->failed_count;
            break;
        case TEST_CASE_RESULT_STATUS_INCOMPLETE:
            ++test_result->incomplete_count;
            break;
        default:
            fprintf(stderr, "The unknown status %d of test case result!\n", test_case_result->status);
            exit(1);
    }
}

test_case_runner_func * resolve_test_case_runner(struct abstract_test_case * test_case)
{
    if (test_case->kind == TEST_CASE_KIND_PROGRAM_RUNNER) {
        return program_runner_test_case_runner;
    }
    return NULL;
}

struct abstract_test_case_result * program_runner_test_case_runner(struct abstract_test_case * test_case)
{
    struct program_runner_test_case * this_test_case = (struct program_runner_test_case *)test_case;

    struct process_output output;

    struct string * executable = this_test_case->program_path;

    struct program_runner_test_case_result * test_case_result = make_program_runner_test_case_result(
            this_test_case->base.name
    );
    test_case_result->executable = executable;

    make_given_file(test_case_result, this_test_case->given_file_content);

    int run_mode = resolve_run_mode_by_stream_code(this_test_case->stream_code);

    if (run_mode == -1) {
        fprintf(
            stderr,
            "The unknown stream code %d!\n",
            this_test_case->stream_code
        );
        exit(1);
    }

    try_to_run_program(
        executable->value,
        test_case_result->given_filename->value,
        run_mode,
        &output
    );

    bool pass = is_test_case_passes(this_test_case->expected_output, &output);

    if (!pass) {
        test_case_result->base.expected = this_test_case->expected_output;
        test_case_result->base.actual = make_string(output.buffer, output.len);
    }

    test_case_result->base.status = pass ? TEST_CASE_RESULT_STATUS_PASS : TEST_CASE_RESULT_STATUS_FAIL;
    test_case_result->error_code = output.error_code;

    return (struct abstract_test_case_result *)test_case_result;
}

struct abstract_test_case_result * run_incomplete_test_case(struct abstract_test_case * test_case)
{
    struct abstract_test_case_result * result = alloc_abstract_test_case_result();
    memset((void *)&result->list_entry, 0, sizeof(struct list));
    result->status = TEST_CASE_RESULT_STATUS_INCOMPLETE;
    result->actual = NULL;
    result->expected = NULL;
    result->name = test_case->name;
    result->kind = TEST_CASE_RESULT_KIND_NONE;
    return result;
}
