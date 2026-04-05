#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


#include "headers/runner.h"
#include "headers/child-process.h"
#include "headers/fs.h"
#include "headers/match.h"
#include "headers/errors.h"
#include "headers/allocator.h"


#define MAX_PROCESS_OUTPUT_BUFFER_LEN (8192)
#define MAX_TEST_GIVEN_FILENAME_LEN (255)


static void fill_file_from_string(FILE * file, const struct string * content);
static struct program_runner_test_result * make_program_runner_test_result(struct program_runner_test * test);
static void make_given_file(const struct string * filename, const struct string * content);
static void resolve_given_filename(struct program_runner_test * test, struct program_runner_test_result * test_result);
static void try_to_run_program(
    const char * program,
    const char * given_filename,
    const char * extra_args,
    struct process_input * stdin_input,
    struct process_output * stdout_output,
    struct process_output * stderr_output,
    int * exit_code
);
static bool is_test_passes(struct string * expected, struct process_output * output);
static struct abstract_test_result * run_incomplete_test(struct abstract_test * test);
static struct abstract_test_result * run_skipped_test(struct abstract_test * test);

static test_runner_func * resolve_test_runner(struct abstract_test * test);

static struct abstract_test_result * program_runner_test_runner(struct abstract_test * test);

static char jcunit_given_file_template[] = "/tmp/jcunit_gf_XXXXXXXXXXXX";
static char stdout_output_buffer[MAX_PROCESS_OUTPUT_BUFFER_LEN];
static char stderr_output_buffer[MAX_PROCESS_OUTPUT_BUFFER_LEN];
static char given_filename[MAX_TEST_GIVEN_FILENAME_LEN];


struct abstract_test_result * test_run(struct abstract_test * test)
{
    if (test->flags & TEST_FLAG_INCOMPLETE) {
        return run_incomplete_test(test);
    }
    if (test->flags & TEST_FLAG_SKIPPED) {
        return run_skipped_test(test);
    }
    {
        test_runner_func * runner = resolve_test_runner(test);
        if (runner == NULL) {
            jcunit_fatal_error("There is no any runner to run the test \"%s\"!", test->name->value);
        }
        return runner(test);
    }
}

void fill_file_from_string(FILE * file, const struct string * content)
{
    fwrite(content->value, sizeof(char), content->len, file);
}

struct program_runner_test_result * make_program_runner_test_result(struct program_runner_test * test)
{
    struct program_runner_test_result * test_result = memory_blob_pool_alloc_zeroed(&memory_pool, sizeof(struct program_runner_test_result));
    test_result->base.test = (struct abstract_test *) test;
    test_result->base.name = test->base.name;
    string_protect(test->base.name);
    test_result->base.kind = TEST_RESULT_KIND_PROGRAM_RUNNER;
    return test_result;
}

void make_given_file(const struct string * filename, const struct string * content)
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
        if (test->given_filename->len - strlen(given_filename_prefix) - 1 >= MAX_TEST_GIVEN_FILENAME_LEN) {
            jcunit_fatal_error("Too long 'filename' of the 'given' requirement!");
        }
        sprintf(given_filename, "%s%s", given_filename_prefix, test->given_filename->value);
    } else {
        strncpy(given_filename, (const char *)jcunit_given_file_template, MAX_TEST_GIVEN_FILENAME_LEN);
        {
            int fd = mkstemp(given_filename);
            if (fd < 0) {
                jcunit_fatal_error("Cannot create temporary file: \"%s\"!", strerror(errno));
            }
            close(fd);
        }
    }
    test_result->given_filename = make_string(given_filename, strlen(given_filename));
}

void try_to_run_program(
    const char * program,
    const char * filename,
    const char * extra_args,
    struct process_input * stdin_input,
    struct process_output * stdout_output,
    struct process_output * stderr_output,
    int * exit_code
) {
    #define MAX_ARGS 32
    static char * args[MAX_ARGS] = {0};
    static char extra_args_buf[1024];
    int argc = 0;

    stdout_output->buffer = stdout_output_buffer;
    stdout_output->size = MAX_PROCESS_OUTPUT_BUFFER_LEN;
    stdout_output->error_code = ERROR_CODE_NONE;
    stdout_output->len = 0;

    stderr_output->buffer = stderr_output_buffer;
    stderr_output->size = MAX_PROCESS_OUTPUT_BUFFER_LEN;
    stderr_output->error_code = ERROR_CODE_NONE;
    stderr_output->len = 0;

    *exit_code = 0;

    args[argc++] = (char *)program;
    if (filename != NULL) {
        args[argc++] = (char *)filename;
    }

    if (extra_args != NULL) {
        size_t len = strlen(extra_args);
        if (len >= sizeof(extra_args_buf)) {
            jcunit_fatal_error("The args string is too long (max %d bytes)!", (int)(sizeof(extra_args_buf) - 1));
        }
        memcpy(extra_args_buf, extra_args, len);
        extra_args_buf[len] = '\0';

        {
            char * token = strtok(extra_args_buf, " \t");
            while (token != NULL) {
                if (argc >= MAX_ARGS - 1) {
                    jcunit_fatal_error("Too many args (max %d)!", MAX_ARGS - 3);
                }
                args[argc++] = token;
                token = strtok(NULL, " \t");
            }
        }
    }
    args[argc] = NULL;

    {
        bool try_to_run = true;

        if (!fs_is_file_exists(program)) {
            stdout_output->error_code = ERROR_CODE_FILE_NOT_FOUND;
            try_to_run = false;
        }

        if (try_to_run && !fs_is_file_executable(program)) {
            stdout_output->error_code = ERROR_CODE_NOT_EXECUTABLE;
            try_to_run = false;
        }

        if (try_to_run) {
            *exit_code = child_process_run(args[0], args, stdin_input, stdout_output, stderr_output);
        }
    }
}

bool is_test_passes(struct string * expected, struct process_output * output)
{
    return expected != NULL
        && pattern_match(expected->value, expected->len,
                         (const char *)output->buffer, output->len);
}

struct tests_results * make_tests_results(unsigned int total_tests_count)
{
    struct tests_results * tests_results = memory_blob_pool_alloc_zeroed(&memory_pool, sizeof(struct tests_results));
    tests_results->results_count = total_tests_count;
    tests_results->results = (struct abstract_test_result **) memory_blob_pool_alloc(&memory_pool, total_tests_count * sizeof(void *));
    return tests_results;
}

void add_test_result_to_test_suite_result(
    struct tests_results * tests_results,
    struct abstract_test_result * test_result,
    unsigned int result_index
) {
    assert(tests_results != NULL);
    assert(test_result != NULL);

    tests_results->results[result_index] = test_result;

    switch (test_result->status) {
        case TEST_RESULT_STATUS_PASS:
            ++tests_results->passed_count;
            break;
        case TEST_RESULT_STATUS_FAILURE:
            ++tests_results->failure_count;
            break;
        case TEST_RESULT_STATUS_INCOMPLETE:
            ++tests_results->incomplete_count;
            break;
        case TEST_RESULT_STATUS_ERROR:
            ++tests_results->error_count;
            break;
        case TEST_RESULT_STATUS_SKIPPED:
            ++tests_results->skipped_count;
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
    struct process_output stdout_output, stderr_output;
    struct process_input stdin_input_data;
    struct process_input * stdin_input = NULL;
    struct string * executable = this_test->program_path;
    struct program_runner_test_result * test_result;
    int child_exit_code;
    bool pass;

    test_result = make_program_runner_test_result(this_test);
    test_result->executable = executable;
    string_protect(executable);

    if (this_test->given_type == TEST_PROGRAM_RUNNER_GIVEN_TYPE_FILE) {
        resolve_given_filename(this_test, test_result);
        make_given_file(test_result->given_filename, this_test->given_file_content);
    } else if (this_test->given_type == TEST_PROGRAM_RUNNER_GIVEN_TYPE_REFERENCE) {
        test_result->given_filename = this_test->given_path;
        string_protect(this_test->given_path);
    } else if (this_test->given_type == TEST_PROGRAM_RUNNER_GIVEN_TYPE_STDIN) {
        stdin_input = &stdin_input_data;
        if (this_test->given_file_content != NULL) {
            stdin_input_data.data = this_test->given_file_content->value;
            stdin_input_data.len = this_test->given_file_content->len;
        } else {
            stdin_input_data.data = NULL;
            stdin_input_data.len = 0;
        }
    }

    try_to_run_program(
        executable->value,
        test_result->given_filename != NULL ? test_result->given_filename->value : NULL,
        this_test->program_args != NULL ? this_test->program_args->value : NULL,
        stdin_input,
        &stdout_output,
        &stderr_output,
        &child_exit_code
    );

    if (this_test->given_type == TEST_PROGRAM_RUNNER_GIVEN_TYPE_FILE) {
        unlink(test_result->given_filename->value);
    }

    test_result->exit_code = child_exit_code;
    test_result->error_code = stdout_output.error_code;

    /* store stderr for debugging display */
    if (stderr_output.len > 0) {
        test_result->stderr_output = make_string(stderr_output.buffer, stderr_output.len);
    }

    if (test_result->error_code != ERROR_CODE_NONE) {
        test_result->base.status = TEST_RESULT_STATUS_ERROR;
        return (struct abstract_test_result *)test_result;
    }

    pass = true;

    /* check stdout if expected */
    if (this_test->expected_stdout != NULL) {
        if (!is_test_passes(this_test->expected_stdout, &stdout_output)) {
            pass = false;
            test_result->expected_stdout = this_test->expected_stdout;
            string_protect(this_test->expected_stdout);
            test_result->actual_stdout = make_string(stdout_output.buffer, stdout_output.len);
        }
    }

    /* check stderr if expected */
    if (this_test->expected_stderr != NULL) {
        if (!is_test_passes(this_test->expected_stderr, &stderr_output)) {
            pass = false;
            test_result->expected_stderr = this_test->expected_stderr;
            string_protect(this_test->expected_stderr);
            test_result->actual_stderr = make_string(stderr_output.buffer, stderr_output.len);
        }
    }

    /* non-zero exit code with unexpected stderr is a failure */
    if (pass && child_exit_code != 0 && stderr_output.len > 0) {
        if (this_test->expected_stderr == NULL) {
            pass = false;
        }
    }

    test_result->base.status = pass ? TEST_RESULT_STATUS_PASS : TEST_RESULT_STATUS_FAILURE;

    return (struct abstract_test_result *)test_result;
}

struct abstract_test_result * run_incomplete_test(struct abstract_test * test)
{
    struct abstract_test_result * test_result = memory_blob_pool_alloc_zeroed(&memory_pool, sizeof(struct abstract_test_result));
    test_result->kind = TEST_RESULT_KIND_PROGRAM_RUNNER;
    test_result->status = TEST_RESULT_STATUS_INCOMPLETE;
    test_result->name = test->name;
    string_protect(test->name);
    return test_result;
}

struct abstract_test_result * run_skipped_test(struct abstract_test * test)
{
    struct abstract_test_result * test_result = memory_blob_pool_alloc_zeroed(&memory_pool, sizeof(struct abstract_test_result));
    test_result->kind = TEST_RESULT_KIND_PROGRAM_RUNNER;
    test_result->status = TEST_RESULT_STATUS_SKIPPED;
    test_result->name = test->name;
    string_protect(test->name);
    return test_result;
}
