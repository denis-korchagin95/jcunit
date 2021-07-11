#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "headers/runner.h"
#include "headers/finder.h"
#include "headers/allocate.h"
#include "headers/child_process.h"

static void test_case_run(struct test_runner_context * context, struct test_case * test_case);
static void fill_file_from_string(FILE * file, struct string * content);
static struct test_case_result * make_test_case_result(void);

void test_run(struct test_runner_context * context)
{
    struct test * test = context->test;

    if (list_is_empty(&test->cases)) {
        fprintf(stderr, "There is no any test cases to run the test!\n");
        exit(1);
    }
    struct test_case * test_case;
    list_foreach(iterator, &test->cases, {
        test_case = list_get_owner(iterator, struct test_case, list_entry);
        test_case_run(context, test_case);
    });
}

void test_case_run(struct test_runner_context * context, struct test_case * test_case)
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

    /* TODO: refactor it */
    char template[] = "/tmp/testXXXXXXXXXXXX";
    char * filename = mktemp(template);
    FILE * file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Can't open the file: %s!\n", strerror(errno));
        exit(1);
    }
    if (given_requirement->content != NULL) {
        fill_file_from_string(file, given_requirement->content);
    }
    fclose(file);

    char buffer[8192];

    struct process_output output;

    output.buffer = buffer;
    output.size = 8192;

    char * args[3] = {0};
    args[0] = when_requirement->extra.path_to_executable->value;
    args[1] = filename;
    args[2] = NULL;

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

    child_process_run(args[0], args, &output, mode);
    /* TODO: refactor it */

    /* TODO: calculate the diff */
    bool pass = true;

    if (then_requirement->content->len != output.len) {
        pass = false;
    } else if (strncmp((const char *)then_requirement->content->value, (const char *)output.buffer, output.len) != 0) {
        pass = false;
    }
    /* TODO: calculate the diff */

    struct test_case_result * test_case_result = make_test_case_result();
    test_case_result->test_case = test_case;
    test_case_result->status = pass ? TEST_CASE_RESULT_STATUS_PASS : TEST_CASE_RESULT_STATUS_FAIL;

    if (!pass) {
        test_case_result->expected = then_requirement->content;
        test_case_result->actual = make_string(output.buffer, output.len);
    }

    if (pass) {
        ++context->passed_count;
    } else {
        ++context->failed_count;
    }

    list_append(&context->results, &test_case_result->list_entry);
}

struct test_runner_context * make_test_runner_context(struct test * test)
{
    struct test_runner_context * context = alloc_test_runner_context();
    list_init(&context->results);
    context->passed_count = 0;
    context->failed_count = 0;
    context->test = test;
    return context;
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
    test_case_result->test_case = NULL;
    test_case_result->expected = NULL;
    test_case_result->actual = NULL;
    return test_case_result;
}
