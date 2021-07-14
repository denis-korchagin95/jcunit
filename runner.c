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


#include "headers/runner.h"
#include "headers/finder.h"
#include "headers/allocate.h"
#include "headers/child_process.h"
#include "headers/fs.h"


#define MAX_PROCESS_OUTPUT_BUFFER_LEN (8192)


static void fill_file_from_string(FILE * file, struct string * content);
static struct test_case_result * make_test_case_result(void);
static void make_given_file(struct test_case_result * result, struct string * content);
static void try_to_run_program(
    const char * program,
    const char * given_filename,
    int run_mode,
    struct process_output * output
);
static int resolve_run_mode(struct requirement * then_requirement);
bool is_test_case_passes(struct string * expected, struct process_output * output);


static char jcunit_given_file_template[] = "/tmp/jcunit_gf_XXXXXXXXXXXX";
static char process_output_buffer[MAX_PROCESS_OUTPUT_BUFFER_LEN];


struct test_case_result * test_case_run(struct test_case * test_case)
{
    struct requirement * given_requirement = find_requirement_by_kind(test_case, REQUIREMENT_KIND_GIVEN);
    if (given_requirement == NULL) {
        fprintf(stderr, "There is no any 'given' kind of the requirement!\n");
        exit(1);
    }
    if (given_requirement->id != REQUIREMENT_ID_GIVEN_FILE) {
        fprintf(stderr, "The unknown %u 'given' kind of requirement!\n", given_requirement->id);
        exit(1);
    }
    struct requirement * when_requirement = find_requirement_by_kind(test_case, REQUIREMENT_KIND_WHEN);
    if (when_requirement->id != REQUIREMENT_ID_WHEN_RUN) {
        fprintf(stderr, "The unknown %u 'when' kind of requirement!\n", when_requirement->id);
        exit(1);
    }
    struct requirement * then_requirement = find_requirement_by_kind(test_case, REQUIREMENT_KIND_THEN);
    if (then_requirement->id != REQUIREMENT_ID_EXPECT_OUTPUT) {
        fprintf(stderr, "The unknown %u 'then' kind of requirement!\n", when_requirement->id);
        exit(1);
    }

    struct process_output output;

    struct string * executable = when_requirement->extra.path_to_executable;

    struct test_case_result * test_case_result = make_test_case_result();
    test_case_result->test_case_name = test_case->name;
    test_case_result->executable = executable;

    make_given_file(test_case_result, given_requirement->content);

    try_to_run_program(
        executable->value,
        test_case_result->given_filename,
        resolve_run_mode(then_requirement),
        &output
    );

    bool pass = is_test_case_passes(then_requirement->content, &output);

    if (!pass) {
        test_case_result->expected = then_requirement->content;
        test_case_result->actual = make_string(output.buffer, output.len);
    }

    test_case_result->status = pass ? TEST_CASE_RESULT_STATUS_PASS : TEST_CASE_RESULT_STATUS_FAIL;
    test_case_result->error_code = output.error_code;

    return test_case_result;
}

void fill_file_from_string(FILE * file, struct string * content)
{
    fwrite((const void *)content->value, sizeof(char), content->len, file);
}

struct test_case_result * make_test_case_result(void)
{
    struct test_case_result * test_case_result = alloc_test_case_result();
    memset((void *)&test_case_result->list_entry, 0, sizeof(struct list));
    test_case_result->status = TEST_CASE_RESULT_STATUS_NONE;
    test_case_result->test_case_name = NULL;
    test_case_result->executable = NULL;
    test_case_result->expected = NULL;
    test_case_result->actual = NULL;
    test_case_result->error_code = ERROR_CODE_NONE;
    return test_case_result;
}

void make_given_file(struct test_case_result * result, struct string * content)
{
    strncpy(result->given_filename, (const char *)jcunit_given_file_template, MAX_TEST_CASE_GIVEN_FILENAME_LEN);
    (void)mktemp(result->given_filename);
    FILE * file = fopen(result->given_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Can't open the file: %s!\n", strerror(errno));
        exit(1);
    }
    if (content != NULL) {
        fill_file_from_string(file, content);
    }
    fclose(file);
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

static int resolve_run_mode(struct requirement * then_requirement)
{
    int mode = -1;

    switch (then_requirement->extra.resource_code) {
        case REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDOUT:
            mode = RUN_MODE_CAPTURE_STDOUT;
            break;
        case REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDERR:
            mode = RUN_MODE_CAPTURE_STDERR;
            break;
    }

    if (mode == -1) {
        fprintf(
            stderr,
            "The unknown resource code %d of 'then' kind of requirement!\n",
            then_requirement->extra.resource_code
        );
        exit(1);
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
    return test_result;
}

void test_result_add_test_case_result(struct test_result * test_result, struct test_case_result * test_case_result)
{
    list_append(&test_result->test_case_results, &test_case_result->list_entry);

    switch (test_case_result->status) {
        case TEST_CASE_RESULT_STATUS_PASS:
            ++test_result->passed_count;
            break;
        case TEST_CASE_RESULT_STATUS_FAIL:
            ++test_result->failed_count;
            break;
        default:
            fprintf(stderr, "The unknown status %d of test case result!\n", test_case_result->status);
            exit(1);
    }
}
